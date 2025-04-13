#include <iostream>
#include <vector>
#include <fstream>
#include <numeric>
#include <string>
#include <filesystem>

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

long KTNS(const vector<int> processos, bool debug);

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

            output << KTNS(processos, false) << endl;

            input.close();
            output.close();

            cout << "Arquivo '" << filename << "' processado. Saída em '" << output_filename << "'" << endl;
        }
    }

    cout << "Processamento concluído." << endl;

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
    vector<int> currentToolLife = toolLife; // Tempo restante de vida das ferramentas

    if (debug) {
        cout << endl << "Matriz de Ferramentas no KTNS" << endl;
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
    
    // Contabilizar as trocas por desgaste
    // ...
    // ...

    for(int i = 0; i < m; ++i) {
        for(int j = 0; j < processos.size(); ++j)
            cout << magazine[i][j] << " ";
        cout << endl;
    }

    cout << endl << endl;


    if (debug) {
        cout << ": " << trocas << "trocas" << endl;
    }

    return trocas + c;
}