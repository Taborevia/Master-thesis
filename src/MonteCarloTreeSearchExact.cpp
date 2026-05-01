#include "MonteCarloTreeSearchExact.hpp"
#include <vector>
#include <set>
#include <tuple>
#include <utility>
#include <random>
#include <algorithm>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <unordered_set>


#include <windows.h>
#include <psapi.h>
#include <iostream>

MonteCarloTreeSearchExact::MonteCarloTreeSearchExact(std::shared_ptr<IGraph> graph, int maxPairs, uint32_t greedySimulationPairs, uint32_t greedySimulationDepth, double PTW_coeff, int32_t dynamicTimeDistribution, int32_t bestKnownTwinWidth) 
    : graph_(graph), maxPairs_(maxPairs), vertices_(graph->getNumberOfVertices()), greedySimulationPairs_(greedySimulationPairs), greedySimulationDepth_(greedySimulationDepth), PTW_coeff_(PTW_coeff), dynamicTimeDistribution_(dynamicTimeDistribution), bestKnownTwinWidth_(bestKnownTwinWidth) {
    root_ = std::make_shared<NodeExact>(std::make_pair(-1,-1),std::weak_ptr<NodeExact>());
    bestTwinWidth_ = INT32_MAX;
}

std::vector<std::pair<int,int>> MonteCarloTreeSearchExact::generateRandomSequence(VerticesPositions vertices, unsigned int seed){
    std::mt19937 rng(seed);
    std::vector<std::pair<int,int>> sequence;
    unsigned int n = vertices.vertices.size();
    for(int i =0;i<n-1;i++){
        std::uniform_int_distribution<unsigned int> dist(0, vertices.vertices.size()-1);
        unsigned int random_1 = dist(rng);
        unsigned int random_2;
        do {
            random_2 = dist(rng);
        } while(random_1 == random_2);
        sequence.emplace_back(vertices.vertices.at(random_1),vertices.vertices.at(random_2));
        vertices.erase(vertices.vertices.at(random_2));
    }
    return sequence;
}

std::vector<std::pair<int,int>> MonteCarloTreeSearchExact::possibleContractions(VerticesPositions& vertices,const std::vector<std::pair<int,int>>& contractionSequence) {
    auto graph = graph_->clone();
    graph->contractGraph(contractionSequence);

    int n = vertices.vertices.size();
    if (n < 2) return {};

    int totalPairs = n * (n - 1) / 2;

    std::vector<std::pair<int,int>> result;

    result.reserve(totalPairs);

    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            if (graph->estimateTwinWidthAfterContraction(vertices.vertices.at(i), vertices.vertices.at(j)) > bestKnownTwinWidth_) {
                continue;
            }
            result.emplace_back(vertices.vertices.at(i), vertices.vertices.at(j));
        }
    }
        
    return result;
}

std::vector<std::pair<int,int>> MonteCarloTreeSearchExact::possibleContractions(VerticesPositions& vertices, int32_t maxPairs) {
    int n = vertices.vertices.size();
    if (n < 2) return {};

    int totalPairs = n * (n - 1) / 2;

    std::vector<std::pair<int,int>> result;

    if( maxPairs >= totalPairs) {
        result.reserve(totalPairs);

        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {
                if (graph_->estimateTwinWidthAfterContraction(vertices.vertices.at(i), vertices.vertices.at(j)) > bestKnownTwinWidth_) {
                    continue;
                }
                result.emplace_back(vertices.vertices.at(i), vertices.vertices.at(j));
            }
        }
        return result;
    }
    result.reserve(maxPairs);
    
    static std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<uint32_t> dist(0, n - 1);
    // after_random = std::chrono::high_resolution_clock::now();
    std::unordered_set<uint64_t> selected;
    selected.reserve(maxPairs);
    // before_selection = std::chrono::high_resolution_clock::now();
    while (selected.size() < maxPairs) {
        uint32_t u = dist(gen);
        uint32_t v;
        do {
            v = dist(gen);
        } while (u == v);
        selected.insert((static_cast<uint64_t>(u) << 32) | v);
    }
    for (uint64_t pair : selected) {
        uint32_t u = pair >> 32;
        uint32_t v = pair & 0xFFFFFFFF;
        result.emplace_back(vertices.vertices.at(u), vertices.vertices.at(v));
    }
    
    return result;
}

