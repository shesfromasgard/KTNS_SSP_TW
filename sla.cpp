#include <iostream>
#include <vector>
#include <numeric>
#include <string>
#include <algorithm>
#include <limits>

using namespace std;

// ***************************************************
// m = número de ferramentas
// n = número de tarefas
// c = capacidade do magazine
// toolLife = tempo de vida útil de cada ferramenta nova
// executionTime = tempo de execução de cada tarefa
// matrix = matriz de ferramentas
// ***************************************************

unsigned int m, n, c;
vector<vector<int>> matrix;
vector<int> toolLife;
vector<int> executionTime;

long KTNS_Wear(const vector<int> processos, bool debug = false); // Renomeei a função

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

    cout << KTNS_Wear(processos, false) << endl; // Chamar a função adaptada

    return 0;
}

// Função KTNS adaptada para trocas por desgaste
long KTNS_Wear(const vector<int> processos, bool debug) {

    vector<int> loaded(m, 0); // 0: não carregada, 1: carregada
    int u = 0; // Ferramentas no magazine
    vector<int> currentToolLife = toolLife; // Vida útil restante das ferramentas carregadas

    vector<vector<int>> priorities(m, vector<int>(processos.size()));
    vector<vector<int>> magazine(m, vector<int>(processos.size())); // Matrix de ferramentas por processo na ordem

    if (debug) {
        cout << endl << "Matriz de Ferramentas no KTNS" << endl;
        for (unsigned j = 0; j < m; j++){
            for (unsigned i = 0; i < n; ++i){ // Usar n aqui, não 8
                cout << matrix[j][i] << " ";
            }
            cout << endl;
        }
        cout << " --------------------- " << endl;
        cout << "Processos" << endl;
        for (unsigned i = 0; i < processos.size(); i++) {
            cout << processos[i] << " ";
        }
        cout << endl;
        cout << endl;
    }

    // Preenche a matriz 'magazine' com base na ordem dos processos
    for (unsigned j = 0; j < m; j++) {
        for (unsigned i = 0; i < processos.size(); i++) {
             magazine[j][i] = matrix[j][processos[i]];
        }
    }

    // Preenche a matriz de prioridades (quando cada ferramenta será necessária novamente)
    for (unsigned i = 0; i < m; ++i){
        for (unsigned j = 0; j < processos.size(); ++j){
            if (magazine[i][j] == 1)
                priorities[i][j] = 0; // Necessária na tarefa atual
            else {
                int proxima = 0;
                bool usa = false;
                for (unsigned k = j + 1; k < processos.size(); ++k){
                    ++proxima;
                    if (magazine[i][k] == 1){
                        usa = true;
                        break;
                    }
                }
                if (usa)
                    priorities[i][j] = proxima; // Número de tarefas até a próxima vez que será usada
                else
                    priorities[i][j] = -1; // Não será mais usada
            }
        }
    }

    // Carregar ferramentas para a primeira tarefa (processos[0])
    for (unsigned j = 0; j < m; j++) {
        if (magazine[j][0] == 1) { // Se a ferramenta j é necessária para a primeira tarefa
            if (toolLife[j] >= executionTime[processos[0]]) { // Verificar vida útil inicial
                loaded[j] = 1;
                ++u;
            } else {
                 cerr << "Erro: Vida útil inicial da ferramenta " << j << " é insuficiente para a primeira tarefa " << processos[0] << "." << endl;
                 return -1; // Indica um erro fatal no input
            }
        }
    }

    int trocas = 0;

    if (debug) {
        cout << u << " ferramentas carregadas na primeira tarefa" << endl;
        cout << "Ferramentas carregadas (inicial):" << endl;
        for(int j = 0; j < m; ++j) cout << loaded[j] << " ";
        cout << endl;
        cout << "Vida útil restante (inicial):" << endl;
        for(int j = 0; j < m; ++j) cout << currentToolLife[j] << " ";
        cout << endl;
    }


    // Processar as tarefas sequencialmente
    for (unsigned i = 0; i < processos.size(); ++i) {
        int currentTaskIndex = processos[i];

        if (debug) {
             cout << "\nProcessando tarefa: " << currentTaskIndex << " (exec time: " << executionTime[currentTaskIndex] << ")" << endl;
             cout << "Ferramentas carregadas antes da tarefa:" << endl;
             for(int j = 0; j < m; ++j) cout << loaded[j] << " ";
             cout << endl;
             cout << "Vida útil restante antes da tarefa:" << endl;
             for(int j = 0; j < m; ++j) cout << currentToolLife[j] << " ";
             cout << endl;
        }


        // 1. Verificar e substituir ferramentas carregadas com vida útil insuficiente para a tarefa atual
        // Percorremos as ferramentas carregadas que são necessárias para esta tarefa
        for (unsigned j = 0; j < m; ++j) {
            if (loaded[j] == 1 && magazine[j][i] == 1) {
                if (currentToolLife[j] < executionTime[currentTaskIndex]) {
                    if (debug) cout << "  --> Ferramenta " << j << " (vida: " << currentToolLife[j] << ") sem vida útil suficiente para a tarefa " << currentTaskIndex << ". Trocando." << endl;
                    loaded[j] = 0; // Descarrega a ferramenta desgastada
                    u--;
                    trocas++;
                    // Carregar uma nova ferramenta
                    loaded[j] = 1;
                    u++; // Incrementa u porque é uma "nova" ferramenta no magazine
                    currentToolLife[j] = toolLife[j]; // Zera a vida útil da nova ferramenta
                }
            }
        }

        // 2. Carregar novas ferramentas necessárias para a tarefa atual que não estão carregadas
        for (unsigned j = 0; j < m; ++j) {
            // Se a ferramenta é necessária para a tarefa atual E não está carregada
            if ((magazine[j][i] == 1) && (loaded[j] == 0)) {
                // Ao carregar uma ferramenta que não estava no magazine, assumimos uma nova instância
                // cuja vida útil inicial deve ser suficiente para a tarefa.
                if (toolLife[j] >= executionTime[currentTaskIndex]) {
                     loaded[j] = 1;
                     ++u;
                     currentToolLife[j] = toolLife[j]; // Inicializa a vida útil da ferramenta carregada
                } else {
                     // Este erro indica que a vida útil inicial de uma ferramenta é menor que o tempo de uma tarefa que a utiliza.
                     cerr << "Erro: Vida útil inicial da ferramenta " << j << " é insuficiente para a tarefa " << currentTaskIndex << "." << endl;
                     return -1; // Indica um problema no input
                }
            }
        }

         if (debug) {
             cout << "Ferramentas carregadas após carregar/trocar para tarefa " << currentTaskIndex << ":" << endl;
             for(int j = 0; j < m; ++j) cout << loaded[j] << " ";
             cout << endl;
             cout << "Vida útil restante após carregar/trocar para tarefa " << currentTaskIndex << ":" << endl;
             for(int j = 0; j < m; ++j) cout << currentToolLife[j] << " ";
             cout << endl;
             cout << "Ferramentas no magazine (u): " << u << endl;
         }


        // 3. Gerenciar a capacidade do magazine *para a próxima tarefa* (se houver)
        // Esta lógica deve decidir quais ferramentas descarregar se o magazine estiver cheio,
        // considerando a prioridade (quando serão necessárias novamente) E a vida útil restante.
        if (i < processos.size() - 1) {
            int nextTaskIndexInProcessos = i + 1; // Índice da próxima tarefa no vetor 'processos'
            int nextActualTaskIndex = processos[nextTaskIndexInProcessos]; // Índice real da próxima tarefa

            if (debug) {
                cout << "Gerenciando magazine para a próxima tarefa: " << nextActualTaskIndex << endl;
            }

            while (u > c) {
                int bestToolToRemove = -1;
                int bestPriority = -2; // Prioridade: quanto maior, mais longe será usada (ou -1 se não for mais usada)
                int lowestRemainingLife = numeric_limits<int>::max(); // Menor vida restante entre os candidatos

                for (unsigned j = 0; j < m; ++j) {
                    // Se a ferramenta está carregada E não é necessária para a *próxima* tarefa
                    if (loaded[j] == 1 && magazine[j][nextTaskIndexInProcessos] == 0) {
                        // Prioridade de remoção: ferramentas que não serão mais usadas (-1 na prioridade) são as primeiras candidatas
                        if (priorities[j][nextTaskIndexInProcessos] == -1) {
                            bestToolToRemove = j;
                            if (debug) cout << "  --> Candidata a remover (nao sera mais usada): " << j << endl;
                            break; // Encontrou o melhor candidato, pode remover imediatamente
                        } else {
                            // Entre as ferramentas que serão usadas novamente, remova aquela que será necessária mais tarde (maior valor de prioridade)
                            if (priorities[j][nextTaskIndexInProcessos] > bestPriority) {
                                bestPriority = priorities[j][nextTaskIndexInProcessos];
                                bestToolToRemove = j;
                                lowestRemainingLife = currentToolLife[j]; // Armazena vida restante para desempate
                                if (debug) cout << "  --> Melhor candidata temporaria (prioridade): " << j << " (pri: " << bestPriority << ")" << endl;
                            } else if (priorities[j][nextTaskIndexInProcessos] == bestPriority) {
                                // Desempate de prioridade: remova a ferramenta com MENOR vida útil restante
                                if (currentToolLife[j] < lowestRemainingLife) {
                                    lowestRemainingLife = currentToolLife[j];
                                    bestToolToRemove = j;
                                     if (debug) cout << "  --> Melhor candidata temporaria (desempate vida): " << j << " (vida: " << lowestRemainingLife << ")" << endl;
                                }
                            }
                        }
                    }
                }

                if (bestToolToRemove != -1) {
                    if (debug) cout << "Removendo ferramenta (capacidade): " << bestToolToRemove << " antes da tarefa " << nextActualTaskIndex << endl;
                    loaded[bestToolToRemove] = 0;
                    u--;
                    // Nota: Esta remoção é devido à capacidade. A troca só é contada se/quando ela for carregada novamente
                    // E for uma nova instância devido ao desgaste anterior. A contagem de trocas já acontece
                    // quando substituímos uma ferramenta por desgaste (Passo 1).
                } else {
                     // Este caso pode ocorrer se todas as ferramentas carregadas forem necessárias para a próxima tarefa
                     // e a capacidade for excedida. Isso pode indicar um problema na formulação ou na capacidade.
                     cerr << "Erro: Não foi possível remover ferramenta para liberar espaço no magazine antes da tarefa " << nextActualTaskIndex << ". Todas as ferramentas carregadas são necessárias ou a capacidade é insuficiente." << endl;
                     exit(1); // Ou retorne um código de erro apropriado
                }
            }
        }

        // 4. Simular desgaste das ferramentas usadas na tarefa atual
        // Este passo simula o consumo de vida útil DURANTE a execução da tarefa.
        for (unsigned j = 0; j < m; ++j) {
            // Se a ferramenta estava carregada E foi usada nesta tarefa
            if (loaded[j] == 1 && magazine[j][i] == 1) {
                currentToolLife[j] -= executionTime[currentTaskIndex];
                 if (debug) cout << "  --> Ferramenta " << j << " usada na tarefa " << currentTaskIndex << ". Vida restante: " << currentToolLife[j] << endl;
            }
        }

         if (debug) {
             cout << "Fim da tarefa: " << currentTaskIndex << endl;
             cout << "Ferramentas carregadas ao final da tarefa:" << endl;
             for(int j = 0; j < m; ++j) cout << loaded[j] << " ";
             cout << endl;
             cout << "Vida útil restante ao final da tarefa:" << endl;
             for(int j = 0; j < m; ++j) cout << currentToolLife[j] << " ";
             cout << endl;
             cout << "Total de trocas até agora: " << trocas << endl;
             cout << "-------------------------------------" << endl;
         }

    }

    // A variável 'trocas' já acumulou todas as trocas (por desgaste e capacidade)
    if (debug) {
        cout << "Total final de trocas: " << trocas << endl;
    }

    return trocas; // O número total de trocas
}