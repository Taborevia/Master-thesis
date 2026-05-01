#include "Benchmark.hpp"
#include "MonteCarloTreeSearch_v1.hpp"
#include "MonteCarloTreeSearchExact.hpp"
#include "GreedySolver.hpp"
#include <iostream>


int main() {
    // Utwórz factory function dla solvera
    auto solverFactory0 = [](std::shared_ptr<IGraph> graph) {
        return std::make_shared<MonteCarloTreeSearch_v1>(graph, 100, 10, 10, 10.0, 0);
    };
    auto solverFactory1 = [](std::shared_ptr<IGraph> graph) {
        return std::make_shared<MonteCarloTreeSearch_v1>(graph, 100, 10, 10, 10.0, 1);
    };
    auto solverFactory2 = [](std::shared_ptr<IGraph> graph) {
        return std::make_shared<MonteCarloTreeSearch_v1>(graph, 100, 10, 10, 10.0, 2);
    };
    auto solverFactoryExact = [](std::shared_ptr<IGraph> graph) {
        return std::make_shared<MonteCarloTreeSearchExact>(graph, -1, 10, 10, 10.0, 0, 2);
    };
    
    // Stwórz benchmark z factory
    Benchmark benchmark0(solverFactory0);
    Benchmark benchmark1(solverFactory1);
    Benchmark benchmark2(solverFactory2);
    Benchmark benchmarkExact(solverFactoryExact);
    
    // auto result = benchmark.runSingleGraph(std::string(PROJECT_ROOT) + "/data/test_small_data_PACE2023/instances/tiny005.gr", 10, 3, 1000, 3);
    std::vector<BenchmarkResult> results;
    // results.push_back(benchmark.runSingleGraph(std::string(PROJECT_ROOT) + "/data/exact-public/instances/exact_198.gr", 30000, 1, 1000, 2));
    // results.push_back(benchmark.runSingleGraph(std::string(PROJECT_ROOT) + "/data/exact-public/instances/exact_198.gr", 60, 1.5, 1000, 2));
    // results.push_back(benchmark.runSingleGraph(std::string(PROJECT_ROOT) + "/data/heuristic-private/instances/heuristic_001.gr", 20, 1.5, 1000, 2));
    results.push_back(benchmarkExact.runSingleGraph(std::string(PROJECT_ROOT) + "/data/test_small_data_PACE2023/instances/tiny005.gr", 3000, 1.5, 1000, 0));
    // results.push_back(benchmark1.runSingleGraph(std::string(PROJECT_ROOT) + "/data/exact-public/instances/exact_068.gr", 30, 1.5, 1000, 2));
    // results.push_back(benchmark2.runSingleGraph(std::string(PROJECT_ROOT) + "/data/exact-public/instances/exact_068.gr", 30, 1.5, 1000, 2));
    for (const auto& result : results) {
        benchmark0.printSummary({result});
    }


    // std::vector<SolverParams> params = {
    //     {60, 1, 1000},
    //     {60, 1, 1000},
    //     {60, 1, 1000},
    //     {60, 1, 1000},
    //     {60, 1.25, 1000},
    //     {60, 1.25, 1000},
    //     {60, 1.25, 1000},
    //     {60, 1.25, 1000},
    //     {60, 1.5, 1000},
    //     {60, 1.5, 1000},
    //     {60, 1.5, 1000},
    //     {60, 1.5, 1000},
    //     {60, 1.75, 1000},
    //     {60, 1.75, 1000},
    //     {60, 1.75, 1000},
    //     {60, 1.75, 1000},
    //     {60, 2.0, 1000},
    //     {60, 2.0, 1000},
    //     {60, 2.0, 1000},
    //     {60, 2.0, 1000},
    //     {60, 2.25, 1000},
    //     {60, 2.25, 1000},
    //     {60, 2.25, 1000},
    //     {60, 2.25, 1000},
    //     {60, 2.5, 1000},
    //     {60, 2.5, 1000},
    //     {60, 2.5, 1000},
    //     {60, 2.5, 1000},          
    // };

    // auto sweepResults = benchmark.runParameterSweep(std::string(PROJECT_ROOT) + "/data/exact-public/instances/exact_068.gr", params, 2, 1);
    // benchmark.printSummary(sweepResults);



    // auto result = benchmark.runSingleGraph(std::string(PROJECT_ROOT) + "/data/heuristic-private/instances/heuristic_001.gr", 60, 3, 3);
    // benchmark.printSummary({result});

    // auto result2 = benchmark.runSingleGraph(std::string(PROJECT_ROOT) + "/data/exact-public/instances/exact_002.gr", 300, 0.5, 0, 3);
    // benchmark.printSummary({result});

    // Testuj exact-public
    // auto exactResults = benchmark.testExactPublic(0.5, 3, 1);
    // benchmark.printSummary(exactResults);
    // benchmark.saveResults(exactResults, "results/exact_public_benchmark.csv");
    
    // Testuj small data
    // auto smallResults = benchmark.testSmallData(10, 0.5, 100, 10);
    // benchmark.printSummary(smallResults);
    // benchmark.saveResults(smallResults, "results/small_data_benchmark.csv");
    
    return 0;
}