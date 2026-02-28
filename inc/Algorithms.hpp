#pragma once
#include <vector>
#include <set>
#include <tuple>
#include <utility>
#include <GraphAL.hpp>
#include <memory>

std::vector<std::pair<int,int>> generateRandomSequence(std::vector<int> vertices, unsigned int seed = 42);

struct Node {
    std::pair<int,int> contraction_;
    int visits_ = 0;
    double value_ = 0.0;
    std::vector<std::pair<int,int>> possibleContractions_;
    std::vector<std::shared_ptr<Node>> children_;
    std::weak_ptr<Node> parent_;

    Node(std::pair<int,int> contraction, std::weak_ptr<Node> parent) : contraction_(contraction), parent_(parent) {}
};

class MonteCarloTreeSearch{

public:
    MonteCarloTreeSearch(std::shared_ptr<IGraph> graph);
    std::vector<std::pair<int,int>> possibleContractions(std::set<int> vertices);
    void findSequence(float resources, float c_parameter);
    
    std::shared_ptr<Node> expand(std::shared_ptr<Node> state, std::set<int>& vertices);
    std::shared_ptr<Node> bestChild(std::shared_ptr<Node> state, float c);
    std::tuple<std::shared_ptr<Node>,std::vector<std::pair<int,int>>,std::set<int>> treePolicy(std::shared_ptr<Node> state, float c, std::set<int>& vertices, std::vector<std::pair<int,int>>& contractionSequence);
    void backPropagation(std::shared_ptr<Node> state, float reward);
    std::pair<int,std::vector<std::pair<int,int>>> defaultPolicy(std::vector<std::pair<int,int>> contractionSequence, std::shared_ptr<IGraph> graph, std::set<int> vertices);
    int getBestTwinWidth() const;
    std::vector<std::pair<int,int>> getBestContractionSequence() const;

private:
    std::shared_ptr<IGraph> graph_;
    std::shared_ptr<Node> root_;
    std::vector<std::pair<int,int>> bestSequence_;
    int bestTwinWidth_;
};