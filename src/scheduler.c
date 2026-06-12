
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "../include/scheduler.h"
#include "../include/kernel.h"

// Searching the first ready process in the process_table;
int find_next_ready_process(int start_index) {
    for (int offset = 0; offset < MAX_PROCESSES; offset++) {
        int index = (start_index + offset) % MAX_PROCESSES;

        if (process_table[index].state == READY) {
            return index;
        }
    }
    return -1;
}

// Implementing Scheduler Loop
void* scheduler_loop(void* arg) {
    (void) arg;

    // Dichiaro e inizializzo un tracker nella Process Table
    int next_index = 0;

    // Scheduler Cycle
    while (1) {

        // Faccio Lock sul kernel
        pthread_mutex_lock(&kernel_lock);

        // Se i processi terminati hanno raggiunto il limite, chiudo il loop
        if (terminated_processes == MAX_PROCESSES) {
            pthread_mutex_unlock(&kernel_lock);
            break;
        }

        // Trovo il prossimo processo READY nella Process Table
        int selected = find_next_ready_process(next_index);

        /* Se la ricerca fallisce, faccio Unlock del mutex
                e riavvio lo Scheduler Loop. 
        */
        if (selected == -1) {
            pthread_mutex_unlock(&kernel_lock);
            continue;
        }

        // Cambio stato del processo in RUNNING
        process_table[selected].state = RUNNING;

        // Stampo Log
        printf("[SCHEDULER] dispath process %d\n",
            process_table[selected].pid);

        // Faccio una Signal al processo per risvergliarlo
        pthread_cond_signal(&process_table[selected].cond);

        /* Finchè lo stato del processo in esecuzione resta
                RUNNING, lo Scheduler deve aspettare una signal
                dal pocesso sulla condition variable.
        */
        while ( process_table[selected].state == RUNNING ) {
            pthread_cond_wait(&scheduler_cond, &kernel_lock);
        }

        // Calcolo l'indice del prossimo processo all'interno della Process Table
        next_index = (selected + 1) % MAX_PROCESSES;

        // Sblocco il Mutex
        pthread_mutex_unlock(&kernel_lock);

        // Sleep di 200 ms
        usleep(200000);
    }

    // Stampo Log di terminazione dello scheduler
    printf("[SCHEDULER] all processes terminated\n");

    return NULL;
}