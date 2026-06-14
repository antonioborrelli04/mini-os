#include <string.h>

#include "../include/memory.h"
#include "../include/swap.h"
#include "../include/process.h"

// This method swaps out a process from the PageTable
void swap_out(SwapSpace* swap, PhysicalMemory* mem, Process* p, int page_num) {

    // Dichiaro un contatore per scorrere l'array e lo inizializzo
    int current_page = 0;

    // Trovo uno slot libero nello SwapSpace
    while (swap->used[current_page] == FRAME_USED) {

        // Incremento, modulo max_frames
        current_page = (current_page + 1) % MAX_FRAMES;
    }

    // Segnalo l'utilizzo del frame
    swap->used[current_page] = FRAME_USED;

    // Calcolo la posizione del frame in memoria principale
    int frame_id = p->pt.entries[page_num].frame_number;

    // Copio i dati della pagina nello swap space
/*
    for (int i=0; i<MAX_FRAMES; i++) {
        swap->data[current_page][i] = mem->data[page_num][i];
    }
*/
    memcpy(swap->data[current_page], mem->data[frame_id], PAGE_SIZE);

    // Segnalo che l'area di memoria associata al frame è libera...
    mem->frames[frame_id] = FRAME_FREE;

    // Aggiorno la PageTableEntry nella page table del Processo
    p->pt.entries[page_num].present = PAGE_FREE;

    // Aggiorno lo swap flag nella PageTable
    p->pt.entries[page_num].in_swap = SWAPPED;

    // Aggiorno il frame_id associato alla pagina
    p->pt.entries[page_num].frame_number = current_page;
}

// Method that swaps in a Page from the swap_space to the main memory
void swap_in(SwapSpace* swap, PhysicalMemory* mem, Process* p, int page_num) {

    // Risalgo al frame_id della Pagina
    int frame_id = p->pt.entries[page_num].frame_number;

    // Cerco un frame libero in memoria principale e lo inizializzo
    int current_frame = 0;
    while (mem->frames[current_frame] == FRAME_USED && current_frame < MAX_FRAMES) {
        current_frame = (current_frame + 1) % MAX_FRAMES;
    } 

    if (current_frame == MAX_FRAMES) return;

    // Copio il registro di memoria dallo SwapSpace alla memoria principale
    memcpy(mem->data[current_frame], swap->data[frame_id], PAGE_SIZE);

    // Aggiorno i flags
    p->pt.entries[page_num].present = FRAME_USED;
    p->pt.entries[page_num].in_swap = PRESENT;
    swap->used[frame_id] = FRAME_FREE;

    // Aggiorno il frame_id associato alla Pagina
    p->pt.entries[page_num].frame_number = current_frame;
}