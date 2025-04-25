#include <iostream>
#include <vector>
#include <fstream>
#include <numeric>
#include <string>
#include <algorithm>

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
vector<vector<int>> historico;
int trocas = 0;

void KTNS(const vector<int> processos, bool debug);
void wear(bool debug);

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

	KTNS(processos, false);
	wear(true);

	cout << endl << endl;

	cout << trocas;

	return 0;
}

void wear(bool debug = false) {
	vector<int> remaining = toolLife;

	for(int i = 0; i < m; ++i) {
		for(int j = 0; j < n; ++j) {
			if(historico[i][j] == 1) {
				// Troca por desgaste
				if(remaining[i] < executionTime[j]) {
					remaining[i] = toolLife[i];
					trocas++;
				}
				remaining[i] -= executionTime[j];
			}
		}
	}
}

void KTNS(const vector<int> processos, bool debug = false) {

	vector<int> carregadas(m,0);
	int u = 0; // ferramentas no magazine
	vector<vector<int>> prioridades(m, vector<int>(processos.size()));
	vector<vector<int>> magazine(m, vector<int>(n));
	historico.resize(m, vector<int>(n));

	if (debug) {
		cout << endl << "Matriz de Ferramentas no KTNS" << endl;
		for (unsigned j = 0; j < m; j++){
			for (unsigned i = 0; i<8; ++i){
				cout << matrix[j][i] << " ";
			}
			cout << endl;
		}
		cout << " --------------------- " << endl;
		cout << "Processos" << endl;
		for (unsigned i = 0; i < processos.size(); i++) {
			cout<< processos[i] << " ";
		}
		cout << endl;
		cout << endl;
	}

	for (unsigned j = 0; j < m; j++) {
		carregadas[j]=matrix[j][processos[0]];
		if (matrix[j][processos[0]]==1)
			++u;

		for (unsigned i =0; i<processos.size(); i++) {
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
		for (unsigned j=0; j < processos.size(); ++j){
			if (magazine[i][j]==1)
				prioridades[i][j] = 0;
			else {
				int proxima = 0;
				bool usa = false;
				for (unsigned k = j + 1;k < processos.size(); ++k){
					++proxima;
					if (magazine[i][k] == 1){
						usa = true;
						break;
					}
				}
				if (usa)
					prioridades[i][j]=proxima;
				else
					prioridades[i][j]=-1;
			}
		}
	}
	if (debug) {
		for (unsigned j = 0; j < m; j++) {
			for (unsigned i =0; i < processos.size(); i++) {
					cout << prioridades[j][i] << " ";
				}
				cout << endl;
		}

		cout << "Ferramentas carregadas: " << endl;
		for (unsigned j = 0; j < m; j++) {
			cout << carregadas[j] << endl;
		}
	}

	for(int j = 0; j < m; ++j)
		historico[j][0] = carregadas[j];

	// Calcula as trocas
	if (debug) {
		cout << u << " carregadas na primeira tarefa" << endl;
	}

	for (unsigned i = 1; i < processos.size(); ++i) {
		for (unsigned j = 0; j < m; ++j){
			if ((magazine[j][i] == 1) && (carregadas[j] == 0)){
				carregadas[j] = 1;
				++u;
			}
		}
		if (debug) {
			cout << u << " Ferramentas carregadas" << endl;
		}
		while (u > c){
			int maior = 0;
			int pMaior = -1;
			for (unsigned j = 0; j < m; ++j) {
				
				if (magazine[j][i] != 1){ // Ferramenta não utilizada pelo processo atual
					if ((carregadas[j] == 1) && (prioridades[j][i] == -1)) { // Essa ferramenta não será mais utilizada e é um excelente candidato a remoção
						pMaior = j;
						break;
					} else {
						if ((prioridades[j][i] > maior) && carregadas[j] == 1) {
							maior = prioridades[j][i];
							pMaior = j;
						}
					}
				}
			}
			carregadas[pMaior] = 0;
			if (debug) {
				cout << "Retirou " << i << ":" << pMaior << endl;
			}
			--u;
			++trocas;
			if (debug) {
				cout << trocas << " trocas " << endl;
			}
		}

		for(int k = 0; k < m; ++k)
			historico[k][i] = carregadas[k];
		
		if (debug) {
			cout << "Ferramentas carregadas: " << endl;
			for (unsigned j = 0; j < m; j++) {
					cout << carregadas[j] << endl;
			}
		}
	}
	if (debug) {
		cout << ": " << trocas << "trocas" << endl;
	}

	trocas += c;
}