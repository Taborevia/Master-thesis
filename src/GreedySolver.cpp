#include "GreedySolver.hpp"
#include <chrono>
#include <iostream>

GreedySolver::GreedySolver(std::shared_ptr<IGraph> graph) : graph_(graph), currentGraph_(graph){
    bestTwinWidth_ = INT32_MAX;
}

std::vector<std::pair<int,int>> GreedySolver::possibleContractions(std::set<int> vertices){
    std::vector<std::pair<int,int>> result;
    int n = vertices.size();
    for (auto it1 = vertices.begin(); it1 != vertices.end(); ++it1) {
        auto it2 = it1;
        ++it2;
        for (; it2 != vertices.end(); ++it2) {
            result.emplace_back(std::make_pair(*it1, *it2));
        }
    }
    return result;
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
        auto contractions = possibleContractions(vertices);
        const auto contractionsEnd = std::chrono::steady_clock::now();
        contractionsTime += contractionsEnd - contractionsStart;

        int bestTwinWidth = INT32_MAX;
        std::pair<int,int> bestContraction;

        for (const auto& contraction : contractions) {
            const auto cloneStart = std::chrono::steady_clock::now();
            auto graphCopy = currentGraph_->clone();
            const auto cloneEnd = std::chrono::steady_clock::now();
            cloneTime += cloneEnd - cloneStart;

            const auto evaluationStart = std::chrono::steady_clock::now();
            graphCopy->contractVertices(contraction.first, contraction.second);
            int twinWidth = graphCopy->getMaxRedDegree();
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
