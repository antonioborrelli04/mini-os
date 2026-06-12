
#include <pthread.h>

#include "../include/kernel.h"
#include "../include/process.h"
#include "../include/scheduler.h"

#define MAX_PROCESSES 4
#define INITIAL_INSTRUCTIONS 5

// Dichiaro un array di processi (Process Table)
Process process_table[MAX_PROCESSES];

// Mutex per garantire la mutua esclusione al kernel
pthread_mutex_t kernel_lock = PTHREAD_MUTEX_INITIALIZER;

// Condition variable of the scheduler
pthread_cond_t scheduler_cond = PTHREAD_COND_INITIALIZER;

// Global to keep track of terminated processes
int terminated_process = 0;

// Starting Point
int main(void) {

    // Dichiaro un thread per lo Scheduler
    pthread_t scheduler_thread;

    /* Imposto tutti i processi della Process Table in
            READY state, per ogni ciclo dello scheduler, ogni processo
            eseguirà rispettivamente INITIAL_INSTRUCTION istruzioni.
            Infine creo il thread con il puntatore al thread data type
            all'interno della struct Process.
    */
    for (int i=0; i<MAX_PROCESSES; i++) {

        // Imposto gli attributi del processo nella process_table
        process_table[i].pid = i;
        process_table[i].state = READY;
        process_table[i].instructions_left = INITIAL_INSTRUCTIONS;

        // Inizializzo la var cond del processo
        pthread_cond_init(&process_table[i].cond, NULL);

        // Creo il thread del Processo
        pthread_create(
            &process_table[i].thread,
            NULL,
            process_run,
            &process_table[i]
        );
    }

    // Creo il thread dello Scheduler
    pthread_create(
        &scheduler_thread,
        NULL,
        scheduler_loop,
        NULL
    );
    
    // Attendo la terminazione dello scheduler thread
    pthread_join(scheduler_thread, NULL);

    /* Risveglio eventuali processi ancora in attesa, attendo la
            terminazione e distruggo le condition variable
    */
    for (int i=0; i<MAX_PROCESSES; i++) {
        pthread_cond_signal(&process_table[i].cond);
        pthread_join(process_table[i].thread, NULL);
        pthread_cond_destroy(&process_table[i].cond);
    }

    // Dealloco le risorse utilizzate
    pthread_mutex_destroy(&kernel_lock);
    pthread_cond_destroy(&scheduler_cond);

    return 0;
}