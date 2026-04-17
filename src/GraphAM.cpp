#include "GraphAM.hpp"
#include <iostream>
#include <iomanip>
#include <memory>

GraphAM::GraphAM(int vertices)
    : vertices_(vertices), maxRedDegree_(0), adjMatrix_(vertices, std::vector<int>(vertices, 0)), redDegrees_(vertices,0) {}

void GraphAM::addEdge(int u, int v, EdgeColor color) {
    int edgeValue = (color == EdgeColor::Black) ? 1 : 2;
    adjMatrix_[u][v] = edgeValue;
    adjMatrix_[v][u] = edgeValue;
}

bool GraphAM::isEdge(int u, int v, EdgeColor color) const {
    int edgeValue = (color == EdgeColor::Black) ? 1 : 2;
    return adjMatrix_[u][v] == edgeValue;
}

void GraphAM::print() const {
    for (int i = 0; i < vertices_; ++i) {
        for (int j = 0; j < vertices_; ++j) {
            if (adjMatrix_[i][j] == 0) {
                std::cout << "- ";
            } else if (adjMatrix_[i][j] == 1) {
                std::cout << "B ";
            } else if (adjMatrix_[i][j] == 2) {
                std::cout << "R ";
            }
        }
        std::cout << std::endl;
    }
}
int GraphAM::estimateTwinWidthAfterContraction(int u, int v) const {
    return -1;
}

void GraphAM::contractVertices(int u, int v) {
    if (u == v) return;
    if(adjMatrix_[u][v] == 2){
        redDegrees_[u]--;
    }
    adjMatrix_[u][v] = 0;
    adjMatrix_[v][u] = 0;
    redDegrees_[v] = 0;
    for (int i = 0; i < vertices_; ++i) {
        if (i == u || i == v) continue;
        int redEdges = (adjMatrix_[u][i] == 2) + (adjMatrix_[v][i] == 2);
        if (adjMatrix_[u][i] != adjMatrix_[v][i]){
            if (adjMatrix_[u][i]!=2){
                redDegrees_[u]++;    
            }
            if(redEdges==0){
                redDegrees_[i]++;
            }
            adjMatrix_[u][i] = 2;
            adjMatrix_[i][u] = 2;
        }
        if (redEdges==2){
            redDegrees_[i]--;
        }
        // removing edges incident to v
        adjMatrix_[v][i] = 0;
        adjMatrix_[i][v] = 0;
        if (redDegrees_[i]>maxRedDegree_) maxRedDegree_=redDegrees_[i];
    }
    if (redDegrees_[u]>maxRedDegree_) maxRedDegree_=redDegrees_[u];
}

int GraphAM::contractGraph(std::vector<std::pair<int,int>> contractionSequnce){
    for (const auto& contraction : contractionSequnce){
        contractVertices(contraction.first, contraction.second);
    }
    return maxRedDegree_;
}

int GraphAM::getMaxRedDegree() const{
    return maxRedDegree_;
};

int GraphAM::getNumberOfVertices() const {
    return vertices_;
}

std::unique_ptr<IGraph> GraphAM::clone() const {
    auto copy = std::make_unique<GraphAM>(vertices_);
    copy->maxRedDegree_ = maxRedDegree_;
    copy->adjMatrix_ = adjMatrix_;
    copy->redDegrees_ = redDegrees_;
    return copy;
}
