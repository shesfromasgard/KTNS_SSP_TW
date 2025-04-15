#include <iostream>
#include <vector>
#include <fstream>
#include <numeric>
#include <string>
#include <algorithm>

using namespace std;

unsigned int m, n, c;
vector<vector<int>> matrix;
vector<int> toolLife;
vector<int> executionTime;

long KTNS(const vector<int>& processos, bool debug);

int main() {
    ifstream input("input.txt");
    ofstream output("output.txt");

    if (!input.is_open()) {
        cerr << "Não foi possível abrir o arquivo de entrada." << endl;
        return 1;
    }
    if (!output.is_open()) {
        cerr << "Não foi possível abrir o arquivo de saída." << endl;
        return 1;
    }

    input >> m >> n >> c;

    toolLife.resize(m);
    executionTime.resize(n);
    matrix.resize(m, vector<int>(n));

    for (int i = 0; i < m; ++i)
        input >> toolLife[i];

    for (int i = 0; i < n; ++i)
        input >> executionTime[i];

    for (int i = 0; i < m; ++i)
        for (int j = 0; j < n; ++j)
            input >> matrix[i][j];

    vector<int> processos(n);
    iota(processos.begin(), processos.end(), 0);

    output << KTNS(processos, false);

    input.close();
    output.close();

    return 0;
}

long KTNS(const vector<int>& processos, bool debug = false) {
    if (processos.empty()) {
        return 0;
    }

    vector<int> loaded(m, 0);
    vector<int> remainingLife(m, 0);    // Vida útil restante das ferramentas carregadas
    int u = 0;                           // Contador de ferramentas carregadas
    long trocas = 0;                     // Contador de trocas

    vector<vector<int>> magazine(m, vector<int>(processos.size()));
    vector<vector<int>> priorities(m, vector<int>(processos.size()));

    for (unsigned j = 0; j < m; ++j) {
        for (unsigned i = 0; i < processos.size(); ++i) {
            int currentTask = processos[i];
            if (currentTask >= 0 && currentTask < n) {
                magazine[j][i] = matrix[j][currentTask];
            } else {
                 magazine[j][i] = 0;
                 if(debug) cerr << "Aviso: ID de tarefa inválido " << currentTask << " no índice " << i << endl;
            }
        }
    }
    for (unsigned i = 0; i < m; ++i) {
        for (unsigned j = 0; j < processos.size(); ++j) {
            if (magazine[i][j] == 1) {
                priorities[i][j] = 0;
            } else {
                int proxima = 0;
                bool usa = false;
                for (unsigned k = j + 1; k < processos.size(); ++k) {
                    ++proxima;
                    if (magazine[i][k] == 1) {
                        usa = true; break;
                    }
                }
                priorities[i][j] = usa ? proxima : -1;
            }
        }
    }

    if (debug) cout << "--- Inicializando para Tarefa 0 (ID: " << processos[0] << ") ---" << endl;
    int task1 = processos[0];
    if (task1 >= 0 && task1 < n) {
        for (unsigned j = 0; j < m; ++j) {
            if (magazine[j][0] == 1) {
                loaded[j] = 1;
                remainingLife[j] = toolLife[j]; // Carrega com vida útil total
                u++;
                if (debug) cout << "  Carregada Ferramenta " << j << " (Vida Inicial: " << remainingLife[j] << ")" << endl;
            }
        }
    } else {
        if(debug) cerr << "Erro: Primeira tarefa inválida!" << endl;
        return c;
    }

    for (unsigned i = 0; i < processos.size(); ++i) {
        int currentTask = processos[i];
        if (currentTask < 0 || currentTask >= n) continue;

        int taskExecTime = executionTime[currentTask];

        if (debug) cout << "\n--- Processando Tarefa " << i << " (ID: " << currentTask << ", Tempo: " << taskExecTime << ") ---" << endl;
        if (debug) cout << "  Estado Antes: u=" << u << ", trocas=" << trocas << endl;

        for (unsigned j = 0; j < m; ++j) {
            if (magazine[j][i] == 1 && loaded[j] == 1) {
                if (remainingLife[j] < taskExecTime) {
                    trocas++;
                    remainingLife[j] = toolLife[j];
                    if (debug) cout << "  (!) Troca PREDITIVA por DESGASTE: Ferramenta " << j << " (Vida: " << remainingLife[j] << " < " << taskExecTime << ") renovada. Novas trocas=" << trocas << endl;
                }

            }
        }

        for (unsigned j = 0; j < m; ++j) {
            if (magazine[j][i] == 1 && loaded[j] == 0) {
                loaded[j] = 1;
                remainingLife[j] = toolLife[j];
                u++;
                if (debug) cout << "  (+) Carregada Ferramenta " << j << " (Vida: " << remainingLife[j] << "). u=" << u << endl;
            }
        }

        while (u > c) {
            if (debug) cout << "  (!) Capacidade excedida: u=" << u << ", c=" << c << ". Procurando ferramenta para remover..." << endl;
            int maior_prio = -2;
            int pMaior = -1;

            for (unsigned j = 0; j < m; ++j) {
                if (loaded[j] == 1 && magazine[j][i] != 1 && priorities[j][i] == -1) {
                    pMaior = j;
                    goto remove_tool_exec;
                }
            }
            maior_prio = -1;
            for (unsigned j = 0; j < m; ++j) {
                if (loaded[j] == 1 && magazine[j][i] != 1) {
                    if (priorities[j][i] > maior_prio) {
                        maior_prio = priorities[j][i];
                           pMaior = j;
                     }
                 }
             }

         remove_tool_exec:
             if (pMaior != -1) {
                if (debug) cout << "    -> Removendo Ferramenta " << pMaior << " (Prioridade: " << (pMaior < m ? priorities[pMaior][i] : -99) << ")" << endl;
                loaded[pMaior] = 0;
                remainingLife[pMaior] = 0;
                u--;
                trocas++;
                if (debug) cout << "     Nova contagem: u=" << u << ", trocas=" << trocas << endl;
             } else {
                if (debug) cerr << "  ERRO: Impossível remover ferramenta! u=" << u << ", c=" << c << endl;
                break;
             }
        }

        for (unsigned j = 0; j < m; ++j) {
            if (magazine[j][i] == 1 && loaded[j] == 1) {
                remainingLife[j] -= taskExecTime;
                if (debug) cout << "  (-) Vida útil da Ferramenta " << j << " decrementada por " << taskExecTime << " para " << remainingLife[j] << endl;
            }
        }

        if (debug) {
            cout << "  Estado Final Tarefa " << i << ": u=" << u << ", trocas=" << trocas << endl;
            cout << "   Loaded: "; for(int l : loaded) cout << l << " "; cout << endl;
            cout << "   Life:   "; for(int l : remainingLife) cout << l << " "; cout << endl;
        }

    }

    if (debug) {
        cout << "\n--- Fim da Simulação ---" << endl;
        cout << "Trocas incrementais (desgaste + capacidade): " << trocas << endl;
        cout << "Capacidade (c): " << c << endl;
        cout << "Resultado Final (trocas + c): " << trocas + c << endl;
    }

    return trocas + c;
}
