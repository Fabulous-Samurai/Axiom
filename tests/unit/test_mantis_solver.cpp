#include <gtest/gtest.h>
#include "mantis_solver.h"
#include <vector>
#include <algorithm>
#include <random>

using namespace AXIOM::Mantis;

// ============================================================================
// FixedMinHeap Tests
// ============================================================================

TEST(FixedMinHeapTest, BasicPushPop) {
    FixedMinHeap heap;
    EXPECT_TRUE(heap.empty());
    EXPECT_EQ(heap.size(), 0);

    EXPECT_TRUE(heap.push(1, 10.0f));
    EXPECT_FALSE(heap.empty());
    EXPECT_EQ(heap.size(), 1);

    EXPECT_TRUE(heap.push(2, 5.0f));
    EXPECT_EQ(heap.size(), 2);

    EXPECT_TRUE(heap.push(3, 15.0f));
    EXPECT_EQ(heap.size(), 3);

    EXPECT_EQ(heap.pop(), 2); // 5.0f is smallest
    EXPECT_EQ(heap.size(), 2);

    EXPECT_EQ(heap.pop(), 1); // 10.0f is next
    EXPECT_EQ(heap.size(), 1);

    EXPECT_EQ(heap.pop(), 3); // 15.0f is last
    EXPECT_TRUE(heap.empty());
}

TEST(FixedMinHeapTest, HeapProperty) {
    FixedMinHeap heap;

    // Deterministic LCG for SonarCloud compliance (S2245)
    uint32_t state = 42;
    auto next_float = [&state]() {
        state = state * 1664525 + 1013904223;
        return static_cast<float>(state) / static_cast<float>(UINT32_MAX) * 100.0f;
    };

    struct Node {
        uint32_t id;
        float cost;
    };
    std::vector<Node> data;

    for (uint32_t i = 0; i < 100; ++i) {
        float c = next_float();
        data.push_back({i, c});
        EXPECT_TRUE(heap.push(i, c));
    }

    std::sort(data.begin(), data.end(), [](const Node& a, const Node& b) {
        return a.cost < b.cost;
    });

    for (uint32_t i = 0; i < 100; ++i) {
        uint32_t popped_id = heap.pop();
        // Since we didn't store the cost in a way that's easily retrievable from the ID
        // without searching, let's just make sure the popped ID's cost in our sorted list
        // is the same as the cost at index i.
        // Actually, for this test to be robust with duplicate costs, we'd need more care,
        // but here costs are likely unique.

        bool found = false;
        for(const auto& item : data) {
            if (item.id == popped_id) {
                EXPECT_NEAR(item.cost, data[i].cost, 1e-5f);
                found = true;
                break;
            }
        }
        EXPECT_TRUE(found);
    }
    EXPECT_TRUE(heap.empty());
}

TEST(FixedMinHeapTest, Overflow) {
    FixedMinHeap heap;
    for (uint32_t i = 0; i < kMaxNodes; ++i) {
        ASSERT_TRUE(heap.push(i, static_cast<float>(kMaxNodes - i)));
    }
    EXPECT_EQ(heap.size(), kMaxNodes);
    EXPECT_FALSE(heap.push(kMaxNodes + 1, 0.0f));
    EXPECT_EQ(heap.size(), kMaxNodes);

    // Verify it still works as a heap after overflow attempt
    EXPECT_EQ(heap.pop(), kMaxNodes - 1); // cost was 1.0
}

// ============================================================================
// AStarNode Tests
// ============================================================================

TEST(AStarNodeTest, AlignmentAndDefaultValues) {
    EXPECT_EQ(alignof(AStarNode), 64);
    AStarNode node;
    EXPECT_EQ(node.id, 0);
    EXPECT_EQ(node.parent_id, UINT32_MAX);
    EXPECT_EQ(node.g_cost, std::numeric_limits<float>::max());
    EXPECT_EQ(node.h_cost, 0.0f);
    EXPECT_EQ(node.f_cost, std::numeric_limits<float>::max());
    EXPECT_FALSE(node.in_closed);
}

