#include "GreedySolver.hpp"
#include <chrono>
#include <iostream>
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

// std::vector<std::pair<int,int>> GreedySolver::samplePossibleContractions(const std::set<int>& vertices, int numberOfContractions) const {
//     std::vector<std::pair<int,int>> result;
//     size_t n = vertices.size();
//     if (n < 2 || numberOfContractions <= 0)
//         return result;

//     size_t totalPairs = n * (n - 1) / 2;
//     if (static_cast<size_t>(numberOfContractions) >= totalPairs)
//         return possibleContractions(vertices);

//     std::vector<int> verts(vertices.begin(), vertices.end());
//     result.reserve(numberOfContractions);

//     std::random_device rd;
//     std::mt19937 gen(rd());
//     std::uniform_int_distribution<size_t> dist(0, totalPairs - 1);

//     std::unordered_set<size_t> selected;
//     selected.reserve(numberOfContractions * 2);
//     while (selected.size() < static_cast<size_t>(numberOfContractions)) {
//         selected.insert(dist(gen));
//     }

//     std::vector<size_t> rowStart(n + 1);
//     rowStart[0] = 0;
//     for (size_t i = 0; i < n - 1; ++i) {
//         rowStart[i + 1] = rowStart[i] + (n - 1 - i);
//     }
//     rowStart[n] = totalPairs;

//     for (size_t idx : selected) {
//         size_t i = static_cast<size_t>(std::upper_bound(rowStart.begin(), rowStart.begin() + n, idx) - rowStart.begin() - 1);
//         size_t offset = idx - rowStart[i];
//         size_t j = i + 1 + offset;
//         result.emplace_back(verts[i], verts[j]);
//     }
//     return result;
// }

void GreedySolver::partialFindSequence(int maxPairs) {
    auto totalStart = std::chrono::steady_clock::now();
    std::chrono::steady_clock::duration contractionsTime{};
    std::chrono::steady_clock::duration cloneTime{};
    std::chrono::steady_clock::duration evaluationTime{};
    std::chrono::steady_clock::duration applyTime{};

    int n = graph_->getNumberOfVertices();
    std::vector<int> vertices;
    for (int i = 0; i < n; i++)
        vertices.push_back(i);

    while (vertices.size() > 1) {
        const auto contractionsStart = std::chrono::steady_clock::now();
        auto contractions = samplePossibleContractions(vertices,maxPairs);
        const auto contractionsEnd = std::chrono::steady_clock::now();
        contractionsTime += contractionsEnd - contractionsStart;
        

        int bestTwinWidth = INT32_MAX;
        std::pair<int,int> bestContraction;

        for (const auto& contraction : contractions) {
            const auto evaluationStart = std::chrono::steady_clock::now();
            int twinWidth = currentGraph_->estimateTwinWidthAfterContraction(contraction.first, contraction.second);
            const auto evaluationEnd = std::chrono::steady_clock::now();
            evaluationTime += evaluationEnd - evaluationStart;
            if (twinWidth < bestTwinWidth) {
                bestTwinWidth = twinWidth;
                bestContraction = contraction;
            }
        }

        const auto applyStart = std::chrono::steady_clock::now();
        currentSequence_.push_back(bestContraction);
        currentGraph_->contractVertices(bestContraction.first, bestContraction.second);
        auto it = std::find(vertices.begin(), vertices.end(), bestContraction.second);
        if (it != vertices.end()) {
            std::iter_swap(it, vertices.end() - 1); // Zamień miejscami z ostatnim
            vertices.pop_back();                   // Usuń ostatni (O(1))
        }
        currentTwinWidth_ = currentGraph_->getMaxRedDegree();
        const auto applyEnd = std::chrono::steady_clock::now();
        applyTime += applyEnd - applyStart;
    }

    auto totalEnd = std::chrono::steady_clock::now();
    auto totalTime = totalEnd - totalStart;

    bestSequence_ = currentSequence_;
    bestTwinWidth_ = currentTwinWidth_;

    std::cout << "Profiling GreedySolver::findSequence()\n"
              << "  possibleContractions: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(contractionsTime).count() << " ms\n"
              << "  clone graph: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(cloneTime).count() << " ms\n"
              << "  evaluation loop: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(evaluationTime).count() << " ms\n"
              << "  apply contraction: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(applyTime).count() << " ms\n"
              << "  total: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(totalTime).count() << " ms\n";
}

void GreedySolver::findSequence(float resources, float c_parameter){
    auto totalStart = std::chrono::steady_clock::now();
    std::chrono::steady_clock::duration contractionsTime{};
    std::chrono::steady_clock::duration cloneTime{};
    std::chrono::steady_clock::duration evaluationTime{};
    std::chrono::steady_clock::duration applyTime{};

    int n = graph_->getNumberOfVertices();
    std::set<int> vertices;
    for (int i = 0; i < n; i++)
        vertices.insert(i);

    while (vertices.size() > 1) {
        const auto contractionsStart = std::chrono::steady_clock::now();

        int bestTwinWidth = INT32_MAX;
        std::pair<int,int> bestContraction;

        for (auto it1 = vertices.begin(); it1 != vertices.end(); ++it1) {
            auto it2 = it1;
            ++it2;
            for (; it2 != vertices.end(); ++it2) {
                const auto evaluationStart = std::chrono::steady_clock::now();
                int twinWidth = currentGraph_->estimateTwinWidthAfterContraction(*it1, *it2);
                const auto evaluationEnd = std::chrono::steady_clock::now();
                evaluationTime += evaluationEnd - evaluationStart;
                if (twinWidth < bestTwinWidth) {
                    bestTwinWidth = twinWidth;
                    bestContraction = {*it1, *it2};
                }
            }
        }

        const auto contractionsEnd = std::chrono::steady_clock::now();
        contractionsTime += contractionsEnd - contractionsStart;

        const auto applyStart = std::chrono::steady_clock::now();
        currentSequence_.push_back(bestContraction);
        currentGraph_->contractVertices(bestContraction.first, bestContraction.second);
        vertices.erase(bestContraction.second);
        currentTwinWidth_ = currentGraph_->getMaxRedDegree();
        const auto applyEnd = std::chrono::steady_clock::now();
        applyTime += applyEnd - applyStart;
    }

    auto totalEnd = std::chrono::steady_clock::now();
    auto totalTime = totalEnd - totalStart;

    bestSequence_ = currentSequence_;
    bestTwinWidth_ = currentTwinWidth_;

    std::cout << "Profiling GreedySolver::findSequence()\n"
              << "  possibleContractions: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(contractionsTime).count() << " ms\n"
              << "  clone graph: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(cloneTime).count() << " ms\n"
              << "  evaluation loop: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(evaluationTime).count() << " ms\n"
              << "  apply contraction: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(applyTime).count() << " ms\n"
              << "  total: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(totalTime).count() << " ms\n";
}

int GreedySolver::getBestTwinWidth() const {
    return bestTwinWidth_;
};

std::vector<std::pair<int,int>> GreedySolver::getBestContractionSequence() const{
    return bestSequence_;
}
