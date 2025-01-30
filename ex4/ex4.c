#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_BUFFER 5      // Capacidade máxima da fila
#define MAX_PRODUCERS 2   // Número de threads produtoras
#define MAX_CONSUMERS 2   // Número de threads consumidoras

// Estrutura de nó da fila encadeada, dada na questão
typedef struct elem {
    int value;
    struct elem *prox;
} Elem;

//Estrutura da fila bloqueante, dada na questão, mas com a adição de um mutex e duas variáveis de condição
typedef struct blockingQueue {
    unsigned int sizeBuffer, statusBuffer;
    Elem *head, *last;
    pthread_mutex_t mutex;
    pthread_cond_t notFull, notEmpty;
} BlockingQueue;

// Função para criar uma nova fila bloqueante
BlockingQueue* newBlockingQueue(unsigned int sizeBuffer) {
    BlockingQueue* Q = (BlockingQueue*) malloc(sizeof(BlockingQueue));
    Q->sizeBuffer = sizeBuffer;
    Q->statusBuffer = 0;
    Q->head = Q->last = NULL;
    pthread_mutex_init(&Q->mutex, NULL);
    pthread_cond_init(&Q->notFull, NULL);
    pthread_cond_init(&Q->notEmpty, NULL);
    return Q;
}

// Função para adicionar um elemento na fila bloqueante
void putBlockingQueue(BlockingQueue* Q, int newValue) {
    pthread_mutex_lock(&Q->mutex);

    while (Q->statusBuffer == Q->sizeBuffer) {
        printf("Fila cheia. Produtor esperando...\n");
        pthread_cond_wait(&Q->notFull, &Q->mutex);
    }

    Elem* newElem = (Elem*) malloc(sizeof(Elem));
    newElem->value = newValue;
    newElem->prox = NULL;

    if (Q->last == NULL) {
        Q->head = Q->last = newElem;
    } else {
        Q->last->prox = newElem;
        Q->last = newElem;
    }

    Q->statusBuffer++;
    printf("[PRODUTOR] Inseriu: %d\n", newValue);

    pthread_cond_broadcast(&Q->notEmpty);  // Acorda consumidores
    pthread_mutex_unlock(&Q->mutex);
}

// Função para retirar um elemento da fila bloqueante
int takeBlockingQueue(BlockingQueue* Q) {
    pthread_mutex_lock(&Q->mutex);

    while (Q->statusBuffer == 0) {
        printf("Fila vazia. Consumidor esperando...\n");
        pthread_cond_wait(&Q->notEmpty, &Q->mutex);
    }

    Elem* temp = Q->head;
    int value = temp->value;

    Q->head = Q->head->prox;
    if (Q->head == NULL) {
        Q->last = NULL;
    }

    free(temp);
    Q->statusBuffer--;
    printf("[CONSUMIDOR] Retirou: %d\n", value);

    pthread_cond_broadcast(&Q->notFull);  // Acorda produtores
    pthread_mutex_unlock(&Q->mutex);

    return value;
}

/*Função para liberar a memória alocada pela fila bloqueante.
Apesar de neste caso termos um loop infinito, a função foi implementada para garantir
a liberação de memória em casos de uso mais genéricos.
*/
void freeBlockingQueue(BlockingQueue *Q) {
    pthread_mutex_lock(&Q->mutex);
    while (Q->head) {
        Elem *temp = Q->head;
        Q->head = Q->head->prox;
        free(temp);
    }
    pthread_mutex_unlock(&Q->mutex);

    pthread_mutex_destroy(&Q->mutex);
    pthread_cond_destroy(&Q->notFull);
    pthread_cond_destroy(&Q->notEmpty);
    free(Q);
}

// Função para os produtores
void* producer(void* arg) {
    BlockingQueue* Q = (BlockingQueue*) arg;
    while (1) {
        int value = rand() % 100;  // Gera valor aleatório
        putBlockingQueue(Q, value);
        sleep(1);  // Simula tempo de produção
    }
    return NULL;
}

// Função para os consumidores
void* consumer(void* arg) {
    BlockingQueue* Q = (BlockingQueue*) arg;
    while (1) {
        int value = takeBlockingQueue(Q);
        sleep(2);  // Simula tempo de consumo
    }
    return NULL;
}


int main() {
    const int P = MAX_PRODUCERS; 
    const int C = MAX_CONSUMERS;
    const int B = MAX_BUFFER;

    BlockingQueue* Q = newBlockingQueue(B);

    pthread_t producers[P], consumers[C];

    for (int i = 0; i < P; i++) {
        pthread_create(&producers[i], NULL, producer, Q);
    }

    for (int i = 0; i < C; i++) {
        pthread_create(&consumers[i], NULL, consumer, Q);
    }

    for (int i = 0; i < P; i++) {
        pthread_join(producers[i], NULL);
    }

    for (int i = 0; i < C; i++) {
        pthread_join(consumers[i], NULL);
    }

    freeBlockingQueue(Q);
    return 0;
}
