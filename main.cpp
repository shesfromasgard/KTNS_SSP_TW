#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <limits>
#include <numeric>

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

int KTNS(const vector<int> processos, bool debug);

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

int KTNS(const vector<int> processos, bool debug = false) {

    vector<int> carregadas(m, 0);
    vector<vector<int>> prioridades(m, vector<int>(processos.size()));
    vector<vector<int>> magazine(m, vector<int>(processos.size()));
    vector<int> remainingLife = toolLife;

    int u = 0; // ferramentas no magazine
    int trocas = 0;

    for (unsigned j = 0; j < m; j++) {
        // preenche o magazine com as ferramentas para a primeira tarefa
		carregadas[j] = matrix[j][processos[0]];
		if (matrix[j][processos[0]] == 1) {
			++u;
            ++trocas;
            // já decrementa o tempo de execução da tarefa 0 das ferramentas carregadas
            remainingLife[j] -= executionTime[processos[0]];
        }
        // preenche o vetor magazine
		for (unsigned i = 0; i < processos.size(); i++) {
            magazine[j][i] = matrix[j][processos[i]];
            if (debug) {
                cout << magazine[j][i] << " ";
            }
        }
        if (debug) {
            cout << endl;
        }
	}
	// Preenche a matriz de prioridades
	for (unsigned i = 0; i < m; ++i){
		for (unsigned j = 0; j < processos.size(); ++j){
			if (magazine[i][j]==1)
				prioridades[i][j] = 0;
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
					prioridades[i][j] = proxima;
				else
					prioridades[i][j] = -1;
			}
		}
	}

    for(unsigned i = 1; i < processos.size(); ++i) { // i = tarefa
        int count = 0;
        for(unsigned j = 0; j < m; ++j) { // j = ferramenta
            // troca preditiva por desgaste
            if(magazine[j][i] == 1 && carregadas[j] == 1) {
                if(remainingLife[j] < executionTime[processos[i]]) {
                    trocas++;
                    remainingLife[j] = toolLife[j];
                }
            }
            // carrega as ferramentas necessárias para a próxima tarefa
            if((magazine[j][i] == 1) && (carregadas[j] == 0)) {
                carregadas[j] = 1;
                ++u;
                ++count;
                // essa ferramenta não tem tempo de vida o suficiente para a tarefa? pega outra
                if(remainingLife[j] < executionTime[i]) {
                    remainingLife[j] = toolLife[j];
                }
            }
        }

        if(u <= c) {
            trocas += count;
        } else {
            while(u > c) {
                int remove = -1;
                bool removed = false;

                for(unsigned j = 0; j < m; ++j) {
                    if(carregadas[j] == 1 && magazine[j][i] != 1 && prioridades[j][i] == -1) {
                        remove = j;
                        removed = true;
                        break;
                    }
                }

                if(!removed) {
                    int min = numeric_limits<int>::max();
                    int index;

                    for(unsigned j = 0; j < m; ++j) {
                        if(carregadas[j] == 1 && magazine[j][i] != 1 && prioridades[j][i] > 0) {
                            // próxima vez que a ferramenta j será usada
                            int nextIndex = i + prioridades[j][i];

                            int current = remainingLife[j] - executionTime[processos[nextIndex]];

                            if(current < min) {
                                min = current;
                                index = j;
                            }

                        }
                    }

                    // se a ferramenta index não tiver tempo suficiente para realizar a sua próxima tarefa, remove ela de uma vez
                    if(min <= 0) {
                        remove = index;
                        remainingLife[index] = toolLife[index];
                    } else {
                        min = 0;
                        for (unsigned j = 0; j < m; ++j) {
                            if (magazine[j][i] != 1){
                                if ((prioridades[j][i] > min) && carregadas[j] == 1) {
                                    min = prioridades[j][i];
                                    remove = j;
                                }
                            }
                        }
                    }
                }

                carregadas[remove] = 0;
                --u;
                trocas++;
            }
        }

        for(unsigned j = 0; j < m; ++j) {
            if(magazine[j][i] == 1 && carregadas[j] == 1) {
                remainingLife[j] -= executionTime[processos[i]];
            }
        }
    }

    return trocas;

}