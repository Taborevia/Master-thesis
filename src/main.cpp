#include "GraphAL.hpp"
#include "Algorithms.hpp"
#include <iostream>
#include <numeric>
#include <fstream>
#include <cmath>

std::shared_ptr<IGraph> readGraphFromFile(const std::string& filename) {
    std::ifstream in(filename);
    if (!in.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }

    std::string type, format;
    int n, m;

    // Oczekujemy: p tww n m
    in >> type >> format >> n >> m;

    if (type != "p" || format != "tww") {
        throw std::runtime_error("Invalid header format, expected: p tww n m");
    }

    std::shared_ptr<IGraph> graph = std::make_shared<GraphAL>(n);

    int u, v;
    for (int i = 0; i < m; i++) {
        if (!(in >> u >> v)) {
            throw std::runtime_error("Error reading edge number " + std::to_string(i));
        }

        graph->addEdge(u-1, v-1, EdgeColor::Black);
    }

    return graph;
}

std::vector<std::pair<int,int>> readSequenceFromFile(const std::string& filename) {
   std::vector<std::pair<int,int>> sequence;

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: cannot open file " << filename << "\n";
        return sequence;  // empty
    }

    int u, v;
    while (file >> u >> v) {
        sequence.emplace_back(u-1, v-1);
    }

    return sequence;
}

int main() {

    std::string path = std::string(PROJECT_ROOT) + "/data/test_small_data_PACE2023/instances/tiny00";
    for(int i = 1;i<10;i++){
        std::shared_ptr<IGraph> g = readGraphFromFile(path + std::to_string(i) + ".gr");
        MonteCarloTreeSearch mcts(g);
        mcts.findSequence(5,3);
        std::cout<<mcts.getBestTwinWidth()<<std::endl;
        for (auto const& i : mcts.getBestContractionSequence()){
            std::cout<<i.first<<" "<<i.second<<std::endl;
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
