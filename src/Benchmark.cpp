#include <Benchmark.hpp>
#include <FileReader.hpp>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <algorithm>
#include <iomanip>
#include <limits>
#include <cmath>

#ifndef PROJECT_ROOT
#define PROJECT_ROOT "."
#endif

namespace fs = std::filesystem;

Benchmark::Benchmark(SolverFactory solverFactory)
    : solverFactory(solverFactory) {
    if (!solverFactory) {
        throw std::invalid_argument("Solver factory cannot be null");
    }
}

std::vector<std::string> Benchmark::getGraphFiles(const std::string& directory) const {
    std::vector<std::string> graphFiles;
    
    if (!fs::exists(directory)) {
        std::cerr << "Directory does not exist: " << directory << std::endl;
        return graphFiles;
    }
    
    std::vector<std::string> tempFiles;
    for (const auto& entry : fs::directory_iterator(directory)) {
        if (entry.is_regular_file() && entry.path().extension() == ".gr") {
            tempFiles.push_back(entry.path().string());
        }
    }
    
    // Sort files to ensure consistent ordering
    std::sort(tempFiles.begin(), tempFiles.end());
    graphFiles = tempFiles;
    
    return graphFiles;
}

std::vector<int> Benchmark::readExpectedResults(const std::string& filename) const {
    std::vector<int> results;
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "Cannot open results file: " << filename << std::endl;
        return results;
    }
    
    int value;
    while (file >> value) {
        results.push_back(value);
    }
    
    file.close();
    return results;
}

std::vector<BenchmarkResult> Benchmark::runBenchmark(
    const std::string& instancesDir,
    const std::string& expectedResultsFile,
    float resources,
    float c_parameter,
    int numThreads) {
    
    std::vector<BenchmarkResult> results;
    results.resize(0);
    
    // Get all graph files
    auto graphFiles = getGraphFiles(instancesDir);
    
    // Read expected results
    auto expectedResults = readExpectedResults(expectedResultsFile);
    
    if (graphFiles.empty()) {
        std::cerr << "No graph files found in: " << instancesDir << std::endl;
        return results;
    }
    
    // Limit threads to reasonable number
    if (numThreads <= 0) numThreads = 1;
    if (numThreads > (int)graphFiles.size()) numThreads = graphFiles.size();
    
    std::cout << "Testing " << graphFiles.size() << " graphs with " << numThreads << " thread(s)..." << std::endl;
    std::cout << "Expected results available: " << expectedResults.size() << std::endl;
    
    // Create result vector with proper size
    results.resize(graphFiles.size());
    
    // If single threaded, use simple loop
    if (numThreads == 1) {
        for (size_t i = 0; i < graphFiles.size(); ++i) {
            results[i] = processGraph(graphFiles[i], expectedResults, i, resources, c_parameter);
        }
    } else {
        // Multi-threaded processing
        std::vector<std::thread> threads;
        
        for (int t = 0; t < numThreads; ++t) {
            threads.emplace_back([this, &graphFiles, &expectedResults, &results, resources, c_parameter, t, numThreads]() {
                // Each thread processes every numThreads-th file
                for (size_t i = t; i < graphFiles.size(); i += numThreads) {
                    results[i] = processGraph(graphFiles[i], expectedResults, i, resources, c_parameter);
                }
            });
        }
        
        // Wait for all threads to complete
        for (auto& thread : threads) {
            thread.join();
        }
    }
    
    return results;
}

std::vector<BenchmarkResult> Benchmark::testSmallData(float resources, float c_parameter, int numThreads) {
    std::cout << "\n========== Testing Small Data PACE2023 ==========" << std::endl;
    return runBenchmark(
        std::string(PROJECT_ROOT) + "/data/test_small_data_PACE2023/instances",
        std::string(PROJECT_ROOT) + "/data/test_small_data_PACE2023/small_data_results.txt",
        resources,
        c_parameter,
        numThreads
    );
}

std::vector<BenchmarkResult> Benchmark::testExactPublic(float resources, float c_parameter, int numThreads) {
    std::cout << "\n========== Testing Exact Public ==========" << std::endl;
    return runBenchmark(
        std::string(PROJECT_ROOT) + "/data/exact-public/instances",
        std::string(PROJECT_ROOT) + "/data/exact-public/exact_public_results.txt",
        resources,
        c_parameter,
        numThreads
    );
}

