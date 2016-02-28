#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

#include "memory.h"
#include "loader.h"
#include "state.h"

int main(int argc, const char **argv) {
    if (argc != 2) {
        fprintf(stderr,"usage: %s <executable>\n",argv[0]);
        exit(-1);
    }
    const char* fileName = argv[1];

    int fd = open(fileName,O_RDONLY);
    if (fd == -1) {
        perror("open file");
        exit(-1);
    }

    Memory* memory = newMemory();
    uint64_t e = load(fd,memory);
    uint64_t pc = read64(memory,e);
    uint64_t r2 = read64(memory,e+8);
    //printf("entry = %lx\n",e);
    //printf("pc = %lx\n",pc);
    //printf("r2 = %lx\n",r2);

    State* s = newState(memory);
    s->pc = pc;
    s->gprs[2] = r2;
    run(s);
    return 0;
}
