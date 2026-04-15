#pragma once
#include <vector>
#include <set>
#include <tuple>
#include <utility>
#include <GraphAL.hpp>
#include <memory>
#include <ITwinWidthSolver.hpp>

class GreedySolver : public ITwinWidthSolver {

public:
    GreedySolver(std::shared_ptr<IGraph> graph);
    std::vector<std::pair<int,int>> possibleContractions(std::set<int> vertices);
    void findSequence(float resources, float c_parameter) override;
    
    int getBestTwinWidth() const override;
    std::vector<std::pair<int,int>> getBestContractionSequence() const override;

private:
    std::shared_ptr<IGraph> graph_;
    std::shared_ptr<IGraph> currentGraph_;
    std::vector<std::pair<int,int>> bestSequence_;
    int bestTwinWidth_;
    std::vector<std::pair<int,int>> currentSequence_;    
    int currentTwinWidth_;
};
