#include <iostream>
#include <vector>
#include <fstream>
#include <numeric>
#include <string>
#include <algorithm>

using namespace std;

// SEQUÊNCIA DE ENTRADAS
// M - número de ferramentas
// N - número de tarefas
// C - capacidade do magazine
// tool life - vida útil de cada ferramenta do conjunto [m]
// executionTime - tempo de execução de cada tarefa [n]
// matrix - matriz de ferramentas [m][n]

unsigned int m, n, c;

vector<vector<int>> matrix;
vector<int> toolLife;
vector<int> executionTime;

long KTNS(const vector<int> processos, bool debug);

int main() {

    ifstream input("input.txt");
    ofstream output("output.txt");

    if(!input.is_open())
        cerr << "Não foi possível abrir o arquivo de saída." << endl;

    if(!output.is_open())
        cerr << "Não foi possível abrir o arquivo de entrada." << endl;

    input >> m >> n >> c;

    toolLife.resize(m);
    executionTime.resize(n);
    matrix.resize(m, vector<int>(n));

    for(int i = 0; i < m; ++i)
        input >> toolLife[i];

    for(int i = 0; i < n; ++i)
        input >> executionTime[n];

    for(int i = 0; i < m; ++i)
        for(int j = 0; j < n; ++j)
            input >> matrix[i][j];

    vector<int> processos(n);

    iota(processos.begin(), processos.end(), 0);

    output << KTNS(processos, true);
    
    input.close();
    output.close();

    return 0;
}

long KTNS(const vector<int> processos, bool debug = false) {
    if (processos.size() == 0) {
        return 0;
    }

    vector<int> loaded(m, 0);
    int u = 0; // Ferramentas no magazine

    vector<vector<int>> priorities(m, vector<int>(processos.size()));
    vector<vector<int>> magazine(m, vector<int>(processos.size()));

    if (debug) {
        cout << std::endl << "Matriz de Ferramentas no KTNS" << std::endl;
        for (unsigned j = 0; j < m; ++j) {
            for (unsigned i = 0; i < n; ++i) {
                cout << matrix[j][i] << " ";
            }
            cout << endl;
        }
        cout << " --------------------- " << endl;
        cout << "Processos" << endl;
        for (unsigned i = 0; i < processos.size(); ++i) {
            cout << processos[i] << " ";
        }
        cout << endl;
        cout << endl;
    }

    cout << "Magazine" << endl << endl;

    for (unsigned j = 0; j < m; ++j) {
        loaded[j] = matrix[j][processos[0]];
        if (matrix[j][processos[0]] == 1)
            ++u;
        for (unsigned i = 0; i < processos.size(); ++i) {
            magazine[j][i] = matrix[j][processos[i]];
            if (debug)
                cout << magazine[j][i] << " ";
        }
        if (debug) {
            cout << endl;
        }
    }

    cout << endl << endl;

    for (unsigned i = 0; i < m; ++i) {
        for (unsigned j = 0; j < processos.size(); ++j) {
            if (magazine[i][j] == 1)
                priorities[i][j] = 0;
            else {
                int proxima = 0;
                bool usa = false;
                for (unsigned k = j + 1; k < processos.size(); ++k) {
                    ++proxima;
                    if (magazine[i][k] == 1) {
                        usa = true;
                        break;
                    }
                }
                if (usa)
                    priorities[i][j] = proxima;
                else
                    priorities[i][j] = -1;
            }
        }
    }

    if (debug) {
        for (unsigned j = 0; j < m; ++j) {
            for (unsigned i = 0; i < processos.size(); ++i) {
                cout << priorities[j][i] << " ";
            }
            cout << endl;
        }

        cout << "Ferramentas carregadas: " << endl;
        for (unsigned j = 0; j < m; j++) {
            if (loaded[j] == 33) exit(0);
            cout << loaded[j] << endl;
        }
    }

    cout << endl << endl;

    if (debug) {
        cout << u << " carregadas na primeira tarefa" << endl;
    }

    int trocas = 0;
    for (unsigned i = 1; i < processos.size(); ++i) {
        for (unsigned j = 0; j < m; ++j) {
            if ((magazine[j][i] == 1) && (loaded[j] == 0)) {
                loaded[j] = 1;
                ++u;
            }
        }
        if (debug) {
            cout << u << " Ferramentas carregadas" << endl;
        }
        while (u > c) {
            int maior = 0;
            int pMaior = -1;
            for (unsigned j = 0; j < m; ++j) {
                if (magazine[j][i] != 1) { // Ferramenta não utilizada pelo processo atual
                    if ((loaded[j] == 1) && (priorities[j][i] == -1)) { // Essa ferramenta não será mais utilizada e é um excelente candidato a remoção
                        pMaior = j;
                        break;
                    } else {
                        if ((priorities[j][i] > maior) && loaded[j] == 1) {
                            maior = priorities[j][i];
                            pMaior = j;
                        }
                    }
                }
            }
            loaded[pMaior] = 0;
            if (debug) {
                cout << "Retirou " << i << ":" << pMaior << endl;
            }
            --u;
            ++trocas;
            if (debug) {
                cout << trocas << " trocas " << endl;
            }
        }
        if (debug) {
            cout << "Ferramentas carregadas: " << endl;
            for (unsigned j = 0; j < m; ++j) {
                cout << loaded[j] << endl;
            }
        }
    }
    if (debug) {
        cout << ": " << trocas << "trocas" << endl;
    }

    return trocas + c;
}