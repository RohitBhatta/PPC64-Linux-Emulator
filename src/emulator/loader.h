#ifndef _LOADER_H_
#define _LOADER_H_

#include <stdint.h>
#include "memory.h"

extern uint64_t load(int fd, Memory* memory);

#endif
