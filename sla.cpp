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

    for (unsigned j = 0; j < m; ++j) {
        for (unsigned i = 0; i < processos.size(); ++i) {
            int currentTaskId = processos[i];
            if (currentTaskId >= 0 && currentTaskId < n) {
                magazine[j][i] = matrix[j][currentTaskId];
            } else {
                 magazine[j][i] = 0;
                 if(debug) cerr << "Aviso: ID de tarefa inválido " << currentTaskId << " no índice " << i << endl;
            }
        }
    }
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
    if (debug) cout << "--- Inicializando para Tarefa 0 (ID: " << processos[0] << ") ---" << endl;
    int firstTaskId = processos[0];
    if (firstTaskId >= 0 && firstTaskId < n) {
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

    // Simulação da Sequência
    for (unsigned i = 0; i < processos.size(); ++i) {
        int currentTaskId = processos[i];
        if (currentTaskId < 0 || currentTaskId >= n) 
            continue;

        int taskExecTime = executionTime[currentTaskId];

        if (debug) cout << "\n--- Processando Tarefa " << i << " (ID: " << currentTaskId << ", Tempo: " << taskExecTime << ") ---" << endl;
        if (debug) cout << "  Estado Antes: u=" << u << ", trocas=" << trocas << endl;
        if (debug) {cout << "   Life Antes: "; for(int l : remainingLife) cout << l << " "; cout << endl;}


        // A. Verificação PREDITIVA de Desgaste ANTES do uso:
        for (unsigned j = 0; j < m; ++j) {
            if (magazine[j][i] == 1 && carregadas[j] == 1) {
                if (remainingLife[j] < taskExecTime) {
                    trocas++;
                    remainingLife[j] = toolLife[j];
                    if (debug) cout << "  (!) Troca PREDITIVA por DESGASTE: Ferramenta " << j << " renovada. Novas trocas=" << trocas << endl;
                }
            }
        }

        // B. Carregamento de Ferramentas Necessárias:
        for (unsigned j = 0; j < m; ++j) {
            if (magazine[j][i] == 1 && carregadas[j] == 0) {
                carregadas[j] = 1;
                remainingLife[j] = toolLife[j]; // Define vida ao carregar
                u++;
                if (debug) cout << "  (+) Carregada Ferramenta " << j << " (Vida: " << remainingLife[j] << "). u=" << u << endl;
            }
        }

        // C. Remoção por Capacidade:
        while (u > c) {
            if (debug) cout << "  (!) Capacidade excedida: u=" << u << ", c=" << c << ". Procurando ferramenta para remover (Nova Politica)..." << endl;

            int toolRemove = -1;
            bool removedNoFuture = false;

            // 1. Priorizar ferramentas que não serão mais usadas (-1)
            for (unsigned j = 0; j < m; ++j) {
                if (carregadas[j] == 1 && magazine[j][i] != 1 && prioridades[j][i] == -1) {
                    toolRemove = j;
                    removedNoFuture = true;
                    if (debug) cout << "    Encontrada candidata ideal: Ferramenta " << j << " (prioridade -1)." << endl;
                    goto removal; // Achou a melhor opção, sai da busca
                }
            }

            // 2. Se não removeu uma com -1, aplica a nova política
            if (!removedNoFuture) {
                int minMetric = numeric_limits<int>::max(); // Começa com valor muito alto

                for (unsigned j = 0; j < m; ++j) {
                    if (carregadas[j] == 1 && magazine[j][i] != 1 && prioridades[j][i] > 0) {
                        int priority = prioridades[j][i];
                        int nextIndex = i + priority;

                        if (nextIndex < processos.size()) {
                            int nextId = processos[nextIndex];
                            if (nextId >= 0 && nextId < n) {
                                int nextTime = executionTime[nextId];
                                int currentMetric = remainingLife[j] - nextTime;

                                if (debug) cout << "    Candidata " << j << ": Prio=" << priority << ", NextTaskIdx=" << nextIndex << ", NextTaskID=" << nextId << ", NextTaskTime=" << nextTime << ", CurrentLife=" << remainingLife[j] << ", Metric=" << currentMetric << endl;

                                if (currentMetric < minMetric) {
                                    minMetric = currentMetric;
                                    toolRemove = j;
                                }
                            } else {
                                if (debug) cerr << "    Aviso: nextId inválido para ferramenta " << j << endl;
                            }
                        } else {
                             if (debug) cerr << "    Aviso: nextIndex fora dos limites para ferramenta " << j << endl;
                        }
                    }
                }
                if (debug && toolRemove != -1) cout << "    Melhor candidata pela nova politica: Ferramenta " << toolRemove << " (Metrica: " << minMetric << ")" << endl;
            }

        removal:
            if (toolRemove != -1) {
                if (debug) cout << "    -> Removendo Ferramenta " << toolRemove << endl;
                carregadas[toolRemove] = 0;
                remainingLife[toolRemove] = 0;
                u--;
                trocas++;
                if (debug) cout << "     Nova contagem: u=" << u << ", trocas=" << trocas << endl;
            } else {
                // Se toolRemove continua -1, significa que não achou nenhuma ferramenta para remover (todas carregadas são necessárias ou houve erro).
                if (debug) cerr << "  ERRO: Impossível remover ferramenta (Nova Politica)! u=" << u << ", c=" << c << endl;
                break; // Sai do while para evitar loop infinito
            }
        }


        // D. Decremento da Vida Útil APÓS o uso:
        for (unsigned j = 0; j < m; ++j) {
            if (magazine[j][i] == 1 && carregadas[j] == 1) {
                remainingLife[j] -= taskExecTime;
                if (debug) cout << "  (-) Vida útil da Ferramenta " << j << " decrementada por " << taskExecTime << " para " << remainingLife[j] << endl;
            }
        }

        if (debug) {
             cout << "  Estado Final Tarefa " << i << ": u=" << u << ", trocas=" << trocas << endl;
             cout << "   Carregadas: "; for(int l : carregadas) cout << l << " "; cout << endl;
             cout << "   Life:       "; for(int l : remainingLife) cout << l << " "; cout << endl;
        }

    }

    if (debug) {
        cout << "\n--- Fim da Simulação ---" << endl;
        cout << "Trocas incrementais (desgaste + capacidade): " << trocas << endl;
    }

    return trocas + c;
}