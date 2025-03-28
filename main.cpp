#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <unordered_set>
#include <random>

using namespace std;

// Structure pour stocker les données du problème
struct SetCoverProblem {
    int m; // Nombre de lignes (éléments de l'univers U)
    int n; // Nombre de colonnes (sous-ensembles)
    vector<int> costs; // Coût de chaque sous-ensemble
    vector<vector<int> > A; // Matrice de couverture binaire
};

// Fonction pour lire un fichier SCP et charger les données dans une structure
SetCoverProblem read_scp_file(const string& filename) {
    ifstream file(filename);
    if (!file) {
        cerr << "Erreur: Impossible d'ouvrir le fichier " << filename << endl;
        exit(1);
    }

    SetCoverProblem problem;

    // Lire m et n (nombre de lignes et colonnes)
    file >> problem.m >> problem.n;

    // Lire les coûts de chaque colonne
    problem.costs.resize(problem.n);
    for (int j = 0; j < problem.n; j++) {
        file >> problem.costs[j];
    }

    // Initialiser la matrice de couverture avec des 0
    problem.A.assign(problem.m, vector<int>(problem.n, 0));

    // Lire les lignes et leurs colonnes couvertes
    for (int i = 0; i < problem.m; i++) {
        int num_covering_columns;
        file >> num_covering_columns; // Nombre de colonnes couvrant la ligne i

        for (int k = 0; k < num_covering_columns; k++) {
            int col_index;
            file >> col_index;
            problem.A[i][col_index - 1] = 1; // Les indices dans le fichier commencent à 1
        }
    }

    file.close();
    return problem;
}

// Fonction pour afficher les données lues (debug)
void display_problem(const SetCoverProblem& problem) {
    cout << "Nombre de lignes (m) : " << problem.m << endl;
    cout << "Nombre de colonnes (n) : " << problem.n << endl;

    cout << "Coûts des sous-ensembles : ";
    for (int cost : problem.costs) {
        cout << cost << " ";
    }
    cout << endl;

    cout << "Matrice de couverture : " << endl;
    for (const auto& row : problem.A) {
        for (int val : row) {
            cout << val << " ";
        }
        cout << endl;
    }
}

#include <unordered_set>
#include <random>

// Génère une solution réalisable de manière naïve (aléatoire)
vector<int> generate_random_solution(const SetCoverProblem& problem) {
    vector<int> selected_columns;
    vector<bool> column_used(problem.n, false);
    unordered_set<int> uncovered_rows;

    for (int i = 0; i < problem.m; i++) {
        uncovered_rows.insert(i);
    }

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> random_col(0, problem.n - 1);

    while (!uncovered_rows.empty()) {
        int col;
        do {
            col = random_col(gen);
        } while (column_used[col]);

        selected_columns.push_back(col);
        column_used[col] = true;

        // Marquer les lignes couvertes par cette colonne
        unordered_set<int>::iterator it = uncovered_rows.begin();
        while (it != uncovered_rows.end()) {
            if (problem.A[*it][col] == 1) {
                unordered_set<int>::iterator temp = it;
                ++it;
                uncovered_rows.erase(temp);
            } else {
                ++it;
            }
        }
    }

    return selected_columns;
}


// Vérifie si une solution est réalisable (couvre toutes les lignes)
bool is_valid_solution(const SetCoverProblem& problem, const vector<int>& solution) {
    vector<bool> covered(problem.m, false);

    for (size_t i = 0; i < solution.size(); i++) {
        int col = solution[i];
        for (int j = 0; j < problem.m; j++) {
            if (problem.A[j][col] == 1) {
                covered[j] = true;
            }
        }
    }

    for (size_t i = 0; i < covered.size(); i++) {
        if (!covered[i]) {
            return false;
        }
    }
    return true;
}

vector<int> add_subset(const SetCoverProblem& problem, const vector<int>& solution) {
    vector<int> new_solution = solution;
    vector<bool> used(problem.n, false);

    for (size_t i = 0; i < solution.size(); i++) {
        used[solution[i]] = true;
    }

    for (int j = 0; j < problem.n; j++) {
        if (!used[j]) {
            new_solution.push_back(j);
            break; // ajouter un seul sous-ensemble
        }
    }

    return new_solution;
}

