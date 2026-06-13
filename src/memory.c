
#include <stdio.h>
#include <pthread.h>

#include "../include/memory.h"
#include "../include/page.h"
#include "../include/mmu.h"
#include "../include/kernel.h"

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

// Memory Read Method implementation
int memory_read(Process* p, PhysicalMemory* mem, int virtual_address, unsigned char* value) {

    // Richiamo la mmu per risalire all'indirizzo fisico
    int physical_address;
    int result = mmu_translate(&p->pt, virtual_address, &physical_address);

    // Controllo se mmu_translate è andata a buon fine e leggo data
    if ( physical_address >= 0 && result >= 0) {

        // Calcolo il frame_id
        int frame_id = physical_address / PAGE_SIZE;

        // Calcolo offset in memoria fisica
        int offset = mmu_get_offset(virtual_address);

        // Copio il contenuto in value
        *value = mem->data[frame_id][offset];
        return 0;
    }

    // Altrimenti cerco la pagina in memoria fisica
    else {

        // Trovo l'identificativo della pagina
        int page_num = mmu_get_page_number(virtual_address);

        // Gestisco il PageFault
        int result = handle_page_fault(p, mem, page_num);
        return result;
    }
}

// Memory Write Method implementation
int memory_write(Process* p, PhysicalMemory* mem, int virtual_address, unsigned char value) {

    // Accedo alla MMU per conoscere il physical_address
    int physical_address;
    int result = mmu_translate(&p->pt, virtual_address, &physical_address);

    // Ottengo il numero di pagina associato all'indirizzo virtuale
    int page_num = mmu_get_page_number(virtual_address);
    
    // Se la MMU riesce a trovare il Frame corrispondente nella PageTable
    if (physical_address >= 0 && result >= 0) {

        // Calcolo il FrameID partendo dal physical address
        int frame_id = physical_address / PAGE_SIZE;

        // Calcolo l'offset all'interno del Frame
        int offset = mmu_get_offset(virtual_address);

        // Scrivo nell'area di memoria
        mem->data[frame_id][offset] = value;

        // Trigger flag DIRTY
        p->pt.entries[page_num].dirty = DIRTY;
        return 0;
    }

    // Altrimenti gestisco il Page Fault
    else {

        // Gestisco il Page Fault
        int result = handle_page_fault(p, mem, page_num);
        
        if (result == -1) return -1; // guard

        // Riprova la scrittura dopo aver caricato la pagina
        return memory_write(p, mem, virtual_address, value);
    }
}