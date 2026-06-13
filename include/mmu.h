
#ifndef _MMU_H
#define _MMU_H

#include "page.h"

// Function to get page id of a virtual address
int mmu_get_page_number(int virtual_address);

// Function to get the page offset of a virtual address
int mmu_get_offset(int virtual_address);

// Get a physical address from a Virtual Address
int mmu_translate(PageTable* pt, int virtual_address, int* physical_address);

#endif