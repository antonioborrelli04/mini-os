
#include <stdio.h>
#include <pthread.h>

#include "../include/process.h"
#include "../include/kernel.h"

// Internal method to get a String from a process STATE
const char* state_to_string(ProcessState state) {
    switch (state) {
        case READY:
            return "READY";
        case RUNNING:
            return "RUNNING";
        case BLOCKED:
            return "BLOCKED";
        case TERMINATED:
            return "TERMINATED";
        default:
            return "UNKNOWN";
    }
}

// Process builder function
void process_init(Process* p, int pid, int instructions) {
    p->pid = pid;
    p->state = READY;
    p->instructions_left = instructions;

    pthread_cond_init(&p->cond, NULL);
}

// Simulates one CPU tick executed by a process.
void process_execute_tick(Process* p) {

    // Logging...
    printf("[PROCESS %d] executing instruction, remaining before = %d\n",
        p->pid,
        p->instructions_left
    );

    // Decremento il contatore del numero di istruzioni rimanenti
    p->instructions_left--;

    /* Se il contatore delle istruzioni rimanenti si azzera
            impostiamo il ProcessState a TERMINATED e incrementiamo
            il contatore dei processi terminati.
    */
    if (p->instructions_left <= 0) {
        p->state = TERMINATED;
        terminated_processes++;

        printf("[PROCESS %d] TERMINATED\n", p->pid);
    }

    // Altrimenti imposta lo stato a READY  
    else {
        p->state = READY;

        // Logging...
        printf("[PROCESS %d] back to READY, remaining = %d\n",
            p->pid,
            p->instructions_left
        );
    }
}

// Method to run a Process
void* process_run(void* arg) {

    // Vedo arg come puntatore a struct Process
    Process* p = (Process*) arg;

    while (1) {
        pthread_mutex_lock(&kernel_lock);

        /* Se il processo non è in esecuzione e non è ancora terminato,
              gli facciamo fare una wait sulla var. cond */
        while (p->state != RUNNING && p->state != TERMINATED) {
            pthread_cond_wait(&p->cond, &kernel_lock);
        }

        // Se lo stato del processo è TERMINATO, gli facciamo rilasciare il Mutex
        if (p->state == TERMINATED) {
            pthread_mutex_unlock(&kernel_lock);
            break;
        }

        // Esegui tick del processo
        process_execute_tick(p);

        // Risveglio lo scheduler e Unlock sul mutex
        pthread_cond_signal(&scheduler_cond);
        pthread_mutex_unlock(&kernel_lock);
    }

    return NULL;
}

// Method to destroy a Process
void process_destroy(Process* p) {

    //Dealloco semplicemente la var. cond
    pthread_cond_destroy(&p->cond);
}