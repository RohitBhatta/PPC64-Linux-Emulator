#ifndef _STATE_H_
#define _STATE_H_

#include <stdint.h>
#include "memory.h"

struct State;

typedef struct State State;

extern State* newState(Memory* memory);

extern void run(State* state);

#endif
