#include <gtest/gtest.h>
#include "mantis_solver.h"
#include <vector>

using namespace AXIOM::Mantis;

TEST(MantisIDAStar, BasicPathfinding) {
    IDAStarSolver solver;
    std::array<AStarNode, 10> nodes;
    for(uint32_t i=0; i<10; ++i) {
        nodes[i].id = i;
        // Mock features for simple linear graph
        for(int j=0; j<8; ++j) nodes[i].features.data[j] = 0.0f;
        nodes[i].features.data[0] = static_cast<float>(i);
    }

    auto get_neighbors = [](uint32_t current, uint32_t* out, uint32_t max) -> uint32_t {
        if (current < 9) {
            out[0] = current + 1;
            return 1;
        }
        return 0;
    };

    // Goal: 10th node (index 9)
    TargetProfileF32 profile;
    profile.weights[0] = 1.0f; // Target feature 0
    solver.set_target_profile(profile);
    solver.set_binary_scaling(0); // 2^0 = 1.0x

    auto result = solver.solve(nodes.data(), 10, 0, 9, get_neighbors);

    EXPECT_TRUE(result.found);
    EXPECT_EQ(result.goal_id, 9);
    EXPECT_GT(result.nodes_evaluated, 0);
}

TEST(MantisIDAStar, BinaryScaling) {
    IDAStarSolver solver;
    AStarNode node;
    node.features.data[0] = 10.0f;
    
    TargetProfileF32 profile;
    profile.weights[0] = 1.0f;
    solver.set_target_profile(profile);

    solver.set_binary_scaling(1); // 2^1 = 2.0x
    EXPECT_NEAR(solver.Heuristic(node), 20.0f, 0.001f);

    solver.set_binary_scaling(-1); // 2^-1 = 0.5x
    EXPECT_NEAR(solver.Heuristic(node), 5.0f, 0.001f);
}
