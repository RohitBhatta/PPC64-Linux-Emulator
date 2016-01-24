#include "loader.h"

#include <elf.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#include "memory.h"

static void readAll(int fd, off_t offset, void* buffer, size_t size) {
    off_t rc = lseek(fd,offset,SEEK_SET);    
    if (rc != offset) {
        perror("seek failed");
        exit(-1);
    }

    size_t togo = size;
    void* p = buffer;

    while (togo > 0) {
        ssize_t n = read(fd,p,togo);
        if (n < 0) {
            perror("read");
            exit(-1);
        }
        p += n;
        togo -= n;
    }
}

uint64_t load(int fd, Memory* memory) {
    Elf64_Ehdr hdr;

    readAll(fd,0,&hdr,sizeof(Elf64_Ehdr));

    uint64_t hoff = __builtin_bswap64(hdr.e_phoff);

    for (uint32_t i=0; i<__builtin_bswap16(hdr.e_phnum); i++) {
        Elf64_Phdr phdr;
        readAll(fd,hoff,&phdr,sizeof(Elf64_Phdr));
        hoff += __builtin_bswap16(hdr.e_phentsize);

        if (__builtin_bswap32(phdr.p_type) == PT_LOAD) {
            uint64_t addr = __builtin_bswap64((uint64_t) phdr.p_vaddr);
            uint64_t filesz = __builtin_bswap64(phdr.p_filesz);
            off_t offset = __builtin_bswap64(phdr.p_offset);

            off_t rc = lseek(fd,offset,SEEK_SET);
            if (rc  != offset) {
                perror("file seek failed");
                exit(-1);
            }


            for (uint64_t i=0; i<filesz; i++) {
                uint8_t c;
                ssize_t n = read(fd,&c,1);
                
                if (n != 1) {
                    perror("read bytes");
                    exit(-1);
                }
                write8(memory,addr,c);
                addr ++;
            }
        }
    }

    return __builtin_bswap64(hdr.e_entry);
}
