#pragma once
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <thread>
#include <mutex>
#include <ITwinWidthSolver.hpp>
#include <IGraph.hpp>

struct BenchmarkResult {
    std::string graphFile;
    int expectedTwinWidth;
    int actualTwinWidth;
    bool passed;
    double executionTime; // in milliseconds
};

// Factory function type for creating solvers
using SolverFactory = std::function<std::shared_ptr<ITwinWidthSolver>(std::shared_ptr<IGraph>)>;

class Benchmark {
public:
    explicit Benchmark(SolverFactory solverFactory);
    ~Benchmark() = default;
    
    // Test all graphs from test_small_data_PACE2023/instances
    std::vector<BenchmarkResult> testSmallData(
        float resources = 1000.0f, 
        float c_parameter = 2.0f,
        float D_parameter = 0.0f,
        int numThreads = 1
    );
    
    // Test all graphs from exact-public/instances
    std::vector<BenchmarkResult> testExactPublic(
        float resources = 1000.0f,
        float c_parameter = 2.0f,
        float D_parameter = 0.0f,
        int numThreads = 1
    );
    
    // Run a single graph file and optionally compare to an expected twin width
    BenchmarkResult runSingleGraph(
        const std::string& graphFile,
        float resources = 1000.0f,
        float c_parameter = 2.0f,
        float D_parameter = 0.0f,
        int expectedTwinWidth = -1
    );
    
    // Print results summary
    void printSummary(const std::vector<BenchmarkResult>& results) const;
    
    // Save results to file
    void saveResults(
        const std::vector<BenchmarkResult>& results,
        const std::string& outputFile
    ) const;
    
private:
    SolverFactory solverFactory;
    mutable std::mutex resultsMutex;
    mutable std::mutex outputMutex;
    
    // Helper function to run benchmarks on given directory
    std::vector<BenchmarkResult> runBenchmark(
        const std::string& instancesDir,
        const std::string& expectedResultsFile,
        float resources,
        float c_parameter,
        float D_parameter,
        int numThreads
    );
    
    // Worker function for processing a single graph
    BenchmarkResult processGraph(
        const std::string& graphFile,
        const std::vector<int>& expectedResults,
        size_t graphIndex,
        float resources,
        float c_parameter,
        float D_parameter
    );
    
    // Read expected results from file
    std::vector<int> readExpectedResults(const std::string& filename) const;
    
    // Get all .gr files from directory
    std::vector<std::string> getGraphFiles(const std::string& directory) const;
};
