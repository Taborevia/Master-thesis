#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <chrono>

#include "FileReader.hpp"
#include "GraphAL.hpp"
#include "MonteCarloTreeSearch_v1.hpp"

// Atrapa funkcji rozwiazujacej problem twin-width
// Podmien ja na swoja wlasciwa implementacje
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
    auto graph = readGraphFromFile(filename);
    // MonteCarloTreeSearch_v1(std::shared_ptr<IGraph> graph, int maxPairs = -1, uint32_t greedySimulationPairs = 0, uint32_t greedySimulationDepth = 0, double PTW_coeff = 0.0, int32_t dynamicTimeDistribution = 0, int32_t numOfBestPairs = 0);
    MonteCarloTreeSearch_v1 solver(graph, pairs_count, greedy_simulation_pairs, sim_depth, ptw_coeff, dynamic_time_dist, use_ranking ? 10 : 0, std::chrono::high_resolution_clock::now().time_since_epoch().count());   
    solver.findSequence(resources, param_C, param_D);
    return solver.getBestTwinWidth(); 
}

void print_usage(const char* prog_name) {
        std::cerr << "Sposob uzycia: " << prog_name << " [OPCJE]\n"
                            << "Opcje wymagane:\n"
                        << "  --file <string>        Nazwa pliku z grafem\n"
                        << "Opcje algorytmu (z domyslnymi wartosciami):\n"
                        << "  --param_c <float>      Parametr C (domyslnie: 1.5)\n"
                        << "  --param_d <float>      Parametr D (domyslnie: 0)\n"
                        << "  --resources <float>    Czas dzialania w sekundach (domyslnie: 60)\n"
                        << "  --time_dist <int>      Dynamic time distribution (domyslnie: 0 - brak)\n"
                        << "  --ranking              Wlacz rankingowe wybieranie wierzcholkow\n"
                        << "  --greedy <int>         Wlacz greedy simulations i podaj liczbe par\n"
                        << "  --pairs <int>          Liczba par do oceny (domyslnie: wszystkie)\n"
                        << "  --ptw_coeff <float>    Wspolczynnik PTW (domyslnie: 0)\n"
                        << "  --depth <int>          Simulation depth (domyslnie: inf)\n"
                        << "  --matix                Wlacz reprezentacje macierzowa grafu\n";
}

int main(int argc, char* argv[]) {
    // 1. Definiowanie domyślnych wartości parametrów
    std::string filename = "";
    float param_C = 1.5f;
    float param_D = 0;
    float resources = 60.0f; // Domyslny czas dzialania w sekundach
    int dynamic_time_dist = 0;
    bool use_ranking = false;      // Flagi domyslnie wylaczone
    bool use_greedy = false;
    int pairs_count = 0;
    float ptw_coeff = 0.0f;
    int sim_depth = 0;
    int greedy_simulation_pairs = 0;
    bool matrix_representation = false;

    // 2. Parsowanie argumentów linii poleceń
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "--file" && i + 1 < argc) {
            filename = argv[++i];
        } else if (arg == "--param_c" && i + 1 < argc) {
            param_C = std::stof(argv[++i]);
        } else if (arg == "--param_d" && i + 1 < argc) {
            param_D = std::stof(argv[++i]);
        } else if (arg == "--resources" && i + 1 < argc) {
            resources = std::stof(argv[++i]);
        } else if (arg == "--time_dist" && i + 1 < argc) {
            dynamic_time_dist = std::stoi(argv[++i]);
        } else if (arg == "--ranking") {
            use_ranking = true; // Flaga bezwartościowa (obecność = true)
        } else if (arg == "--greedy" && i + 1 < argc) {
            use_greedy = true;
            greedy_simulation_pairs = std::stoi(argv[++i]);  
        } else if (arg == "--pairs" && i + 1 < argc) {
            pairs_count = std::stoi(argv[++i]);
        } else if (arg == "--ptw_coeff" && i + 1 < argc) {
            ptw_coeff = std::stof(argv[++i]);
        } else if (arg == "--depth" && i + 1 < argc) {
            sim_depth = std::stoi(argv[++i]);
        } else if (arg == "--help" || arg == "-h") {
            print_usage(argv[0]);
            return 0;
        } else if (arg == "--matrix") {
            matrix_representation = true; // Flaga bezwartościowa (obecność = true)
        } else {
            std::cerr << "Nieznany lub niekompletny argument: " << arg << std::endl;
            print_usage(argv[0]);
            return 1;
        }
    }

    // 3. Walidacja wymaganych parametrów
    if (filename.empty()) {
        std::cerr << "Blad: Nie podano pliku z grafem za pomoca `--file`!" << std::endl;
        print_usage(argv[0]);
        return 1;
    }

    // 4. Uruchomienie algorytmu

    std::streambuf* old_cout_buffer = std::cout.rdbuf();
    std::cout.rdbuf(std::cerr.rdbuf()); 
    // std::cout.rdbuf(nullptr); // Przekierowanie cout do null (wyciszenie) - przydatne, gdy algorytm sam wypisuje dużo informacji diagnostycznych, a my chcemy tylko wynik końcowy.; 
    
    int result_twin_width = solve_twin_width(
        filename, param_C, param_D, resources, dynamic_time_dist, 
        use_ranking, pairs_count, ptw_coeff,
        sim_depth, greedy_simulation_pairs, matrix_representation
    );

    std::cout.rdbuf(old_cout_buffer);

    // 5. Wypisanie TYLKO wyniku koncowego na standardowe wyjscie
    std::cout <<"\n"<< result_twin_width << std::endl;

    return 0;
}