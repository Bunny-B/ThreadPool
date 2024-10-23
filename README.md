# Thread Pool and Window Management

This project provides a **Thread Pool** implementation for managing concurrent tasks and a simple system to create Windows GUI applications with multi-threading support.

## Usage

### 1. Initialize the Thread Pool

To start, initialize a thread pool with a specified number of threads.

```c
#include <windows.h>
#include "thread_pool.h"  // Include the thread pool header

// Function to be executed by a thread in the thread pool
void myTaskFunction(void* arg) {
    // Perform some computation or task here
    // This example simply simulates a task with Sleep
  while(true)
    Sleep(1000); // Simulate a long-running task
}
void myTaskFunction_(void* arg) {
    // Perform some computation or task here
    // This example simply simulates a task with Sleep
  while(true)
    Sleep(1000); // Simulate a long-running task
}


typedef struct {
    int start;
    int add;
} TaskParams,*PTaskParams;

void myTaskFunction(void* arg) {
    PTaskParams params = (PTaskParams)arg;
    while (1) {
        params->start += params->add;
        Sleep(1000); // Simulate a long-running task
    }
}

PTaskParams createParams(int start, int add) {
    PTaskParams params = (TaskParams*)malloc(sizeof(TaskParams));
    if (params != NULL) {
        params->start = start; 
        params->add = add; 
    }
    return params;
}

// Main entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Initialize the thread pool
    PThreadPool pool = thread_pool_init(4); // Create a thread pool with 4 threads

    // Add a task to perform some computation
    thread_pool_add_task(pool, myTaskFunction, NULL); // Add the task with NULL argument
    thread_pool_add_task(pool, myTaskFunction_, NULL); // Add the task with NULL argument

    PTaskParams params = createParams(0, 1);
    thread_pool_add_task(pool, myTaskFunction, params);

    // Stop the thread pool and clean up
    thread_pool_stop(pool);
    free(pool->threads);
    free(pool);
    free(params);

    return 0;
}

