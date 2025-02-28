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
    vector<bool> covered(problem.m, false);
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




int main() {
    string filename = "resources/scp41.txt"; // Remplacez par le fichier à lire

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

    if (is_valid_solution(problem, random_solution)) {
        cout << "✅ La solution est valide et couvre toutes les lignes." << endl;
    } else {
        cout << "❌ La solution n'est PAS valide !" << endl;
    }


    return 0;
}
