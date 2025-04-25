#include <iostream>
#include <vector>
#include <fstream>
#include <numeric>
#include <string>
#include <algorithm>
#include <limits>

using namespace std;

// ***************************************************
// m = número de ferramentas
// n = número de tarefas
// c = capacidade do magazine
// toolLife = tempo de vida últil de cada ferramenta nova
// executionTime = tempo de execução de cada tarefa
// matrix = matriz de ferramentas
// ***************************************************

unsigned int m, n, c;
vector<vector<int>> matrix;
vector<int> toolLife;
vector<int> executionTime;

int KTNS(const vector<int>& processos, bool debug);

int main() {

    cin >> m >> n >> c;

    toolLife.resize(m);
    executionTime.resize(n);
    matrix.resize(m, vector<int>(n));

    for (int i = 0; i < m; ++i)
        cin >> toolLife[i];

    for (int i = 0; i < n; ++i)
        cin >> executionTime[i];

    for (int i = 0; i < m; ++i)
        for (int j = 0; j < n; ++j)
            cin >> matrix[i][j];

    vector<int> processos(n);
    iota(processos.begin(), processos.end(), 0);

    cout << KTNS(processos, true);

    return 0;
}

int KTNS(const vector<int>& processos, bool debug = false) {

    if (processos.empty()) {
        return 0;
    }

    vector<int> carregadas(m, 0);
    vector<int> remainingLife(m, 0);
    int u = 0;
    int trocas = 0;

    vector<vector<int>> magazine(m, vector<int>(processos.size()));
    vector<vector<int>> prioridades(m, vector<int>(processos.size()));

    // Preenchendo o segundo vetor do magazine
    for (unsigned j = 0; j < m; ++j) {
        for (unsigned i = 0; i < processos.size(); ++i) {
            if (processos[i] >= 0 && processos[i] < n) {
                magazine[j][i] = matrix[j][processos[i]];
            } else {
                magazine[j][i] = 0;
                if(debug) cerr << "Aviso: index de tarefa inválido " << processos[i] << " no índice " << i << endl;
            }
        }
    }

    // Preenchendo a matriz de prioridades
    for (unsigned i = 0; i < m; ++i) {
        for (unsigned j = 0; j < processos.size(); ++j) {
            if (magazine[i][j] == 1) {
                prioridades[i][j] = 0;
            } else {
                int proxima = 0;
                bool usa = false;
                for (unsigned k = j + 1; k < processos.size(); ++k) {
                    ++proxima;
                    if (magazine[i][k] == 1) {
                        usa = true; break;
                    }
                }
                prioridades[i][j] = usa ? proxima : -1;
            }
        }
    }

    // Tarefa 0
    if (debug) cout << "Inicializando para Tarefa 0 (ID: " << processos[0] << ") ---" << endl;
    if (processos[0] >= 0 && processos[0] < n) {
        for (unsigned j = 0; j < m; ++j) {
            if (magazine[j][0] == 1) {
                carregadas[j] = 1;
                remainingLife[j] = toolLife[j];
                u++;
                if (debug) cout << "  Carregada Ferramenta " << j << " (Vida Inicial: " << remainingLife[j] << ")" << endl;
            }
        }
    } else {
        if(debug)
            cerr << "Erro: Primeira tarefa inválida!" << endl;
        return 0;
    }

    for (unsigned i = 0; i < processos.size(); ++i) {
        if (processos[i] < 0 || processos[i] >= n)
            continue;

        if (debug) cout << "\nProcessando Tarefa " << i << " (ID: " << processos[i] << ", Tempo: " << executionTime[processos[i]] << ")" << endl;
        if (debug) cout << "  Estado Antes: u=" << u << ", trocas=" << trocas << endl;
        if (debug) {cout << "   Life Antes: "; for(int l : remainingLife) cout << l << " "; cout << endl;}


        // Verificação preditiva de desgaste antes do uso:
        for (unsigned j = 0; j < m; ++j) {
            if (magazine[j][i] == 1 && carregadas[j] == 1) {
                if (remainingLife[j] < executionTime[processos[i]]) {
                    trocas++;
                    remainingLife[j] = toolLife[j];
                    if (debug) cout << "  Troca preditiva por desgaste: Ferramenta " << j << " renovada. Novas trocas=" << trocas << endl;
                }
            }
        }

        // Carregamento de ferramentas necessárias:
        for (unsigned j = 0; j < m; ++j) {
            if (magazine[j][i] == 1 && carregadas[j] == 0) {
                carregadas[j] = 1;
                remainingLife[j] = toolLife[j];
                u++;
                if (debug) cout << "  Carregada ferramenta " << j << " (Vida: " << remainingLife[j] << "). u=" << u << endl;
            }
        }

        // Remoção por capacidade:
        while (u > c) {
            if (debug) cout << "  Capacidade excedida: u=" << u << ", c=" << c << ". Procurando ferramenta para remover" << endl;

            int remove = -1;
            bool removed = false;

            for (unsigned j = 0; j < m; ++j) {
                if (carregadas[j] == 1 && magazine[j][i] != 1 && prioridades[j][i] == -1) {
                    remove = j;
                    removed = true;
                    if (debug) cout << "    Encontrada candidata ideal: Ferramenta " << j << " (prioridade -1)." << endl;
                    break;
                }
            }

            // Se não removeu uma com -1
            if (!removed) {
                int min = numeric_limits<int>::max();

                for (unsigned j = 0; j < m; ++j) {
                    if (carregadas[j] == 1 && magazine[j][i] != 1 && prioridades[j][i] > 0) {
                        int nextIndex = i + prioridades[j][i];

                        if (nextIndex < processos.size()) {
                            if (processos[nextIndex] >= 0 && processos[nextIndex] < n) {
                                int current = remainingLife[j] - executionTime[processos[nextIndex]];

                                if (debug) cout << "    Candidata " << j << ": Prioridade=" << prioridades[j][i] << ", NextTaskIdx=" << nextIndex << ", NextTaskID=" << processos[nextIndex] << ", NextTaskTime=" << executionTime[processos[nextIndex]] << ", CurrentLife=" << remainingLife[j] << ", Metric=" << current << endl;

                                if (current < min) {
                                    min = current;
                                    remove = j;
                                }
                            } else {
                                if (debug) cerr << "    Aviso: id inválido para ferramenta " << j << endl;
                            }
                        } else {
                            if (debug) cerr << "    Aviso: nextIndex fora dos limites para ferramenta " << j << endl;
                        }
                    }
                }
                if (debug && remove != -1) cout << "    Melhor candidata: Ferramenta " << remove << " (Metrica: " << min << ")" << endl;
            }

            if (remove != -1) {
                if (debug) cout << "    -> Removendo ferramenta " << remove << endl;
                carregadas[remove] = 0;
                remainingLife[remove] = 0;
                u--;
                trocas++;
                if (debug) cout << "     Nova contagem: u=" << u << ", trocas=" << trocas << endl;
            } else {
                if (debug) cerr << "  ERRO: Impossível remover ferramenta u=" << u << ", c=" << c << endl;
                break;
            }
        }

        // Decremento da vida útil após o uso:
        for (unsigned j = 0; j < m; ++j) {
            if (magazine[j][i] == 1 && carregadas[j] == 1) {
                remainingLife[j] -= executionTime[processos[i]];
                if (debug) cout << "  Vida útil da Ferramenta " << j << " decrementada por " << executionTime[processos[i]] << " para " << remainingLife[j] << endl;
            }
        }

        if (debug) {
            cout << "  Estado final tarefa " << i << ": u=" << u << ", trocas=" << trocas << endl;
            cout << "   Carregadas: "; for(int l : carregadas) cout << l << " "; cout << endl;
            cout << "   Life:       "; for(int l : remainingLife) cout << l << " "; cout << endl;
        }

    }

    cout << endl << endl;

    return trocas + c;
}
