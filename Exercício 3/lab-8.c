#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <math.h>

#define LIMITE 100

sem_t bufferPreenchido, bufferVazio, mutexCons;
int n, m, nthreads, *Buffer, *Sequencia;

// Função auxiliar que verifica primalidade dada pela professora
int ehPrimo(long long int n) {
    int i;
    if (n<=1) return 0;
    if (n==2) return 1;
    if (n%2==0) return 0;
    for (i=3; i<sqrt(n)+1; i+=2)
        if(n%i==0) return 0;
    return 1;
}

// Função auxiliar para gerar um array de n inteiros aleatórios
int* gerarSequencia(int n) {
    int* sequencia = malloc(n * sizeof(int));
    if (!sequencia) return NULL;
    
    // Inicializa gerador de números aleatórios
    srand(time(NULL));

    for (int i = 0; i < n; i++) {
        sequencia[i] = rand() % (LIMITE + 1);  // Números aleatórios de 0 a LIMITE
    }
    return sequencia;
}

// Função auxiliar para checar corretude do algorítmo concorrente
int checagemSequencial(int* sequencia) {
    int totalPrimos = 0;
    for (int i = 0; i < n; i++) {
        if (ehPrimo(sequencia[i])) totalPrimos++;
    }
    return totalPrimos;
}

void *Produtora (void *args) {
    int inicio = 0;
    while (inicio < n) {
        // Só podemos produzir quando existe sinal no bufferVazio
        sem_wait(&bufferVazio);

        // Primeiro geramos a sequência de n inteiros, depois definimos a parte dessa sequência que entrará no buffer
        int fim = inicio + m;
        if (fim > n) fim = n; 
        int quantidade = fim - inicio;

        for(int i = inicio, j = 0; i < fim && j < m; i++, j++) {
            Buffer[j] = Sequencia[i];
        }

        // A próxima iteração da produtora precisa começar de onde a outra terminou
        inicio = fim;

        // Liberamos sinais de acordo com quantos inteiros colocamos no buffer
        for (int i = 0; i < quantidade; i++)
            sem_post(&bufferPreenchido);
    }
    // No final da produção, teremos as threads aguardando o bufferPreenchido mas ele estará sem sinal pois acaboram os números
    // Assim, enviamos sinais extras para elas acordarem e verificarem que não precisam mais consumir o buffer
    for (int i = 0; i < nthreads; i++) sem_post(&bufferPreenchido);
    pthread_exit(NULL);
}

void *Consumidora (void *args) {
    // Variável global que armazena o índice do buffer que será consumido
    static int out = 0;
    static int elementosConsumidos = 0;
    // Variável que guarda o número de primos encontrados pela thread
    int* primosEncontrados = malloc(sizeof(int));
    if (!primosEncontrados) pthread_exit(NULL);
    *primosEncontrados = 0;
    
    while (1) {
    
        // Só podem passar x consumidoras
        // x = número de inteiros colocados no buffer
        sem_wait(&bufferPreenchido);

        // Só passa 1 consumidora por vez (seção crítica)
        sem_wait(&mutexCons);

        // Se todos os elementos já foram consumidos, libera e sai
        if (elementosConsumidos >= n) {
            sem_post(&mutexCons);
            break;
        }

        // Senão, removemos o inteiro do buffer e verificamos se é primo
        int inteiroRemovido = Buffer[out];
        if (ehPrimo(inteiroRemovido)) {
            *primosEncontrados = *primosEncontrados + 1;
        }
        out = (out + 1) % m;
        elementosConsumidos++;

        // Se for a última consumidora, libera mais uma iteração da produtora
        if (out == 0) sem_post(&bufferVazio);

        sem_post(&mutexCons);
    }
    pthread_exit((void*) primosEncontrados);
    free(primosEncontrados);
}

int main(int argc, char *argv[]) {
    pthread_t *tid;
    int* retornoThread;
    int threadVencedora, maiorQtdPrimos = 0;
    int primosEncontrados = 0;

    // Parâmetros de entrada
    if(argc<4) {
        printf("Digite: %s <N> <M> <Qtd. threads>\n", argv[0]);
        return 1;
    }
    n = atoi(argv[1]);
    m = atoi(argv[2]);
    nthreads = atoi(argv[3]);

    // Alocação de estruturas
    tid = (pthread_t*) malloc(sizeof(pthread_t)*(nthreads+1));
    if(!tid) {
        puts("[ERRO] Malloc Tid"); 
        return 2;
    }

    Buffer = malloc(m * sizeof(int));
    if (!Buffer) {
        puts("[ERRO] Malloc Buffer");
        return 2;
    } 

    // Geramos uma sequência de 
    Sequencia = malloc(n * sizeof(int));
    if (!Sequencia) {
        puts("[ERRO] Malloc Sequencia");
        return 2;
    }
    Sequencia = gerarSequencia(n);

    // Inicializa os semáforos
    sem_init(&bufferPreenchido, 0, 0);
    sem_init(&bufferVazio, 0, 1);
    sem_init(&mutexCons, 0, 1);

    // Cria thread que deposita os inteiros no canal
    if (pthread_create(&tid[nthreads], NULL, Produtora, (void*)(long int)n)) {
        printf("--ERRO: pthread_create()\n");
        exit(-1);
    }

    // Cria as threads que consomem os inteiros do canal
    for(long int t=0; t<nthreads; t++) {
        if (pthread_create(&tid[t], NULL, Consumidora, (void *)t)) {
            printf("--ERRO: pthread_create()\n");
            exit(-1);
        }
    }
    
    // Espera até que as threads terminem
    for (int t=0; t<nthreads+1; t++) {
        if (pthread_join(tid[t], (void**) &retornoThread)) {
            printf("--ERRO: pthread_join() \n");
            exit(-1);
        }
        // A thread produtora retorna null
        if (retornoThread != NULL) {
            printf("Thread %d encontrou %d primos! ;)\n", t, *retornoThread);
            // Verifica se a thread encontrou mais primos que a vencedora temporária
            if (*retornoThread > maiorQtdPrimos) {
                maiorQtdPrimos = *retornoThread;
                threadVencedora = t;
            }
            primosEncontrados += *retornoThread;
            free(retornoThread);
        }
    }

    printf("\nThread %d é a vencedora!\n\nO algoritmo concorrente encontrou %d primos.\n", threadVencedora, primosEncontrados);
    printf("O algoritmo sequencial encontrou %d primos.\n", checagemSequencial(Sequencia));

    // Finaliza os semáforos
    sem_destroy(&bufferPreenchido);
    sem_destroy(&bufferVazio);
    sem_destroy(&mutexCons);

    free(tid);
    free(Buffer);
    free(Sequencia);
    
    return 0;
}