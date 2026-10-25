#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <chrono>
#include <cmath>

#include "FileReader.hpp"
#include "GraphAL.hpp"
#include "MonteCarloTreeSearch.hpp"
#include "GreedySolver.hpp"
#include "MDAdapter.hpp"
#include <functional>

int solve_greedy_twin_width(const std::string& filename, bool matrix_representation) {
    auto graph = readGraphFromFile(filename, matrix_representation);
    GreedySolver solver(graph);    
    solver.findSequence(0,0);
    return solver.getBestTwinWidth(); 
}

int solve_twin_width(const std::string& filename, 
                     float param_C, 
                     float param_D, 
                     float resources,
                     int dynamic_time_dist, 
                     bool use_ranking, 
                     int pairs_count, 
                     float ptw_coeff, 
                     int sim_depth,
                     int greedy_simulation_pairs,
                     bool matrix_representation) 
{
    auto graph = readGraphFromFile(filename, matrix_representation);
    MonteCarloTreeSearch solver(graph, pairs_count, greedy_simulation_pairs, sim_depth, ptw_coeff, dynamic_time_dist, use_ranking ? pairs_count : 0, std::chrono::high_resolution_clock::now().time_since_epoch().count());   
    solver.findSequence(resources, param_C, param_D);
    return solver.getBestTwinWidth(); 
}

void print_usage(const char* prog_name) {
        std::cerr << "Sposob uzycia:\n"
                        << "Opcje wymagane:\n"
                        << "  --file <string>        Nazwa pliku z grafem\n"
                        << "Opcje algorytmu MCTS:\n"
                        << "  --greedy_solver        Wlacz greedy solver (domyslnie: MCTS)\n"
                        << "  --param_c <float>      Parametr C (domyslnie: 0.025)\n"
                        << "  --param_d <float>      Parametr D (domyslnie: 1)\n"
                        << "  --param_e <float>      Parametr E (domyslnie: 0.5)\n"
                        << "  --resources <float>    Czas dzialania w sekundach (domyslnie: 60)\n"
                        << "  --time_dist <int>      Dynamic time distribution, parametr p (domyslnie: 1 - liniowy)\n"
                        << "  --ranking              Wlacz rankingowe wybieranie wierzcholkow\n"
                        << "  --greedy [<int>]       Wlacz greedy simulations i opcjonalnie podaj liczbe par (domyslnie: floor(sqrt(|V|)))\n"
                        << "  --pairs <int>          Liczba par do oceny, 0 = bez limitu (domyslnie: |V|, dla wlaczonego ranking floor(sqrt(|V|)))\n"
                        << "  --depth <int>          Simulation depth (domyslnie: floor(sqrt(|V|)))\n"
                        << "  --modular              Wlacz modularna dekompozycje\n"
                        << "  --matrix               Wlacz reprezentacje macierzowa grafu\n";
}

int main(int argc, char* argv[]) {
    std::string filename = "";
    bool greedy_solver = false;
    float param_C = 0.025f;
    float param_D = 1;
    float resources = 60.0f;
    int dynamic_time_dist = 1;
    bool use_ranking = false;
    bool use_greedy = false;
    int pairs_count = -1;
    float ptw_coeff = 0.5f;
    int sim_depth = -1;                // default to |V|
    int greedy_simulation_pairs = -1;  // default to |V|
    bool matrix_representation = false;
    bool use_modular = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "--file" && i + 1 < argc) {
            filename = argv[++i];
        } else if (arg == "--greedy_solver") {
            greedy_solver = true;
        } else if (arg == "--param_c" && i + 1 < argc) {
            param_C = std::stof(argv[++i]);
        } else if (arg == "--param_d" && i + 1 < argc) {
            param_D = std::stof(argv[++i]);
        } else if (arg == "--resources" && i + 1 < argc) {
            resources = std::stof(argv[++i]);
        } else if (arg == "--time_dist" && i + 1 < argc) {
            dynamic_time_dist = std::stoi(argv[++i]);
        } else if (arg == "--ranking") {
            use_ranking = true;
        } else if (arg == "--greedy") {
            use_greedy = true;
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                greedy_simulation_pairs = std::stoi(argv[++i]);
            }
        } else if (arg == "--pairs" && i + 1 < argc) {
            pairs_count = std::stoi(argv[++i]);
        } else if (arg == "--param_e" && i + 1 < argc) {
            ptw_coeff = std::stof(argv[++i]);
        } else if (arg == "--depth" && i + 1 < argc) {
            sim_depth = std::stoi(argv[++i]);
        } else if (arg == "--help" || arg == "-h") {
            print_usage(argv[0]);
            return 0;
        } else if (arg == "--matrix") {
            matrix_representation = true;
        } else if (arg == "--modular") {
            use_modular = true;
        } else {
            std::cerr << "Nieznany lub niekompletny argument: " << arg << std::endl;
            print_usage(argv[0]);
            return 1;
        }
    }

    if (filename.empty()) {
        std::cerr << "Blad: Nie podano pliku z grafem za pomoca `--file`!" << std::endl;
        print_usage(argv[0]);
        return 1;
    }

    auto graph = readGraphFromFile(filename, matrix_representation);
    int n = graph->getNumberOfVertices();
    int sqrt_n = static_cast<int>(std::sqrt(n));
    if (use_greedy){
        if (greedy_simulation_pairs < 0) {
            greedy_simulation_pairs = sqrt_n;
        }
        if (sim_depth < 0) {
            sim_depth = sqrt_n;
        }
    } else {
        greedy_simulation_pairs = 0;
        sim_depth = 0;
    }
    if (pairs_count < 0) {
        if (use_ranking) {
            pairs_count = sqrt_n;
        } else {
            pairs_count = n;
        }
    }

    std::streambuf* old_cout_buffer = std::cout.rdbuf();
    std::cout.rdbuf(std::cerr.rdbuf()); 
    std::cout.rdbuf(nullptr); // silence output to stdout
    int result_twin_width = -1;
    if (use_modular) {
        // factory for prime graphs
        auto factory = [pairs_count, greedy_simulation_pairs, sim_depth, ptw_coeff, dynamic_time_dist, use_ranking, greedy_solver](std::shared_ptr<IGraph> g) -> std::unique_ptr<ITwinWidthSolver> {
            if (greedy_solver) {
                return std::make_unique<GreedySolver>(g);
            }
            unsigned int seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
            return std::make_unique<MonteCarloTreeSearch>(g, pairs_count, greedy_simulation_pairs, sim_depth, ptw_coeff, dynamic_time_dist, use_ranking ? pairs_count : 0, seed);
        };

        MDAdapter adapter(filename, factory);
        adapter.findSequence(resources, param_C, param_D);
        result_twin_width = adapter.getBestTwinWidth();

    } else if (greedy_solver) {
        result_twin_width = solve_greedy_twin_width(filename, matrix_representation);
    } else {
        result_twin_width = solve_twin_width(
            filename, param_C, param_D, resources, dynamic_time_dist, 
            use_ranking, pairs_count, ptw_coeff,
         sim_depth, greedy_simulation_pairs, matrix_representation
        );
    }

    std::cout.rdbuf(old_cout_buffer);

    std::cout <<"\n"<< result_twin_width << std::endl;

    return 0;
}