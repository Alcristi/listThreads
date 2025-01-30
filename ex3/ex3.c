#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_VEHICLES_PER_DIRECTION 10
#define MAX_BRIDGE_CAPACITY 3     

/*
Nossa dupla escolheu utilizar um algoritmo de escalonamento baseado em prioridade dinâmica com alternância de direção.
As direções podem ser 0 ou 1. A capacidade da ponte e número de veículos foi definido de forma genérica nas
funções e pode ser alterada diretamente no #define. 
A lib unistd foi utilizada para a função sleep e a lib stdlib para a função rand.
*/

typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t cond[2]; // cond[0] para direção 0, cond[1] para direção 1
    int carsOnBridge;       // Número atual de carros na ponte
    int currentDirection;   // Direção atual da ponte (0 ou 1)
    int waiting[2];         // Carros esperando em cada direção
} bridge_t;

void crossBridge(int id, int direction) {
    printf("Veículo %d (direção %d) está atravessando a ponte...\n", id, direction);
    sleep(rand() % 3 + 1); // Usamos um sleep aleatório para simular o tempo de travessia
}

void *vehicle(void *arg) {
    bridge_t *bridge = (bridge_t *)((void **)arg)[0];
    int id = *((int *)((void **)arg)[1]);
    int direction = *((int *)((void **)arg)[2]);

    pthread_mutex_lock(&bridge->mutex);

    // Marca que este veículo está esperando
    bridge->waiting[direction]++;

    // Aguarda condições para acessar a ponte:
    while (bridge->carsOnBridge == MAX_BRIDGE_CAPACITY ||
           (bridge->carsOnBridge > 0 && bridge->currentDirection != direction)) {
        pthread_cond_wait(&bridge->cond[direction], &bridge->mutex);
    }

    // Entra na ponte
    bridge->waiting[direction]--;
    bridge->carsOnBridge++;
    bridge->currentDirection = direction;
    printf("Veículo %d (direção %d) entrou na ponte. Carros na ponte: %d\n", id, direction, bridge->carsOnBridge);
    pthread_mutex_unlock(&bridge->mutex);

    // Atravessa a ponte
    crossBridge(id, direction);
    pthread_mutex_lock(&bridge->mutex);

    // Sai da ponte
    bridge->carsOnBridge--;
    printf("Veículo %d (direção %d) saiu da ponte. Carros na ponte: %d\n", id, direction, bridge->carsOnBridge);

    // Se não há mais carros na ponte, sinaliza a direção oposta (fairness)
    if (bridge->carsOnBridge == 0) {
        int otherDirection = 1 - direction; // Direção oposta
        if (bridge->waiting[otherDirection] > 0) {
            pthread_cond_broadcast(&bridge->cond[otherDirection]);
        } else if (bridge->waiting[direction] > 0) {
            pthread_cond_broadcast(&bridge->cond[direction]);
        }
    }

    pthread_mutex_unlock(&bridge->mutex);

    return NULL;
}

int main() {

    srand(time(NULL));

    // Inicializa a ponte
    bridge_t bridge;
    pthread_mutex_init(&bridge.mutex, NULL);
    pthread_cond_init(&bridge.cond[0], NULL);
    pthread_cond_init(&bridge.cond[1], NULL);
    bridge.carsOnBridge = 0;
    bridge.currentDirection = -1; // Nenhuma direção, inicialmente
    bridge.waiting[0] = bridge.waiting[1] = 0;

    // Cria threads para os veículos
    pthread_t threads[NUM_VEHICLES_PER_DIRECTION * 2];
    void *args[NUM_VEHICLES_PER_DIRECTION * 2][3];
    for (int i = 0; i < NUM_VEHICLES_PER_DIRECTION * 2; i++) {
        int *id = malloc(sizeof(int));
        int *direction = malloc(sizeof(int));
        *id = i;
        *direction = i < NUM_VEHICLES_PER_DIRECTION ? 0 : 1; // Primeiros N veículos na direção 0, depois na direção 1

        args[i][0] = &bridge;
        args[i][1] = id;
        args[i][2] = direction;

        pthread_create(&threads[i], NULL, vehicle, args[i]);
        usleep(100000); // Simula intervalo de chegada dos veículos
    }

    // Aguarda todas as threads terminarem
    for (int i = 0; i < NUM_VEHICLES_PER_DIRECTION * 2; i++) {
        pthread_join(threads[i], NULL);
        free(args[i][1]);
        free(args[i][2]);
    }

    pthread_mutex_destroy(&bridge.mutex);
    pthread_cond_destroy(&bridge.cond[0]);
    pthread_cond_destroy(&bridge.cond[1]);

    return 0;
}
