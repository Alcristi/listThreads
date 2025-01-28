#Compilar e rodar o código em C
#./ex1 <palavra> <arquivo1> <arquivo2>
#<palavra vai ser pego dinamicamente pelo script

PALAVRA=$1
gcc -o ex1 ex1.c
./ex1  $PALAVRA ./file_1.txt ./file_2.txt ./file_3.txt