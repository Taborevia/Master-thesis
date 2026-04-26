#include "MonteCarloTreeSearch_v1.hpp"
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

void printMemoryUsage() {
    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
        // Wynik jest w bajtach, dzielimy przez 1024*1024 dla megabajtów
        std::cout << "Aktualne zuzycie RAM: " << pmc.WorkingSetSize / (1024 * 1024) << " MB" << std::endl;
    }
}


MonteCarloTreeSearch_v1::MonteCarloTreeSearch_v1(std::shared_ptr<IGraph> graph, int maxPairs) : graph_(graph), maxPairs_(maxPairs), vertices_(graph->getNumberOfVertices()) {
    root_ = std::make_shared<Node>(std::make_pair(-1,-1),std::weak_ptr<Node>());
    bestTwinWidth_ = INT32_MAX;
}

MonteCarloTreeSearch_v1::MonteCarloTreeSearch_v1(std::shared_ptr<IGraph> graph, int maxPairs,  uint32_t greedySimulationPairs, uint32_t greedySimulationDepth) : graph_(graph), maxPairs_(maxPairs), vertices_(graph->getNumberOfVertices()), greedySimulationPairs_(greedySimulationPairs), greedySimulationDepth_(greedySimulationDepth) {
    root_ = std::make_shared<Node>(std::make_pair(-1,-1),std::weak_ptr<Node>());
    bestTwinWidth_ = INT32_MAX;
}

MonteCarloTreeSearch_v1::MonteCarloTreeSearch_v1(std::shared_ptr<IGraph> graph, int maxPairs,  uint32_t greedySimulationPairs, uint32_t greedySimulationDepth, double PTW_coeff) : graph_(graph), maxPairs_(maxPairs), vertices_(graph->getNumberOfVertices()), greedySimulationPairs_(greedySimulationPairs), greedySimulationDepth_(greedySimulationDepth), PTW_coeff_(PTW_coeff) {
    root_ = std::make_shared<Node>(std::make_pair(-1,-1),std::weak_ptr<Node>());
    bestTwinWidth_ = INT32_MAX;
}

std::vector<std::pair<int,int>> MonteCarloTreeSearch_v1::generateRandomSequence(VerticesPositions vertices, unsigned int seed){
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

std::vector<std::pair<int,int>> MonteCarloTreeSearch_v1::possibleContractions(VerticesPositions& vertices, int32_t maxPairs) {
    // auto buildStart = std::chrono::high_resolution_clock::now();
    // auto after_random = std::chrono::high_resolution_clock::now();
    // auto after_selection = std::chrono::high_resolution_clock::now();
    // auto after_rowStart = std::chrono::high_resolution_clock::now();
    // auto after_construction = std::chrono::high_resolution_clock::now();
    // auto before_selection = std::chrono::high_resolution_clock::now();

    int n = vertices.vertices.size();
    if (n < 2) return {};

    int totalPairs = n * (n - 1) / 2;

    std::vector<std::pair<int,int>> result;

    if (maxPairs == -1 or maxPairs >= totalPairs) {
        result.reserve(totalPairs);

        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {
                result.emplace_back(vertices.vertices.at(i), vertices.vertices.at(j));
            }
        }
    } else {
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
        // std::uniform_int_distribution<int> dist(0, totalPairs - 1);
        // after_random = std::chrono::high_resolution_clock::now();
        // std::unordered_set<int> selected;
        // selected.reserve(maxPairs);
        // before_selection = std::chrono::high_resolution_clock::now();
        // while (selected.size() < maxPairs) {
        //     selected.insert(dist(gen));
        // }
        // after_selection = std::chrono::high_resolution_clock::now();
        // std::vector<int> rowStart(n + 1);
        // rowStart[0] = 0;
        // for (int i = 0; i < n - 1; ++i) {
        //     rowStart[i + 1] = rowStart[i] + (n - 1 - i);
        // }
        // rowStart[n] = maxPairs;
        // after_rowStart = std::chrono::high_resolution_clock::now();
        // for (int idx : selected) {
        //     int i = static_cast<int>(std::upper_bound(rowStart.begin(), rowStart.begin() + n, idx) - rowStart.begin() - 1);
        //     int offset = idx - rowStart[i];
        //     int j = i + 1 + offset;
        //     result.emplace_back(vertices.vertices.at(i), vertices.vertices.at(j));
        // }
        // after_construction = std::chrono::high_resolution_clock::now();
    }

    // auto buildEnd = std::chrono::high_resolution_clock::now();
    // std::chrono::duration<double> buildTime = buildEnd - buildStart;
    // std::cout  << "after_random: " << std::chrono::duration_cast<std::chrono::microseconds>(after_random - buildStart).count() << " us, "
    //           << "after_selection: " << std::chrono::duration_cast<std::chrono::microseconds>(after_selection - before_selection).count() << " us, "
    //           << "after_rowStart: " << std::chrono::duration_cast<std::chrono::microseconds>(after_rowStart - after_selection).count() << " us, "
    //           << "after_construction: " << std::chrono::duration_cast<std::chrono::microseconds>(after_construction - after_rowStart).count() << " us\n";
    // std::cout << "possibleContractions generated " << result.size() << " pairs in " << std::chrono::duration_cast<std::chrono::microseconds>(buildTime).count() << " us\n";
    
    return result;
}

