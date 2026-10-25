#include "MonteCarloTreeSearch.hpp"
#include <vector>
#include <set>
#include <tuple>
#include <utility>
#include <random>
#include <algorithm>
#include <chrono>
#include <unordered_set>
#include <queue>

MonteCarloTreeSearch::MonteCarloTreeSearch(std::shared_ptr<IGraph> graph, int maxPairs, uint32_t greedySimulationPairs, uint32_t greedySimulationDepth, double PTW_coeff, int32_t dynamicTimeDistribution, int32_t numOfBestPairs, unsigned int seed) 
    : graph_(graph), maxPairs_(maxPairs), vertices_(graph->getNumberOfVertices()), greedySimulationPairs_(greedySimulationPairs), greedySimulationDepth_(greedySimulationDepth), PTW_coeff_(PTW_coeff), dynamicTimeDistribution_(dynamicTimeDistribution), numOfBestPairs_(numOfBestPairs) {
    root_ = std::make_shared<Node>(std::make_pair(-1,-1),std::weak_ptr<Node>());
    bestTwinWidth_ = INT32_MAX;
    rng_.seed(seed);
}

std::vector<std::pair<int,int>> MonteCarloTreeSearch::generateRandomSequence(VerticesPositions vertices){
    std::vector<std::pair<int,int>> sequence;
    unsigned int n = vertices.vertices.size();
    for(int i =0;i<n-1;i++){
        std::uniform_int_distribution<unsigned int> dist(0, vertices.vertices.size()-1);
        unsigned int random_1 = dist(rng_);
        unsigned int random_2;
        do {
            random_2 = dist(rng_);
        } while(random_1 == random_2);
        sequence.emplace_back(vertices.vertices.at(random_1),vertices.vertices.at(random_2));
        vertices.erase(vertices.vertices.at(random_2));
    }
    return sequence;
}

std::vector<std::pair<int,int>> MonteCarloTreeSearch::possibleContractions(VerticesPositions& vertices, int32_t maxPairs) {
    int n = vertices.vertices.size();
    if (n < 2) return {};

    int totalPairs = n * (n - 1) / 2;

    std::vector<std::pair<int,int>> result;

    if (maxPairs == 0 || maxPairs >= totalPairs) {
        result.reserve(totalPairs);

        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {
                result.emplace_back(vertices.vertices.at(i), vertices.vertices.at(j));
            }
        }
    } else {
        result.reserve(maxPairs);
        
        std::uniform_int_distribution<uint32_t> dist(0, n - 1);
        std::unordered_set<uint64_t> selected;
        selected.reserve(maxPairs);
        while (selected.size() < maxPairs) {
            uint32_t u = dist(rng_);
            uint32_t v;
            do {
                v = dist(rng_);
            } while (u == v);
            selected.insert((static_cast<uint64_t>(u) << 32) | v);
        }
        for (uint64_t pair : selected) {
            uint32_t u = pair >> 32;
            uint32_t v = pair & 0xFFFFFFFF;
            result.emplace_back(vertices.vertices.at(u), vertices.vertices.at(v));
        }
    }

    return result;
}

std::vector<std::pair<int,int>> MonteCarloTreeSearch::bestPossibleContractions(
    const IGraph& graph, 
    VerticesPositions& vertices, 
    int32_t numberOfContractions,
    Node& node) {
    
    int n = vertices.vertices.size();
    if (n < 2) return {};

    int totalPairs = n * (n - 1) / 2;
    if (numberOfContractions < 0 || numberOfContractions > totalPairs) {
        return possibleContractions(vertices, numberOfContractions);
    }

    size_t k = static_cast<size_t>(numberOfContractions);
    std::priority_queue<std::pair<uint32_t, std::pair<int,int>>> bestPairs;

    for (size_t i = 0; i < n; ++i) {
        uint32_t u = vertices.vertices[i]; 
        for (size_t j = i + 1; j < n; ++j) {
            uint32_t v = vertices.vertices[j];
            
            auto value = graph.estimateTwinWidthAfterContraction(u, v);
            
            if (bestPairs.size() < k) {
                bestPairs.emplace(value, std::make_pair(u, v));
            } else if (value < bestPairs.top().first) {
                bestPairs.pop();
                bestPairs.emplace(value, std::make_pair(u, v));
            }
        }
    }

    std::vector<std::pair<int,int>> result;
    result.reserve(bestPairs.size());
    node.maxChildTW_ = bestPairs.top().first;
    while (!bestPairs.empty()) {
        result.push_back(bestPairs.top().second);
        if (bestPairs.size()==1){
            node.minChildTW_ = bestPairs.top().first;
        }
        bestPairs.pop();
    }

    std::reverse(result.begin(), result.end());
    return result;
}


