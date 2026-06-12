
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

        // Stampo Log
        printf("[PROCESS %d] executing instruction, remaining before = %d\n",
            p->pid,
            p->instructions_left
        );

        // Decremento il numero di istruzioni rimanenti per quel processo.
        p->instructions_left--;

        /* Quando le istruzioni rimanenti del processo si azzerano,
                Lo mettiamo in stato TERMINATO, incrementiamo il contatore
                globale terminated_process e stampo Log.
        */
        if (p->instructions_left <= 0) {
            p->state = TERMINATED;
            terminated_processes++;

            printf("[PROCESS %d] TERMINATED\n", p->pid);
        }

        /* In qualsiasi altro caso imposto lo stato del processo
                a READY e stampo Log.
        */
        else {
            p->state = READY;

            printf("[PROCESS %d] back to READY, remaining = %d\n",
                p->pid,
                p->instructions_left
            );
        }

        // Risveglio lo scheduler e Unlock sul mutex
        pthread_cond_signal(&scheduler_cond);
        pthread_mutex_unlock(&kernel_lock);
    }

    return NULL;
}