float MonteCarloTreeSearchExact::UCT(const std::shared_ptr<NodeExact>& state, int child, float c) const {
    // if (state->children_.at(child)->visits_ > 1) {
    //     std::cout << "\n\n\nvalue: " << (1-(((float)state->children_.at(child)->value_/state->maxTwinWidth_)/state->children_.at(child)->visits_)) << std::endl;
    //     std::cout << "visits: " << state->children_.at(child)->visits_ << "parent visits: " << state->visits_ << "value: "<< state->children_.at(child)->value_ << "MaxTW: " << state->maxTwinWidth_ << std::endl;
    // }
    float value = (1-(((float)state->children_.at(child)->value_/state->maxTwinWidth_)/state->children_.at(child)->visits_))+(c*sqrt(log(state->visits_)/state->children_.at(child)->visits_));
    if (state->maxTwinWidth_ == 0){
        return 0;
    }
    return value;
}

float MonteCarloTreeSearchExact::SPUCT_PTW(const std::shared_ptr<NodeExact>& state, int child, float c, float D) const {
    
    double mean = (1-(((double)state->children_.at(child)->value_/state->maxTwinWidth_)/state->children_.at(child)->visits_));
    double value_2 = (c*sqrt(log(state->visits_)/state->children_.at(child)->visits_));
    double value_3 = sqrt(((state->children_.at(child)->squaredValue_) - (state->children_.at(child)->visits_ * (mean*mean)) + D)/state->children_.at(child)->visits_);
    value_3 = value_3 / (state->maxTwinWidth_ * state->maxTwinWidth_);
    double value_4 = 0;
    if (state->maxTwinWidth_ > state->minTwinWidth_){
        value_4 = (state->maxTwinWidth_ - state->children_.at(child)->currentTwinWidth) / (state->maxTwinWidth_ - state->minTwinWidth_);
    } else {
        value_4 = 1;
    }
    // if (state->children_.at(child)->visits_ > 20) {
    //     std::cout << "\nchildren value: " << state->children_.at(child)->value_ << " visits: " << state->children_.at(child)->visits_ << " maxTwinWidth: " << state->maxTwinWidth_ << std::endl;
    //     std::cout << "SPUCT value: " << mean << " + " << value_2 << " + " << value_3 << std::endl;
    // }
    if (state->maxTwinWidth_ == 0){
        return value_2;
    }
    
    return mean + value_2 + value_3 + PTW_coeff_*value_4;
}

float MonteCarloTreeSearchExact::SPUCT(const std::shared_ptr<NodeExact>& state, int child, float c, float D) const {
    
    double mean = (1-(((double)state->children_.at(child)->value_/state->maxTwinWidth_)/state->children_.at(child)->visits_));
    double value_2 = (c*sqrt(log(state->visits_)/state->children_.at(child)->visits_));
    double value_3 = sqrt(((state->children_.at(child)->squaredValue_) - (state->children_.at(child)->visits_ * (mean*mean)) + D)/state->children_.at(child)->visits_);
    value_3 = value_3 / (state->maxTwinWidth_ * state->maxTwinWidth_);
    // if (state->children_.at(child)->visits_ > 20) {
    //     std::cout << "\nchildren value: " << state->children_.at(child)->value_ << " visits: " << state->children_.at(child)->visits_ << " maxTwinWidth: " << state->maxTwinWidth_ << std::endl;
    //     std::cout << "SPUCT value: " << mean << " + " << value_2 << " + " << value_3 << std::endl;
    // }
    if (state->maxTwinWidth_ == 0){
        return value_2;
    }
    
    return mean + value_2 + value_3;
}

