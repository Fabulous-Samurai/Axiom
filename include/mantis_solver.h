/**
 * @file mantis_solver.h
 * @brief Mantis A* Solver with hardware-accelerated heuristic dispatch
 *
 * Zero-allocation A* implementation using thread_local scratch buffers
 * and stack-allocated fixed-capacity containers. The Heuristic() call
 * dispatches to MantisHeuristic::evaluate_f32 on the hot path.
 *
 * Latency Budget: < 5ns per node evaluation
 * Memory Policy:  Zero heap allocation in the hot loop
 */

#pragma once

#ifndef MANTIS_SOLVER_H
#define MANTIS_SOLVER_H

#include "mantis_heuristic.h"
#include "axiom_export.h"
#include <array>
#include <cstdint>
#include <cstddef>
#include <limits>
#include <cmath>

namespace AXIOM {
namespace Mantis {

// ============================================================================
// A* Node — compact, cache-friendly layout
// ============================================================================
inline constexpr size_t kMaxNodes = 4096;  // Fixed-capacity for zero-allocation

struct alignas(64) AStarNode {
    uint32_t id         = 0;
    uint32_t parent_id  = UINT32_MAX;  // Sentinel: no parent
    float    g_cost     = std::numeric_limits<float>::max();  // Cost from start
    float    h_cost     = 0.0f;        // Heuristic estimate to goal
    float    f_cost     = std::numeric_limits<float>::max();  // g + h
    bool     in_closed  = false;

    NodeFeatureVecF32 features{};      // Feature vector for heuristic eval
};

// ============================================================================
// Fixed-Capacity Min-Heap (no allocation)
// ============================================================================
class FixedMinHeap {
public:
    AXIOM_FORCE_INLINE bool empty() const noexcept { return size_ == 0; }
    AXIOM_FORCE_INLINE size_t size() const noexcept { return size_; }

    AXIOM_FORCE_INLINE bool push(uint32_t node_id, float f_cost) noexcept {
        if (size_ >= kMaxNodes) [[unlikely]] return false;

        entries_[size_] = {node_id, f_cost};
        sift_up(size_);
        ++size_;
        return true;
    }

    AXIOM_FORCE_INLINE uint32_t pop() noexcept {
        const uint32_t top_id = entries_[0].node_id;
        --size_;
        entries_[0] = entries_[size_];
        if (size_ > 0) sift_down(0);
        return top_id;
    }

private:
    struct Entry {
        uint32_t node_id = 0;
        float    f_cost  = 0.0f;
    };

    std::array<Entry, kMaxNodes> entries_{};
    size_t size_ = 0;

    AXIOM_FORCE_INLINE void sift_up(size_t idx) noexcept {
        while (idx > 0) {
            const size_t parent = (idx - 1) / 2;
            if (entries_[idx].f_cost < entries_[parent].f_cost) {
                std::swap(entries_[idx], entries_[parent]);
                idx = parent;
            } else break;
        }
    }

    AXIOM_FORCE_INLINE void sift_down(size_t idx) noexcept {
        while (true) {
            size_t smallest = idx;
            const size_t left  = 2 * idx + 1;
            const size_t right = 2 * idx + 2;

            if (left < size_ && entries_[left].f_cost < entries_[smallest].f_cost)
                smallest = left;
            if (right < size_ && entries_[right].f_cost < entries_[smallest].f_cost)
                smallest = right;

            if (smallest != idx) {
                std::swap(entries_[idx], entries_[smallest]);
                idx = smallest;
            } else break;
        }
    }
};

// ============================================================================
// IDAStarSolver — zero-allocation IDDFS+A* with Binary Scaling
// ============================================================================
class AXIOM_EXPORT IDAStarSolver {
public:
    struct SearchResult {
        bool     found             = false;
        uint32_t goal_id           = UINT32_MAX;
        float    total_cost        = 0.0f;
        uint32_t nodes_evaluated   = 0;
        uint32_t iterations        = 0;
    };

    /**
     * @brief Set the target profile that the heuristic matches against.
     */
    void set_target_profile(const TargetProfileF32& profile) noexcept {
        target_profile_ = profile;
    }

