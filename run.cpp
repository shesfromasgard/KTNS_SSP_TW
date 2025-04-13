#include <iostream>
#include <vector>
#include <fstream>
#include <numeric>
#include <string>
#include <filesystem>
#include <limits>
#include <algorithm>

using namespace std;
namespace fs = std::filesystem;

unsigned int m, n, c;

// SEQUÊNCIA DE ENTRADAS
// M - número de ferramentas
// N - número de tarefas
// C - capacidade do magazine
// matrix - matriz de ferramentas [m][n]
// tool life - vida útil de cada ferramenta do conjunto [m]
// executionTime - tempo de execução de cada tarefa [n]

vector<vector<int>> matrix;
vector<int> toolLife;
vector<int> executionTime;

long KTNS(vector<int> processos, bool debug);

int main(int argc, char *argv[]) {
    if (argc != 3) {
        cerr << "Uso: " << argv[0] << " <diretorio_entrada> <diretorio_saida>" << endl;
        return 1;
    }

    fs::path input_dir = argv[1];
    fs::path output_dir = argv[2];

    if (!fs::is_directory(input_dir)) {
        cerr << "Erro: Diretório de entrada '" << input_dir << "' não encontrado." << endl;
        return 1;
    }

    if (!fs::is_directory(output_dir)) {
        cerr << "Erro: Diretório de saída '" << output_dir << "' não encontrado." << endl;
        return 1;
    }

    for (const auto& entry : fs::directory_iterator(input_dir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".txt") {
            fs::path input_filepath = entry.path();
            fs::path filename = input_filepath.filename();
            fs::path base_name = filename.stem();
            fs::path output_filename = base_name.string() + "_output.txt";
            fs::path output_filepath = output_dir / output_filename;

            ifstream input(input_filepath);
            ofstream output(output_filepath);

            if (!input.is_open()) {
                cerr << "Erro ao abrir o arquivo de entrada: " << input_filepath << endl;
                continue; // Ir para o próximo arquivo
            }

            if (!output.is_open()) {
                cerr << "Erro ao abrir o arquivo de saída: " << output_filepath << endl;
                input.close();
                continue; // Ir para o próximo arquivo
            }

            input >> m >> n >> c;

            matrix.resize(m, vector<int>(n));
            toolLife.resize(m);
            executionTime.resize(n);

            for(int i = 0; i < m; ++i)
                input >> toolLife[i];

            for(int i = 0; i < n; ++i)
                input >> executionTime[i];

            for (int j = 0; j < m; ++j)
                for (int k = 0; k < n; ++k)
                    input >> matrix[j][k];

            vector<int> processos(n);
            iota(processos.begin(), processos.end(), 0);

            output << KTNS(processos, true) << endl;

            input.close();
            output.close();

            cout << "Arquivo '" << filename << "' processado. Saída em '" << output_filename << "'" << endl;
        }
    }

    cout << "Processamento concluído." << endl;

    return 0;
}

