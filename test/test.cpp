#include "GraphAM.hpp"
#include <gtest/gtest.h>

struct GraphAMTest : public ::testing::Test {
protected:
    GraphAM g{5};
};

TEST_F(GraphAMTest, InitiallyNoEdges) {
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 5; ++j) {
            EXPECT_FALSE(g.isEdge(i, j, EdgeColor::Black));
            EXPECT_FALSE(g.isEdge(i, j, EdgeColor::Red));
        }
    }
}

TEST_F(GraphAMTest, AddEdgesAndSymmetry) {
    g.addEdge(0, 1, EdgeColor::Black);
    g.addEdge(2, 3, EdgeColor::Black);
    g.addEdge(0, 4, EdgeColor::Red);

    EXPECT_TRUE(g.isEdge(0,1, EdgeColor::Black));
    EXPECT_TRUE(g.isEdge(1,0, EdgeColor::Black));
    EXPECT_TRUE(g.isEdge(2,3, EdgeColor::Black));
    EXPECT_TRUE(g.isEdge(3,2, EdgeColor::Black));
    EXPECT_TRUE(g.isEdge(0,4, EdgeColor::Red));
    EXPECT_TRUE(g.isEdge(4,0, EdgeColor::Red));

    EXPECT_FALSE(g.isEdge(0,1, EdgeColor::Red));
}

TEST_F(GraphAMTest, ContractVerticesUpdates) {
    // przygotuj graf
    g.addEdge(0, 1, EdgeColor::Black);
    g.addEdge(2, 3, EdgeColor::Black);
    g.addEdge(0, 4, EdgeColor::Red);

    // kontraktuj 0 (u) i 2 (v)
    g.contractVertices(0, 2);

    // v==2 powinien mieć wszystkie incydentne krawędzie usunięte
    EXPECT_FALSE(g.isEdge(2,3, EdgeColor::Black));
    EXPECT_FALSE(g.isEdge(2,3, EdgeColor::Red));
    EXPECT_FALSE(g.isEdge(3,2, EdgeColor::Black));
    EXPECT_FALSE(g.isEdge(3,2, EdgeColor::Red));

    // 0 powinno mieć teraz krawędź do 1 (była czarna -> teraz czerwona)
    EXPECT_TRUE(g.isEdge(0,1, EdgeColor::Red));
    EXPECT_FALSE(g.isEdge(0,1, EdgeColor::Black));

    // 0 powinno mieć krawędź do 3 (pochodząca z v=2 -> czerwona)
    EXPECT_TRUE(g.isEdge(0,3, EdgeColor::Red));
    EXPECT_FALSE(g.isEdge(0,3, EdgeColor::Black));

    // istniejąca czerwona krawędź 0-4 powinna pozostać czerwona
    EXPECT_TRUE(g.isEdge(0,4, EdgeColor::Red));

    // maxRedDegree powinno byc rowne 2
    EXPECT_EQ(g.getMaxRedDegree(),2);

    // kontrakt ponownie nie powinien powodować awarii
    g.contractVertices(0, 2);
    SUCCEED();
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}