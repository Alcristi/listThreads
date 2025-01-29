#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_VEICULOS_POR_DIRECAO 10 
#define CAPACIDADE_PONTE 3     

/*
Nossa dupla escolheu utilizar um algoritmo de escalonamento baseado em prioridade dinâmica com alternância de direção.
As direções podem ser 0 ou 1. A capacidade da ponte e número de veículos foi definido de forma genérica nas
funções e pode ser alterada diretamente no #define. 
A lib unistd foi utilizada para a função sleep e a lib stdlib para a função rand.
*/

typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t cond[2]; // cond[0] para direção 0, cond[1] para direção 1
    int carrosNaPonte;      // Número atual de carros na ponte
    int direcaoAtual;       // Direção atual da ponte (0 ou 1)
    int esperando[2];       // Carros esperando em cada direção
} ponte_t;

void atravessarPonte(int id, int direcao) {
    printf("Veículo %d (direção %d) está atravessando a ponte...\n", id, direcao);
    sleep(rand() % 3 + 1); // Usamos um sleep aleatório para simular o tempo de travessia
}

void *veiculo(void *arg) {
    ponte_t *ponte = (ponte_t *)((void **)arg)[0];
    int id = *((int *)((void **)arg)[1]);
    int direcao = *((int *)((void **)arg)[2]);

    pthread_mutex_lock(&ponte->mutex);

    // Marca que este veículo está esperando
    ponte->esperando[direcao]++;

    // Aguarda condições para acessar a ponte:
    while (ponte->carrosNaPonte == CAPACIDADE_PONTE || 
           (ponte->carrosNaPonte > 0 && ponte->direcaoAtual != direcao)) {
        pthread_cond_wait(&ponte->cond[direcao], &ponte->mutex);
    }

    // Entra na ponte
    ponte->esperando[direcao]--;
    ponte->carrosNaPonte++;
    ponte->direcaoAtual = direcao;
    printf("Veículo %d (direção %d) entrou na ponte. Carros na ponte: %d\n", id, direcao, ponte->carrosNaPonte);
    pthread_mutex_unlock(&ponte->mutex);

    // Atravessa a ponte
    atravessarPonte(id, direcao);
    pthread_mutex_lock(&ponte->mutex);

    // Sai da ponte
    ponte->carrosNaPonte--;
    printf("Veículo %d (direção %d) saiu da ponte. Carros na ponte: %d\n", id, direcao, ponte->carrosNaPonte);

    // Se não há mais carros na ponte, sinaliza a direção oposta (fairness)
    if (ponte->carrosNaPonte == 0) {
        int outraDirecao = 1 - direcao; // Direção oposta
        if (ponte->esperando[outraDirecao] > 0) {
            pthread_cond_broadcast(&ponte->cond[outraDirecao]);
        } else if (ponte->esperando[direcao] > 0) {
            pthread_cond_broadcast(&ponte->cond[direcao]);
        }
    }

    pthread_mutex_unlock(&ponte->mutex);

    return NULL;
}

int main() {

    srand(time(NULL));

    // Inicializa a ponte
    ponte_t ponte;
    pthread_mutex_init(&ponte.mutex, NULL);
    pthread_cond_init(&ponte.cond[0], NULL);
    pthread_cond_init(&ponte.cond[1], NULL);
    ponte.carrosNaPonte = 0;
    ponte.direcaoAtual = -1; // Nenhuma direção, inicialmente
    ponte.esperando[0] = ponte.esperando[1] = 0;

    // Cria threads para os veículos
    pthread_t threads[NUM_VEICULOS_POR_DIRECAO * 2];
    void *args[NUM_VEICULOS_POR_DIRECAO * 2][3];
    for (int i = 0; i < NUM_VEICULOS_POR_DIRECAO * 2; i++) {
        int *id = malloc(sizeof(int));
        int *direcao = malloc(sizeof(int));
        *id = i;
        *direcao = i < NUM_VEICULOS_POR_DIRECAO ? 0 : 1; // Primeiros N veículos na direção 0, depois na direção 1

        args[i][0] = &ponte;
        args[i][1] = id;
        args[i][2] = direcao;

        pthread_create(&threads[i], NULL, veiculo, args[i]);
        usleep(100000); // Simula intervalo de chegada dos veículos
    }

    // Aguarda todas as threads terminarem
    for (int i = 0; i < NUM_VEICULOS_POR_DIRECAO * 2; i++) {
        pthread_join(threads[i], NULL);
        free(args[i][1]); 
        free(args[i][2]); 
    }

    pthread_mutex_destroy(&ponte.mutex);
    pthread_cond_destroy(&ponte.cond[0]);
    pthread_cond_destroy(&ponte.cond[1]);

    return 0;
}
