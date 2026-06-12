
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_PROCESSES 4

// PROCESS STATE
typedef enum {
    READY,
    RUNNING,
    BLOCKED,
    TERMINATED,

} ProcessState;

// Process struct
typedef struct {
    int pid;
    pthread_t thread;
    ProcessState state;

} Process;

// Dichiaro un array di processi (Process Table)
Process process_table[MAX_PROCESSES];

// Mutex per garantire la mutua esclusione al kernel
pthread_mutex_t kernel_lock = PTHREAD_MUTEX_INITIALIZER;

/*
|--------------------------------------------------
|     PROCESS LIFE-TIME PROCEDURE IMPLEMENTATION
|--------------------------------------------------
*/

void* process_run(void* arg) {

    // Vedo arg come puntatore a struct Process
    Process* p = (Process*) arg;

    /*
    |
    |     FASE DI AVVIO
    |
    */

    // Lock sul Mutex
    pthread_mutex_lock(&kernel_lock);

    // Cambio lo stato del processo
    p->state = RUNNING;

    // Stampo log
    printf("[PROCESS %d] RUNNING\n", p->pid);

    // Unlock sul Mutex
    pthread_mutex_unlock(&kernel_lock);

    // Eseguo le istruzioni all'interno del processo
    for (int i=0; i<5; i++) {
        printf("[PROCESS %d] instruction %d\n", p->pid, i);
        sleep(1);
    }

    /*
    |
    |     FASE DI TERMINAZIONE
    |
    */

    // Lock sul Mutex
    pthread_mutex_lock(&kernel_lock);

    // Cambio lo stato (TERMINATED);
    p->state = TERMINATED;

    // Stampo Log
    printf("[PROCESS %d] TERMINATED\n", p->pid);

    // Sblocco Mutex
    pthread_mutex_unlock(&kernel_lock);

    return NULL;
}

// Starting Point
int main() {

    // Attivo il processo, e segnalo con stato READY
    for (int i=0; i<MAX_PROCESSES; i++) {
        process_table[i].pid = i;
        process_table[i].state = READY;

        // Creo un thread a partire dalla Process Table
        pthread_create(
            &process_table[i].thread,
            NULL,
            process_run,
            &process_table[i]
        );
    }

    // Avvio tutti i threads.
    for (int i=0; i<MAX_PROCESSES; i++) {
        pthread_join(process_table[i].thread, NULL);
    }

    return 0;
}