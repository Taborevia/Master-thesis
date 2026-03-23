#include "GraphAL.hpp"
#include <iostream>
#include <algorithm>
#include <fstream>
#include <map>

GraphAL::GraphAL(int vertices)
    : vertices_(vertices), maxRedDegree_(0), redDegrees_(vertices,0)
{
    adjList_.resize(vertices_);
}

void GraphAL::addEdge(int u, int v, EdgeColor color) {
    int edgeColor = (color == EdgeColor::Black) ? 1 : 2;
    auto itU = std::lower_bound(adjList_[u].begin(), adjList_[u].end(), std::make_pair(v, edgeColor),
        [](const std::pair<int, int>& a, const std::pair<int, int>& b) {
            return a.first < b.first;
        });
    if (itU == adjList_[u].end() || itU->first != v) {
        adjList_[u].insert(itU, {v, edgeColor});
    }

    auto itV = std::lower_bound(adjList_[v].begin(), adjList_[v].end(), std::make_pair(u, edgeColor),
        [](const std::pair<int, int>& a, const std::pair<int, int>& b) {
            return a.first < b.first; 
        });
    if (itV == adjList_[v].end() || itV->first != u) {
        adjList_[v].insert(itV, {u, edgeColor});
    }
}

bool GraphAL::isEdge(int u, int v, EdgeColor color) const {
    int edgeColor = (color == EdgeColor::Black) ? 1 : 2;
    for (const auto& neighbor : adjList_[u]) {
        if (neighbor.first == v && neighbor.second == edgeColor) return true;
    }
    return false;
}

std::unique_ptr<IGraph> GraphAL::clone() const {
    return std::make_unique<GraphAL>(*this);
}


void GraphAL::deleteEdge(int u, int v){
    auto itU = std::lower_bound(adjList_[u].begin(), adjList_[u].end(), std::make_pair(v, 1),
        [](const std::pair<int, int>& a, const std::pair<int, int>& b) {
            return a.first < b.first;
        });
    if (itU != adjList_[u].end() && itU->first == v) {
        adjList_[u].erase(itU);
    }
}

void GraphAL::changeEdgeColor(int u, int v, int color){
    auto itU = std::lower_bound(adjList_[u].begin(), adjList_[u].end(), std::make_pair(v, color),
        [](const std::pair<int, int>& a, const std::pair<int, int>& b) {
            return a.first < b.first;
        });
    if (itU != adjList_[u].end() && itU->first == v) {
        itU->second=color;
    }
}
void GraphAL::changeEdge(int u, int v, int new_v, int color){
    auto itU = std::lower_bound(adjList_[u].begin(), adjList_[u].end(), std::make_pair(v, color),
        [](const std::pair<int, int>& a, const std::pair<int, int>& b) {
            return a.first < b.first;
        });
    if (itU != adjList_[u].end() && itU->first == v) {
        itU->first=new_v;
        itU->second=color;
    }

    while (itU != adjList_[u].begin() && itU->first < std::prev(itU)->first) {
        std::iter_swap(itU, std::prev(itU));
        --itU;
    }

    while (std::next(itU) != adjList_[u].end() && itU->first > std::next(itU)->first) {
        std::iter_swap(itU, std::next(itU));
        ++itU;
    }
}

int GraphAL::getNumberOfVertices() const {
    return vertices_;
}

void GraphAL::print() const {
    for (int i = 0; i < vertices_; ++i) {
        std::cout << i << ": ";
        for (const auto& neighbor : adjList_[i]) {
            std::cout << neighbor.first << "(" << (neighbor.second == 1 ? "B" : "R") << ") ";
        }
        std::cout << std::endl;
    }
}

void GraphAL::contractVertices(int u, int v) {
    std::vector<std::pair<int, int>> mergedEdges;

    auto itU = adjList_[u].begin();
    auto itV = adjList_[v].begin();
    int newVertexRedDegree = 0;

    while (itU != adjList_[u].end() && itV != adjList_[v].end()) {
        if (itU->first == v) {
            ++itU;
            continue;
        }
        if (itV->first == u) {
            ++itV;
            continue;
        }
        if (itU->first < itV->first) {
            mergedEdges.push_back({itU->first, 2});
            newVertexRedDegree++;
            changeEdgeColor(itU->first,u,2);
            if (itU->second!=2){
                redDegrees_[itU->first]++;
                if (redDegrees_[itU->first]>maxRedDegree_){
                    maxRedDegree_ = redDegrees_[itU->first];
                }
            }
            ++itU;
        } else if (itV->first < itU->first) {
            mergedEdges.push_back({itV->first, 2});
            newVertexRedDegree++;
            changeEdge(itV->first,v,u,2);
            if (itV->second!=2){
                redDegrees_[itV->first]++;
                if (redDegrees_[itV->first]>maxRedDegree_){
                    maxRedDegree_ = redDegrees_[itV->first];
                }
            }
            ++itV;
        } else {
            bool uEdgeColor = itU->second == 2;
            bool vEdgeColor = itV->second == 2;
            int color = (uEdgeColor || vEdgeColor)
                                  ? 2
                                  : 1;
            mergedEdges.push_back({itU->first, color});
            deleteEdge(itU->first,v);
            changeEdgeColor(itU->first,u,color);
            if (uEdgeColor && vEdgeColor){
                redDegrees_[itU->first]--;
            }
            if (color == 2){
                newVertexRedDegree++;
            }
            ++itU;
            ++itV;
        }
    }

    while (itU != adjList_[u].end()) {
        if (itU->first == v) {
            ++itU;
            continue;
        }
        mergedEdges.push_back({itU->first, 2});
        newVertexRedDegree++;
        changeEdgeColor(itU->first,u,2);
        if (itU->second!=2){
            redDegrees_[itU->first]++;
            if (redDegrees_[itU->first]>maxRedDegree_){
                maxRedDegree_ = redDegrees_[itU->first];
            }
        }
        ++itU;
    }

    while (itV != adjList_[v].end()) {
        if (itV->first == u) {
            ++itV;
            continue;
        }
        mergedEdges.push_back({itV->first, 2});
        newVertexRedDegree++;
        changeEdge(itV->first,v,u,2);
        if (itV->second!=2){
            redDegrees_[itV->first]++;
            if (redDegrees_[itV->first]>maxRedDegree_){
                maxRedDegree_ = redDegrees_[itV->first];
            }
        }
        ++itV;
    }
    // Usuń stare krawędzie i przypisz nową listę sąsiedztwa do wierzchołka u
    adjList_[u] = std::move(mergedEdges);
    redDegrees_[u] = newVertexRedDegree;
    if (newVertexRedDegree>maxRedDegree_){
        maxRedDegree_ = newVertexRedDegree;
    }
    adjList_[v].clear();
    redDegrees_[v]=0;
}

int GraphAL::getMaxRedDegree() const{
    return maxRedDegree_;
}

int GraphAL::contractGraph(std::vector<std::pair<int,int>> contractionSequnce){
    for (const auto& contraction : contractionSequnce){
        // print();
        // std::cout<<"Contract: "<<contraction.first<<" z "<<contraction.second;
        contractVertices(contraction.first, contraction.second);
        // std::cout<<"Max red degree: "<<maxRedDegree_<<"\n";
        // for (const auto& deg : redDegrees_){
            // std::cout<<deg<<" ";
        // }
        // std::cout<<"\n";
    }
    // std::cout<<"\n";
    // std::cout<<maxRedDegree_;
    return maxRedDegree_; 
}