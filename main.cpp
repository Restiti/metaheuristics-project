#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>

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

int main() {
    string filename = "resources/scp41.txt"; // Remplacez par le fichier à lire

    // Lire les données
    SetCoverProblem problem = read_scp_file(filename);

    // Afficher les données lues
    display_problem(problem);

    return 0;
}
