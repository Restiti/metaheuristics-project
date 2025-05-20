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

vector<int> generate_better_random_solution(const SetCoverProblem& problem) {
    vector<int> selected_columns;
    unordered_set<int> uncovered_rows;
    for (int i = 0; i < problem.m; i++) {
        uncovered_rows.insert(i);
    }

    vector<unordered_set<int>> column_coverage(problem.n);
    for (int j = 0; j < problem.n; j++) {
        for (int i = 0; i < problem.m; i++) {
            if (problem.A[i][j] == 1) {
                column_coverage[j].insert(i);
            }
        }
    }

    while (!uncovered_rows.empty()) {
        int best_col = -1;
        size_t max_covered = 0;

        for (int j = 0; j < problem.n; j++) {
            size_t covered = 0;
            for (int row : column_coverage[j]) {
                if (uncovered_rows.count(row)) {
                    covered++;
                }
            }

            if (covered > max_covered) {
                max_covered = covered;
                best_col = j;
            }
        }

        if (best_col == -1) {
            cerr << "❌ Erreur : aucune colonne ne couvre les lignes restantes." << endl;
            break;
        }

        selected_columns.push_back(best_col);
        for (int row : column_coverage[best_col]) {
            uncovered_rows.erase(row);
        }
    }

    return selected_columns;
}

