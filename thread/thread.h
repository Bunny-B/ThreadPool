#ifndef THREAD_H
#define THREAD_H
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif
#define MAX_TASKS 10

    typedef struct {
        void (*function)(void* arg);
        void* arg;
    } Task;

    typedef struct {
        Task tasks[MAX_TASKS];
        int task_count;
        CRITICAL_SECTION lock;
        HANDLE task_available;
        HANDLE stop_event;
        HANDLE* threads;
        int thread_count;
    } ThreadPool, * PThreadPool;

    extern char is_thread_pool_running;
    extern PThreadPool thread_pool_init(int thread_count);
    extern void thread_pool_add_task(PThreadPool pool, void (*function)(void*), void* arg);
    extern void thread_pool_stop(PThreadPool pool);


#ifdef __cplusplus
}
#endif
#endif // !THREAD_H
