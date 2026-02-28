#pragma once
#include <vector>
#include "IGraph.hpp"

class GraphAM : public IGraph {
public:
    GraphAM(int vertices);

    void addEdge(int u, int v, EdgeColor color = EdgeColor::Black) override;
    bool isEdge(int u, int v, EdgeColor color = EdgeColor::Black) const override;
    void print() const override;
    void contractVertices(int u, int v) override;
    int contractGraph(std::vector<std::pair<int,int>>) override;
    int getMaxRedDegree() const;

private:
    int vertices_;
    int maxRedDegree_;
    std::vector<std::vector<int>> adjMatrix_; // 0: no edge, 1: black, 2: red
    std::vector<int> redDegrees_;
};