float MonteCarloTreeSearch::UCT(const std::shared_ptr<Node>& state, int child, float c) const {
    double raw_mean = (double)state->children_.at(child)->value_ / state->children_.at(child)->visits_;
    double value_1 = 0;
    if (state->maxTwinWidth_ != 0) {
        value_1 = 1 - (raw_mean / state->maxTwinWidth_);
    }else {
        value_1 = 1;
    }
    double value_2 = c*sqrt(log(state->visits_)/state->children_.at(child)->visits_);

    return value_1 + value_2;
}

float MonteCarloTreeSearch::SPUCT_PTW(const std::shared_ptr<Node>& state, int child, float c, float D) const {

    double raw_mean = (double)state->children_.at(child)->value_ / state->children_.at(child)->visits_;
    double value_1 = 0;
    if (state->maxTwinWidth_ != 0){
        value_1 = 1 - (raw_mean / state->maxTwinWidth_);
    }else{
        value_1 = 1;
    }
    double value_2 = (c*sqrt(log(state->visits_)/state->children_.at(child)->visits_));

    double value_3 = 0;
    if (state->maxTwinWidth_ != 0){
        value_3 = sqrt((std::max(0.0,(state->children_.at(child)->squaredValue_) - (state->children_.at(child)->visits_ * (raw_mean*raw_mean))) + D)/state->children_.at(child)->visits_);
        value_3 = value_3 / state->maxTwinWidth_;
    }
    double value_4 = 0;
    if (state->maxChildTW_ > state->minChildTW_ && state->maxChildTW_ != -1){
        value_4 = (state->maxChildTW_ - state->children_.at(child)->currentTwinWidth) / (state->maxChildTW_ - state->minChildTW_);
    } else {
        value_4 = 1;
    }
    return value_1 + value_2 + value_3 + PTW_coeff_*value_4;
}

float MonteCarloTreeSearch::SPUCT(const std::shared_ptr<Node>& state, int child, float c, float D, bool print) const {
    double raw_mean = (double)state->children_.at(child)->value_ / state->children_.at(child)->visits_;
    double value_1 = 0;
    if (state->maxTwinWidth_ != 0){
        value_1 = 1 - (raw_mean / state->maxTwinWidth_);
    }else{
        value_1 = 1;
    }
    double value_2 = (c*sqrt(log(state->visits_)/state->children_.at(child)->visits_));
    double value_3 = 0;
    if (state->maxTwinWidth_ != 0) {
        value_3 = sqrt((std::max(0.0,(state->children_.at(child)->squaredValue_) - (state->children_.at(child)->visits_ * (raw_mean*raw_mean))) + D)/state->children_.at(child)->visits_);
        value_3 = value_3 / state->maxTwinWidth_;
    }

    return value_1 + value_2 + value_3;
}

std::shared_ptr<Node> MonteCarloTreeSearch::bestChild(const std::shared_ptr<Node>& state, float c, float D) {
    int best_child = -1;
    float best_value = -std::numeric_limits<float>::infinity();
    size_t num_children = state->children_.size();

    if (PTW_coeff_ > 0) {
        for (int i = 0; i < num_children; ++i) {
            double v = SPUCT_PTW(state, i, c, D);
            if (v > best_value) { best_value = v; best_child = i; }
        }
    } else if (D > 0) {
        for (int i = 0; i < num_children; ++i) {
            double v = SPUCT(state, i, c, D);
            if (v > best_value) { best_value = v; best_child = i; }
        }
    } else {
        for (int i = 0; i < num_children; ++i) {
            double v = UCT(state, i, c);
            if (v > best_value) { best_value = v; best_child = i; }
        }
    }

    return state->children_.at(best_child);
}

