#Compilar e rodar o código em C
#./ex1 <palavra> <arquivo1> <arquivo2>
#<palavra vai ser pego dinamicamente pelo script

#!/bin/bash

PALAVRA=$1

if [ -z "$PALAVRA" ]; then
    echo "Para executar o programa : ./test.sh <palavra>"
    exit 1
fi

gcc -o ex1 ex1.c

./ex1 "$PALAVRA" ./file_1.txt ./file_2.txt ./file_3.txt
