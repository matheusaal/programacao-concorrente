#!/bin/bash

echo "=== Teste de Corretude ==="
echo ""

cd "$(dirname "$0")"

# Compila
echo "Compilando..."
javac Atividade3.java || exit 1
echo ""

# Casos de teste: "N NTHREADS"
tests=("10 1" "10 2" "10 4" "50 1" "50 5" "50 10" "100 4" "100 8" "100 16" "500 10" "1000 1" "1000 10" "1000 20" "5000 10" "10000 20")

echo "Executando testes..."
for test in "${tests[@]}"; do
    echo "Testando: N=$(echo $test | cut -d' ' -f1), NTHREADS=$(echo $test | cut -d' ' -f2)"
    java Atividade3 $test 2>&1 | grep -E "(primos|CORRETO|ERRO)"
    echo ""
done

echo "Concluído!"
