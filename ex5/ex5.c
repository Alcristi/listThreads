#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define N 4 // Maximum number of worker threads

typedef struct
{
    void *(*funexec)(void *); // Function to be executed
    void *args;               // Arguments for the function
    int id;                   // Execution ID
} Task;

typedef struct
{
    int id;
    int result;
} Result;

typedef struct
{
    Task *buffer;               // Buffer for pending tasks
    int buffer_size;            // Size of the buffer
    int buffer_count;           // Number of tasks in the buffer
    int next_id;                // Next ID to be assigned
    Result *results;            // Buffer for results
    int results_count;          // Number of results in the buffer
    size_t threads_active;      // Number of threads currently executing tasks
    pthread_mutex_t mutex;      // Mutex for synchronization
    pthread_cond_t cond;        // Condition variable for synchronization
    pthread_cond_t result_cond; // Condition variable for results
} Scheduler;

Scheduler scheduler;
pthread_mutex_t execution_mutex; // Mutex to protect threads_active

void initScheduler(int buffer_size)
{
    scheduler.buffer = (Task *)calloc(buffer_size, sizeof(Task));
    scheduler.buffer_size = buffer_size;
    scheduler.buffer_count = 0;
    scheduler.next_id = 0;
    scheduler.threads_active = 0;
    scheduler.results_count = 0;
    scheduler.results = (Result *)calloc(N * buffer_size, sizeof(Result));
    pthread_mutex_init(&scheduler.mutex, NULL);
    pthread_cond_init(&scheduler.cond, NULL);
    pthread_cond_init(&scheduler.result_cond, NULL);
    pthread_mutex_init(&execution_mutex, NULL); // Initialize the mutex for threads_active
}

void destroyScheduler()
{
    free(scheduler.buffer);
    free(scheduler.results);
    pthread_mutex_destroy(&scheduler.mutex);
    pthread_cond_destroy(&scheduler.cond);
    pthread_cond_destroy(&scheduler.result_cond);
    pthread_mutex_destroy(&execution_mutex); // Destroy the mutex for threads_active
}

int scheduleExecution(void *(*funexec)(void *), void *args)
{
    pthread_mutex_lock(&scheduler.mutex);

    while (scheduler.buffer_count == scheduler.buffer_size)
        pthread_cond_wait(&scheduler.cond, &scheduler.mutex);

    Task task = {.id = scheduler.next_id++, .funexec = funexec, .args = args};

    scheduler.buffer[scheduler.buffer_count++] = task;

    pthread_cond_signal(&scheduler.cond); // Wake up the dispatcher thread
    pthread_mutex_unlock(&scheduler.mutex);

    return task.id;
}

void *worker(void *arg)
{
    Task *task = (Task *)arg;

    int result = *(int *)task->funexec(task->args);

    pthread_mutex_lock(&scheduler.mutex);
    printf("Worker %d - Result: %d\n", task->id, result);
    scheduler.results[scheduler.results_count++] = (Result){.id = task->id, .result = result};
    scheduler.threads_active--;
    pthread_cond_signal(&scheduler.result_cond);
    pthread_mutex_unlock(&scheduler.mutex);

    return NULL;
}

void *dispatcher(void *arg)
{
    pthread_t workers[N];

    int i = 0;
    while (1)
    {
        pthread_mutex_lock(&scheduler.mutex);

        while (scheduler.buffer_count == 0 || scheduler.threads_active == N)
            pthread_cond_wait(&scheduler.cond, &scheduler.mutex);

        pthread_create(&workers[i++], NULL, worker, &scheduler.buffer[--scheduler.buffer_count]);

        scheduler.threads_active++;

        pthread_mutex_unlock(&scheduler.mutex);

        if (i == N)
        {
            i = 0;
            for (int j = 0; j < N; j++)
            {
                pthread_join(workers[j], NULL);
            }
        }
    }

    return NULL;
}

void swapWithLastPosition(Result arr[], int pos, int size)
{
    if (pos < 0 || pos >= size)
    {
        printf("Invalid position.\n");
        return;
    }

    // Swap the value at the specified position with the value at the last position
    Result temp = arr[pos];
    arr[pos] = arr[size - 1];
    arr[size - 1] = temp;
}

int getExecutionResult(int id)
{
    Result result;

    int has_id = 0;
    while (!has_id)
    {
        pthread_mutex_lock(&scheduler.mutex);
        for (int i = 0; i < scheduler.results_count; i++)
        {
            if (scheduler.results[i].id == id)
            {
                has_id = 1;
                result = scheduler.results[i];
                swapWithLastPosition(scheduler.results, i, scheduler.results_count);
                scheduler.results_count--;
                break;
            }
        }
        if (!has_id)
        {
            pthread_cond_wait(&scheduler.result_cond, &scheduler.mutex);
        }
        pthread_mutex_unlock(&scheduler.mutex);
    }

    return result.result;
}

void *example_function(void *arg)
{
    int *value = (int *)arg;
    int *result = (int *)malloc(sizeof(int));
    *result = *value * 2;
    sleep(1); // Simulate a time-consuming operation
    return result;
}

int main()
{
    initScheduler(10);

    // Create the dispatcher thread
    pthread_t dispatcher_thread;
    pthread_create(&dispatcher_thread, NULL, dispatcher, NULL);

    int args[10];
    int ids[10];

    // Schedule tasks
    for (int i = 0; i < 10; i++)
    {
        args[i] = i + 1;
        ids[i] = scheduleExecution(example_function, &args[i]);
    }

    // Collect results
    for (int i = 0; i < 10; i++)
    {
        int result = getExecutionResult(ids[i]);
        printf("Execution %d result: %d\n", ids[i], result);
    }

    // Terminate the program
    pthread_cancel(dispatcher_thread);
    destroyScheduler();

    return 0;
}