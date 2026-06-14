#ifndef _SWAP_SPACE_H
#define _SWAP_SPACE_H

#define SWAPPED 1
#define PRESENT 0

#include "page.h"
#include "memory.h"
#include "process.h"

// Spazio di accumulo
typedef struct {
    unsigned char data[MAX_PAGES][PAGE_SIZE]; // Contenuto delle pagine
    int used[MAX_PAGES];    // 0 = slot libero, 1 = occupato
} SwapSpace;

void swap_out(SwapSpace* swap, PhysicalMemory* mem, Process* p, int page_num);
void swap_in(SwapSpace* swap, PhysicalMemory* mem, Process* p, int page_num);

#endif