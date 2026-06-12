
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

const char* state_to_string(ProcessState state);
void* process_run(void* arg);

#endif