std::shared_ptr<NodeExact> MonteCarloTreeSearchExact::bestChild(const std::shared_ptr<NodeExact>& state, float c, float D) {
    int best_child = -1;
    float best_value = -std::numeric_limits<float>::infinity();
    size_t num_children = state->children_.size();

    if (PTW_coeff_ > 0) {
        for (int i = 0; i < num_children; ++i) {
            if (!state->children_.at(i)->active_) {
                state->children_.at(i) = std::move(state->children_.back());
                state->children_.pop_back();
                num_children--;
                i--;
                continue;
            }
            double v = SPUCT_PTW(state, i, c, D);
            if (v > best_value) { best_value = v; best_child = i; }
        }
    } else if (D > 0) {
        for (int i = 0; i < num_children; ++i) {
            if (!state->children_.at(i)->active_) {
                state->children_.at(i) = std::move(state->children_.back());
                state->children_.pop_back();
                num_children--;
                i--;
                continue;
            }
            double v = SPUCT(state, i, c, D);
            if (v > best_value) { best_value = v; best_child = i; }
        }
    } else {
        for (int i = 0; i < num_children; ++i) {
            if (!state->children_.at(i)->active_) {
                state->children_.at(i) = std::move(state->children_.back());
                state->children_.pop_back();
                num_children--;
                i--;
                continue;
            }
            double v = UCT(state, i, c);
            if (v > best_value) { best_value = v; best_child = i; }
        }
    }
    if (best_child == -1) {
        return nullptr;
    }
    return state->children_.at(best_child);
}

std::shared_ptr<NodeExact> MonteCarloTreeSearchExact::expand(const std::shared_ptr<NodeExact>& state, VerticesPositions& vertices, const std::vector<std::pair<int,int>>& contractionSequence){
    // auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    // std::mt19937 rng(static_cast<unsigned>(seed));
    // std::uniform_int_distribution<int> genRand(0, state->possible_actions.size()-1);
    // int move = genRand(rng);
    auto new_contraction = state->possibleContractions_.back();
    state->possibleContractions_.pop_back();
    vertices.erase(new_contraction.second);
    auto new_state = std::make_shared<NodeExact>(new_contraction,state);
    new_state->possibleContractions_ = possibleContractions(vertices, contractionSequence);

    state->children_.push_back(new_state);

    return new_state;
}

std::shared_ptr<NodeExact> MonteCarloTreeSearchExact::treePolicy(const std::shared_ptr<NodeExact>& state, float c, float D, VerticesPositions& vertices, std::vector<std::pair<int,int>>& contractionSequence){
    if(state->possibleContractions_.size()==0 && state->children_.size()==0){
        return state;
    }
    // do dodania, warunek konca kontrakcji
    if (state->possibleContractions_.size()!=0){
        std::cout<< "current twin width: " << state->currentTwinWidth << " possible contractions: " << state->possibleContractions_.size() << " children: " << state->children_.size() << std::endl;
        auto result = expand(state,vertices, contractionSequence);
        contractionSequence.push_back(result->contraction_);
        return result;
    }else{
        std::cout<< "current twin width: " << state->currentTwinWidth << " possible contractions: " << state->possibleContractions_.size() << " children: " << state->children_.size() << std::endl;
        auto child = bestChild(state,c,D);
        if (child == nullptr) {
            return treePolicy(state,c,D,vertices,contractionSequence);
        }
        vertices.erase(child->contraction_.second);
        contractionSequence.push_back(child->contraction_);
        return treePolicy(child,c,D,vertices,contractionSequence);
    }
}

uint32_t MonteCarloTreeSearchExact::randomDefaultPolicy(std::vector<std::pair<int,int>> contractionSequence, VerticesPositions vertices){
    uint32_t result = 0;
    auto graphCopyStart = std::chrono::high_resolution_clock::now();
    auto graphCopy = graph_->clone();
    auto graphCopyEnd = std::chrono::high_resolution_clock::now();
    auto randomContractionSequence = generateRandomSequence(vertices, std::chrono::high_resolution_clock::now().time_since_epoch().count());
    auto randomSequenceEnd = std::chrono::high_resolution_clock::now();
    contractionSequence.insert(contractionSequence.end(),randomContractionSequence.begin(),randomContractionSequence.end());
    auto contractionSequnceInsertEnd = std::chrono::high_resolution_clock::now();
    result = graphCopy->contractGraph(contractionSequence);
    auto contractGraphEnd = std::chrono::high_resolution_clock::now();
    if (bestTwinWidth_>result){
        bestTwinWidth_=result;
        bestSequence_=contractionSequence;
    }
    // std::cout << "defaultPolicy profiling:\n"
    //           << "  graph clone: " << std::chrono::duration_cast<std::chrono::microseconds>(graphCopyEnd - graphCopyStart).count() << " us\n"
    //           << "  generate random sequence: " << std::chrono::duration_cast<std::chrono::microseconds>(randomSequenceEnd - graphCopyEnd).count() << " us\n"
    //           << "  insert contraction sequence: " << std::chrono::duration_cast<std::chrono::microseconds>(contractionSequnceInsertEnd - randomSequenceEnd).count() << " us\n"
    //           << "  contractGraph: " << std::chrono::duration_cast<std::chrono::microseconds>(contractGraphEnd - contractionSequnceInsertEnd).count() << " us\n";
    return result;
}

