
#include "../include/mmu.h"
#include "../include/page.h"

// Function to get the page number from a virtual address
int mmu_get_page_number(int virtual_address) {
    if (virtual_address < 0) return -1; // guard
    return virtual_address / PAGE_SIZE;
}

// Function to get virtual address offset
int mmu_get_offset(int virtual_address) {
    return virtual_address % PAGE_SIZE;
}

// Funcion to get a physical address from a virtual address
int mmu_translate(PageTable* pt, int virtual_address, int* physical_address) {

    // Get page number of the virtual address
    int page_n = mmu_get_page_number(virtual_address);
    if (pt->entries[page_n].present == 0) return -1; // guard

    // Get the offset of the virtual address
    int offset = mmu_get_offset(virtual_address);

    // Get the frame_number of the virtual_address
    int frame_number = pt->entries[page_n].frame_number;

    //calcolo il physical address
    *physical_address = frame_number * PAGE_SIZE + offset;

    return 0; // ok
}