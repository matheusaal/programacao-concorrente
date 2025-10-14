#!/bin/bash
# Script para testar o lab-8 com diferentes parâmetros

# Contador de testes
num_teste=0

# Função para executar um teste múltiplas vezes e verificar corretude
executar_teste() {
    local n=$1
    local m=$2
    local threads=$3
    local repeticoes=10
    local sucessos=0
    local falhas=0

    ((num_teste++))

    echo "=========================================="
    echo "Teste $num_teste: N=$n, M=$m, Threads=$threads"

    for i in $(seq 1 $repeticoes); do
        # echo "Execução $i/$repeticoes..."

        # Captura a saída do programa
        output=$(./lab-8 $n $m $threads 2>&1)

        # Extrai os números de primos encontrados
        concorrente=$(echo "$output" | grep "O algoritmo concorrente encontrou" | grep -o '[0-9]\+ primos' | grep -o '[0-9]\+')
        sequencial=$(echo "$output" | grep "O algoritmo sequencial encontrou" | grep -o '[0-9]\+ primos' | grep -o '[0-9]\+')

        # Se os valores forem iguais, versão concorrente funcionou
        if [ "$concorrente" == "$sequencial" ]; then
            ((sucessos++))
        else
            ((falhas++))
        fi
    done

    echo "Número de primos encontrados: $concorrente"
    echo " - Sucessos: $sucessos/$repeticoes"
    echo " - Falhas: $falhas/$repeticoes"
    if [ $falhas -eq 0 ]; then
        echo "Status: ✓ PASSOU"
    else
        echo "Status: ✗ FALHOU"
    fi
    echo "=========================================="
    echo ""
}

# Variando N (tamanho da sequência)
echo ""
executar_teste 50 10 2
executar_teste 100 10 2
executar_teste 500 10 2
executar_teste 1000 10 2
executar_teste 5000 50 4

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