
#ifndef _KERNEL_H
#define _KERNEL_H

#include <pthread.h>
#include "process.h"

#define MAX_PROCESSES 4
#define INITIAL_INSTRUCTIONS 5

#define CPU_QUANTUM 2

extern Process process_table[MAX_PROCESSES];

extern pthread_mutex_t kernel_lock;
extern pthread_cond_t scheduler_cond;

extern int terminated_processes;

/*
    extern indica che queste variabili esitono
    ma non saranno create qui, infatti verrano 
    inizializzate all'interno di main.c
*/

#endif