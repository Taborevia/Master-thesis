#pragma once
#include <memory>
#include <string>
#include <functional>
#include <vector>
#include <utility>

#include "ds/graph/Graph.hpp"
#include "modular/MDTree.hpp"
#include "IGraph.hpp"
#include "ITwinWidthSolver.hpp"

class MDAdapter : ITwinWidthSolver {

public:
    MDAdapter(std::string filename);
    // Construct with a factory that creates a prime-graph solver for each induced prime graph.
    // The factory receives the induced graph and must return a unique_ptr to an ITwinWidthSolver.
    MDAdapter(std::string filename, std::function<std::unique_ptr<ITwinWidthSolver>(std::shared_ptr<IGraph>)> primeSolverFactory);
    std::shared_ptr<IGraph> induceAndRelabelGraph(std::vector<int> const& vertices);
    std::vector<std::pair<int, int>> normalizeContractionSequence(std::vector<std::pair<int, int>> const& seq);
    modular::MDTree getMDTree();

    void findSequence(float resources, float c_parameter, float D_parameter = 0) override;
    int getBestTwinWidth() const override;
    std::vector<std::pair<int,int>> getBestContractionSequence() const override;

private:
    std::string filename_;
    ds::graph::Graph graph_;
    modular::MDTree mdTree_;
    // Factory for creating solvers for prime graphs. If empty, a default MCTS-based factory is used.
    std::function<std::unique_ptr<ITwinWidthSolver>(std::shared_ptr<IGraph>)> primeSolverFactory_;
    int bestTwinWidth_ = 0;
    std::vector<std::pair<int,int>> bestContractionSequence_;

};