uint32_t MonteCarloTreeSearchExact::greedyDefaultPolicy(std::vector<std::pair<int,int>> contractionSequence, VerticesPositions vertices, const std::shared_ptr<NodeExact>& NodeExact, int maxPairs, int depth){
    if (vertices.vertices.size() <= 1) {
        return NodeExact->currentTwinWidth;
    }
    auto graph = graph_->clone();
    graph->contractGraph(contractionSequence);
    NodeExact->currentTwinWidth = graph->getMaxRedDegree();
    if (NodeExact->currentTwinWidth > bestKnownTwinWidth_){
        NodeExact->active_ = false;
        return NodeExact->currentTwinWidth;
    }
    while (depth > 0 && vertices.vertices.size() > 1) {
        auto contractions = possibleContractions(vertices, maxPairs);
        
        if (contractions.empty()) {
            break;
        }
        
        std::pair<int,int> bestPair;
        uint32_t bestTwinWidth = UINT32_MAX;
        for (const auto& [u, v] : contractions) {
            uint32_t twinWidthAfterContraction = graph->estimateTwinWidthAfterContraction(u, v);
            if (twinWidthAfterContraction < bestTwinWidth) {
                bestTwinWidth = twinWidthAfterContraction;
                bestPair = {u, v};
            }
        }
        graph->contractVertices(bestPair.first, bestPair.second);
        vertices.erase(bestPair.second);
        contractionSequence.push_back(bestPair);
        depth--;
    }
    if (vertices.vertices.size() <= 1 && graph->getMaxRedDegree() <= bestTwinWidth_){
        bestTwinWidth_ = graph->getMaxRedDegree();
        bestSequence_ = contractionSequence;
    }
    std::cout << "left vertices: " << vertices.vertices.size() << " current twin width: " << graph->getMaxRedDegree() << " best twin width: " << bestTwinWidth_ << std::endl;
    return graph->getMaxRedDegree();
}

