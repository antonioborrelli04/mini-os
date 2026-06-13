
#include <stdio.h>

#include "../include/memory.h"
#include "../include/page.h"

// Memory init, imposto tutti i frame a 0
void memory_init(PhysicalMemory* mem) {
    for (int i=0; i<MAX_FRAMES; i++) {
        mem->frames[i] = FRAME_FREE;
    }
}

// Allocating a frame in memory
int memory_allocate_frame(PhysicalMemory* mem) {

    // Scorro l'array finché non trovo un frame libero e ritorno l'indice
    for (int i=0; i<MAX_FRAMES; i++) {
        if (mem->frames[i] == FRAME_FREE) {
            mem->frames[i] = FRAME_USED;
            return i;
        }
    }

    // Se la memoria è piena ritorna -1
    return -1;
}

// Richiedo il frame associato alla pagina del processo in memoria
int handle_page_fault(Process* p, PhysicalMemory* mem, int page_num) {

    // Alloco un frame in memoria e ritorno frame_id
    int frame_id = memory_allocate_frame(mem);

    if (frame_id >= 0) {

        // Creo una nuova PageTableEntry
        PageTableEntry new_entry = {
            page_num,
            frame_id,
            FRAME_USED,
            CLEAN,
            NOT_REFERENCED,
        };

        // Associo il frame_id al processo
        p->pt.entries[page_num] = new_entry;

        // Logging...
        printf("[PAGE FAULT] process %d, page %d\n", p->pid, page_num);
        return 0;
    }

    // Altrimenti lancio un Segmentation fault
    else {
        printf("[SEGMENTATION FAULT] process %d, page %d\n", p->pid, page_num);
        return -1;
    }
}