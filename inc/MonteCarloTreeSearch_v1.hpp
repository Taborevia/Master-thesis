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
    uint32_t maxTwinWidth_ = -1;
    double squaredValue_ = 0;

    std::vector<std::pair<int,int>> possibleContractions_;
    std::vector<std::shared_ptr<Node>> children_;
    std::weak_ptr<Node> parent_;

    Node(std::pair<int,int> contraction, std::weak_ptr<Node> parent) : contraction_(contraction), parent_(parent) {}
};

class MonteCarloTreeSearch_v1 : public ITwinWidthSolver {

public:
    MonteCarloTreeSearch_v1(std::shared_ptr<IGraph> graph, int maxPairs = -1);
    std::vector<std::pair<int,int>> possibleContractions(VerticesPositions& vertices);
    void findSequence(float resources, float c_parameter, float D_parameter = 0) override;
    
    std::shared_ptr<Node> expand(std::shared_ptr<Node> state, VerticesPositions& vertices);
    std::shared_ptr<Node> bestChild(std::shared_ptr<Node> state, float c, float D);
    std::tuple<std::shared_ptr<Node>,std::vector<std::pair<int,int>>> treePolicy(std::shared_ptr<Node> state, float c, float D, VerticesPositions& vertices, std::vector<std::pair<int,int>>& contractionSequence);
    void backPropagation(std::shared_ptr<Node> state, int reward);
    std::pair<int,std::vector<std::pair<int,int>>> defaultPolicy(std::vector<std::pair<int,int>> contractionSequence, std::shared_ptr<IGraph> graph, VerticesPositions vertices);
    int getBestTwinWidth() const override;
    std::vector<std::pair<int,int>> getBestContractionSequence() const override;

private:
    std::shared_ptr<IGraph> graph_;
    std::shared_ptr<Node> root_;
    std::vector<std::pair<int,int>> bestSequence_;
    int bestTwinWidth_;
    std::vector<std::pair<int,int>> currentSequence_;    
    int currentTwinWidth_;
    int maxPairs;

    float UCT(std::shared_ptr<Node> state, int child, float c) const;
    float SPUCT(std::shared_ptr<Node> state, int child, float c, float D) const;
    std::shared_ptr<Node> bestContraction();
    void makeContraction(float resources, float c_parameter, float D_parameter);
    std::vector<std::pair<int,int>> generateRandomSequence(VerticesPositions vertices, unsigned int seed);
    std::vector<std::pair<int,int>> generateGreedySequence(std::vector<int> vertices, int maxPairs, unsigned int seed);
};
