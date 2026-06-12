
#ifndef _PROCESS_H
#define _PROCESS_H

#include <pthread.h>

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

} Process;

// Method to get a string from a PROCESS STATE
const char* state_to_string(ProcessState state);

// Method to executing Processes
void* process_run(void* arg);

// Process Builder Function
void process_init(Process* p, int pid, int instructions);

#endif