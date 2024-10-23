#include "thread.h"
#include <stdio.h>

char is_thread_pool_running = FALSE;

DWORD WINAPI worker(LPVOID pool) {
    ThreadPool* thread_pool = (ThreadPool*)pool;

    while (TRUE) {
        // Wait for a task or stop event
        HANDLE events[] = { thread_pool->task_available, thread_pool->stop_event };
        DWORD wait_result = WaitForMultipleObjects(2, events, FALSE, INFINITE);

        if (wait_result == WAIT_OBJECT_0 + 1) {
            // Stop event is signaled, break the loop
            break;
        }

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

PThreadPool thread_pool_init(int thread_count) {
    PThreadPool pool = (PThreadPool)malloc(sizeof(ThreadPool));
    if (pool != NULL) {
        pool->task_count = 0;
        pool->thread_count = thread_count;

        pool->threads = (HANDLE*)malloc(thread_count * sizeof(HANDLE));
        if (pool->threads != NULL) {
            InitializeCriticalSection(&pool->lock);
            pool->task_available = CreateSemaphore(NULL, 0, MAX_TASKS, NULL);
            pool->stop_event = CreateEvent(NULL, TRUE, FALSE, NULL); // Stop event

            for (int i = 0; i < thread_count; i++) {
                pool->threads[i] = CreateThread(NULL, 0, worker, pool, 0, NULL);
            }
            is_thread_pool_running = TRUE;
        }
    }
    return pool;
}

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

void thread_pool_stop(PThreadPool pool) {
    if (is_thread_pool_running == FALSE) return;
    if (pool == NULL) return;

#ifdef _DEBUG
    printf("thread_pool_stop\n");
#endif // _DEBUG

    // Signal all threads to stop
    SetEvent(pool->stop_event);

    // Wake up all threads
    for (int i = 0; i < pool->thread_count; i++) {
        ReleaseSemaphore(pool->task_available, 1, NULL);
    }

    // Wait for all threads to finish
    for (int i = 0; i < pool->thread_count; i++) {
        if (pool->threads[i] != NULL) {
            WaitForSingleObject(pool->threads[i], INFINITE);
            CloseHandle(pool->threads[i]);
            pool->threads[i] = NULL;
        }
    }

    // Cleanup
    if (pool->task_available != NULL) {
        CloseHandle(pool->task_available);
        pool->task_available = NULL;
    }
    if (pool->stop_event != NULL) {
        CloseHandle(pool->stop_event);
        pool->stop_event = NULL;
    }

    DeleteCriticalSection(&pool->lock);
    free(pool);
    pool = NULL;
    is_thread_pool_running = FALSE;

#ifdef _DEBUG
    printf("ThreadPool cleaned\n");
#endif // _DEBUG
}

