
#ifndef _PROCESS_H
#define _PROCESS_H

#include <pthread.h>
#include "page.h"

// PROCESS STATE
typedef enum {
    READY,
    RUNNING,
    BLOCKED,
    TERMINATED

} ProcessState;

// PROCESS STRUCT
typedef struct {
    int pid;
    pthread_t thread;
    ProcessState state;

    pthread_cond_t cond;
    int instructions_left;

    PageTable pt;

} Process;

// Method to get a string from a PROCESS STATE
const char* state_to_string(ProcessState state);

// Method to executing Processes
void* process_run(void* arg);

// Process Builder Function
void process_init(Process* p, int pid, int instructions);

// Process Destroy Function
void process_destroy(Process* p);

// Process Execute Quantum Tick
void process_execute_quantum(Process* p, int quantum);

#endif