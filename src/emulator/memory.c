#include "memory.h"
#include <stdlib.h>
#include <stdio.h>

/* Not built for speed but hopefully easier to understand */
/* one byte at a time is not a good idea */

#define N 527

typedef struct Entry {
    struct Entry* next;
    uint64_t addr;
    uint8_t data;
} Entry;

struct Memory {
    Entry *chains[N];    
};

Memory* newMemory() {
    Memory* p = (Memory*) malloc(sizeof(*p));
    if (p == NULL) {
        perror("create memory");
        exit(-1);
    }

    for (int i=0; i<N; i++) {
        p->chains[i] = NULL;
    }
    return p;
}

void freeMemory(Memory* p) {
    if (p == NULL) return;
    for (int i=0; i<N; i++) {
        Entry* q = p->chains[i];
        while (q != NULL) {
            Entry* next = q->next;
            free(q);
            q = next;
       }
    }
    free(p);
}

static Entry* get(Memory* p, uint64_t addr) {
    Entry** first = &p->chains[addr % N];
    Entry *prev = NULL;

    Entry* q = *first;
    while (q != NULL) {
        if (q->addr == addr) {
            if (prev != NULL) {
                /* MTF */
                prev->next = q->next;
                q->next = *first;
                *first = q;
            }
            return q;
        }
        prev = q;
        q = q->next;
    }

    q = (Entry*) malloc(sizeof(*q));
    if (q == NULL) {
        perror("allocate entry");
        exit(-1);
    }
    q->addr = addr;
    q->data = 0;
    q->next = *first;
    *first = q;
    return q;
}
    
void write8(Memory* mem, uint64_t addr, uint8_t data) {
    Entry* e = get(mem,addr);
    e->data = data;
}

void write16(Memory* mem, uint64_t addr, uint16_t data) {
    MISSING();
}

void write32(Memory* mem, uint64_t addr, uint32_t data) {
    MISSING();
}

void write64(Memory* mem, uint64_t addr, uint64_t data) {
    MISSING();
}

uint8_t read8(Memory* mem, uint64_t addr) {
    Entry *e = get(mem,addr);
    return e->data;
}

uint16_t read16(Memory* mem, uint64_t addr) {
    MISSING();
    return 0;
}

uint32_t read32(Memory* mem, uint64_t addr) {
    MISSING();
    return 0;
}

uint64_t read64(Memory* mem, uint64_t addr) {
    MISSING();
    return 0;
}