vector<int> remove_subset(const SetCoverProblem& problem, const vector<int>& solution) {
    for (size_t i = 0; i < solution.size(); i++) {
        vector<int> candidate_solution = solution;
        candidate_solution.erase(candidate_solution.begin() + i);
        if (is_valid_solution(problem, candidate_solution)) {
            return candidate_solution;
        }
    }

    // Aucun retrait possible sans rendre la solution invalide
    return solution;
}

vector<int> swap_subset(const SetCoverProblem& problem, const vector<int>& solution) {
    vector<bool> used(problem.n, false);
    for (size_t i = 0; i < solution.size(); i++) {
        used[solution[i]] = true;
    }

    for (size_t i = 0; i < solution.size(); i++) {
        int current = solution[i];
        for (int j = 0; j < problem.n; j++) {
            if (!used[j]) {
                vector<int> candidate_solution = solution;
                candidate_solution[i] = j;
                if (is_valid_solution(problem, candidate_solution)) {
                    return candidate_solution;
                }
            }
        }
    }

    // Aucun échange faisable
    return solution;
}

// Calcule le coût total d'une solution (somme des coûts des colonnes sélectionnées)
int compute_cost(const SetCoverProblem& problem, const vector<int>& solution) {
    int total_cost = 0;
    for (size_t i = 0; i < solution.size(); i++) {
        total_cost += problem.costs[solution[i]];
    }
    return total_cost;
}


vector<int> vns(const SetCoverProblem& problem, int max_iterations) {
    vector<int> current_solution = generate_random_solution(problem);
    int current_cost = compute_cost(problem, current_solution);

    for (int iter = 0; iter < max_iterations; iter++) {
        bool improved = false;

        // Liste des voisinages à tester dans cet ordre
        for (int k = 0; k < 3; k++) {
            vector<int> neighbor;

            if (k == 0) {
                neighbor = add_subset(problem, current_solution);
            } else if (k == 1) {
                neighbor = remove_subset(problem, current_solution);
            } else if (k == 2) {
                neighbor = swap_subset(problem, current_solution);
            }

            // Vérifier si solution est réalisable
            if (is_valid_solution(problem, neighbor)) {
                int neighbor_cost = compute_cost(problem, neighbor);

                if (neighbor_cost < current_cost) {
                    current_solution = neighbor;
                    current_cost = neighbor_cost;
                    improved = true;
                    break; // Repart de k = 0
                }
            }
        }

    }

    return current_solution;
}

#include <chrono>
#include <fstream>

void benchmark_algorithms(const SetCoverProblem& problem, const string& output_csv) {
    ofstream file(output_csv);
    file << "Méthode,Coût,Temps(s)\n";

    // Mesure Random
    auto start = chrono::high_resolution_clock::now();
    vector<int> random_solution = generate_random_solution(problem);
    auto end = chrono::high_resolution_clock::now();
    double time_random = chrono::duration<double>(end - start).count();
    int cost_random = compute_cost(problem, random_solution);
    file << "Random," << cost_random << "," << time_random << "\n";

    // LS Add
    start = chrono::high_resolution_clock::now();
    vector<int> add_solution = add_subset(problem, random_solution);
    end = chrono::high_resolution_clock::now();
    double time_add = chrono::duration<double>(end - start).count();
    int cost_add = compute_cost(problem, add_solution);
    file << "LocalSearch_Add," << cost_add << "," << time_add << "\n";

    // LS Remove
    start = chrono::high_resolution_clock::now();
    vector<int> remove_solution = remove_subset(problem, random_solution);
    end = chrono::high_resolution_clock::now();
    double time_remove = chrono::duration<double>(end - start).count();
    int cost_remove = compute_cost(problem, remove_solution);
    file << "LocalSearch_Remove," << cost_remove << "," << time_remove << "\n";

    // LS Swap
    start = chrono::high_resolution_clock::now();
    vector<int> swap_solution = swap_subset(problem, random_solution);
    end = chrono::high_resolution_clock::now();
    double time_swap = chrono::duration<double>(end - start).count();
    int cost_swap = compute_cost(problem, swap_solution);
    file << "LocalSearch_Swap," << cost_swap << "," << time_swap << "\n";

    // VNS
    start = chrono::high_resolution_clock::now();
    vector<int> vns_solution = vns(problem, 100); // Tu peux ajuster les itérations
    end = chrono::high_resolution_clock::now();
    double time_vns = chrono::duration<double>(end - start).count();
    int cost_vns = compute_cost(problem, vns_solution);
    file << "VNS," << cost_vns << "," << time_vns << "\n";

    file.close();
    cout << "✅ Résultats enregistrés dans : " << output_csv << endl;
}