float MonteCarloTreeSearch_v1::UCT(const std::shared_ptr<Node>& state, int child, float c) const {
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

float MonteCarloTreeSearch_v1::SPUCT_PTW(const std::shared_ptr<Node>& state, int child, float c, float D) const {
    
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

float MonteCarloTreeSearch_v1::SPUCT(const std::shared_ptr<Node>& state, int child, float c, float D) const {
    
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

std::shared_ptr<Node> MonteCarloTreeSearch_v1::bestChild(const std::shared_ptr<Node>& state, float c, float D) {
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

std::shared_ptr<Node> MonteCarloTreeSearch_v1::expand(const std::shared_ptr<Node>& state, VerticesPositions& vertices){
    // auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    // std::mt19937 rng(static_cast<unsigned>(seed));
    // std::uniform_int_distribution<int> genRand(0, state->possible_actions.size()-1);
    // int move = genRand(rng);
    auto new_contraction = state->possibleContractions_.back();
    state->possibleContractions_.pop_back();
    vertices.erase(new_contraction.second);
    auto new_state = std::make_shared<Node>(new_contraction,state);
    new_state->possibleContractions_ = possibleContractions(vertices, maxPairs_);

    state->children_.push_back(new_state);

    return new_state;
}

std::shared_ptr<Node> MonteCarloTreeSearch_v1::treePolicy(const std::shared_ptr<Node>& state, float c, float D, VerticesPositions& vertices, std::vector<std::pair<int,int>>& contractionSequence){
    if(state->possibleContractions_.size()==0 && state->children_.size()==0){
        return state;
    }
    // do dodania, warunek konca kontrakcji
    if (state->possibleContractions_.size()!=0){
        auto result = expand(state,vertices);
        contractionSequence.push_back(result->contraction_);
        return result;
    }else{
        auto child = bestChild(state,c,D);
        vertices.erase(child->contraction_.second);
        contractionSequence.push_back(child->contraction_);
        return treePolicy(child,c,D,vertices,contractionSequence);
    }
}

uint32_t MonteCarloTreeSearch_v1::randomDefaultPolicy(std::vector<std::pair<int,int>> contractionSequence, VerticesPositions vertices){
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

uint32_t MonteCarloTreeSearch_v1::greedyDefaultPolicy(std::vector<std::pair<int,int>> contractionSequence, VerticesPositions vertices, const std::shared_ptr<Node>& node, int maxPairs, int depth){
    auto graph = graph_->clone();
    // std::cout << "Greedy default policy with depth " << depth << " and maxPairs " << maxPairs << std::endl;
    graph->contractGraph(contractionSequence);
    node->currentTwinWidth = graph->getMaxRedDegree();
    while (depth > 0 && vertices.vertices.size() > 1) {
        std::pair<int,int> bestPair;
        auto contractions = possibleContractions(vertices, maxPairs);
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
        depth--;
    }
    return graph->getMaxRedDegree();
}

void MonteCarloTreeSearch_v1::backPropagation(const std::shared_ptr<Node>& state, int reward){
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

std::shared_ptr<Node> MonteCarloTreeSearch_v1::bestContraction(){
    auto current = root_;
    int32_t mostVisitedChild = 0;
    std::shared_ptr<Node> bestChild = nullptr;
    
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

void MonteCarloTreeSearch_v1::makeContraction(float resources, float c_parameter, float D_parameter){
    int simulationCounter = 0;
    auto start = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration{};
    // std::cout << "Starting contraction with " << n << " vertices. Time limit: " << resources << " seconds\n";
    auto buildStart = std::chrono::high_resolution_clock::now();
    root_->possibleContractions_ = possibleContractions(vertices_, maxPairs_);
    auto buildEnd = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> buildTime = buildEnd - buildStart;

    std::chrono::duration<double> treePolicyTime{};
    std::chrono::duration<double> defaultPolicyTime{};
    std::chrono::duration<double> backPropagationTime{};

    while(duration.count() < resources){
        simulationCounter++;
        auto temp = vertices_;
        std::vector<std::pair<int,int>> contractionSequence;

        auto treeStart = std::chrono::high_resolution_clock::now();
        auto node = treePolicy(root_,c_parameter,D_parameter,temp,contractionSequence);
        auto treeEnd = std::chrono::high_resolution_clock::now();
        treePolicyTime += treeEnd - treeStart;

        auto defaultStart = std::chrono::high_resolution_clock::now();
        uint32_t result;
        if (greedySimulationPairs_ > 0 && greedySimulationDepth_ > 0) {
            result = greedyDefaultPolicy(contractionSequence, temp, node, greedySimulationPairs_, greedySimulationDepth_);
        } else {
            result = randomDefaultPolicy(contractionSequence, temp);
        }
        auto defaultEnd = std::chrono::high_resolution_clock::now();
        defaultPolicyTime += defaultEnd - defaultStart;

        auto backStart = std::chrono::high_resolution_clock::now();
        backPropagation(node,result);
        auto backEnd = std::chrono::high_resolution_clock::now();
        backPropagationTime += backEnd - backStart;

        auto now = std::chrono::high_resolution_clock::now();
        duration = now - start;
    }

    auto best = bestContraction();
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
              << " total time for makeContraction: " << std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() << " ms\n";
}

void MonteCarloTreeSearch_v1::findSequence(float resources, float c_parameter, float D_parameter){
    int n = graph_->getNumberOfVertices();
    std::vector<double> timeLimits;
    timeLimits.reserve(n - 1);
    if (dynamicTimeDistribution_) {
        for (int i = n-2; i >= 0; --i) {
            timeLimits[(n-2)-i] = resources * ((i + 1) / (n*(n-1)/2.0));
        }
        for (int i = 0; i < n - 1; ++i) {
            // printMemoryUsage();
            makeContraction(timeLimits[i], c_parameter, D_parameter);
        }
        if (bestTwinWidth_ > currentTwinWidth_){
            bestTwinWidth_ = currentTwinWidth_;
            bestSequence_ = currentSequence_;
        }
    } else {
        for (int i = 0; i < n - 1; ++i) {
            makeContraction(resources / (n - 1), c_parameter, D_parameter);
        }
        if (bestTwinWidth_ > currentTwinWidth_){
            bestTwinWidth_ = currentTwinWidth_;
            bestSequence_ = currentSequence_;
        }
    }
}

int MonteCarloTreeSearch_v1::getBestTwinWidth() const {
    return bestTwinWidth_;
};

std::vector<std::pair<int,int>> MonteCarloTreeSearch_v1::getBestContractionSequence() const{
    return bestSequence_;
}