std::shared_ptr<Node> MonteCarloTreeSearch::expand(const std::shared_ptr<Node>& state, VerticesPositions& vertices, IGraph& graph){
    auto new_contraction = state->possibleContractions_.back();
    state->possibleContractions_.pop_back();
    vertices.erase(new_contraction.second);
    auto new_state = std::make_shared<Node>(new_contraction,state);
    graph.contractVertices(new_contraction.first, new_contraction.second);
    if (numOfBestPairs_ > 0){
        new_state->possibleContractions_ = bestPossibleContractions(graph, vertices, numOfBestPairs_, *new_state);
    } else {
        new_state->possibleContractions_ = possibleContractions(vertices, maxPairs_);
    }
    state->children_.push_back(new_state);

    return new_state;
}

std::shared_ptr<Node> MonteCarloTreeSearch::treePolicy(const std::shared_ptr<Node>& state, float c, float D, VerticesPositions& vertices, std::vector<std::pair<int,int>>& contractionSequence, IGraph& graph){
    if(state->possibleContractions_.size()==0 && state->children_.size()==0){
        return state;
    }
    if (state->possibleContractions_.size()!=0){
        auto result = expand(state,vertices, graph);
        contractionSequence.push_back(result->contraction_);
        result->currentTwinWidth = graph.getMaxRedDegree();
        return result;
    }else{
        auto child = bestChild(state,c,D);
        vertices.erase(child->contraction_.second);
        contractionSequence.push_back(child->contraction_);
        graph.contractVertices(child->contraction_.first, child->contraction_.second);
        return treePolicy(child,c,D,vertices,contractionSequence, graph);
    }
}

uint32_t MonteCarloTreeSearch::randomDefaultPolicy(std::vector<std::pair<int,int>> contractionSequence, VerticesPositions vertices, IGraph& graph){
    uint32_t result = 0;
    auto randomContractionSequence = generateRandomSequence(vertices);
    contractionSequence.insert(contractionSequence.end(),randomContractionSequence.begin(),randomContractionSequence.end());
    result = graph.contractGraph(randomContractionSequence);
    if (bestTwinWidth_>result){
        bestTwinWidth_=result;
        bestSequence_=contractionSequence;
    }
    return result;
}

uint32_t MonteCarloTreeSearch::greedyDefaultPolicy(std::vector<std::pair<int,int>> contractionSequence, VerticesPositions vertices, const std::shared_ptr<Node>& node, int maxPairs, int depth, IGraph& graph){
    while (depth > 0 && vertices.vertices.size() > 1) {
        std::pair<int,int> bestPair;
        auto contractions = possibleContractions(vertices, maxPairs);
        uint32_t bestTwinWidth = UINT32_MAX;
        for (const auto& [u, v] : contractions) {
            uint32_t twinWidthAfterContraction = graph.estimateTwinWidthAfterContraction(u, v);
            if (twinWidthAfterContraction < bestTwinWidth) {
                bestTwinWidth = twinWidthAfterContraction;
                bestPair = {u, v};
            }
        }

        graph.contractVertices(bestPair.first, bestPair.second);
        vertices.erase(bestPair.second);

        depth--;
    }

    return graph.getMaxRedDegree();
}

void MonteCarloTreeSearch::backPropagation(const std::shared_ptr<Node>& state, int reward){
    state->value_+=reward;
    state->visits_+=1;
    state->squaredValue_ += reward * reward;
    if (reward > state->maxTwinWidth_){
        state->maxTwinWidth_ = reward;
    }
    if (reward < state->minTwinWidth_){
        state->minTwinWidth_ = reward;
    }
    if(auto parent = state->parent_.lock()){
        backPropagation(parent,reward);
    }
}

