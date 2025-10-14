#!/bin/bash
# Script para testar o lab-8 com diferentes parâmetros

# Compila o programa
gcc -o lab-8 lab-8.c -lpthread -lm

# Contador de testes
num_teste=0

# Função para executar um teste múltiplas vezes e verificar corretude
executar_teste() {
    local n=$1
    local m=$2
    local threads=$3
    local repeticoes=20
    local sucessos=0
    local falhas=0
    local vencedoras=()

    ((num_teste++))

    echo "Teste $num_teste: N=$n, M=$m, Threads=$threads"

    for i in $(seq 1 $repeticoes); do
        
        # Captura a saída do programa
        output=$(./lab-8 $n $m $threads 2>&1)

        # Extrai os números de primos encontrados
        concorrente=$(echo "$output" | grep "O algoritmo concorrente encontrou" | grep -o '[0-9]\+ primos' | grep -o '[0-9]\+')
        sequencial=$(echo "$output" | grep "O algoritmo sequencial encontrou" | grep -o '[0-9]\+ primos' | grep -o '[0-9]\+')
        
        # Extrai a thread vencedora
        vencedora=$(echo "$output" | grep "Thread .* é a vencedora!" | grep -o 'Thread [0-9]\+' | grep -o '[0-9]\+')
        vencedoras+=("$vencedora")

        # Se os valores forem iguais, versão concorrente funcionou
        if [ "$concorrente" == "$sequencial" ]; then
            ((sucessos++))
        else
            ((falhas++))
        fi
    done

    echo "Número de primos encontrados: $concorrente"
    echo "Vencedoras: ${vencedoras[*]}"
    echo " - Sucessos: $sucessos/$repeticoes"
    echo " - Falhas: $falhas/$repeticoes"
    if [ $falhas -eq 0 ]; then
        echo "Status: Sucesso"
    else
        echo "Status: Falhou"
    fi
    echo ""
}

# Variando N (tamanho da sequência)
echo ""
executar_teste 50 10 2
executar_teste 100 10 2
executar_teste 500 10 2
executar_teste 1000 10 2
executar_teste 5000 50 2

# Variando M (tamanho do buffer)
echo ""
executar_teste 1000 5 4
executar_teste 1000 25 4
executar_teste 1000 50 4
executar_teste 1000 100 4
executar_teste 1000 500 4

# Variando número de threads
echo ""
executar_teste 1000 50 1
executar_teste 1000 50 2
executar_teste 1000 50 4
executar_teste 1000 50 8
executar_teste 1000 50 16
