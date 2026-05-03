#include "FileReader.hpp"
#include <GraphAL.hpp>
#include <GraphAM.hpp>
#include <fstream>
#include <iostream>
#include <stdexcept>

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