BenchmarkResult Benchmark::processGraph(
    const std::string& graphFile,
    const std::vector<int>& expectedResults,
    size_t graphIndex,
    float resources,
    float c_parameter) {
    
    BenchmarkResult result;
    result.graphFile = fs::path(graphFile).filename().string();
    
    {
        std::lock_guard<std::mutex> lock(outputMutex);
        std::cout << "Processing: " << result.graphFile;
    }
    
    try {
        // Read graph
        auto graph = readGraphFromFile(graphFile);
        
        // Create new solver for this graph
        auto solver = solverFactory(graph);
        
        // Record execution time
        auto startTime = std::chrono::high_resolution_clock::now();
        
        // Run solver
        solver->findSequence(resources, c_parameter);
        
        auto endTime = std::chrono::high_resolution_clock::now();
        result.executionTime = std::chrono::duration<double, std::milli>(
            endTime - startTime
        ).count();
        
        // Get results from solver
        result.actualTwinWidth = solver->getBestTwinWidth();
        
        // Compare with expected result if available
        if (graphIndex < expectedResults.size()) {
            result.expectedTwinWidth = expectedResults[graphIndex];
            result.passed = (result.actualTwinWidth == result.expectedTwinWidth);
        } else {
            result.expectedTwinWidth = -1;
            result.passed = false;
        }
        
        // Print result with thread-safe output
        {
            std::lock_guard<std::mutex> lock(outputMutex);
            std::cout << " -> Twin Width: " << result.actualTwinWidth;
            if (result.expectedTwinWidth != -1) {
                std::cout << " (expected: " << result.expectedTwinWidth << ")";
                if (result.passed) {
                    std::cout << " ✓";
                } else {
                    std::cout << " ✗";
                }
            }
            std::cout << " (" << std::fixed << std::setprecision(2) 
                      << result.executionTime << " ms)" << std::endl;
        }
        
    } catch (const std::exception& e) {
        {
            std::lock_guard<std::mutex> lock(outputMutex);
            std::cerr << " ERROR: " << e.what() << std::endl;
        }
        result.actualTwinWidth = -1;
        result.expectedTwinWidth = (graphIndex < expectedResults.size()) ? expectedResults[graphIndex] : -1;
        result.passed = false;
        result.executionTime = -1.0;
    }
    
    return result;
}

