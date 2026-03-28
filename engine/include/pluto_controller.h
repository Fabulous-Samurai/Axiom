// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO .agents/workflows/agent_must_obey.md
/**
 * @file pluto_controller.h
 * @brief Pluto Swarm Orchestrator driven by Mantis IDA*
 */

#pragma once

#ifndef PLUTO_CONTROLLER_H
#define PLUTO_CONTROLLER_H

#include "mantis_solver.h"
#include "axiom_export.h"
#include "fixed_vector.h"
#include "lock_free_ring_buffer.h"
#include <mutex>
#include <atomic>

namespace AXIOM {
namespace Pluto {

/**
 * @brief Petri Net State (P1-P4 tokens)
 */
struct AXIOM_EXPORT PlutoState {
    std::atomic<uint32_t> p1_queue{0};
    std::atomic<uint32_t> p2_active{0};
    std::atomic<uint32_t> p3_done{0};
    std::atomic<uint32_t> p4_available{0};

    PlutoState() = default;
    PlutoState(const PlutoState& other) noexcept {
        p1_queue.store(other.p1_queue.load(std::memory_order_relaxed));
        p2_active.store(other.p2_active.load(std::memory_order_relaxed));
        p3_done.store(other.p3_done.load(std::memory_order_relaxed));
        p4_available.store(other.p4_available.load(std::memory_order_relaxed));
    }
    
    PlutoState& operator=(const PlutoState& other) noexcept {
        if (this != &other) {
            p1_queue.store(other.p1_queue.load(std::memory_order_relaxed));
            p2_active.store(other.p2_active.load(std::memory_order_relaxed));
            p3_done.store(other.p3_done.load(std::memory_order_relaxed));
            p4_available.store(other.p4_available.load(std::memory_order_relaxed));
        }
        return *this;
    }

    bool operator==(const PlutoState& other) const noexcept {
        return p1_queue.load(std::memory_order_relaxed) == other.p1_queue.load(std::memory_order_relaxed) &&
               p2_active.load(std::memory_order_relaxed) == other.p2_active.load(std::memory_order_relaxed) &&
               p3_done.load(std::memory_order_relaxed) == other.p3_done.load(std::memory_order_relaxed) &&
               p4_available.load(std::memory_order_relaxed) == other.p4_available.load(std::memory_order_relaxed);
    }
};

/**
 * @brief PlutoController manages the swarm state and search tree.
 */
class AXIOM_EXPORT PlutoController {
public:
    static PlutoController& instance() {
        static PlutoController inst;
        return inst;
    }

    /**
     * @brief Initialize the swarm.
     */
    void init(uint32_t num_experts, uint32_t num_tasks) noexcept;

    /**
     * @brief Run one step of the IDA* search to find the next transition.
     */
    void step_search() noexcept;

    /**
     * @brief Get the current scaling factor from the internal solver.
     */
    float get_scaling_factor() const noexcept { return solver_.get_scaling_factor(); }

    /**
     * @brief Get the current search tree nodes for streaming to GUI.
     */
    size_t get_search_tree(Mantis::AStarNode* out_nodes, size_t max_count) noexcept;

    /**
     * @brief Map a Petri Net state to a Mantis feature vector.
     */
    static Mantis::NodeFeatureVecF32 state_to_features(const PlutoState& state) noexcept {
        Mantis::NodeFeatureVecF32 f;
        f.data[0] = static_cast<float>(state.p1_queue.load(std::memory_order_relaxed));
        f.data[1] = static_cast<float>(state.p2_active.load(std::memory_order_relaxed));
        f.data[2] = static_cast<float>(state.p3_done.load(std::memory_order_relaxed));
        f.data[3] = static_cast<float>(state.p4_available.load(std::memory_order_relaxed));
        // data[4..7] remains 0.0f
        return f;
    }

private:
    PlutoController() = default;

    std::mutex config_mutex_; // Only for non-hotpath configuration
    PlutoState current_state_;
    std::atomic<uint32_t> num_experts_{10};
    
    // Lock-free queue for streaming tree nodes to UI
    SPSCQueue<Mantis::AStarNode, Mantis::kMaxNodes> tree_queue_;
    Mantis::IDAStarSolver solver_;
    
    // Internal scratchpad for solver (Zenith Pillar: Zero-allocation)
    std::array<Mantis::AStarNode, Mantis::kMaxNodes> scratch_nodes_{};
};

} // namespace Pluto
} // namespace AXIOM

#endif // PLUTO_CONTROLLER_H
