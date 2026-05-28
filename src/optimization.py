import subprocess
import os
import optuna

CPP_EXECUTABLE = "./twin_width_solver.exe"

TEST_GRAPHS = ["../data/exact-public/instances/exact_008.gr", "../data/exact-public/instances/exact_012.gr", "../data/exact-public/instances/exact_030.gr"]
GRAPHS_scale = [10, 8, 3]

def objective(trial):
    # 1. Definiujemy hiperparametry, które Optuna ma dobrać
    c_param = trial.suggest_float("c_param", 0.1, 10.0)
    # alpha_param = trial.suggest_float("alpha_param", 0.0, 1.0)
    # Przykład parametru całkowitoliczbowego (np. głębokość rolloutu lub próg)
    # threshold = trial.suggest_int("threshold", 10, 100)
    
    total_score = 0.0
    
    # 2. Testujemy ten zestaw parametrów na zdefiniowanych grafach
    for graph_path, scale in zip(TEST_GRAPHS, GRAPHS_scale):
        # Budujemy polecenie systemowe, przekazując parametry do C++
        # Przykład: ./sp_mcts --graph graphs/test_1.txt --c 1.41 --alpha 0.2 --thresh 50
        command = [
            CPP_EXECUTABLE,
            "--file", graph_path,
            "--param_c", str(c_param),
            "--resources", "20"
        ]
        graph_score = 0.0
        for i in range(1, 6):
            try:
                # Uruchomienie programu C++ i przechwycenie stdout
                result = subprocess.run(
                    command,
                    capture_output=True,
                    text=True,
                    check=True,  # Rzuci wyjątek, jeśli C++ zwróci kod błędu (np. crash)
                    timeout=30   # Bezpiecznik: zabije proces Pythona, jeśli C++ się zawiesi
                )
                
                # Pobieramy to, co C++ wypisał na cout
                output = result.stdout.strip()
                
                # Parsowanie wyniku. Jeśli wypisujesz samą liczbę, wystarczy float(output).
                # Jeśli wypisujesz coś więcej, musisz wyciągnąć samą wartość, np.:
                # if "RESULT:" in output: score = float(output.split(":")[-1])
                score = float(output)/scale
                
                graph_score += score
                
            except subprocess.TimeoutExpired:
                print(f"Próba przerwana: Program C++ przekroczył czas dla grafu {graph_path}")
                return float('inf')  # Kara dla Optuny za zawieszenie algorytmu
            
            except (subprocess.CalledProcessError, ValueError) as e:
                print(f"Błąd wykonania C++ lub błąd parsowania: {e}")
                return float('inf')  # Kara za błędy/crashe
                
        total_score += graph_score / 5  # Średni wynik dla tego grafu (5 prób)
            
    # Zwracamy średni wynik z testowanych grafów (Optuna dąży do jego minimalizacji)
    return total_score / len(TEST_GRAPHS)

if __name__ == "__main__":
    # Sprawdzenie, czy plik C++ istnieje przed uruchomieniem
    if not os.path.exists(CPP_EXECUTABLE):
        print(f"Błąd: Nie znaleziono pliku wykonywalnego: {CPP_EXECUTABLE}")
        exit(1)

    # Tworzymy badanie nastawione na minimalizację wyniku (kierunek: minimize)
    study = optuna.create_study(
        study_name="mcts_twin_width_optimization",
        direction="minimize",
        sampler=optuna.samplers.TPESampler()  # Optymalizacja Bayesowska
    )
    
    # Uruchomienie szukania - np. 40 prób (trials)
    print("Rozpoczynam optymalizację hiperparametrów...")
    study.optimize(objective, n_trials=80)
    
    # Wyświetlenie najlepszych rezultatów
    print("\n=== OPTYMALIZACJA ZAKOŃCZONA ===")
    print(f"Najlepszy średni score: {study.best_value}")
    print("Najlepsze parametry:")
    for key, value in study.best_params.items():
        print(f"  {key}: {value}")