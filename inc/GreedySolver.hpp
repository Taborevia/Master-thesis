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
    std::vector<std::pair<int,int>> possibleContractions(const std::set<int>& vertices) const;
    std::vector<std::pair<int,int>> possibleContractions(const std::vector<int>& vertices) const;
    std::vector<std::pair<int,int>> samplePossibleContractions(const std::vector<int>& vertices, int numberOfContractions) const;
    void findSequence(float resources, float c_parameter, float D_parameter = 0) override;
    void fullyFindSequence(float resources, float c_parameter);
    void partialFindSequence(int maxPairs);

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
