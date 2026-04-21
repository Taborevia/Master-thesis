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
    double squaredValue_ = 0;

    std::vector<std::pair<int,int>> possibleContractions_;
    std::vector<std::shared_ptr<Node>> children_;
    std::weak_ptr<Node> parent_;

    Node(std::pair<int,int> contraction, std::weak_ptr<Node> parent) : contraction_(contraction), parent_(parent) {}
};

class MonteCarloTreeSearch_v1 : public ITwinWidthSolver {

public:
    MonteCarloTreeSearch_v1(std::shared_ptr<IGraph> graph, int maxPairs = -1);
    MonteCarloTreeSearch_v1(std::shared_ptr<IGraph> graph, int maxPairs, uint32_t greedySimulationPairs, uint32_t greedySimulationDepth);
    std::vector<std::pair<int,int>> possibleContractions(VerticesPositions& vertices, int32_t maxPairs);
    void findSequence(float resources, float c_parameter, float D_parameter = 0) override;
    
    std::shared_ptr<Node> expand(std::shared_ptr<Node> state, VerticesPositions& vertices);
    std::shared_ptr<Node> bestChild(std::shared_ptr<Node> state, float c, float D);
    std::tuple<std::shared_ptr<Node>,std::vector<std::pair<int,int>>> treePolicy(std::shared_ptr<Node> state, float c, float D, VerticesPositions& vertices, std::vector<std::pair<int,int>>& contractionSequence);
    void backPropagation(std::shared_ptr<Node> state, int reward);
    uint32_t randomDefaultPolicy(std::vector<std::pair<int,int>> contractionSequence, VerticesPositions vertices);
    uint32_t greedyDefaultPolicy(std::vector<std::pair<int,int>> contractionSequence, VerticesPositions vertices, int maxPairs, int depth);
    
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

    float UCT(std::shared_ptr<Node> state, int child, float c) const;
    float SPUCT(std::shared_ptr<Node> state, int child, float c, float D) const;
    std::shared_ptr<Node> bestContraction();
    void makeContraction(float resources, float c_parameter, float D_parameter);
    std::vector<std::pair<int,int>> generateRandomSequence(VerticesPositions vertices, unsigned int seed);
};
