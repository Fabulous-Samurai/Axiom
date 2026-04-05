#include <gtest/gtest.h>
#include "pluto_controller.h"

using namespace AXIOM::Pluto;
using namespace AXIOM::Mantis;

TEST(PlutoCore, Initialization) {
    auto& controller = PlutoController::instance();
    controller.init(10, 50); // 10 experts, 50 tasks

    // Check initial state mapping
    AStarNode node;
    // P1=50, P2=0, P3=0, P4=10
    node.features = PlutoController::state_to_features({50, 0, 0, 10});

    EXPECT_EQ(node.features.data[0], 50.0f);
    EXPECT_EQ(node.features.data[3], 10.0f);
}

TEST(PlutoCore, BridgeIntegration) {
    // This tests the bridge indirectly via the controller
    auto& controller = PlutoController::instance();
    controller.init(10, 5);

    AStarNode buffer[10];
    size_t count = controller.get_search_tree(buffer, 10);
    EXPECT_GT(count, 0);
}
