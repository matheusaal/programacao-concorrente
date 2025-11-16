/* Disciplina: Programacao Concorrente */
/* Prof.: Silvana Rossetto */
/* Laboratório: 11 */
/* Codigo: Criando um pool de threads em Java */

import java.util.LinkedList;

/**
 * FilaTarefas - Pool de threads para executar tarefas Runnable.
 * 
 * Cria um número fixo de threads que ficam esperando tarefas para executar.
 * As tarefas são colocadas numa fila e as threads pegam e executam conforme disponível.
 */
class FilaTarefas {
    private final int nThreads;              // Número de threads no pool
    private final MyPoolThreads[] threads;   // Threads trabalhadoras
    private final LinkedList<Runnable> queue; // Fila de tarefas
    private boolean shutdown;                 // Se o pool foi encerrado

    // Construtor: cria e inicia as threads do pool
    public FilaTarefas(int nThreads) {
        this.shutdown = false;
        this.nThreads = nThreads;
        queue = new LinkedList<Runnable>();
        threads = new MyPoolThreads[nThreads];
        for (int i=0; i<nThreads; i++) {
            threads[i] = new MyPoolThreads();
            threads[i].start();
        } 
    }

    // Adiciona uma tarefa à fila para ser executada
    // Usa synchronized pois várias threads podem chamar execute ao mesmo tempo
    public void execute(Runnable r) {
        synchronized(queue) {
            if (this.shutdown) return;
            queue.addLast(r);
            queue.notify();
        }
    }
    
    // Encerra o pool: sinaliza shutdown e espera todas as threads terminarem
    public void shutdown() {
        synchronized(queue) {
            this.shutdown=true;
            queue.notifyAll();
        }
        for (int i=0; i<nThreads; i++) {
          try { threads[i].join(); } catch (InterruptedException e) { return; }
        }
    }

    // Thread trabalhadora: fica em loop pegando e executando tarefas da fila
    private class MyPoolThreads extends Thread {
       public void run() {
         Runnable r;
         while (true) {
           synchronized(queue) {
             while (queue.isEmpty() && (!shutdown)) {
               try { queue.wait(); }
               catch (InterruptedException ignored){}
             }
             if (queue.isEmpty()) return;   
             r = (Runnable) queue.removeFirst();
           }
           try { r.run(); }
           catch (RuntimeException e) {}
         } 
       } 
    } 
}
//-------------------------------------------------------------------------------

// Passo 1: Cria uma classe que implementa a interface Runnable 
class Hello implements Runnable {
   String msg;
   public Hello(String m) { msg = m; }

   // Método executado pela thread
   public void run() {
      System.out.println(msg);
   }
}

class Primo implements Runnable {
    long numero;
    
    // Construtor da classe
    public Primo(long n) { numero = n; }

    // Função para determinar se um número é primo
    public boolean ehPrimo(long n) {
      if(n <= 1) return false;
      if(n == 2) return true;
      if(n % 2 == 0) return false;
      long limite = (long) Math.sqrt(n) + 1;
      for(long i = 3; i < limite; i += 2) {
        if(n % i == 0) return false;
      }
      return true;
    }

    public void run() {
      System.out.printf("O número %d %s%n", numero, ehPrimo(numero) ? "é primo!" : "não é primo!");
    }
}

// Classe da aplicação (método main)
class Atividade1 {
    private static final int NTHREADS = 10;

    public static void main (String[] args) {
      // Passo 2: Cria o pool de threads
      FilaTarefas pool = new FilaTarefas(NTHREADS); 
      
      // Passo 3: Dispara a execução dos objetos runnable usando o pool de threads
      for (int i = 0; i < 25; i++) {
        // final String m = "Hello da tarefa " + i;
        // Runnable hello = new Hello(m);
        // pool.execute(hello);
        Runnable primo = new Primo(i);
        pool.execute(primo);
      }

      // Passo 4: Esperar pelo termino das threads
      pool.shutdown();
      System.out.println("Terminou");
   }
}
