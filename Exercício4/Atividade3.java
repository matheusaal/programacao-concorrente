/* Disciplina: Programacao Concorrente */
/* Prof.: Silvana Rossetto */
/* Laboratório: 11 */
/* Codigo: Exemplo de uso de futures */
/* -------------------------------------------------------------------*/

import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;

import java.util.ArrayList;
import java.util.List;

// Classe que implementa versão sequencial de contagem de primos para verificação de corretude
class PrimoSequencial {

    // Função para determinar se um numero é primo
    public static boolean ehPrimo(long n) {
      if(n <= 1) return false;
      if(n == 2) return true;
      if(n % 2 == 0) return false;
      long limite = (long) Math.sqrt(n) + 1;
      for(long i = 3; i < limite; i += 2) {
        if(n % i == 0) return false;
      }
      return true;
    }

    // Função para contar o número de primos de 1 até N
    public static long contarPrimos(int N) {
        long qtd_primos = 0;
        for (int i = 0; i < N; i++) {
            if (ehPrimo(i)) {
                qtd_primos++;
            }
        }
        return qtd_primos;
    }
}

// Classe que implementa versão concorrente e assíncrona de contagem de primos
class PrimoConcorrente implements Callable<Boolean> {
    public long numero;

    // Construtor da classe
    public PrimoConcorrente(long n) { numero = n; }

    // Função para determinar se um numero e primo
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

    // Função que obtém o resultado de primalidade de um número e printa na tela
    public Boolean call() throws Exception {
        Boolean resultado = ehPrimo(numero);
        System.out.printf("O número %d %s%n", numero, ehPrimo(numero) ? "é primo!" : "não é primo!");
        return resultado;
    }
}

// Classe de execução do programa
public class Atividade3  {
  // Parâmetros do programa
  private static int N;
  private static int NTHREADS;

  public static void main(String[] args) {
    
    // Lê parâmetros de entrada do programa
    if (args.length >= 1) {
      try { N = Integer.parseInt(args[0]); }
      catch (NumberFormatException e) { System.err.println("ERRO! Formato esperado do programa: <N> <NTHREADS>"); }
    }
    if (args.length >= 2) {
      try { NTHREADS = Integer.parseInt(args[1]); }
      catch (NumberFormatException e) { System.err.println("ERRO! Formato esperado do programa: <N> <NTHREADS>"); }
    }
    if (N <= 0 || NTHREADS <= 0) {
      System.err.println("ERRO! N e NTHREADS devem ser inteiros positivos!");
      return;
    }
    
    // Cria um pool de threads com NTHREADS
    ExecutorService executor = Executors.newFixedThreadPool(NTHREADS);
    
    // Cria uma lista para armazenar referências de chamadas assincronas
    List<Future<Boolean>> list = new ArrayList<Future<Boolean>>();

    for (int i = 0; i < N; i++) {
      Callable<Boolean> worker = new PrimoConcorrente(i);
      Future<Boolean> submit = executor.submit(worker);
      list.add(submit);
    }

    // Recupera os resultados e faz o somatório final
    long qtd_primos = 0;
    for (Future<Boolean> future : list) {
      try {
        // Se o resultado for true, somamos um na quantidade de primos
        if (future.get()) qtd_primos++; //bloqueia se a computação nao tiver terminado
      } catch (InterruptedException e) {
        e.printStackTrace();
      } catch (ExecutionException e) {
        e.printStackTrace();
      }
    }
    System.out.println(qtd_primos);
    executor.shutdown();
    
    // Verifica corretude comparando com versão sequencial
    long qtd_primos_seq = PrimoSequencial.contarPrimos(N);
    if (qtd_primos == qtd_primos_seq) {
      System.out.println("CORRETO! Resultado concorrente = sequencial = " + qtd_primos);
    } else {
      System.out.println("ERRO! Concorrente: " + qtd_primos + " | Sequencial: " + qtd_primos_seq);
    }
  }
}
