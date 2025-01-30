#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define ARRAY_SIZE 10
#define NUM_READERS 3
#define NUM_WRITERS 2

int array[ARRAY_SIZE];

/*
Nossa dupla utilizou Lamport's Bakery Algorithm para garantir a ordem justa entre escritores,
em que um escritor recebe um ticket e espera até que seja a sua vez de escrever. Acreditamos
que essa abordagem é mais justa para evitar starvation de escritores, garantindo que todos os
escritores terão a chance de escrever, mesmo que haja muitos leitores ativos.
Nossas Referências:
- Vimos a respeito deste algoritmo em algumas fontes antes de implementar. Nos baseamos em
mais de uma, mas especialmente, nas implementações do link abaixo:
https://www.geeksforgeeks.org/bakery-algorithm-in-process-synchronization/
- Link do artigo original: https://lamport.azurewebsites.net/pubs/bakery.pdf
*/

// Mutexes e variáveis de condição
pthread_mutex_t read_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t write_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t read_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t write_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t ticket_cond = PTHREAD_COND_INITIALIZER;
int reader_count = 0;
int writer_waiting = 0;

// Sistema de tickets para garantir fairness entre escritores
unsigned long next_ticket = 0;
unsigned long current_ticket = 0;
pthread_mutex_t ticket_mutex = PTHREAD_MUTEX_INITIALIZER;

void *reader(void *arg) {
    int id = *(int *)arg;
    free(arg);
    while (1) {
        pthread_mutex_lock(&read_mutex);
        while (writer_waiting > 0) {
            pthread_cond_wait(&read_cond, &read_mutex);
        }
        reader_count++;
        pthread_mutex_unlock(&read_mutex);

        int local_copy[ARRAY_SIZE];
        for (int i = 0; i < ARRAY_SIZE; i++) {
            local_copy[i] = array[i];
        }

        pthread_mutex_lock(&print_mutex);
        printf("Leitor %d leu: [", id);
        for (int i = 0; i < ARRAY_SIZE; i++) {
            printf("%d ", local_copy[i]);
        }
        printf("]\n");
        pthread_mutex_unlock(&print_mutex);

        usleep(100000); 

        pthread_mutex_lock(&read_mutex);
        reader_count--;
        if (reader_count == 0) {
            pthread_cond_broadcast(&write_cond);
        }
        pthread_mutex_unlock(&read_mutex);
    }
    return NULL;
}

void *writer(void *arg) {
    int id = *(int *)arg;
    free(arg);
    while (1) {
        pthread_mutex_lock(&ticket_mutex);
        unsigned long my_ticket = next_ticket++;
        pthread_mutex_unlock(&ticket_mutex);

        // Espera até ser a vez deste escritor
        pthread_mutex_lock(&ticket_mutex);
        while (current_ticket != my_ticket) {
            pthread_cond_wait(&ticket_cond, &ticket_mutex);
        }
        pthread_mutex_unlock(&ticket_mutex);

        // Bloqueia acesso para escrita
        pthread_mutex_lock(&write_mutex);

        pthread_mutex_lock(&read_mutex);
        writer_waiting++;
        while (reader_count > 0) {
            pthread_cond_wait(&write_cond, &read_mutex);
        }
        writer_waiting--;
        pthread_mutex_unlock(&read_mutex);

        int index = rand() % ARRAY_SIZE;
        int value = rand() % 100;
        array[index] = value;

        pthread_mutex_lock(&print_mutex);
        printf("Escritor %d escreveu %d na posição %d\n", id, value, index);
        pthread_mutex_unlock(&print_mutex);

        usleep(100000);

        pthread_mutex_unlock(&write_mutex);

        // Avança para o próximo ticket
        pthread_mutex_lock(&ticket_mutex);
        current_ticket++;
        pthread_cond_broadcast(&ticket_cond);
        pthread_mutex_unlock(&ticket_mutex);

        pthread_mutex_lock(&read_mutex);
        pthread_cond_broadcast(&read_cond);
        pthread_mutex_unlock(&read_mutex);
    }
    return NULL;
}

int main() {
    pthread_t readers[NUM_READERS], writers[NUM_WRITERS];
    srand(time(NULL));

    for (int i = 0; i < ARRAY_SIZE; i++) {
        array[i] = 0;
    }

    for (int i = 0; i < NUM_READERS; i++) {
        int *id = malloc(sizeof(int));
        *id = i;
        pthread_create(&readers[i], NULL, reader, id);
    }

    for (int i = 0; i < NUM_WRITERS; i++) {
        int *id = malloc(sizeof(int));
        *id = i;
        pthread_create(&writers[i], NULL, writer, id);
    }

    for (int i = 0; i < NUM_READERS; i++) {
        pthread_join(readers[i], NULL);
    }
    for (int i = 0; i < NUM_WRITERS; i++) {
        pthread_join(writers[i], NULL);
    }

    return 0;
}