long KTNS(vector<int> processos, bool debug = false) {
    if (processos.empty()) {
        return 0;
    }

    // --- Variáveis de Estado (usando nomes originais) ---
    vector<int> loaded(m, 0);           // Ferramentas no magazine (0=não, 1=sim)
    int u = 0;                          // Contador de ferramentas no magazine
    vector<int> currentToolLife(m, 0);  // Vida útil restante das ferramentas carregadas
    long trocas_normais = 0;            // Contador para trocas KTNS/novas
    long trocas_desgaste = 0;           // Contador para trocas por desgaste

    // --- Matriz de Prioridades (como no original) ---
    // magazine e priorities são calculadas dentro do loop ou antes, como você preferir.
    // Vamos calcular priorities antes, é mais eficiente.
    vector<vector<int>> priorities(m, vector<int>(processos.size()));
     for (unsigned int j = 0; j < m; ++j) {
        for (int i = processos.size() - 1; i >= 0; --i) { // Itera de trás pra frente
             int current_job_original_index = processos[i];
             if (matrix[j][current_job_original_index] == 1) {
                 priorities[j][i] = 0; // Necessária neste job (índice i na sequencia)
             } else {
                 if (i == (int)processos.size() - 1) {
                     priorities[j][i] = -1; // Último job e não precisa
                 } else {
                     if (priorities[j][i + 1] == -1) {
                         priorities[j][i] = -1; // Não precisa agora nem depois
                     } else {
                         priorities[j][i] = priorities[j][i + 1] + 1; // Passos = passos do próx + 1
                     }
                 }
             }
        }
    }

    if (debug) {
        cout << "\n--- KTNS com Desgaste (Variáveis Originais) ---" << endl;
        cout << "Sequência de Processos: ";
        for(int p : processos) cout << p << " ";
        cout << "\nVida Útil Inicial: ";
        for(int l : toolLife) cout << l << " ";
        cout << "\nTempo Execução: ";
        for(int t : executionTime) cout << t << " ";
        cout << "\nCapacidade Magazine: " << c << endl;
        cout << "\nMatriz Prioridades (Ferramenta x Índice na Sequência):" << endl;
        for (unsigned int tool = 0; tool < m; ++tool) {
             cout << "T" << tool << ": ";
            for (unsigned int i = 0; i < processos.size(); ++i) {
                cout << priorities[tool][i] << "\t";
            }
            cout << endl;
        }
         cout << "----------------------------------" << endl;
    }


    // --- Processamento Sequencial dos Jobs ---
    for (unsigned int i = 0; i < processos.size(); ++i) {
        int current_job_original_index = processos[i];
        int current_job_exec_time = executionTime[current_job_original_index];

        if (debug) {
             cout << "\nProcessando Job " << current_job_original_index << " (Índice Sequência " << i << ", Tempo Exec: " << current_job_exec_time << ")" << endl;
             cout << "  Magazine ANTES (u=" << u << "): { ";
             for(unsigned int tool=0; tool<m; ++tool) if(loaded[tool]==1) cout << "T" << tool << "(" << currentToolLife[tool] << ") ";
             cout << "}" << endl;
        }

        vector<int> tools_needed_now; // Ferramentas necessárias para o job atual
        for(unsigned int tool = 0; tool < m; ++tool) {
            if (matrix[tool][current_job_original_index] == 1) {
                tools_needed_now.push_back(tool);
            }
        }

        // --- 1. Trocas Obrigatórias por Desgaste ---
        // Verifica ferramentas *já carregadas* que são necessárias agora
        for (int tool_needed : tools_needed_now) {
            if (loaded[tool_needed] == 1) { // Se está carregada e é necessária
                if (currentToolLife[tool_needed] < current_job_exec_time) {
                    if (debug) cout << "  TROCA DESGASTE: T" << tool_needed << " precisa trocar (Vida " << currentToolLife[tool_needed] << " < " << current_job_exec_time << ")" << endl;
                    trocas_desgaste++;
                    currentToolLife[tool_needed] = toolLife[tool_needed]; // Reseta vida (nova ferramenta)
                    // 'loaded' continua 1, 'u' não muda aqui
                }
            }
        }

        // --- 2. Identificar Ferramentas Novas a Carregar ---
        vector<int> tools_to_load_now;
        for (int tool_needed : tools_needed_now) {
            if (loaded[tool_needed] == 0) { // Se é necessária mas NÃO está carregada
                tools_to_load_now.push_back(tool_needed);
            }
        }

        // --- 3. Evicção KTNS (se carregar novas ferramentas exceder capacidade) ---
        int slots_needed = u + tools_to_load_now.size();
        int evictions_needed = max(0, slots_needed - (int)c);

        if (debug && evictions_needed > 0) {
             cout << "  Capacidade: Precisa carregar " << tools_to_load_now.size() << ", tem " << u << ". Evictar " << evictions_needed << " ferramenta(s)." << endl;
        }

        for (int k = 0; k < evictions_needed; ++k) {
            int pMaior = -1; // Índice da ferramenta a remover (original: pMaior)
            int maior_prioridade = -2; // Prioridade da ferramenta a remover (original: maior)
            int best_never_used_tool = -1; // Otimização: ferramenta que nunca mais será usada

            for (unsigned int j = 0; j < m; ++j) { // Itera sobre todas as ferramentas j
                 bool needed_now = false;
                 for(int tn : tools_needed_now) {
                     if ((int)j == tn) {
                         needed_now = true;
                         break;
                     }
                 }

                // Condição para evicção: carregada (loaded[j]==1) E NÃO necessária agora (!needed_now)
                if (loaded[j] == 1 && !needed_now) {
                    int prioridade_atual = priorities[j][i]; // Prioridade da ferramenta j no passo i da sequência

                    if (prioridade_atual == -1) { // Prioridade máxima: nunca mais usada
                        best_never_used_tool = j;
                        break; // Achou a melhor opção, pode parar de procurar
                    }

                    if (prioridade_atual > maior_prioridade) {
                        maior_prioridade = prioridade_atual;
                        pMaior = j;
                    }
                }
            } // Fim for j (procura ferramenta para evictar)

            if (best_never_used_tool != -1) {
                pMaior = best_never_used_tool; // Prioriza remover a que nunca mais será usada
            }


            if (pMaior != -1) {
                 if (debug) cout << "  EVICTANDO KTNS: T" << pMaior << " (Prioridade: " << priorities[pMaior][i] << ")" << endl;
                 loaded[pMaior] = 0; // Remove a ferramenta
                 currentToolLife[pMaior] = 0; // Zera a vida útil ao remover
                 --u; // Decrementa contador do magazine
                 ++trocas_normais; // Conta como troca normal
            } else {
                 // Se chegou aqui, não achou ferramenta para remover, mesmo precisando. Erro de lógica ou cenário impossível?
                 cerr << "ERRO: Nao foi possivel encontrar ferramenta para evictar no job " << current_job_original_index << endl;
                 break; // Sai do loop de evicção
            }
        } // Fim for k (loop de evicção)

        // --- 4. Carregar Novas Ferramentas ---
        for (int tool_to_load : tools_to_load_now) {
            if (u < (int)c) { // Verifica se há espaço (deve haver após evicção)
                 if (debug) cout << "  CARREGANDO NOVA: T" << tool_to_load << endl;
                 loaded[tool_to_load] = 1; // Carrega
                 currentToolLife[tool_to_load] = toolLife[tool_to_load]; // Reseta vida
                 ++u; // Incrementa contador do magazine
                 ++trocas_normais; // Conta como troca normal
            } else {
                 // Se chegou aqui, a lógica de evicção falhou.
                 cerr << "ERRO: Magazine cheio (" << u << "), nao foi possivel carregar T" << tool_to_load << " para job " << current_job_original_index << endl;
            }
        }

        // --- 5. Decrementar Vida Útil (Após o job ser concluído) ---
        if (debug) cout << "  Decrementando vida das ferramentas usadas no Job " << current_job_original_index << ":" << endl;
        for (int tool_used : tools_needed_now) {
             if (loaded[tool_used] == 1) { // Só decrementa se estava carregada
                 currentToolLife[tool_used] -= current_job_exec_time;
                 if (debug) cout << "    T" << tool_used << " vida reduzida em " << current_job_exec_time << " -> " << currentToolLife[tool_used] << endl;
             } else {
                 // Isso seria um erro - a ferramenta deveria estar carregada para ser usada
                 cerr << "ERRO: Ferramenta T" << tool_used << " necessaria para job " << current_job_original_index << " nao estava carregada para decrementar vida." << endl;
             }
        }

        if (debug) {
             cout << "  Magazine DEPOIS (u=" << u << "): { ";
             for(unsigned int tool=0; tool<m; ++tool) if(loaded[tool]==1) cout << "T" << tool << "(" << currentToolLife[tool] << ") ";
             cout << "}" << endl;
             cout << "  Trocas Acumuladas: Normais=" << trocas_normais << ", Desgaste=" << trocas_desgaste << ", Total=" << (trocas_normais + trocas_desgaste) << endl;
             cout << "----------------------------------" << endl;
         }

    } // Fim for i (loop principal de jobs)

    // O retorno original era trocas + c. Isso pode incluir a carga inicial.
    // Se você quer contar apenas as trocas *entre* jobs, retorne a soma.
    // Se precisa replicar o comportamento original (contando carga inicial como 'c' trocas),
    // talvez seja melhor retornar trocas_normais + trocas_desgaste.
    // Vou retornar a soma das trocas, que é o mais comum. Ajuste se necessário.
    // A carga inicial do primeiro job *não* está sendo contada como troca aqui.
    return trocas_normais + trocas_desgaste;
}