    /**
     * @brief Set the Dog threshold for conditional normalization.
     */
    void set_dog_threshold(float threshold) noexcept {
        dog_threshold_ = threshold;
    }

    /**
     * @brief Binary Up/Down Scaling.
     * @param exponent Power of 2 to scale the heuristic (2^exp).
     * Positive = Up-scaling (more conservative/precise).
     * Negative = Down-scaling (faster/greedier).
     */
    void set_binary_scaling(int32_t exponent) noexcept {
        scaling_factor_ = std::pow(2.0f, static_cast<float>(exponent));
    }

    /**
     * @brief Evaluate the heuristic with binary scaling.
     */
    AXIOM_FORCE_INLINE float Heuristic(const AStarNode& node) const noexcept {
        float raw_h = MantisHeuristic::evaluate_f32(
            node.features, target_profile_, dog_threshold_);
        return raw_h * scaling_factor_;
    }

    /**
     * @brief Run IDA* search (Iterative Deepening A*).
     */
    template<typename AdjacencyFn>
    SearchResult solve(
        AStarNode* nodes,
        uint32_t num_nodes,
        uint32_t start_id,
        uint32_t goal_id,
        AdjacencyFn&& get_neighbors) noexcept
    {
        if (num_nodes == 0 || num_nodes > kMaxNodes || start_id >= num_nodes) [[unlikely]] {
            return {};
        }

        SearchResult result{};
        float threshold = Heuristic(nodes[start_id]);

        while (true) {
            ++result.iterations;
            
            // Reset node visit states for this DFS pass
            for (uint32_t i = 0; i < num_nodes; ++i) {
                nodes[i].in_closed = false;
            }

            float next_threshold = std::numeric_limits<float>::max();
            bool found = search(nodes, start_id, goal_id, 0.0f, threshold, 
                                next_threshold, result, get_neighbors, num_nodes);

            if (found) {
                result.found = true;
                result.goal_id = goal_id;
                result.total_cost = nodes[goal_id].g_cost;
                return result;
            }

            if (next_threshold == std::numeric_limits<float>::max()) {
                return result; // No path exists
            }
            
            threshold = next_threshold;
            
            // Safety break to prevent infinite loops in malformed graphs
            if (result.iterations > 1024) break;
        }

        return result;
    }

private:
    /**
     * @brief Recursive DFS search for IDA*.
     */
    template<typename AdjacencyFn>
    bool search(
        AStarNode* nodes,
        uint32_t current_id,
        uint32_t goal_id,
        float g_cost,
        float threshold,
        float& next_threshold,
        SearchResult& result,
        AdjacencyFn& get_neighbors,
        uint32_t num_nodes) noexcept
    {
        ++result.nodes_evaluated;
        const float f_cost = g_cost + Heuristic(nodes[current_id]);

        if (f_cost > threshold) {
            if (f_cost < next_threshold) next_threshold = f_cost;
            return false;
        }

        if (current_id == goal_id) {
            nodes[goal_id].g_cost = g_cost;
            return true;
        }

        nodes[current_id].in_closed = true;

        thread_local std::array<uint32_t, 64> neighbor_buf{};
        const uint32_t n_count = get_neighbors(
            current_id, neighbor_buf.data(), neighbor_buf.size());

        for (uint32_t i = 0; i < n_count; ++i) {
            const uint32_t neighbor = neighbor_buf[i];
            if (neighbor >= num_nodes || nodes[neighbor].in_closed) continue;

            // IDA* depth-first branch
            if (search(nodes, neighbor, goal_id, g_cost + 1.0f, threshold, 
                       next_threshold, result, get_neighbors, num_nodes)) {
                nodes[neighbor].parent_id = current_id;
                return true;
            }
        }

        nodes[current_id].in_closed = false; // Backtrack
        return false;
    }

    TargetProfileF32 target_profile_{};
    float dog_threshold_  = kDogThreshold;
    float scaling_factor_ = 1.0f;
};

} // namespace Mantis
} // namespace AXIOM

#endif // MANTIS_SOLVER_H

