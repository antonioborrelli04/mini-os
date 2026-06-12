
#include "../include/mmu.h"
#include "../include/page.h"

int mmu_get_page_number(int virtual_address) {
    return virtual_address / PAGE_SIZE;
}

int mmu_get_offset(int virtual_address) {
    return virtual_address % PAGE_SIZE;
}