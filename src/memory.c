
#include <stdio.h>
#include <pthread.h>
#include <limits.h>

#include "../include/memory.h"
#include "../include/page.h"
#include "../include/mmu.h"
#include "../include/kernel.h"

// Inizializzo un contatore per Page Fault
static int load_counter = 0;

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
            0, // load_order
        };

        // Associo il frame_id al processo
        p->pt.entries[page_num] = new_entry;

        // Incremento il contatore dei Page Fault
        p->pt.entries[page_num].load_order = load_counter++;

        // Logging...
        printf("[PAGE FAULT] process %d, page %d\n", p->pid, page_num);
        return 0;
    }

    // Rimpiazzo la pagina allocata meno recentemente in memoria, con la nuova pagina
    else {
        return replace_page_fifo(p, mem, page_num);
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

// Select Victim fifo swapping procedure
int select_victim_fifo(Process* p) {

    // Inizializzo una variabile per tenere traccia del minimo
    int min_load_order = INT_MAX;

    // Inizializzo una variabile che segue il page_num del minimo
    int victim_page = -1;

    // Scorro i record present all'interno della page table
    for (int i=0; i<MAX_PAGES; i++) {

        // Se il record è presente e il load order e minore del minimo...
        if (p->pt.entries[i].present == 1 && p->pt.entries[i].load_order < min_load_order) {

            // ...aggiorno il minimo e victim_page
            min_load_order = p->pt.entries[i].load_order;
            victim_page = i;
        }
    }

    // Ritorno il numero di pagina del load_min
    return victim_page;
}

// Method to replace a page with FIFO algorithm
int replace_page_fifo(Process* p, PhysicalMemory* mem, int new_page) {
    
    // Seleziono la vittima
    int page_id = select_victim_fifo(p);

    // Libero il frame
    int frame_id = p->pt.entries[page_id].frame_number;
    mem->frames[frame_id] = FRAME_FREE;

    // Libero il frame occupato dal processo nella physical memory
    p->pt.entries[page_id].present = PAGE_FREE;

    // Chiamo handle_page_fault per caricare la nuova pagina nel frame liberato
    int result = handle_page_fault(p, mem, new_page);
    return result;
}