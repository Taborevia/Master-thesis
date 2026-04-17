#pragma once
#include <vector>
#include <memory>
#include <utility>
#include "IGraph.hpp"

class GraphAL : public IGraph {
public:
    GraphAL(int vertices);

    // Nadpisanie funkcji z IGraph
    void addEdge(int u, int v, EdgeColor color = EdgeColor::Black) override;
    bool isEdge(int u, int v, EdgeColor color = EdgeColor::Black) const override;
    void print() const override;
    void contractVertices(int u, int v) override;
    int estimateTwinWidthAfterContraction(int u, int v) const override;
    int contractGraph(std::vector<std::pair<int,int>>) override;
    int getNumberOfVertices() const override;
    int getMaxRedDegree() const override;
    std::unique_ptr<IGraph> clone() const override;

    void deleteEdge(int u, int v);
    void changeEdgeColor(int u, int v, int color);
    void changeEdge(int u, int v, int new_v, int color);
    

private:
    int vertices_;
    int maxRedDegree_;
    std::vector<std::vector<std::pair<int,int>>> adjList_;
    std::vector<int> redDegrees_;
};