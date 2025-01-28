#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define MAX_LINE_LENGTH 1024

// Criação de struct para passar argumentos para a thread
typedef struct
{
    const char *filename;
    const char *word;
} ThreadArg;

void *search_in_file(void *arg)
{
    ThreadArg *thread_arg = (ThreadArg *)arg;

    char line[MAX_LINE_LENGTH];

    int line_number = 0;

    FILE *file = fopen(thread_arg->filename, "r");

    if (!file)
    {
        printf("Erro ao abrir o arquivo: %s\n", thread_arg->filename);
        pthread_exit(NULL);
    }

    // Utilização da função fgets para ler linha a linha do arquivo
    while (fgets(line, MAX_LINE_LENGTH, file))
    {
        int len = strlen(line);
        // verificação se o fgets pegou a linha toda ou ainda falta uma parte
        if (line[len - 1] == '\n' || feof(file))
            line_number++;

        if (strstr(line, thread_arg->word))
            printf("%s:%d\n", thread_arg->filename, line_number);
    }

    fclose(file);
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    // Verificação se o número de argumentos é suficiente, para evitar segmentation fault
    if (argc < 3)
    {
        printf("Uso: %s <palavra> <arquivo1> <arquivo2> ... <arquivoN>\n", argv[0]);
        return -1;
    }

    char *word = argv[1];
    int num_files = argc - 2;

    // Alocação de memória para os vetores de threads e argumentos
    pthread_t *threads = (pthread_t *)calloc(num_files, sizeof(pthread_t));
    ThreadArg *thread_args = (ThreadArg *)calloc(num_files, sizeof(ThreadArg));

    // Criação de threads para cada arquivo
    for (int i = 0; i < num_files; i++)
    {
        thread_args[i].filename = argv[i + 2];
        thread_args[i].word = word;

        if (pthread_create(&threads[i], NULL, search_in_file, &thread_args[i]) != 0)
        {
            printf("Erro ao criar thread para o arquivo: %s\n", argv[i + 2]);
            return -1;
        }
    }

    // Utilização da função pthread_join para esperar o término de todas as threads
    for (int i = 0; i < num_files; i++)
        pthread_join(threads[i], NULL);

    return 0;
}
