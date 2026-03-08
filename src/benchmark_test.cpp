#include "Benchmark.hpp"
#include "MonteCarloTreeSearch_v1.hpp"

int main() {
    // Utwórz factory function dla solvera
    auto solverFactory = [](std::shared_ptr<IGraph> graph) {
        return std::make_shared<MonteCarloTreeSearch_v1>(graph);
    };
    
    // Stwórz benchmark z factory
    Benchmark benchmark(solverFactory);
    
    // Testuj exact-public
    auto exactResults = benchmark.testExactPublic(10, 3, 10);
    benchmark.printSummary(exactResults);
    benchmark.saveResults(exactResults, "results/exact_public_benchmark.csv");
    
    // // Testuj small data
    // auto smallResults = benchmark.testSmallData(60, 3, 10);
    // benchmark.printSummary(smallResults);
    // benchmark.saveResults(smallResults, "results/small_data_benchmark.csv");
    
    return 0;
}