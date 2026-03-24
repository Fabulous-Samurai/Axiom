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
#include <mutex>

namespace AXIOM {
namespace Pluto {

/**
 * @brief Petri Net State (P1-P4 tokens)
 */
struct AXIOM_EXPORT PlutoState {
    uint32_t p1_queue     = 0;
    uint32_t p2_active    = 0;
    uint32_t p3_done      = 0;
    uint32_t p4_available = 0;

    bool operator==(const PlutoState& other) const {
        return p1_queue == other.p1_queue &&
               p2_active == other.p2_active &&
               p3_done == other.p3_done &&
               p4_available == other.p4_available;
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
     * @brief Get the current search tree nodes for streaming to GUI.
     */
    size_t get_search_tree(Mantis::AStarNode* out_nodes, size_t max_count) noexcept;

    /**
     * @brief Map a Petri Net state to a Mantis feature vector.
     */
    static Mantis::NodeFeatureVecF32 state_to_features(const PlutoState& state) noexcept {
        Mantis::NodeFeatureVecF32 f;
        f.data[0] = static_cast<float>(state.p1_queue);
        f.data[1] = static_cast<float>(state.p2_active);
        f.data[2] = static_cast<float>(state.p3_done);
        f.data[3] = static_cast<float>(state.p4_available);
        // data[4..7] remains 0.0f
        return f;
    }

private:
    PlutoController() = default;

    std::mutex mutex_;
    PlutoState current_state_;
    uint32_t num_experts_ = 10;
    
    FixedVector<Mantis::AStarNode, Mantis::kMaxNodes> search_nodes_;
    Mantis::IDAStarSolver solver_;
};

} // namespace Pluto
} // namespace AXIOM

#endif // PLUTO_CONTROLLER_H
