#include <iostream>
#include <memory>
#include "MDAdapter.hpp"

int main(int argc, char* argv[]) {
    std::cout << "=== MDAdapter Test Program ===" << std::endl;

    try {
        // Sprawdzenie czy podano nazwę pliku
        if (argc < 2) {
            std::cerr << "Usage: " << argv[0] << " <graph_file.gr>" << std::endl;
            std::cerr << "Example: " << argv[0] << " ../data/test_small_data_PACE2023/instances/tiny009.gr" << std::endl;
            return 1;
        }

        std::string filename = argv[1];
        std::cout << "Loading graph from: " << filename << std::endl;

        // Tworzymy MDAdapter
        MDAdapter adapter(filename);

        // Pobieramy MDTree
        modular::MDTree mdtree = adapter.getMDTree();

        // Wypisujemy strukturę drzewa dekompozycji
        std::cout << mdtree.to_string() << std::endl;

        adapter.findSequence(10,1.5,1000);
        std::cout << "Best Twin Width: " << adapter.getBestTwinWidth() << std::endl;
        // std::cout << "Best Contraction Sequence: ";
        // for (const auto& pair : adapter.getBestContractionSequence()) {
        //     std::cout << "(" << pair.first << ", " << pair.second << ") ";
        // }

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;
    }
}
