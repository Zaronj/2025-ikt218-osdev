#include "libc/gdt.h"
#include "libc/idt.h"
#include "libc/util.h"

extern void gdt_flush(addr_t);
extern void tss_flush();

struct gdt_entry_struct gdt_entries[6];
struct gdt_ptr_struct gdt_ptr;
struct tss_entry_struct tss_entry;

void initGdt() {
    gdt_ptr.limit = (sizeof(struct gdt_entry_struct) * 6) - 1;
    gdt_ptr.base = (uint32_t)&gdt_entries;

    setGdtGate(0,0,0,0,0);  // Null segment
    setGdtGate(1,0, 0xFFFFFFFF, 0x9A, 0xCF);       // Kernel code segment,  9A = 1001 1010
    setGdtGate(2,0, 0xFFFFFFFF, 0x92, 0xCF);       // Kernel data segment,  92 = 1001 0010
    setGdtGate(3,0, 0xFFFFFFFF, 0xFA, 0xCF);       // User code segment,  FA = 1111 1010
    setGdtGate(4,0, 0xFFFFFFFF, 0xF2, 0xCF);       // User data segment,  9A = 1111 0010
    writeTSS(5, 0x10, 0x0); 

    gdt_flush((uint32_t)&gdt_ptr);
    tss_flush();

}

void writeTSS(uint32_t num, uint16_t ss0, uint32_t esp0) {
    uint32_t base = (uint32_t) &tss_entry;
    uint32_t limit = base + sizeof(tss_entry);

    setGdtGate(num, base, limit, 0xE9, 0x00); // Task State Segment, E9 = 1110 1001
    memset(&tss_entry, 0, sizeof(tss_entry)); //Sets all values to 0

    tss_entry.ss0 = ss0;
    tss_entry.esp0 = esp0;

    tss_entry.cs = 0x08 | 0x3;
    tss_entry.ss =tss_entry.ds = tss_entry.es = tss_entry.fs = 
    tss_entry.gs = 0x10 | 0x3;
    
}


void setGdtGate(uint32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t granularity) {

    gdt_entries[num].base_low = (base & 0xFFFF);
    gdt_entries[num].base_middle = (base >> 16) & 0xFF;
    gdt_entries[num].base_high = (base >> 24) & 0xFF;

    gdt_entries[num].limit = (limit & 0xFFFF);
    gdt_entries[num].flags = (limit >> 16) & 0x0F;
    gdt_entries[num].flags |= (granularity & 0xF0);

    gdt_entries[num].access = access;

}

