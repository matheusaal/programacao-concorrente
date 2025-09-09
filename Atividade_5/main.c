#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// Variável compartilhada entre as threads
long int soma = 0; 
// Variável de lock para exclusão mútua
pthread_mutex_t mutex; 
// Variável de condição para a thread extra
pthread_cond_t cond_extra;
// Variável de condição para as threads ExecutaTarefa 
pthread_cond_t cond_exec; 
// Flag para indicar que há múltiplo de 1000 para imprimir
int multiplo_1000_disponivel = 0;
// Flag para indicar que a thread extra terminou
int extra_terminou = 0; 

void *ExecutaTarefa (void *arg) {
    long int id = (long int) arg;
    printf("Thread : %ld esta executando...\n", id);
    
    for (int i=0; i<100000; i++) {

        // Entrada na seção crítica
        pthread_mutex_lock(&mutex);
        soma++; //incrementa a variavel compartilhada
        
        // Verifica se soma é múltiplo de 1000
        if (soma % 1000 == 0) {

            // Sinaliza que há um múltiplo de 1000 disponível
            multiplo_1000_disponivel = 1;

            // Acorda a thread extra para imprimir
            pthread_cond_signal(&cond_extra);

            // Espera a thread extra processar o múltiplo
            while (multiplo_1000_disponivel == 1 && !extra_terminou) {
                pthread_cond_wait(&cond_exec, &mutex);
            }
        }
        
        // Saída da seção crítica
        pthread_mutex_unlock(&mutex);
    }
    
    printf("Thread : %ld terminou!\n", id);
    pthread_exit(NULL);
}

void *extra (void *args) {
    printf("Extra : esta executando...\n");
    long int n_threads = (long int) args;

    // Entrada na seção crítica
    pthread_mutex_lock(&mutex);
    
    // Continua enquanto não processou todos os múltiplos esperados
    while (1) {

        // Espera por um múltiplo de 1000
        while (multiplo_1000_disponivel == 0 && soma < 100000 * n_threads) {
            pthread_cond_wait(&cond_extra, &mutex);
        }
        
        // Verifica se todas as threads terminaram
        if (soma >= 100000 * n_threads) {
            break;
        }

        // Imprime o múltiplo de 1000
        printf("Soma = %ld\n", soma);

        // Marca que o múltiplo foi processado
        multiplo_1000_disponivel = 0;
        
        // Libera as threads ExecutaTarefa para continuar
        pthread_cond_broadcast(&cond_exec);
    }
    
    // Marca que a thread extra terminou
    extra_terminou = 1;
    
    // Libera qualquer thread que esteja esperando
    pthread_cond_broadcast(&cond_exec);
    
    pthread_mutex_unlock(&mutex);
    
    printf("Extra : terminou!\n");
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    pthread_t *tid; //identificadores das threads no sistema
    int nthreads; //qtde de threads (passada linha de comando)
    
    // Parâmetros de entrada
    if(argc<2) {
        printf("Digite: %s <numero de threads>\n", argv[0]);
        return 1;
    }
    nthreads = atoi(argv[1]);
    
    // Alocação de estruturas
    tid = (pthread_t*) malloc(sizeof(pthread_t)*(nthreads+1));
    if(tid==NULL) {
        puts("ERRO--malloc"); 
        return 2;
    }
    
    // Inicializa o mutex (variável de exclusão mútua)
    pthread_mutex_init(&mutex, NULL);
    
    // Inicializa as variáveis de condição
    pthread_cond_init(&cond_extra, NULL);
    pthread_cond_init(&cond_exec, NULL);
    
    // Cria as threads que executam a tarefa
    for(long int t=0; t<nthreads; t++) {
        if (pthread_create(&tid[t], NULL, ExecutaTarefa, (void *)t)) {
            printf("--ERRO: pthread_create()\n");
            exit(-1);
        }
    }
    
    // Cria thread que loga os múltiplos de 1000
    if (pthread_create(&tid[nthreads], NULL, extra, (void*)(long int)nthreads)) {
        printf("--ERRO: pthread_create()\n");
        exit(-1);
    }
    
    // Espera até que as threads terminem
    for (int t=0; t<nthreads+1; t++) {
        if (pthread_join(tid[t], NULL)) {
            printf("--ERRO: pthread_join() \n");
            exit(-1);
        }
    }
    
    // Finaliza o mutex e as variáveis de condição
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond_extra);
    pthread_cond_destroy(&cond_exec);
    
    printf("Valor final de soma = %ld\n", soma);
    printf("Total de multiplos de 1000 esperados: %d\n", nthreads * 100);
    
    free(tid);

    return 0;
}