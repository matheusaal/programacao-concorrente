#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Valor máximo de rand gerado
#define RAND_LIMITE 900

int escrever_arquivo_binario(int N, float *vetor_1, float* vetor_2, float prod_interno_seq, char* nome_arquivo) {
    // Abertura do arquivo
    FILE *f = fopen(nome_arquivo, "wb");
    if (f == NULL) {
        printf("\nERRO: Não foi possível abrir o arquivo binario '%s'.\n", nome_arquivo);
        return 1;
    }

    // Escrita do arquivo
    fwrite(&N, sizeof(int), 1, f);
    fwrite(vetor_1, sizeof(float), N, f);
    fwrite(vetor_2, sizeof(float), N, f);
    fwrite(&prod_interno_seq, sizeof(float), 1, f);

    // Fechamento do arquivo
    fclose(f);

    return 0;
} 


int main(int argc, char*argv[]) {
    // Geração com sinais independentes (cada elemento pode ser positivo ou negativo aleatoriamente)
    // Verifica se N foi passado como argumento na chamada do programa e valida o parâmetro
    if(argc<3) {
        printf("[ERRO] Digite <dimensao vetor> <nome arquivo saida>\n");
        return 1;
    }
    int N = atoi(argv[1]);
    if (N <= 0) {
        printf("[ERRO] Dimensao do vetor deve ser maior que 0.\n");
        return 1;
    }
    char* nome_arquivo = argv[2];

    // Variáveis que serão geradas e escritas no arquivo binário
    float vetor_1[N]; 
    float vetor_2[N];
    float prod_interno_seq = 0;

    // Gera vetores randômicos e calcula o produto interno
    srand(time(NULL));
    for(int i = 0; i < N; i++) {
        // Valor base positivo em [0, RAND_LIMITE/3)
        float elem_1 = (rand() % RAND_LIMITE) / 3.0;
        float elem_2 = (rand() % RAND_LIMITE) / 3.0;
        // Atribui sinal aleatório independente (bit menos significativo de rand())
        if (rand() & 1) elem_1 = -elem_1;
        if (rand() & 1) elem_2 = -elem_2;
        // Finalmente, atribui o elemento ao vetor e calcula a parcela do produto interno correspondente
        vetor_1[i] = elem_1;
        vetor_2[i] = elem_2;
        prod_interno_seq += elem_1 * elem_2;
    }

    // Verifica se o arquivo foi escrito sem erro
    if(escrever_arquivo_binario(N, vetor_1, vetor_2, prod_interno_seq, nome_arquivo) == 0) return 0;
    return 1;
}