void Benchmark::printSummary(const std::vector<BenchmarkResult>& results) const {
    if (results.empty()) {
        std::cout << "No results to summarize." << std::endl;
        return;
    }
    
    int totalTests = results.size();
    int passedTests = 0;
    int failedTests = 0;
    int errorTests = 0;
    double totalTime = 0.0;
    double minTime = std::numeric_limits<double>::max();
    double maxTime = 0.0;
    std::vector<double> executionTimes;
    
    double sumDiff = 0.0;
    double sumPercentDiff = 0.0;
    int diffCount = 0;
    int diffCountWithValidExpected = 0;  // Count for percentage calculation (expectedTwinWidth > 0)
    double minDiff = std::numeric_limits<double>::max();
    double maxDiff = 0.0;
    double minPercentDiff = std::numeric_limits<double>::max();
    double maxPercentDiff = 0.0;
    
    for (const auto& result : results) {
        if (result.actualTwinWidth == -1) {
            errorTests++;
        } else if (result.passed) {
            passedTests++;
        } else if (result.expectedTwinWidth != -1) {
            failedTests++;
            double diff = std::abs(result.actualTwinWidth - result.expectedTwinWidth);
            sumDiff += diff;
            diffCount++;
            minDiff = std::min(minDiff, diff);
            maxDiff = std::max(maxDiff, diff);
            
            // Calculate percentage difference
            if (result.expectedTwinWidth > 0) {
                double percentDiff = (diff / result.expectedTwinWidth) * 100.0;
                sumPercentDiff += percentDiff;
                diffCountWithValidExpected++;
                minPercentDiff = std::min(minPercentDiff, percentDiff);
                maxPercentDiff = std::max(maxPercentDiff, percentDiff);
            } else if (result.expectedTwinWidth == 0 && result.actualTwinWidth != 0) {
                // Special case: expected 0 but got something else = 100% error
                sumPercentDiff += 100.0;
                diffCountWithValidExpected++;
                minPercentDiff = std::min(minPercentDiff, 100.0);
                maxPercentDiff = std::max(maxPercentDiff, 100.0);
            }
        }
        
        if (result.executionTime > 0) {
            totalTime += result.executionTime;
            minTime = std::min(minTime, result.executionTime);
            maxTime = std::max(maxTime, result.executionTime);
            executionTimes.push_back(result.executionTime);
        }
    }
    
    // Calculate median execution time
    double medianTime = 0.0;
    if (!executionTimes.empty()) {
        std::sort(executionTimes.begin(), executionTimes.end());
        size_t mid = executionTimes.size() / 2;
        if (executionTimes.size() % 2 == 0) {
            medianTime = (executionTimes[mid - 1] + executionTimes[mid]) / 2.0;
        } else {
            medianTime = executionTimes[mid];
        }
    }
    
    double avgTime = (totalTime > 0 && !executionTimes.empty()) ? totalTime / executionTimes.size() : 0.0;
    double avgDiff = (diffCount > 0) ? sumDiff / diffCount : 0.0;
    double avgPercentDiff = (diffCountWithValidExpected > 0) ? sumPercentDiff / diffCountWithValidExpected : 0.0;
    
    if (minPercentDiff == std::numeric_limits<double>::max()) {
        minPercentDiff = 0.0;
    }
    
    std::cout << "\n========== Benchmark Summary ==========" << std::endl;
    std::cout << "Total tests: " << totalTests << std::endl;
    
    // Accuracy metrics
    std::cout << "\n--- Accuracy Metrics ---" << std::endl;
    std::cout << "Passed tests: " << passedTests << " / " << totalTests;
    if (totalTests > 0) {
        std::cout << " (" << std::fixed << std::setprecision(1) 
                  << (passedTests * 100.0 / totalTests) << "%)";
    }
    std::cout << std::endl;
    
    std::cout << "Failed tests: " << failedTests << " / " << totalTests;
    if (totalTests > 0) {
        std::cout << " (" << std::fixed << std::setprecision(1) 
                  << (failedTests * 100.0 / totalTests) << "%)";
    }
    std::cout << std::endl;
    
    std::cout << "Error tests: " << errorTests << " / " << totalTests;
    if (totalTests > 0) {
        std::cout << " (" << std::fixed << std::setprecision(1) 
                  << (errorTests * 100.0 / totalTests) << "%)";
    }
    std::cout << std::endl;
    std::cout << "Average percentage difference: " << std::fixed << std::setprecision(2) << avgPercentDiff << "%" << std::endl;
    std::cout << "Min difference: " << std::fixed << std::setprecision(0) << minDiff << std::endl;
    std::cout << "Max difference: " << std::fixed << std::setprecision(0) << maxDiff << std::endl;
    std::cout << "Min percentage difference: " << std::fixed << std::setprecision(2) << minPercentDiff << "%" << std::endl;
    std::cout << "Max percentage difference: " << std::fixed << std::setprecision(2) << maxPercentDiff << "%";
    if (diffCount > 0) {
        std::cout << "\n--- Twin Width Difference Statistics ---" << std::endl;
        std::cout << "Average difference: " << std::fixed << std::setprecision(2) << avgDiff << std::endl;
        std::cout << "Min difference: " << std::fixed << std::setprecision(0) << minDiff << std::endl;
        std::cout << "Max difference: " << std::fixed << std::setprecision(0) << maxDiff << std::endl;
    }
    
    // Timing statistics
    if (!executionTimes.empty()) {
        std::cout << "\n--- Execution Time Statistics ---" << std::endl;
        std::cout << "Total time: " << std::fixed << std::setprecision(2) 
                  << totalTime << " ms" << std::endl;
        std::cout << "Average time per test: " << std::fixed << std::setprecision(2) 
                  << avgTime << " ms" << std::endl;
        std::cout << "Median time per test: " << std::fixed << std::setprecision(2) 
                  << medianTime << " ms" << std::endl;
        std::cout << "Min time: " << std::fixed << std::setprecision(2) 
                  << minTime << " ms" << std::endl;
        std::cout << "Max time: " << std::fixed << std::setprecision(2) 
                  << maxTime << " ms" << std::endl;
    }
    
    std::cout << "======================================" << std::endl;
}

void Benchmark::saveResults(
    const std::vector<BenchmarkResult>& results,
    const std::string& outputFile) const {
    
    std::ofstream file(outputFile);
    if (!file.is_open()) {
        std::cerr << "Cannot open output file: " << outputFile << std::endl;
        return;
    }
    
    file << "Graph File,Expected Twin Width,Actual Twin Width,Passed,Execution Time (ms)" << std::endl;
    
    for (const auto& result : results) {
        file << result.graphFile << ",";
        file << (result.expectedTwinWidth >= 0 ? std::to_string(result.expectedTwinWidth) : "N/A") << ",";
        file << (result.actualTwinWidth >= 0 ? std::to_string(result.actualTwinWidth) : "ERROR") << ",";
        file << (result.passed ? "YES" : "NO") << ",";
        file << std::fixed << std::setprecision(2) << result.executionTime << std::endl;
    }
    
    file.close();
    std::cout << "Results saved to: " << outputFile << std::endl;
}
