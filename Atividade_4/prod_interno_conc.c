#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include "timer.h"

// Variáveis globais para armazenar os dados lidos do arquivo
// Dimensão dos vetores
int N = 0;             
// Primeiro Vetor
float *vetor_1 = NULL;  
// Segundo vetor
float *vetor_2 = NULL;  
// Produto interno sequencial
float prod_interno_seq = 0.0; 

// Estrutura para passar os dados para as threads
typedef struct {
    // Tamanho do vetor
    long int n;
    // Número de threads
    short int n_threads;
    // Identificador da threads
    short int id_thread; 
} t_Args;

// Lê o arquivo binário que geramos com o programa anterior
int ler_arquivo_binario(char* caminho) {
    FILE *f = fopen(caminho, "rb");
    if (f == NULL) {
        fprintf(stderr, "\n[ERRO] Não foi possível abrir o arquivo binário '%s'.\n", caminho);
        return 1;
    }

    // Lê N
    if (fread(&N, sizeof(int), 1, f) != 1) {
        fprintf(stderr, "[ERRO] Falha ao ler N do arquivo.\n");
        fclose(f);
        return 1;
    }

    // Aloca vetores
    vetor_1 = (float*) malloc(sizeof(float) * N);
    vetor_2 = (float*) malloc(sizeof(float) * N);
    if (vetor_1 == NULL || vetor_2 == NULL) {
        fprintf(stderr, "[ERRO] Falha na alocação de memória para os vetores.\n");
        fclose(f);
        free(vetor_1); 
        free(vetor_2);
        return 1;
    }

    // Lê vetor_1
    if (fread(vetor_1, sizeof(float), N, f) != (size_t)N) {
        fprintf(stderr, "[ERRO] Falha ao ler vetor_1.\n");
        fclose(f);
        free(vetor_1); free(vetor_2);
        return 1;
    }

    // Lê vetor_2
    if (fread(vetor_2, sizeof(float), N, f) != (size_t)N) {
        fprintf(stderr, "[ERRO] Falha ao ler vetor_2.\n");
        fclose(f);
        free(vetor_1); 
        free(vetor_2);
        return 1;
    }

    // Lê produto interno
    if (fread(&prod_interno_seq, sizeof(float), 1, f) != 1) {
        fprintf(stderr, "[ERRO] Falha ao ler produto interno.\n");
        fclose(f);
        free(vetor_1); 
        free(vetor_2);
        return 1;
    }

    fclose(f);
    return 0;
}

// Task atribuida para cada thread
// Basicamente define um escopo por igual para cada thread, com exceção da última que fica com todo o final caso não feche o múltiplo.
void* ProdutoInternoVetor(void* arg) { 
    t_Args* args = (t_Args*) arg; // Argumentos da thread
    float* somaLocal = malloc(sizeof(float)); // Nossa variável de interesse
    if (somaLocal == NULL) { free(args); pthread_exit(NULL); }
    // Variáveis auxiliares para dividir os vetores em blocos
    *somaLocal = 0.0;
    int bloco = args->n / args->n_threads;
    int inicio = args->id_thread * bloco;
    int fim = inicio + bloco;

    if (args->id_thread == args->n_threads-1) fim = args->n;
    
    for (int i = inicio; i < fim; i++) {
        *somaLocal += vetor_1[i] * vetor_2[i];
    }
    free(args);
    pthread_exit((void*) somaLocal);
}

int main(int argc, char *argv[]) {
    short int n_threads;
    char* caminho_arquivo;    
    float* retorno_thread;
    float prod_interno_conc = 0;

    // Recebe e valida os parâmetros de entrada
    if(argc < 3) {
        printf("[ERRO] Entrada invalida.\nDigite: <qtd de threads> <caminho do arquivo>.\n");
        return 1;
    }
    n_threads = atoi(argv[1]);
    if (n_threads <= 0) {
        printf("[ERRO] Numero de threads deve ser maior que 0.\n");
        return 1;
    }
    caminho_arquivo = argv[2];

    // Lê dados do arquivo binário (preenche N, vetor_1, vetor_2, produtoInterno globais)
    if(ler_arquivo_binario(caminho_arquivo) != 0) return 1;

    // Variáveis para calcular o tempo de execução do produto interno
    double inicio_calculo, fim_calculo;
    // Instante inicial do cálculo do produto interno
    GET_TIME(inicio_calculo); 

    // Cria as threads
    pthread_t tids[n_threads];
    for(int i = 0; i < n_threads; i++) {
        t_Args *args = malloc(sizeof(t_Args));
        if(args==NULL) {    
            printf("[ERRO]: Falha no malloc.\n"); 
            return 1;
        }
        args->n = N;
        args->n_threads = n_threads;
        args->id_thread = i;

        if (pthread_create(&tids[i], NULL, ProdutoInternoVetor, (void*) args)) {
            printf("[ERRO] pthread_create() da thread %d\n", i); 
            return 1;
        }
    }

    // Espera as threads terminarem e soma cada resultado individual à nossa soma global
    for (int i = 0; i < n_threads; i++) {
        if (pthread_join(tids[i], (void*) &retorno_thread)) {
            printf("[ERRO] pthread_join() da thread %d\n", i);
            return 1;
        }
        prod_interno_conc += *retorno_thread;
        // Libera cada somaLocal que foi alocada na task
        free(retorno_thread); 
    }

    // Instante final do cálculo do produto interno
    GET_TIME(fim_calculo); 

    // Libera cada vetor alocado dinamicamente lido do arquivo binário
    free(vetor_1);
    free(vetor_2);

    // Printa nossos resultados
    printf("Produto interno sequencial: %f\n", prod_interno_seq);
    printf("Produto interno concorrente: %f\n", prod_interno_conc);
    printf("Variacao relativa: %.6f\n", fabs((prod_interno_seq - prod_interno_conc) / prod_interno_seq));
    printf("Tempo de execucao do produto interno(ms): %e\n", (fim_calculo - inicio_calculo)*1000);
    return 0;
}