#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define N 4 // Número máximo de threads trabalhadoras

/*
 - Neste exercício, implementamos um escalonador de tarefas que distribui tarefas entre N threads
    trabalhadoras. O escalonador é composto por um buffer de tarefas e um buffer de resultados.
    - O escalonador é inicializado com um buffer de tamanho fixo e uma quantidade de threads trabalhadoras.
    - As tarefas são agendadas com uma função de execução e argumentos, e um ID é retornado.
    - As tarefas são executadas por threads trabalhadoras, que colocam o resultado no buffer de resultados.
    - O resultado de uma execução pode ser obtido com base no ID da execução, no qual a função bloqueia até que o resultado esteja disponível.
    - A utilizção buffer de resultados é gerenciado por results_count, que é incrementado e decrementado conforme os resultados são armazenados e obtidos.
    - O escalonador é sincronizado com mutex e variáveis de condição para garantir a sincronização entre as threads.
 */

// Estrutura que representa uma tarefa
typedef struct
{
    void *(*funexec)(void *); // Função a ser executada
    void *args;               // Argumentos para a função
    int id;                   // Identificador da execução
} Task;

// Estrutura que armazena o resultado de uma execução
typedef struct
{
    int id;     // Identificador da execução
    int result; // Resultado da execução
} Result;

// Estrutura que representa o escalonador
typedef struct
{
    Task *buffer;               // Buffer para tarefas pendentes
    int buffer_size;            // Tamanho do buffer
    int buffer_count;           // Número de tarefas no buffer
    int next_id;                // Próximo ID a ser atribuído
    Result *results;            // Buffer para armazenar resultados
    int results_count;          // Número de resultados armazenados
    pthread_mutex_t mutex;      // Mutex para sincronização
    pthread_cond_t cond;        // Variável de condição para sincronização
    pthread_cond_t result_cond; // Variável de condição para resultados
} Scheduler;

Scheduler scheduler; // Instância global do escalonador
int running = 1;
// Inicializa o escalonador
void initScheduler(int buffer_size)
{
    scheduler.buffer = (Task *)calloc(buffer_size, sizeof(Task));
    scheduler.buffer_size = buffer_size;
    scheduler.buffer_count = 0;
    scheduler.next_id = 0;
    scheduler.results_count = 0;
    scheduler.results = (Result *)calloc(N * buffer_size, sizeof(Result));
    pthread_mutex_init(&scheduler.mutex, NULL);
    pthread_cond_init(&scheduler.cond, NULL);
    pthread_cond_init(&scheduler.result_cond, NULL);
}

// Destroi o escalonador e libera memória
void destroyScheduler()
{
    free(scheduler.buffer);
    free(scheduler.results);
    pthread_mutex_destroy(&scheduler.mutex);
    pthread_cond_destroy(&scheduler.cond);
    pthread_cond_destroy(&scheduler.result_cond);
}

// Agenda uma execução de função e retorna o ID atribuído, e espera se o buffer estiver cheio
int scheduleExecution(void *(*funexec)(void *), void *args)
{
    pthread_mutex_lock(&scheduler.mutex);

    while (scheduler.buffer_count == scheduler.buffer_size)
        pthread_cond_wait(&scheduler.cond, &scheduler.mutex);

    Task task = {.id = scheduler.next_id++, .funexec = funexec, .args = args};

    scheduler.buffer[scheduler.buffer_count++] = task;

    pthread_cond_broadcast(&scheduler.cond); // Notifica a thread do dispatcher
    pthread_mutex_unlock(&scheduler.mutex);

    return task.id;
}

// Função executada por cada thread trabalhadora, para executar as funções do buffer
void *worker(void *arg)
{
    Task task = *(Task *)arg;

    int *result = (int *)task.funexec(task.args);

    pthread_mutex_lock(&scheduler.mutex);
    scheduler.results[scheduler.results_count++] = (Result){.id = task.id, .result = *result};
    pthread_cond_broadcast(&scheduler.result_cond);
    pthread_mutex_unlock(&scheduler.mutex);

    free(result);
    return NULL;
}

// Função que distribui tarefas entre as threads
void *dispatcher(void *arg)
{
    pthread_t workers[N];

    int i = 0;
    while (running)
    {
        pthread_mutex_lock(&scheduler.mutex);
        // Aguarda até que haja tarefas no buffer ou todas as threads estejam ocupadas
        while (scheduler.buffer_count == 0)
            pthread_cond_wait(&scheduler.cond, &scheduler.mutex);

        pthread_create(&workers[i], NULL, worker, &scheduler.buffer[--scheduler.buffer_count]);
        pthread_detach(workers[i++]);
        pthread_mutex_unlock(&scheduler.mutex);
        // Aguarda até que todas as threads tenham sido criadas e executadas
        if (i == N)
            i = 0;
    }

    return NULL;
}

// Troca um resultado com a última posição no array para facilitar a remoção
void swapWithLastPosition(Result arr[], int pos, int size)
{
    Result temp = arr[pos];
    arr[pos] = arr[size - 1];
    arr[size - 1] = temp;
}

// Obtém o resultado da execução de uma tarefa com base no ID
int getExecutionResult(int id)
{
    Result result;
    int has_id = 0;

    while (!has_id)
    {
        pthread_mutex_lock(&scheduler.mutex);

        for (int i = 0; i < scheduler.results_count; i++)
            if (scheduler.results[i].id == id)
            {
                has_id = 1;
                result = scheduler.results[i];
                swapWithLastPosition(scheduler.results, i, scheduler.results_count);
                scheduler.results_count--;
                break;
            }

        if (!has_id)
            pthread_cond_wait(&scheduler.result_cond, &scheduler.mutex);

        pthread_mutex_unlock(&scheduler.mutex);
    }

    return result.result;
}

// Função de exemplo para ser executada em uma thread
void *example_function(void *arg)
{
    int *value = (int *)arg;
    int *result = (int *)malloc(sizeof(int));
    *result = *value * 2;
    sleep(time(NULL) % 3 + 1);
    return result;
}

int main()
{
    initScheduler(10);
    srand(time(NULL));
    // Criação da thread do dispatcher
    pthread_t dispatcher_thread;
    pthread_create(&dispatcher_thread, NULL, dispatcher, NULL);

    int args[10];
    int ids[10];

    // Agenda tarefas
    for (int i = 0; i < 10; i++)
    {
        args[i] = i + 1;
        ids[i] = scheduleExecution(example_function, &args[i]);
    }

    // Coleta resultados
    for (int i = 0; i < 10; i++)
    {
        int result = getExecutionResult(ids[i]);
        printf("Execução %d resultado: %d\n", ids[i], result);
    }

    // Finaliza o programa
    running = 0;
    pthread_cancel(dispatcher_thread);     // Iniciar o cancelamento da thread do dispatcher
    pthread_join(dispatcher_thread, NULL); // Aguardar o término da thread do dispatcher
    destroyScheduler();

    pthread_exit(NULL);
}