// ============================================================================
// IDAStarSolver Tests
// ============================================================================

TEST(IDAStarSolverTest, BasicPathfinding) {
    IDAStarSolver solver;
    std::array<AStarNode, 5> nodes;
    for (uint32_t i = 0; i < 5; ++i) {
        nodes[i].id = i;
        // Simple linear distance features
        for(int j=0; j<8; ++j) nodes[i].features.data[j] = 0.0f;
        nodes[i].features.data[0] = static_cast<float>(i);
    }

    // Graph: 0 -> 1 -> 2 -> 3 -> 4
    auto get_neighbors = [](uint32_t current, uint32_t* out, uint32_t max) -> uint32_t {
        if (current < 4) {
            out[0] = current + 1;
            return 1;
        }
        return 0;
    };

    TargetProfileF32 profile;
    profile.weights[0] = 1.0f; // Each step increases cost by 1.0
    solver.set_target_profile(profile);
    solver.set_binary_scaling(0); // scale = 1.0

    auto result = solver.solve(nodes.data(), 5, 0, 4, get_neighbors);

    EXPECT_TRUE(result.found);
    EXPECT_EQ(result.goal_id, 4);
    EXPECT_NEAR(result.total_cost, 4.0f, 1e-5f);
    EXPECT_GT(result.nodes_evaluated, 0);
    EXPECT_GT(result.iterations, 0);
}

TEST(IDAStarSolverTest, NoPathExists) {
    IDAStarSolver solver;
    std::array<AStarNode, 2> nodes;
    nodes[0].id = 0;
    nodes[1].id = 1;

    // Disconnected graph
    auto get_neighbors = [](uint32_t current, uint32_t* out, uint32_t max) -> uint32_t {
        return 0;
    };

    auto result = solver.solve(nodes.data(), 2, 0, 1, get_neighbors);
    EXPECT_FALSE(result.found);
    EXPECT_GT(result.nodes_evaluated, 0);
}

TEST(IDAStarSolverTest, EdgeCases) {
    IDAStarSolver solver;
    std::array<AStarNode, 2> nodes;
    nodes[0].id = 0;
    nodes[1].id = 1;

    auto get_neighbors = [](uint32_t current, uint32_t* out, uint32_t max) -> uint32_t {
        return 0;
    };

    // 1. Start equals Goal
    auto result1 = solver.solve(nodes.data(), 2, 0, 0, get_neighbors);
    EXPECT_TRUE(result1.found);
    EXPECT_EQ(result1.goal_id, 0);
    EXPECT_NEAR(result1.total_cost, 0.0f, 1e-5f);

    // 2. Zero nodes
    auto result2 = solver.solve(nodes.data(), 0, 0, 0, get_neighbors);
    EXPECT_FALSE(result2.found);

    // 3. num_nodes > kMaxNodes
    auto result3 = solver.solve(nodes.data(), static_cast<uint32_t>(kMaxNodes + 1), 0, 1, get_neighbors);
    EXPECT_FALSE(result3.found);

    // 4. Invalid start_id
    auto result4 = solver.solve(nodes.data(), 1, 1, 1, get_neighbors);
    EXPECT_FALSE(result4.found);
}

TEST(IDAStarSolverTest, BinaryScalingAndHeuristic) {
    IDAStarSolver solver;
    AStarNode node;
    for(int j=0; j<8; ++j) node.features.data[j] = 0.0f;
    node.features.data[0] = 10.0f;

    TargetProfileF32 profile;
    profile.weights[0] = 1.0f;
    solver.set_target_profile(profile);

    // Default scaling: 1.0x
    EXPECT_NEAR(solver.Heuristic(node), 10.0f, 1e-5f);

    solver.set_binary_scaling(1); // 2.0x
    EXPECT_NEAR(solver.Heuristic(node), 20.0f, 1e-5f);

    solver.set_binary_scaling(-1); // 0.5x
    EXPECT_NEAR(solver.Heuristic(node), 5.0f, 1e-5f);
}
