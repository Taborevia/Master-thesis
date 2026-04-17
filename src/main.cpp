#include "GraphAL.hpp"
#include "ITwinWidthSolver.hpp"
#include "MonteCarloTreeSearch_v1.hpp"
#include "GreedySolver.hpp"
#include "FileReader.hpp"
#include <iostream>
#include <numeric>
#include <cmath>
#include <chrono>
#include <filesystem>
#include <algorithm>
#include <fstream>
#include <cctype>

#ifndef PROJECT_ROOT
#define PROJECT_ROOT "."
#endif

std::vector<std::string> collectExactPublicInstancePaths() {
    std::filesystem::path instancesPath = std::filesystem::path(PROJECT_ROOT) / "data" / "exact-public" / "instances";
    std::vector<std::string> paths;

    if (!std::filesystem::exists(instancesPath) || !std::filesystem::is_directory(instancesPath)) {
        return paths;
    }

    for (const auto& entry : std::filesystem::directory_iterator(instancesPath)) {
        if (entry.is_regular_file()) {
            paths.push_back(entry.path().generic_string());
        }
    }

    std::sort(paths.begin(), paths.end());
    return paths;
}

std::string extractInstanceNumber(const std::filesystem::path& instancePath) {
    std::string stem = instancePath.stem().generic_string();
    std::string digits;
    for (char c : stem) {
        if (std::isdigit(static_cast<unsigned char>(c))) {
            digits.push_back(c);
        }
    }
    return digits.empty() ? stem : digits;
}

std::filesystem::path resultPathForInstance(const std::filesystem::path& instancePath) {
    std::filesystem::path resultsDir = std::filesystem::path(PROJECT_ROOT) / "data" / "heuristic-private" / "results";
    std::filesystem::create_directories(resultsDir);
    std::string graphNumber = extractInstanceNumber(instancePath);
    std::string filename = "result_" + graphNumber + ".tww";
    return resultsDir / filename;
}

bool writeGreedyResultForInstance(const std::filesystem::path& instancePath) {
    std::shared_ptr<IGraph> g = readGraphFromFile(instancePath.generic_string());
    std::shared_ptr<GreedySolver> solver = std::make_shared<GreedySolver>(g);

    solver->partialFindSequence(10000);
    auto sequence = solver->getBestContractionSequence();
    auto resultPath = resultPathForInstance(instancePath);

    std::ofstream out(resultPath);
    if (!out.is_open()) {
        std::cerr << "Cannot open output file: " << resultPath.generic_string() << std::endl;
        return false;
    }

    for (const auto& contraction : sequence) {
        out << (contraction.first + 1) << " " << (contraction.second + 1) << "\n";
    }

    std::cout << "Written " << resultPath.generic_string()
              << "\n twin-width=" << solver->getBestTwinWidth() << std::endl;
    return true;
}

int main() {

    std::vector<std::string> instancePaths = collectExactPublicInstancePaths();
    // std::vector<std::string> instancePaths = {"D:/Studia/magisterka/Kod/data/heuristic-private/instances/heuristic_001.gr"};
    for (const auto& pathStr : instancePaths) {
        std::filesystem::path path(pathStr);
        std::cout << "Processing " << path.generic_string() << std::endl;
        if (!writeGreedyResultForInstance(path)) {
            std::cerr << "Failed for instance: " << path.generic_string() << std::endl;
        }
    }
    
    // g = readGraphFromFile(path);
    // std::cout<<"\nCheck:"<<g->contractGraph(mcts.getBestContractionSequence());

    
    // std::vector<int> vertices(10);

    // std::iota(vertices.begin(), vertices.end(),0);
    // vertices = {3,4,5,6,7};
    // auto sequence = generateRandomSequence(vertices);
    // for (const auto& i : sequence){
    //     std::cout<<i.first<<" "<<i.second<<"\n";
    // }
    // std::string pathInstances = "data/test_small_data_PACE2023/instances/tiny00";
    // // std::string pathSolutions = "data/test_small_data_PACE2023/solutions/tiny00";
    // for(int i = 1;i<10;i++){
    //     GraphAL g = readGraphFromFile(pathInstances + std::to_string(i) + ".gr");
    //     // std::vector<std::pair<int,int>> s = readSequenceFromFile(pathSolutions + std::to_string(i) + ".tww");
    //     // std::cout<<g.contractGraph(s)<<"\n";
        
    // }

    return 0;
}
