#include "Benchmark.hpp"
#include "MonteCarloTreeSearch_v1.hpp"
#include "GreedySolver.hpp"

int main() {
    // Utwórz factory function dla solvera
    auto solverFactory = [](std::shared_ptr<IGraph> graph) {
        // return std::make_shared<MonteCarloTreeSearch_v1>(graph,100,100,10); // maxPairs ustawione na 1000 dla testów
        return std::make_shared<GreedySolver>(graph); // maxPairs ustawione na 1000 dla testów
        // return std::make_shared<MonteCarloTreeSearch_v1>(graph, 100, 10, 10); // maxPairs ustawione na 1000 dla testów
    };
    
    // Stwórz benchmark z factory
    Benchmark benchmark(solverFactory);
    
    // auto result = benchmark.runSingleGraph(std::string(PROJECT_ROOT) + "/data/test_small_data_PACE2023/instances/tiny005.gr", 10, 3, 1000, 3);
    std::vector<BenchmarkResult> results;
    results.push_back(benchmark.runSingleGraph(std::string(PROJECT_ROOT) + "/data/exact-public/instances/exact_198.gr", 30000, 1, 1000, 2));
    // results.push_back(benchmark.runSingleGraph(std::string(PROJECT_ROOT) + "/data/exact-public/instances/exact_068.gr", 60, 1.5, 1000, 3));
    // results.push_back(benchmark.runSingleGraph(std::string(PROJECT_ROOT) + "/data/exact-public/instances/exact_068.gr", 60, 1.75, 1000, 3));
    // results.push_back(benchmark.runSingleGraph(std::string(PROJECT_ROOT) + "/data/exact-public/instances/exact_068.gr", 60, 2, 1000, 3));
    for (const auto& result : results) {
        benchmark.printSummary({result});
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