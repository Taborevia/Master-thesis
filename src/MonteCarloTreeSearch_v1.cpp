#include "MonteCarloTreeSearch_v1.hpp"
#include <vector>
#include <set>
#include <tuple>
#include <utility>
#include <random>
#include <algorithm>
#include <iostream>
#include <chrono>

std::vector<std::pair<int,int>> generateRandomSequence(std::vector<int> vertices, unsigned int seed){
    std::mt19937 rng(seed);
    std::vector<std::pair<int,int>> sequence;
    unsigned int n = vertices.size();
    for(int i =0;i<n-1;i++){
        std::uniform_int_distribution<unsigned int> dist(0, vertices.size()-1);
        unsigned int random_1 = dist(rng);
        unsigned int random_2;
        do{
            random_2 = dist(rng);
        }while(random_1 == random_2);
        sequence.emplace_back(vertices.at(random_1),vertices.at(random_2));
        std::swap(vertices.at(random_2), vertices.back());
        vertices.pop_back();
    }
    return sequence;
}

MonteCarloTreeSearch_v1::MonteCarloTreeSearch_v1(std::shared_ptr<IGraph> graph) : graph_(graph){
    root_ = std::make_shared<Node>(std::make_pair(-1,-1),std::weak_ptr<Node>());
    bestTwinWidth_ = INT32_MAX;
}

std::vector<std::pair<int,int>> MonteCarloTreeSearch_v1::possibleContractions(std::set<int> vertices){
    std::vector<std::pair<int,int>> result;
    int n = vertices.size();
    for (auto it1 = vertices.begin(); it1 != vertices.end(); ++it1) {
        auto it2 = it1;
        ++it2;
        for (; it2 != vertices.end(); ++it2) {
            result.emplace_back(*it1, *it2);
        }
    }
    return result;
}

std::shared_ptr<Node> MonteCarloTreeSearch_v1::bestChild(std::shared_ptr<Node> state, float c){
    int best_child= INT_MIN;
    float best_value = INT_MIN;
    for (int i = 0;i<state->children_.size();i++){
        float value = (-1*state->children_.at(i)->value_/state->children_.at(i)->visits_)+(c*sqrt(log(state->visits_)/state->children_.at(i)->visits_));
        if (value > best_value){
            best_child = i;
            best_value = value;
        }
    }
    return state->children_.at(best_child);
}

std::shared_ptr<Node> MonteCarloTreeSearch_v1::expand(std::shared_ptr<Node> state, std::set<int>& vertices){
    // auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    // std::mt19937 rng(static_cast<unsigned>(seed));
    // std::uniform_int_distribution<int> genRand(0, state->possible_actions.size()-1);
    // int move = genRand(rng);
    auto new_contraction = state->possibleContractions_.back();
    state->possibleContractions_.pop_back();
    vertices.erase(new_contraction.second);
    auto new_state = std::make_shared<Node>(new_contraction,state);
    new_state->possibleContractions_ = possibleContractions(vertices);

    state->children_.push_back(new_state);

    return new_state;
}

std::tuple<std::shared_ptr<Node>,std::vector<std::pair<int,int>>,std::set<int>> MonteCarloTreeSearch_v1::treePolicy(std::shared_ptr<Node> state, float c, std::set<int>& vertices, std::vector<std::pair<int,int>>& contractionSequence){
    if(state->possibleContractions_.size()==0 && state->children_.size()==0){
        return std::make_tuple(state,contractionSequence,vertices);
    }
    // do dodania, warunek konca kontrakcji
    if (state->possibleContractions_.size()!=0){
        auto result = expand(state,vertices);
        contractionSequence.push_back(result->contraction_);
        return std::make_tuple(result,contractionSequence,vertices);
    }else{
        auto child = bestChild(state,c);
        vertices.erase(child->contraction_.second);
        contractionSequence.push_back(child->contraction_);
        return treePolicy(bestChild(state,c),c,vertices,contractionSequence);
    }
}

std::pair<int,std::vector<std::pair<int,int>>> MonteCarloTreeSearch_v1::defaultPolicy(std::vector<std::pair<int,int>> contractionSequence, std::shared_ptr<IGraph> graph, std::set<int> vertices){
    int result = 0;
    auto graphCopy = graph_->clone();
    auto randomContractionSequence = generateRandomSequence(std::vector<int>(vertices.begin(),vertices.end()),std::chrono::high_resolution_clock::now().time_since_epoch().count());
    contractionSequence.insert(contractionSequence.end(),randomContractionSequence.begin(),randomContractionSequence.end());
    result = graphCopy->contractGraph(contractionSequence);
    return std::make_pair(result,contractionSequence);
}

void MonteCarloTreeSearch_v1::backPropagation(std::shared_ptr<Node> state, float reward){
    state->value_+=reward;
    state->visits_+=1;

    if(auto parent = state->parent_.lock()){
        backPropagation(parent,reward);
    }
}

void MonteCarloTreeSearch_v1::findSequence(float resources, float c_parameter){
    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    int n = graph_->getNumberOfVertices();
        std::set<int> vertices;
    for (int i =0;i<n;i++)
        vertices.insert(i);
    root_->possibleContractions_ = possibleContractions(vertices);
    while(duration.count() < resources){
        auto temp = vertices;
        std::vector<std::pair<int,int>> contractionSequence;
        auto [a,b,c] = treePolicy(root_,c_parameter,temp,contractionSequence);
        std::shared_ptr<Node> v = a;
        // std::cout<<"Poczatek\n";
        auto result = defaultPolicy(b,graph_,c);
        for(const auto& i : result.second){
            // std::cout<<i.first<<" z "<<i.second<<std::endl;
        }
        backPropagation(v,result.first);
        if (bestTwinWidth_>result.first){
            bestTwinWidth_=result.first;
            bestSequence_=result.second;
        }
        end = std::chrono::high_resolution_clock::now();
        duration = end - start;
    }
}

int MonteCarloTreeSearch_v1::getBestTwinWidth() const {
    return bestTwinWidth_;
};

std::vector<std::pair<int,int>> MonteCarloTreeSearch_v1::getBestContractionSequence() const{
    return bestSequence_;
}
