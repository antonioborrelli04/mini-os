
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_PROCESSES 4
#define INITIAL_INSTRUCTIONS 5

// PROCESS STATE
typedef enum {
    READY,
    RUNNING,
    BLOCKED,
    TERMINATED

} ProcessState;

// Process struct
typedef struct {
    int pid;
    pthread_t thread;
    ProcessState state;

    pthread_cond_t cond;
    int instructions_left;

} Process;

// Dichiaro un array di processi (Process Table)
Process process_table[MAX_PROCESSES];

// Mutex per garantire la mutua esclusione al kernel
pthread_mutex_t kernel_lock = PTHREAD_MUTEX_INITIALIZER;

// Condition variable of the scheduler
pthread_cond_t scheduler_cond = PTHREAD_COND_INITIALIZER;

// Global to keep track of terminated processes
int terminated_process = 0;

/*
|-------------------------
|   INTERNAL PROCEDURES
|-------------------------
*/

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

/*
|--------------------------------------------------
|     PROCESS LIFE-TIME PROCEDURE IMPLEMENTATION
|--------------------------------------------------
*/

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
            terminated_process++;

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

// Scheduler Loop Procedure
void* scheduler_loop(void* arg) {
    (void) arg;

    // Dichiaro e inizializzo un tracker nella Process Table
    int next_index = 0;

    // Scheduler Cycle
    while (1) {

        // Faccio Lock sul kernel
        pthread_mutex_lock(&kernel_lock);

        // Se i processi terminati hanno raggiunto il limite, chiudo il loop
        if (terminated_process == MAX_PROCESSES) {
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