
#include <pthread.h>
#include <stdio.h>

#include "../include/kernel.h"
#include "../include/process.h"
#include "../include/scheduler.h"
#include "../include/mmu.h"
#include "../include/memory.h"

// Dichiaro un array di processi (Process Table)
Process process_table[MAX_PROCESSES];

// Mutex per garantire la mutua esclusione al kernel
pthread_mutex_t kernel_lock = PTHREAD_MUTEX_INITIALIZER;

// Condition variable of the scheduler
pthread_cond_t scheduler_cond = PTHREAD_COND_INITIALIZER;

// Global to keep track of terminated processes
int terminated_processes = 0;

// Allocating simulated physical memory
PhysicalMemory RAM_MEMORY;

// Starting Point
int main(void) {

    // Inizializzo la memoria RAM
    memory_init(&RAM_MEMORY);

    // Dichiaro un thread per lo Scheduler
    pthread_t scheduler_thread;

    /* Imposto tutti i processi della Process Table in
            READY state, per ogni ciclo dello scheduler, ogni processo
            eseguirà rispettivamente INITIAL_INSTRUCTION istruzioni.
            Infine creo il thread con il puntatore al thread data type
            all'interno della struct Process.
    */
    for (int i=0; i<MAX_PROCESSES; i++) {

        // Process builder
        process_init(&process_table[i], i, INITIAL_INSTRUCTIONS);

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
        process_destroy(&process_table[i]);
    }

    // Dealloco le risorse utilizzate
    pthread_mutex_destroy(&kernel_lock);
    pthread_cond_destroy(&scheduler_cond);


    // MMU DEBUG
    int virtual_address = 515;

    // Creating and updating the PageTable
    PageTable pt;
    pt.entries[2].present = 1;
    pt.entries[2].frame_number = 7;

    // Creating getting physical address...
    int physical_address; 
    mmu_translate(&pt, virtual_address, &physical_address);

    // Logging...
    printf("\n[MMU TEST] virtual address = %d\n", virtual_address);
    printf("[MMU TEST] page number = %d\n", mmu_get_page_number(virtual_address));
    printf("[MMU TEST] offset = %d\n", mmu_get_offset(virtual_address));
    printf("[MUU TEST] physical address = %d\n", physical_address);

    // PAGE FAULT TEST

    // process_table[0] ha pt non inizializzata -> present == 0
    int result = handle_page_fault(&process_table[0], &RAM_MEMORY, 3);
    printf("[PAGE FALULT TEST] result = %d\n", result);

    return 0;
}