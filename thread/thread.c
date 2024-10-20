#include "thread.h"
#include <stdio.h>

// Worker thread function
DWORD WINAPI worker(LPVOID pool) {
    ThreadPool* thread_pool = (ThreadPool*)pool;

    while (1) {
        WaitForSingleObject(thread_pool->task_available, INFINITE); // Wait for a task

        EnterCriticalSection(&thread_pool->lock);
        if (thread_pool->task_count == 0) {
            LeaveCriticalSection(&thread_pool->lock);
            continue;
        }

        // Get the next task
        Task task = thread_pool->tasks[--thread_pool->task_count];
        LeaveCriticalSection(&thread_pool->lock);

        // Execute the task
        task.function(task.arg);
    }
    return 0;
}

// ThreadPool initialization
PThreadPool thread_pool_init(int thread_count) {
    PThreadPool pool = (PThreadPool)malloc(sizeof(ThreadPool));
    pool->task_count = 0;
    pool->thread_count = thread_count;

    pool->threads = (HANDLE*)malloc(thread_count * sizeof(HANDLE));
    InitializeCriticalSection(&pool->lock);
    pool->task_available = CreateSemaphore(NULL, 0, MAX_TASKS, NULL);
    pool->stop_event = CreateEvent(NULL, TRUE, FALSE, NULL); // Stop event

    for (int i = 0; i < thread_count; i++) {
        pool->threads[i] = CreateThread(NULL, 0, worker, pool, 0, NULL);
    }
    return pool;
}

// Add a task to the thread pool
void thread_pool_add_task(PThreadPool pool, void (*function)(void*), void* arg) {
    EnterCriticalSection(&pool->lock);

    // Check if there's space for more tasks
    if (pool->task_count < MAX_TASKS) {
        pool->tasks[pool->task_count++] = (Task){ function, arg };
        ReleaseSemaphore(pool->task_available, 1, NULL); // Signal a worker thread
    }
    else {
        printf("Task queue is full!\n");
    }

    LeaveCriticalSection(&pool->lock);
}

// Stop the thread pool
void thread_pool_stop(PThreadPool pool) {
    SetEvent(pool->stop_event); // Signal all threads to stop

    for (int i = 0; i < pool->thread_count; i++) {
        ReleaseSemaphore(pool->task_available, 1, NULL); // Wake up all threads
    }

    for (int i = 0; i < pool->thread_count; i++) {
        WaitForSingleObject(pool->threads[i], INFINITE);
        CloseHandle(pool->threads[i]);
    }

    CloseHandle(pool->task_available);
    CloseHandle(pool->stop_event);
    DeleteCriticalSection(&pool->lock);
}

// Example task function
void example_task(void* arg) {
    int task_number = *((int*)arg);
    printf("Executing task %d on thread %lu\n", task_number, GetCurrentThreadId());
    Sleep(100); // Simulate work
}

// Example usage
int usage() {
    PThreadPool pool = thread_pool_init(4); // Create a thread pool with 4 threads

    // Create and add tasks to the pool
    for (int i = 0; i < 10; i++) {
        int* task_number = malloc(sizeof(int));
        *task_number = i;
        thread_pool_add_task(pool, example_task, task_number);
    }

    // Wait for tasks to complete and clean up
    thread_pool_stop(pool);

    free(pool->threads);
    free(pool);
    return 0;
}