vector<int> generate_cost_coverage_greedy(const SetCoverProblem& P) {
    unordered_set<int> uncovered;
    for(int i=0; i<P.m; i++) uncovered.insert(i);

    vector<bool> used(P.n,false);
    vector<unordered_set<int>> cov(P.n);
    for(int j=0; j<P.n; j++)
        for(int i=0; i<P.m; i++)
            if(P.A[i][j]) cov[j].insert(i);

    vector<int> solution;
    while(!uncovered.empty()) {
        int best = -1;
        double bestRatio = numeric_limits<double>::infinity();

        for(int j=0; j<P.n; j++) if(!used[j]) {
            // nombre de nouvelles lignes couvertes
            int newCov = 0;
            for(int i: cov[j]) if(uncovered.count(i)) newCov++;
            if(newCov==0) continue;
            double ratio = double(P.costs[j]) / newCov;
            if(ratio < bestRatio) {
                bestRatio = ratio;
                best = j;
            }
        }

        if(best<0) {
            cerr<<"Échec de couverture\n";
            break;
        }

        // sélection et mise à jour
        solution.push_back(best);
        used[best] = true;
        for(int i: cov[best]) uncovered.erase(i);
    }
    return solution;
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

vector<int> shake_remove_k(const SetCoverProblem& P, const vector<int>& S, int k) {
    vector<int> Ssh = S;
    int sz = (int)Ssh.size();
    if (sz <= k) {
        // Si k >= taille, on enlève tout sauf un pour garder solution valide
        Ssh.resize(max(1, sz - 1));
        return Ssh;
    }
    vector<int> idx(sz);
    iota(idx.begin(), idx.end(), 0);
    random_device rd;
    mt19937 gen(rd());
    shuffle(idx.begin(), idx.end(), gen);
    unordered_set<int> to_remove(idx.begin(), idx.begin() + k);
    vector<int> result;
    result.reserve(sz - k);
    for (int i = 0; i < sz; i++) {
        if (!to_remove.count(i)) result.push_back(Ssh[i]);
    }
    return result;
}

vector<int> repair_with_greedy(const SetCoverProblem& P, const vector<int>& partial) {
    int m = P.m, n = P.n;
    vector<bool> used(n, false);
    for (int j : partial) used[j] = true;
    vector<bool> covered(m, false);
    for (int j : partial)
        for (int i = 0; i < m; i++) if (P.A[i][j]) covered[i] = true;
    vector<unordered_set<int>> cov(n);
    for (int j = 0; j < n; j++)
        for (int i = 0; i < m; i++) if (P.A[i][j]) cov[j].insert(i);

    vector<int> sol = partial;
    unordered_set<int> uncovered;
    for (int i = 0; i < m; i++) if (!covered[i]) uncovered.insert(i);

    while (!uncovered.empty()) {
        int best_j = -1;
        double best_ratio = numeric_limits<double>::infinity();
        for (int j = 0; j < n; j++) {
            if (used[j]) continue;
            int cnt = 0;
            for (int r : cov[j]) if (uncovered.count(r)) cnt++;
            if (cnt == 0) continue;
            double ratio = double(P.costs[j]) / cnt;
            if (ratio < best_ratio) {
                best_ratio = ratio;
                best_j = j;
            }
        }
        if (best_j < 0) break;
        sol.push_back(best_j);
        used[best_j] = true;
        for (int r : cov[best_j]) uncovered.erase(r);
    }
    return sol;
}

// 3) Recherche locale Best-Improvement sur Add, Remove, Swap
vector<int> best_local_search(const SetCoverProblem& P, const vector<int>& init) {
    vector<int> best_sol = init;
    int best_cost = compute_cost(P, best_sol);
    int m = P.m, n = P.n;
    unordered_set<int> in_sol(init.begin(), init.end());

    // Add
    for (int j = 0; j < n; j++) {
        if (in_sol.count(j)) continue;
        vector<int> cand = init;
        cand.push_back(j);
        int c = compute_cost(P, cand);
        if (c < best_cost) {
            best_cost = c;
            best_sol = move(cand);
        }
    }

    // Remove
    for (size_t i = 0; i < init.size(); i++) {
        vector<int> cand = init;
        cand.erase(cand.begin() + i);
        if (!is_valid_solution(P, cand)) continue;
        int c = compute_cost(P, cand);
        if (c < best_cost) {
            best_cost = c;
            best_sol = move(cand);
        }
    }

    // Swap
    for (size_t i = 0; i < init.size(); i++) {
        for (int j = 0; j < n; j++) {
            if (in_sol.count(j)) continue;
            vector<int> cand = init;
            cand[i] = j;
            if (!is_valid_solution(P, cand)) continue;
            int c = compute_cost(P, cand);
            if (c < best_cost) {
                best_cost = c;
                best_sol = move(cand);
            }
        }
    }
    return best_sol;
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

vector<int> vns_optimized(const SetCoverProblem& P,
                          int maxIter = 500,
                          int maxK    = 5)
{
    // 1) Initialisation par glouton coût/coverage
    vector<int> S = generate_cost_coverage_greedy(P);
    int bestCost = compute_cost(P, S);

    int k = 1, iter = 0, stagn = 0;
    constexpr int STAG_THRESHOLD = 50;

    while (iter < maxIter && stagn < STAG_THRESHOLD) {
        // 2) Shake : suppression aléatoire de k sous-ensembles
        vector<int> Ssh = shake_remove_k(P, S, k);

        // 3) Réparation gloutonne
        Ssh = repair_with_greedy(P, Ssh);

        // 4) Local Search (best improvement)
        vector<int> Snew = best_local_search(P, Ssh);
        int newCost = compute_cost(P, Snew);

        if (newCost < bestCost) {
            S = move(Snew);
            bestCost = newCost;
            k = 1;
            stagn = 0;
        } else {
            k = min(k+1, maxK);
            stagn++;
        }

        iter++;
    }

    return S;
}


void benchmark_algorithms(const vector<string>& filenames, const string& output_csv) {
    ofstream file(output_csv);
    file << "Fichier,Méthode,Moyenne_Coût,Moyenne_Temps(s)\n";

    const int NUM_RUNS = 5;  // Réduit pour accélérer
    for (const string& filename : filenames) {
        cout << "📂 Traitement de : " << filename << endl;
        SetCoverProblem P = read_scp_file(filename);

        vector<string> method_names = {
            "Random",
            "BetterRandom",
            "Greedy_CostCoverage",
            "VNS_Optimized",
            "LocalSearch_Add",
            "LocalSearch_Remove",
            "LocalSearch_Swap",
            "VNS_Basic"
        };
        size_t m = method_names.size();
        vector<vector<int>> costs(m);
        vector<vector<double>> times(m);

        for (int run = 0; run < NUM_RUNS; run++) {
            cout << "  ➤ Run " << (run+1) << "/" << NUM_RUNS << endl;

            // Random
            auto t0 = chrono::high_resolution_clock::now();
            auto sol0 = generate_random_solution(P);
            auto t1 = chrono::high_resolution_clock::now();
            times[0].push_back(chrono::duration<double>(t1 - t0).count());
            costs[0].push_back(compute_cost(P, sol0));

            // BetterRandom
            t0 = chrono::high_resolution_clock::now();
            auto sol1 = generate_better_random_solution(P);
            t1 = chrono::high_resolution_clock::now();
            times[1].push_back(chrono::duration<double>(t1 - t0).count());
            costs[1].push_back(compute_cost(P, sol1));

            // Greedy_CostCoverage
            t0 = chrono::high_resolution_clock::now();
            auto sol2 = generate_cost_coverage_greedy(P);
            t1 = chrono::high_resolution_clock::now();
            times[2].push_back(chrono::duration<double>(t1 - t0).count());
            costs[2].push_back(compute_cost(P, sol2));

            // VNS_Optimized (itérations réduites)
            t0 = chrono::high_resolution_clock::now();
            auto sol3 = vns_optimized(P, 100, 3);
            t1 = chrono::high_resolution_clock::now();
            times[3].push_back(chrono::duration<double>(t1 - t0).count());
            costs[3].push_back(compute_cost(P, sol3));

            // LocalSearch_Add
            t0 = chrono::high_resolution_clock::now();
            auto sol4 = add_subset(P, sol0);
            t1 = chrono::high_resolution_clock::now();
            times[4].push_back(chrono::duration<double>(t1 - t0).count());
            costs[4].push_back(compute_cost(P, sol4));

            // LocalSearch_Remove
            t0 = chrono::high_resolution_clock::now();
            auto sol5 = remove_subset(P, sol0);
            t1 = chrono::high_resolution_clock::now();
            times[5].push_back(chrono::duration<double>(t1 - t0).count());
            costs[5].push_back(compute_cost(P, sol5));

            // LocalSearch_Swap
            t0 = chrono::high_resolution_clock::now();
            auto sol6 = swap_subset(P, sol0);
            t1 = chrono::high_resolution_clock::now();
            times[6].push_back(chrono::duration<double>(t1 - t0).count());
            costs[6].push_back(compute_cost(P, sol6));

            // VNS_Basic
            t0 = chrono::high_resolution_clock::now();
            auto sol7 = vns(P, 50);  // itérations réduites
            t1 = chrono::high_resolution_clock::now();
            times[7].push_back(chrono::duration<double>(t1 - t0).count());
            costs[7].push_back(compute_cost(P, sol7));
        }

        for (size_t i = 0; i < m; i++) {
            double avg_cost = accumulate(costs[i].begin(), costs[i].end(), 0.0) / costs[i].size();
            double avg_time = accumulate(times[i].begin(), times[i].end(), 0.0) / times[i].size();
            file << filename << "," << method_names[i] << "," << avg_cost << "," << avg_time << "\n";
        }
    }

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

    vector<string> filenames;

    // Ajout manuel des chemins de fichiers
    filenames.push_back("resources/scp41.txt");
    filenames.push_back("resources/maison_borne.txt");


    benchmark_algorithms(filenames, "resultats.csv");

    return 0;
}
