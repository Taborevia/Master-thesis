#pragma once
#include <vector>
#include <tuple>
#include <utility>
#include <GraphAL.hpp>
#include <memory>
#include <ITwinWidthSolver.hpp>
#include <VerticesPositions.hpp>

struct Node {
    std::pair<int,int> contraction_;
    uint32_t visits_ = 0;
    double value_ = 0;
    int32_t maxTwinWidth_ = -1;
    int32_t minTwinWidth_ = INT32_MAX;
    double squaredValue_ = 0;
    int32_t currentTwinWidth = -1;

    std::vector<std::pair<int,int>> possibleContractions_;
    std::vector<std::shared_ptr<Node>> children_;
    std::weak_ptr<Node> parent_;

    Node(std::pair<int,int> contraction, std::weak_ptr<Node> parent) : contraction_(contraction), parent_(parent) {}
};

class MonteCarloTreeSearch_v1 : public ITwinWidthSolver {

public:
    int32_t dynamicTimeDistribution_ = 0;


    MonteCarloTreeSearch_v1(std::shared_ptr<IGraph> graph, int maxPairs = -1, uint32_t greedySimulationPairs = 0, uint32_t greedySimulationDepth = 0, double PTW_coeff = 0.0, int32_t dynamicTimeDistribution = 0, int32_t numOfBestPairs = 0);
    std::vector<std::pair<int,int>> possibleContractions(VerticesPositions& vertices, int32_t maxPairs);
    std::vector<std::pair<int,int>> bestPossibleContractions(const IGraph& graph, VerticesPositions& vertices, int32_t numberOfContractions);
    void findSequence(float resources, float c_parameter, float D_parameter = 0) override;
    
    std::shared_ptr<Node> expand(const std::shared_ptr<Node>& state, VerticesPositions& vertices, IGraph& graph);
    std::shared_ptr<Node> bestChild(const std::shared_ptr<Node>& state, float c, float D);
    std::shared_ptr<Node> treePolicy(const std::shared_ptr<Node>& state, float c, float D, VerticesPositions& vertices, std::vector<std::pair<int,int>>& contractionSequence, IGraph& graph);
    void backPropagation(const std::shared_ptr<Node>& state, int reward);
    uint32_t randomDefaultPolicy(std::vector<std::pair<int,int>> contractionSequence, VerticesPositions vertices, IGraph& graph);
    uint32_t greedyDefaultPolicy(std::vector<std::pair<int,int>> contractionSequence, VerticesPositions vertices, const std::shared_ptr<Node>& node, int maxPairs, int depth, IGraph& graph);
    
    int getBestTwinWidth() const override;
    std::vector<std::pair<int,int>> getBestContractionSequence() const override;

private:
    std::shared_ptr<IGraph> graph_;
    VerticesPositions vertices_;
    std::shared_ptr<Node> root_;
    std::vector<std::pair<int,int>> bestSequence_;
    int bestTwinWidth_;
    std::vector<std::pair<int,int>> currentSequence_;    
    int currentTwinWidth_;
    int maxPairs_;
    uint32_t greedySimulationPairs_ = 0;
    uint32_t greedySimulationDepth_ = 0;
    uint32_t numOfBestPairs_ = 0;
    double PTW_coeff_ = 0.0;

    float UCT(const std::shared_ptr<Node>& state, int child, float c) const;
    // Single Player UCT
    float SPUCT(const std::shared_ptr<Node>& state, int child, float c, float D) const;
    // SPUT with ,,partial twin-width''
    float SPUCT_PTW(const std::shared_ptr<Node>& state, int child, float c, float D) const;
    std::shared_ptr<Node> bestContraction();
    void makeContraction(float resources, float c_parameter, float D_parameter);
    std::vector<std::pair<int,int>> generateRandomSequence(VerticesPositions vertices, unsigned int seed);
};