int main() {
    string filename = "resources/scp41.txt"; // Remplacez par le fichier à lire
    //string filename = "resources/maison_borne.txt"; // Remplacez par le fichier à lire

    // Lire les données
    SetCoverProblem problem = read_scp_file(filename);

    // Afficher les données lues
    display_problem(problem);

    vector<int> random_solution = generate_random_solution(problem);

    cout << "Solution aléatoire générée : ";
    for (size_t i = 0; i < random_solution.size(); i++) {
        cout << random_solution[i] + 1 << " "; // Indices en base 1
    }
    cout << endl;

    cout << "Coût total : " << compute_cost(problem, random_solution) << endl;

    if (is_valid_solution(problem, random_solution)) {
        cout << "✅ La solution est valide et couvre toutes les lignes." << endl;
    } else {
        cout << "❌ La solution n'est PAS valide !" << endl;
    }

    // Tester les recherches locales
    cout << "-----------------------------" << endl;
    cout << "🔧 Test des recherches locales" << endl;

    vector<int> base_solution = generate_random_solution(problem);

    cout << "Solution de départ : ";
    for (size_t i = 0; i < base_solution.size(); i++) {
        cout << base_solution[i] + 1 << " ";
    }
    cout << endl;
    cout << "Coût total : " << compute_cost(problem, base_solution) << endl;

    vector<int> added_solution = add_subset(problem, base_solution);
    cout << "[+ Ajout] Solution après ajout : ";
    for (size_t i = 0; i < added_solution.size(); i++) {
        cout << added_solution[i] + 1 << " ";
    }
    cout << "| Valide : " << (is_valid_solution(problem, added_solution) ? "oui" : "non");
    cout << " | Coût total : " << compute_cost(problem, added_solution) << endl;

    vector<int> removed_solution = remove_subset(problem, base_solution);
    cout << "[- Suppression] Solution après retrait : ";
    for (size_t i = 0; i < removed_solution.size(); i++) {
        cout << removed_solution[i] + 1 << " ";
    }
    cout << "| Valide : " << (is_valid_solution(problem, removed_solution) ? "oui" : "non");
    cout << " | Coût total : " << compute_cost(problem, removed_solution) << endl;

    vector<int> swapped_solution = swap_subset(problem, base_solution);
    cout << "[~ Échange] Solution après échange : ";
    for (size_t i = 0; i < swapped_solution.size(); i++) {
        cout << swapped_solution[i] + 1 << " ";
    }
    cout << "| Valide : " << (is_valid_solution(problem, swapped_solution) ? "oui" : "non");
    cout << " | Coût total : " << compute_cost(problem, swapped_solution) << endl;

    cout << "-----------------------------" << endl;

    // 🔄 Lancement de la VNS
    cout << "🚀 Lancement de VNS..." << endl;
    int max_iterations = 100;
    vector<int> vns_solution = vns(problem, max_iterations);

    cout << "✅ Solution trouvée par VNS : ";
    for (size_t i = 0; i < vns_solution.size(); i++) {
        cout << vns_solution[i] + 1 << " ";
    }
    cout << endl;
    cout << "✔️ Est valide ? " << (is_valid_solution(problem, vns_solution) ? "Oui" : "Non") << endl;
    cout << "💰 Coût total : " << compute_cost(problem, vns_solution) << endl;

    benchmark_algorithms(problem, "resultats.csv");

    return 0;
}
