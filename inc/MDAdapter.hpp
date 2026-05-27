#pragma once
#include <memory>
#include <string>

#include "ds/graph/Graph.hpp"
#include "modular/MDTree.hpp"
#include "IGraph.hpp"
#include "ITwinWidthSolver.hpp"

class MDAdapter : ITwinWidthSolver {

public:
    MDAdapter(std::string filename);
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
    std::unique_ptr<ITwinWidthSolver> primeSolver_;
    int bestTwinWidth_ = 0;
    std::vector<std::pair<int,int>> bestContractionSequence_;

};
