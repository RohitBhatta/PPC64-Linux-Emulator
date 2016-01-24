#ifndef _MEMORY_H_
#define _MEMORY_H_

#include <stdint.h>

#define MISSING() do { \
    printf("missing code %s:%d\n", __FILE__, __LINE__); \
    exit(-1); \
} while (0)

/* Hide the implementation details */
struct Memory;
typedef struct Memory Memory;

/* Memory API */

extern Memory* newMemory();
extern void freeMemory(Memory*);

extern void write8(Memory* mem, uint64_t address, uint8_t data);
extern void write16(Memory* mem, uint64_t address, uint16_t data);
extern void write32(Memory* mem, uint64_t address, uint32_t data);
extern void write64(Memory* mem, uint64_t address, uint64_t data);

extern uint8_t read8(Memory* mem, uint64_t address);
extern uint16_t read16(Memory* mem, uint64_t address);
extern uint32_t read32(Memory* mem, uint64_t address);
extern uint64_t read64(Memory* mem, uint64_t address);


#endif
