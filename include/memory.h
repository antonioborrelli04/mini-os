
#ifndef _MEMORY_H
#define _MEMORY_H

#define MEMORY_SIZE 4096

#define FRAME_USED 1
#define FRAME_FREE 0
#define DIRTY 1
#define CLEAN 0
#define REFERENCED 1
#define NOT_REFERENCED 0

#include "process.h"
#include "page.h"

// Abstracting a physical memory
typedef struct {
    int frames[MAX_FRAMES];
    unsigned char data[MAX_FRAMES][PAGE_SIZE];
} PhysicalMemory;

// Creating a global external memory space
extern PhysicalMemory RAM_MEMORY;

// Method Prototypes
void memory_init(PhysicalMemory* mem);
int memory_allocate_frame(PhysicalMemory* mem);
int handle_page_fault(Process* p, PhysicalMemory* mem, int page_num);

// Swapping & Page replacement algorithms
int select_victim_fifo(Process* p);
int replace_page_fifo(Process* p, PhysicalMemory* mem, int new_page);
int select_victim_second_chance(Process* p);
int replace_page_second_chance(Process* p, PhysicalMemory* mem, int new_page);

// Memory operation prototypes
int memory_read(Process* p, PhysicalMemory* mem, int virtual_address, unsigned char* value);
int memory_write(Process* p, PhysicalMemory* mem, int virtual_address, unsigned char value);

#endif