std::shared_ptr<Node> MonteCarloTreeSearch::bestContraction(float c_parameter, float D_parameter){
    auto current = root_;
    int32_t mostVisitedChild = 0;
    double bestValue = 0.0;
    std::shared_ptr<Node> bestChild = nullptr;
    
    for (auto& child : current->children_){
        float avgValue = (child->visits_ > 0) ? (float)child->value_ / child->visits_ : 0.0f;
        if (child->visits_ > mostVisitedChild){
            mostVisitedChild = child->visits_;
            bestChild = child;
        }
    }
    
    return bestChild;
}

void MonteCarloTreeSearch::makeContraction(float resources, float c_parameter, float D_parameter){
    auto start = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration{};

    if (numOfBestPairs_ > 0 && root_->children_.empty()) {
        root_->possibleContractions_ = bestPossibleContractions(*graph_, vertices_, numOfBestPairs_, *root_);
    } else { 
        root_->possibleContractions_ = possibleContractions(vertices_, maxPairs_);
    }

    while (duration.count() < resources) {
        auto graphCopy = graph_->clone();
        auto temp = vertices_;
        std::vector<std::pair<int,int>> contractionSequence;

        auto node = treePolicy(root_, c_parameter, D_parameter, temp, contractionSequence, *graphCopy);
        uint32_t result;
        if (greedySimulationPairs_ > 0 && greedySimulationDepth_ > 0) {
            result = greedyDefaultPolicy(contractionSequence, temp, node, greedySimulationPairs_, greedySimulationDepth_, *graphCopy);
        } else {
            result = randomDefaultPolicy(contractionSequence, temp, *graphCopy);
        }

        backPropagation(node, result);
        auto now = std::chrono::high_resolution_clock::now();
        duration = now - start;
    }

    auto best = bestContraction(c_parameter, D_parameter);
    root_ = best;
    currentSequence_.push_back(best->contraction_);
    vertices_.erase(best->contraction_.second);
    graph_->contractVertices(best->contraction_.first, best->contraction_.second);
    currentTwinWidth_ = graph_->getMaxRedDegree();
}

void MonteCarloTreeSearch::findSequence(float resources, float c_parameter, float D_parameter){
    int n = graph_->getNumberOfVertices();
    std::vector<int32_t> twinWidths;
    std::vector<double> timeLimits;
    timeLimits.reserve(n - 1);
    if (dynamicTimeDistribution_ == 0) {
        for (int i = 0; i < n - 1; ++i) {
            makeContraction(resources / (n - 1), c_parameter, D_parameter);
            twinWidths.push_back(currentTwinWidth_);
        }
        if (bestTwinWidth_ > currentTwinWidth_){
            bestTwinWidth_ = currentTwinWidth_;
            bestSequence_ = currentSequence_;
        }
    }else if (dynamicTimeDistribution_ == 1) {
        for (int i = n-2; i >= 0; --i) {
            timeLimits[(n-2)-i] = resources * ((i + 1) / (n*(n-1)/2.0));
        }
        for (int i = 0; i < n - 1; ++i) {
            makeContraction(timeLimits[i], c_parameter, D_parameter);
            twinWidths.push_back(currentTwinWidth_);
        }
        if (bestTwinWidth_ > currentTwinWidth_){
            bestTwinWidth_ = currentTwinWidth_;
            bestSequence_ = currentSequence_;
        }
    } else if (dynamicTimeDistribution_ == 2) {
        double quadraticSum = (double)(n-1) * n * (2*n - 1) / 6.0;
        for (int i = n-2; i >= 0; --i) {
            double weight = (i + 1) * (i + 1);
            timeLimits[(n-2)-i] = resources * (weight / quadraticSum);
        }
        for (int i = 0; i < n - 1; ++i) {
            makeContraction(timeLimits[i], c_parameter, D_parameter);
            twinWidths.push_back(currentTwinWidth_);
        }
        if (bestTwinWidth_ > currentTwinWidth_){
            bestTwinWidth_ = currentTwinWidth_;
            bestSequence_ = currentSequence_;
        }
    } else {
        makeContraction(resources, c_parameter, D_parameter);
    }
}

int MonteCarloTreeSearch::getBestTwinWidth() const {
    return bestTwinWidth_;
};

std::vector<std::pair<int,int>> MonteCarloTreeSearch::getBestContractionSequence() const{
    return bestSequence_;
}
