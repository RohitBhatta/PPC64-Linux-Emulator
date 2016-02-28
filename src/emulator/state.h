#ifndef _STATE_H_
#define _STATE_H_

#include <stdint.h>
#include "memory.h"

struct State {
    uint64_t pc;
    uint16_t gprs[32];
    uint64_t lr;
    uint64_t cr;
    Memory *mem;
};

struct State;

typedef struct State State;

extern State* newState(Memory* memory);

extern void run(State* state);

#endif
