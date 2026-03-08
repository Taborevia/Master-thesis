#include "GraphAL.hpp"
#include "ITwinWidthSolver.hpp"
#include "MonteCarloTreeSearch_v1.hpp"
#include "FileReader.hpp"
#include <iostream>
#include <numeric>
#include <cmath>

#ifndef PROJECT_ROOT
#define PROJECT_ROOT "."
#endif

int main() {

    std::string path = std::string(PROJECT_ROOT) + "/data/test_small_data_PACE2023/instances/tiny00";
    for(int i = 1;i<10;i++){
        std::shared_ptr<IGraph> g = readGraphFromFile(path + std::to_string(i) + ".gr");
        std::shared_ptr<ITwinWidthSolver> solver = std::make_shared<MonteCarloTreeSearch_v1>(g);
        solver->findSequence(5,3);
        std::cout<<solver->getBestTwinWidth()<<std::endl;
        for (auto const& contraction : solver->getBestContractionSequence()){
            std::cout<<contraction.first<<" "<<contraction.second<<std::endl;
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
