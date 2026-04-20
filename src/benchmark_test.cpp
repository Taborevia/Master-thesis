#include "Benchmark.hpp"
#include "MonteCarloTreeSearch_v1.hpp"
#include "GreedySolver.hpp"

int main() {
    // Utwórz factory function dla solvera
    auto solverFactory = [](std::shared_ptr<IGraph> graph) {
        return std::make_shared<MonteCarloTreeSearch_v1>(graph); // maxPairs ustawione na 1000 dla testów
    };
    
    // Stwórz benchmark z factory
    Benchmark benchmark(solverFactory);
    
    auto result = benchmark.runSingleGraph(std::string(PROJECT_ROOT) + "/data/test_small_data_PACE2023/instances/tiny005.gr", 10, 3, 1000, 3);
    // auto result = benchmark.runSingleGraph(std::string(PROJECT_ROOT) + "/data/exact-public/instances/exact_002.gr", 10, 3, 1000, 3);
    // auto result = benchmark.runSingleGraph(std::string(PROJECT_ROOT) + "/data/heuristic-private/instances/heuristic_001.gr", 10, 3, 3);
    // benchmark.printSummary({result});

    // Testuj exact-public
    // auto exactResults = benchmark.testExactPublic(0.5, 3, 1);
    // benchmark.printSummary(exactResults);
    // benchmark.saveResults(exactResults, "results/exact_public_benchmark.csv");
    
    // Testuj small data
    // auto smallResults = benchmark.testSmallData(30, 3, 100, 10);
    // benchmark.printSummary(smallResults);
    // benchmark.saveResults(smallResults, "results/small_data_benchmark.csv");
    
    return 0;
}