
#ifndef _KERNEL_H
#define _KERNEL_H

#include <pthread.h>
#include "process.h"
#include "swap.h"

#define MAX_PROCESSES 4
#define INITIAL_INSTRUCTIONS 5

// 0 = FIFO, 1 = SECOND_CHANCE
#define PAGE_REPLACEMENT_ALGO 1   

#define CPU_QUANTUM 2

// PROCESS DATA
extern Process process_table[MAX_PROCESSES];
extern pthread_mutex_t kernel_lock;
extern pthread_cond_t scheduler_cond;
extern int terminated_processes;

// MEMORY DATA
extern SwapSpace SWAP_SPACE;
extern PhysicalMemory RAM_MEMORY;

/*
    extern indica che queste variabili esitono
    ma non saranno create qui, infatti verrano 
    inizializzate all'interno di main.c
*/

#endif