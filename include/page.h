

#ifndef _PAGE_H
#define _PAGE_H

#define PAGE_FREE 0
#define PAGE_USED 1

// Ogni pagina Virtuale contiene 256 byte
#define PAGE_SIZE 256
#define MAX_PAGES 16
#define MAX_FRAMES 8

// PageTableEntry Struct
typedef struct {
    int page_number;
    int frame_number;
    int present;
    int dirty;
    int referenced;
} PageTableEntry;

// PageTable Struct
typedef struct {
    PageTableEntry entries[MAX_PAGES];
} PageTable;

#endif