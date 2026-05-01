#pragma once
#include <vector>
#include <tuple>
#include <utility>
#include <GraphAL.hpp>
#include <memory>
#include <ITwinWidthSolver.hpp>
#include <VerticesPositions.hpp>

struct NodeExact {
    std::pair<int,int> contraction_;
    uint32_t visits_ = 0;
    double value_ = 0;
    int32_t maxTwinWidth_ = -1;
    int32_t minTwinWidth_ = INT32_MAX;
    double squaredValue_ = 0;
    int32_t currentTwinWidth = -1;
    bool active_ = true;

    std::vector<std::pair<int,int>> possibleContractions_;
    std::vector<std::shared_ptr<NodeExact>> children_;
    std::weak_ptr<NodeExact> parent_;

    NodeExact(std::pair<int,int> contraction, std::weak_ptr<NodeExact> parent) : contraction_(contraction), parent_(parent) {}
};

class MonteCarloTreeSearchExact : public ITwinWidthSolver {

public:
    int32_t dynamicTimeDistribution_ = 0;
    int32_t bestKnownTwinWidth_ = INT32_MAX;

    MonteCarloTreeSearchExact(std::shared_ptr<IGraph> graph, int maxPairs = -1, uint32_t greedySimulationPairs = 0, uint32_t greedySimulationDepth = 0, double PTW_coeff = 0.0, int32_t dynamicTimeDistribution = 0, int32_t bestKnownTwinWidth = INT32_MAX);
    std::vector<std::pair<int,int>> possibleContractions(VerticesPositions& vertices, const std::vector<std::pair<int,int>>& contractionSequence);
    std::vector<std::pair<int,int>> possibleContractions(VerticesPositions& vertices, int32_t maxPairs);
    void findSequence(float resources, float c_parameter, float D_parameter = 0) override;
    
    std::shared_ptr<NodeExact> expand(const std::shared_ptr<NodeExact>& state, VerticesPositions& vertices, const std::vector<std::pair<int,int>>& contractionSequence);
    std::shared_ptr<NodeExact> bestChild(const std::shared_ptr<NodeExact>& state, float c, float D);
    std::shared_ptr<NodeExact> treePolicy(const std::shared_ptr<NodeExact>& state, float c, float D, VerticesPositions& vertices, std::vector<std::pair<int,int>>& contractionSequence);
    void backPropagation(const std::shared_ptr<NodeExact>& state, int reward);
    uint32_t randomDefaultPolicy(std::vector<std::pair<int,int>> contractionSequence, VerticesPositions vertices);
    uint32_t greedyDefaultPolicy(std::vector<std::pair<int,int>> contractionSequence, VerticesPositions vertices, const std::shared_ptr<NodeExact>& NodeExact, int maxPairs, int depth);
    
    int getBestTwinWidth() const override;
    std::vector<std::pair<int,int>> getBestContractionSequence() const override;

private:
    std::shared_ptr<IGraph> graph_;
    VerticesPositions vertices_;
    std::shared_ptr<NodeExact> root_;
    std::vector<std::pair<int,int>> bestSequence_;
    int bestTwinWidth_;
    std::vector<std::pair<int,int>> currentSequence_;    
    int currentTwinWidth_;
    int maxPairs_;
    uint32_t greedySimulationPairs_ = 0;
    uint32_t greedySimulationDepth_ = 0;
    double PTW_coeff_ = 0.0;

    float UCT(const std::shared_ptr<NodeExact>& state, int child, float c) const;
    // Single Player UCT
    float SPUCT(const std::shared_ptr<NodeExact>& state, int child, float c, float D) const;
    // SPUT with ,,partial twin-width''
    float SPUCT_PTW(const std::shared_ptr<NodeExact>& state, int child, float c, float D) const;
    std::shared_ptr<NodeExact> bestContraction();
    void makeContraction(float resources, float c_parameter, float D_parameter);
    std::vector<std::pair<int,int>> generateRandomSequence(VerticesPositions vertices, unsigned int seed);
};
