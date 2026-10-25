#include "GreedySolver.hpp"
#include <random>
#include <algorithm>
#include <unordered_set>

GreedySolver::GreedySolver(std::shared_ptr<IGraph> graph) : graph_(graph), currentGraph_(graph){
    bestTwinWidth_ = INT32_MAX;
}

std::vector<std::pair<int,int>> GreedySolver::possibleContractions(const std::set<int>& vertices) const {
    std::vector<std::pair<int,int>> result;
    if (vertices.size() < 2)
        return result;

    result.reserve((vertices.size() * (vertices.size() - 1)) / 2);
    for (auto it1 = vertices.begin(); it1 != vertices.end(); ++it1) {
        auto it2 = it1;
        ++it2;
        for (; it2 != vertices.end(); ++it2) {
            result.emplace_back(*it1, *it2);
        }
    }
    return result;
}

std::vector<std::pair<int,int>> GreedySolver::possibleContractions(const std::vector<int>& vertices) const {
    std::vector<std::pair<int,int>> result;
    size_t n = vertices.size();
    if (n < 2) return result;

    result.reserve(n * (n - 1) / 2);

    for (size_t i = 0; i < n; ++i) {
        for (size_t j = i + 1; j < n; ++j) {
            result.emplace_back(vertices[i], vertices[j]);
        }
    }
    return result;
}

std::vector<std::pair<int,int>> GreedySolver::samplePossibleContractions(const std::vector<int>& vertices, int numberOfContractions) const {
    std::vector<std::pair<int,int>> result;
    size_t n = vertices.size();
    if (n < 2 || numberOfContractions <= 0)
        return result;

    size_t totalPairs = n * (n - 1) / 2;
    if (static_cast<size_t>(numberOfContractions) >= totalPairs)
        return possibleContractions(vertices);

    std::vector<int> verts(vertices.begin(), vertices.end());
    result.reserve(numberOfContractions);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dist(0, totalPairs - 1);

    std::unordered_set<size_t> selected;
    selected.reserve(numberOfContractions * 2);
    while (selected.size() < static_cast<size_t>(numberOfContractions)) {
        selected.insert(dist(gen));
    }

    std::vector<size_t> rowStart(n + 1);
    rowStart[0] = 0;
    for (size_t i = 0; i < n - 1; ++i) {
        rowStart[i + 1] = rowStart[i] + (n - 1 - i);
    }
    rowStart[n] = totalPairs;

    for (size_t idx : selected) {
        size_t i = static_cast<size_t>(std::upper_bound(rowStart.begin(), rowStart.begin() + n, idx) - rowStart.begin() - 1);
        size_t offset = idx - rowStart[i];
        size_t j = i + 1 + offset;
        result.emplace_back(verts[i], verts[j]);
    }

    return result;
}

void GreedySolver::partialFindSequence(int maxPairs) {
    int n = graph_->getNumberOfVertices();
    std::vector<int> vertices;
    for (int i = 0; i < n; i++)
        vertices.push_back(i);

    while (vertices.size() > 1) {
        auto contractions = samplePossibleContractions(vertices, maxPairs);

        int bestTwinWidth = INT32_MAX;
        std::pair<int,int> bestContraction;

        for (const auto& contraction : contractions) {
            int twinWidth = currentGraph_->estimateTwinWidthAfterContraction(contraction.first, contraction.second);
            if (twinWidth < bestTwinWidth) {
                bestTwinWidth = twinWidth;
                bestContraction = contraction;
            }
        }

        currentSequence_.push_back(bestContraction);
        currentGraph_->contractVertices(bestContraction.first, bestContraction.second);
        auto it = std::find(vertices.begin(), vertices.end(), bestContraction.second);
        if (it != vertices.end()) {
            std::iter_swap(it, vertices.end() - 1);
            vertices.pop_back();
        }
        currentTwinWidth_ = currentGraph_->getMaxRedDegree();
    }

    bestSequence_ = currentSequence_;
    bestTwinWidth_ = currentTwinWidth_;
}

void GreedySolver::fullyFindSequence(float resources, float c_parameter){
    int n = graph_->getNumberOfVertices();
    std::set<int> vertices;
    for (int i = 0; i < n; i++)
        vertices.insert(i);

    while (vertices.size() > 1) {
        int bestTwinWidth = INT32_MAX;
        std::pair<int,int> bestContraction;

        for (auto it1 = vertices.begin(); it1 != vertices.end(); ++it1) {
            auto it2 = it1;
            ++it2;
            for (; it2 != vertices.end(); ++it2) {
                int twinWidth = currentGraph_->estimateTwinWidthAfterContraction(*it1, *it2);
                if (twinWidth < bestTwinWidth) {
                    bestTwinWidth = twinWidth;
                    bestContraction = {*it1, *it2};
                }
            }
        }

        currentSequence_.push_back(bestContraction);
        currentGraph_->contractVertices(bestContraction.first, bestContraction.second);
        vertices.erase(bestContraction.second);
        currentTwinWidth_ = currentGraph_->getMaxRedDegree();
    }

    bestSequence_ = currentSequence_;
    bestTwinWidth_ = currentTwinWidth_;
}

void GreedySolver::findSequence(float resources, float c_parameter, float D_parameter){
    if (resources > 0) {
        partialFindSequence((int)resources); // maxPairs ustawione na 1000 dla testów
    } else {
        fullyFindSequence(resources, c_parameter);
    }
}

int GreedySolver::getBestTwinWidth() const {
    return bestTwinWidth_;
};

std::vector<std::pair<int,int>> GreedySolver::getBestContractionSequence() const{
    return bestSequence_;
}
