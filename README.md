# Keep Tools Needed Soonest (KTNS)

O KTNS disponibilizado neste repositório é para resolução do subproblema de ferramentas (Tooling Problem, TP) ([Paiva e Carvalho, 2017](https://doi.org/10.1016/j.cor.2017.07.013)) do Problema de Sequenciamento de Tarefas e Trocas de Ferramentas com Desgaste de Ferramentas (Job Sequencing and Tool Switching Problem with Tool Wear, SSP-TW), uma variação do SSP uniforme ([Tang e Denardo, 1988](https://www.jstor.org/stable/171322)).

Esse código foi adaptado do KTNS disponibilizado por [Soares e Carvalho (2020)](https://doi.org/10.1016/j.ejor.2020.02.047) para ser executado corretamente no Windows.

As instâncias usadas para teste nesse protótipo foram retiradas da tabela 1 de [Crama et al. (1994)](https://doi.org/10.1007/BF01324874), adaptadas para os requisitos específicos do SSP-TW e estão localizadas na pasta '[input_files](https://github.com/shesfromasgard/KTNS_SSP_TW/tree/main/input_files)'. Caso queira testar outras instâncias, adicione-as à pasta de entrada. As soluções serão impressas em arquivos de texto individuais na pasta '[output_files](https://github.com/shesfromasgard/KTNS_SSP_TW/tree/main/output_files)'.

Caso tenha alguma dúvida sobre o problema ou implementação, mande-me uma mensagem no meu endereço de e-mail[^1], responderei assim que possível.

Para compilar o código, use o comando:

```
g++ main.cpp -o main.exe -std=c++17
```

Para rodar o código, use o comando:

```
python .\runner.py
```

[^1]: lauracsrocha0@gmail.com