void MonteCarloTreeSearchExact::backPropagation(const std::shared_ptr<NodeExact>& state, int reward){
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

std::shared_ptr<NodeExact> MonteCarloTreeSearchExact::bestContraction(){
    auto current = root_;
    int32_t mostVisitedChild = 0;
    std::shared_ptr<NodeExact> bestChild = nullptr;
    
    // std::cout << "\n=== Stan wszystkich dzieci ===" << std::endl;
    for (auto& child : current->children_){
        // float avgValue = (child->visits_ > 0) ? (float)child->value_ / child->visits_ : 0.0f;
        // std::cout << "Kontrakcja: (" << child->contraction_.first << ", " 
        //           << child->contraction_.second << ") | "
        //           << "Wizyty: " << child->visits_ << " | "
        //           << "Wartosc: " << child->value_ << " | "
        //           << "Średnia: " << avgValue << std::endl;
        if (child->visits_ > mostVisitedChild){
            mostVisitedChild = child->visits_;
            bestChild = child;
        }
    }
    // std::cout << "Wybrana kontrakcja: (" << bestChild->contraction_.first << ", " 
    //           << bestChild->contraction_.second << ") z " << bestChild->visits_ << " wizytami\n";
    // std::cout << "============================\n" << std::endl;
    
    return bestChild;
}

void MonteCarloTreeSearchExact::makeContraction(float resources, float c_parameter, float D_parameter){
    int simulationCounter = 0;
    auto start = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration{};
    // std::cout << "Starting contraction with " << n << " vertices. Time limit: " << resources << " seconds\n";
    auto buildStart = std::chrono::high_resolution_clock::now();
    root_->possibleContractions_ = possibleContractions(vertices_, std::vector<std::pair<int,int>>{});
    auto buildEnd = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> buildTime = buildEnd - buildStart;

    std::chrono::duration<double> treePolicyTime{};
    std::chrono::duration<double> defaultPolicyTime{};
    std::chrono::duration<double> backPropagationTime{};

    while(duration.count() < resources){
        if(bestTwinWidth_ <= bestKnownTwinWidth_){
            std::cout << "i've found it, time: "<< std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() << " ms, best twin width: " << bestTwinWidth_ << std::endl;
            break;
        }
        simulationCounter++;
        auto temp = vertices_;
        std::vector<std::pair<int,int>> contractionSequence;
        auto treeStart = std::chrono::high_resolution_clock::now();
        auto NodeExact = treePolicy(root_,c_parameter,D_parameter,temp,contractionSequence);
        auto treeEnd = std::chrono::high_resolution_clock::now();
        treePolicyTime += treeEnd - treeStart;

        auto defaultStart = std::chrono::high_resolution_clock::now();
        uint32_t result;
        if (greedySimulationPairs_ > 0 && greedySimulationDepth_ > 0) {
            result = greedyDefaultPolicy(contractionSequence, temp, NodeExact, greedySimulationPairs_, greedySimulationDepth_);
        } else {
            result = randomDefaultPolicy(contractionSequence, temp);
        }

        auto defaultEnd = std::chrono::high_resolution_clock::now();
        defaultPolicyTime += defaultEnd - defaultStart;

        auto backStart = std::chrono::high_resolution_clock::now();
        backPropagation(NodeExact,result);
        auto backEnd = std::chrono::high_resolution_clock::now();
        backPropagationTime += backEnd - backStart;

        auto now = std::chrono::high_resolution_clock::now();
        duration = now - start;
    }

    auto best = bestContraction();
    if (!best) {
        std::cout << "No best contraction found!\n";
        return;
    }
    root_ = best;
    currentSequence_.push_back(best->contraction_);
    vertices_.erase(best->contraction_.second);
    auto contractStart = std::chrono::high_resolution_clock::now();
    graph_->contractVertices(best->contraction_.first,best->contraction_.second);
    auto contractEnd = std::chrono::high_resolution_clock::now();
    auto contractTime = contractEnd - contractStart;

    currentTwinWidth_ = graph_->getMaxRedDegree();

    std::cout << "makeContraction profiling:\n"
              << "  build possibleContractions: " << std::chrono::duration_cast<std::chrono::milliseconds>(buildTime).count() << " ms\n"
              << "  treePolicy total: " << std::chrono::duration_cast<std::chrono::milliseconds>(treePolicyTime).count() << " ms\n"
              << "  defaultPolicy total: " << std::chrono::duration_cast<std::chrono::milliseconds>(defaultPolicyTime).count() << " ms\n"
              << "  backPropagation total: " << std::chrono::duration_cast<std::chrono::milliseconds>(backPropagationTime).count() << " ms\n"
              << "  contractVertices: " << std::chrono::duration_cast<std::chrono::milliseconds>(contractTime).count() << " ms\n"
              << "  simulations counter: " << simulationCounter << "\n"
              << " total time for makeContraction: " << std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() << " ms\n"
              << " Current TwinWidth: "<< currentTwinWidth_ <<"\n";
}

void MonteCarloTreeSearchExact::findSequence(float resources, float c_parameter, float D_parameter){
    int n = graph_->getNumberOfVertices();
    makeContraction(resources, c_parameter, D_parameter);
}

int MonteCarloTreeSearchExact::getBestTwinWidth() const {
    return bestTwinWidth_;
};

std::vector<std::pair<int,int>> MonteCarloTreeSearchExact::getBestContractionSequence() const{
    return bestSequence_;
}
