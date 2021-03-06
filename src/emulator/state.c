#include "state.h"
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>

State* newState(Memory* memory) {
    State *s = (State *) malloc(sizeof(State));

    uint64_t link = 0;
    uint64_t cond = 0;

    //Initialize state variables here
    for (int i = 0; i < 32; i++) {
        s -> gprs[i] = 0;
    }
    s -> lr = link;
    s -> cr = cond;
    s -> mem = memory;

    return s;
}

void run(State* s) {
    Memory *memory = s -> mem;
    //Set exit to 1 when there is a system call
    int exit = 0;
    while (!exit) {
        uint32_t instr = read32(memory, s -> pc);
        uint16_t opcode = (uint16_t) ((instr >> 26) & 0x3F);
        switch (opcode) {
            //cmpdi
            case 11 : {
                uint16_t srcA = (uint16_t) ((instr >> 16) & 0x1F);
                int imm = (int) (instr << 16);
                imm = imm >> 16;
                if ((s -> gprs[srcA]) < (imm)) {
                    s -> cr = 0x80000000;
                }
                else if ((s -> gprs[srcA]) > (imm)) {
                    s -> cr = 0x40000000;
                }
                else {
                    s -> cr = 0x20000000;
                }
                s -> pc += 4;
                break;
            }
            //addi, li
            case 14 : {
                uint16_t dest = (uint16_t) ((instr >> 21) & 0x1F);
                uint16_t src = (uint16_t) ((instr >> 16) & 0x1F);
                int imm = (int) (instr << 16);
                imm = imm >> 16;
                //addi
                if (src != 0) {
                    s -> gprs[dest] = s -> gprs[src] + imm;
                }
                //li
                else {
                    s -> gprs[dest] = imm;
                }
                s -> pc += 4;
                break;
            }
            //lis
            case 15 : {
                uint16_t dest = (uint16_t) ((instr >> 21) & 0x1F);
                uint16_t src = (uint16_t) ((instr >> 16) & 0x1F);
                int imm = (int) (instr << 16 & 0xFFFF0000);
                if (src == 0) {
                    s -> gprs[dest] = imm;
                }
                else {
                    s -> gprs[dest] = s -> gprs[src] + imm;
                }
                s -> pc += 4;
                break;
            }
            //branch conditionals: beq, bne, ble, bge            
            case 16 : {
                uint16_t bo = (uint16_t) ((instr >> 23) & 0x7);
                uint16_t bi = (uint16_t) ((instr >> 16) & 0x1F);
                int target = (int) (instr << 16);
                target = target >> 18;
                target = target << 2;
                uint64_t conds = s -> cr;
                conds = (conds >> 29) & 0x7;
                if (bo == 1 && bi == 0 && conds != 0b100) {
                    s -> pc += target;
                }
                else if (bo == 1 && bi == 1 && conds != 0b010) {
                    s -> pc += target;
                }
                else if (bo == 1 && bi == 2 && conds != 0b001) {
                    s -> pc += target;
                }
                else if (bo == 3 && bi == 0 && conds == 0b100) {
                    s -> pc += target;
                }
                else if (bo == 3 && bi == 1 && conds == 0b010) {
                    s -> pc += target;
                }
                else if (bo == 3 && bi == 2 && conds == 0b001) {
                    s -> pc += target;
                }
                else {
                    s -> pc += 4;
                }
                break;
            }
            //sc, system call, either print or exit (if exit set exit to 1)
            case 17 : {
                //printf("%lu\n", s -> gprs[0]);
                //exit
                if (s -> gprs[0] == 1) {
                    exit = 1;
                }
                //print
                else {
                    uint64_t r4 = s -> gprs[4];
                    char printNum = read8(memory, r4);
                    printf("%c", printNum);
                    s -> pc += 4;
                }
                break;
            }
            //b, bl
            case 18 : {
                uint16_t ext = (uint16_t) (instr & 0x1);
                int eAddr = (int) (instr << 6);
                eAddr = eAddr >> 8;
                eAddr = eAddr << 2;
                switch (ext) {
                    //b
                    case 0 : {
                        s -> pc += eAddr;
                        break;
                    }
                    //bl
                    case 1 : {
                        s -> lr = s -> pc + 4;
                        s -> pc += eAddr;
                        break;
                    }
                }
                break;
            }
            //blr, branch to linked register
            case 19 : {
                s -> pc = s -> lr;
                break;
            }
            //ori
            case 24 : {
                uint16_t dest = (uint16_t) ((instr >> 16) & 0x1F);
                uint16_t src = (uint16_t) ((instr >> 21) & 0x1F);
                uint64_t imm = (uint64_t) (instr & 0xFFFF);
                s -> gprs[dest] = (s -> gprs[src]) | imm;
                s -> pc += 4;
                break;
            }
            //oris
            case 25 : {
                uint16_t dest = (uint16_t) ((instr >> 16) & 0x1F);
                uint16_t src = (uint16_t) ((instr >> 21) & 0x1F);
                uint64_t imm = (uint64_t) (instr & 0xFFFF);
                imm = (imm << 16) & 0xFFFF0000;
                s -> gprs[dest] = (s -> gprs[src]) | imm;
                s -> pc += 4;
                break;
            }
            //rldicr
            case 30 : {
                uint16_t dest = (uint16_t) ((instr >> 16) & 0x1F);
                uint64_t temp = s -> gprs[dest];
                temp = (temp << 32) & 0xFFFFFFFF00000000;
                s -> gprs[dest] = temp;
                s -> pc += 4;
                break;
            }
            //mr, add, mulld, cmpd, mflr, mtlr
            case 31 : {
                uint16_t ext = (uint16_t) ((instr >> 1) & 0x1FF);
                switch (ext) {
                    //add
                    case 266 : {
                        uint16_t dest = (uint16_t) ((instr >> 21) & 0x1F);
                        uint16_t srcA = (uint16_t) ((instr >> 16) & 0x1F);
                        uint16_t srcB = (uint16_t) ((instr >> 11) & 0x1F);
                        s -> gprs[dest] = (s -> gprs[srcA]) + (s -> gprs[srcB]);
                        s -> pc += 4;
                        break;
                    }
                    //subf
                    case 40 : {
                        uint16_t dest = (uint16_t) ((instr >> 21) & 0x1F);
                        uint16_t srcA = (uint16_t) ((instr >> 16) & 0x1F);
                        uint16_t srcB = (uint16_t) ((instr >> 11) & 0x1F);
                        s -> gprs[dest] = (s -> gprs[srcB]) - (s -> gprs[srcA]);
                        s -> pc += 4;
                        break;
                    }
                    //mulld, might be 235
                    case 233 : {
                        uint16_t dest = (uint16_t) ((instr >> 21) & 0x1F);
                        uint16_t srcA = (uint16_t) ((instr >> 16) & 0x1F);
                        uint16_t srcB = (uint16_t) ((instr >> 11) & 0x1F);
                        s -> gprs[dest] = (s -> gprs[srcA]) * (s -> gprs[srcB]);
                        s -> pc += 4;
                        break;
                    }
                    //divdu
                    case 457 : {
                        uint16_t dest = (uint16_t) ((instr >> 21) & 0x1F);
                        uint16_t srcA = (uint16_t) ((instr >> 16) & 0x1F);
                        uint16_t srcB = (uint16_t) ((instr >> 11) & 0x1F);
                        s -> gprs[dest] = (s -> gprs[srcA])/(s -> gprs[srcB]);
                        s -> pc += 4;
                        break;
                    }
                    //mr
                    case 444 : {
                        uint16_t dest = (uint16_t) ((instr >> 16) & 0x1F);
                        uint16_t src = (uint16_t) ((instr >> 21) & 0x1F);
                        s -> gprs[dest] = s -> gprs[src];
                        s -> pc += 4;
                        break;
                    }
                    //mflr
                    case 339 : {
                        uint16_t dest = (uint16_t) ((instr >> 21) & 0x1F);
                        s -> gprs[dest] = s -> lr;
                        s -> pc += 4;
                        break;
                    }
                    //mtlr
                    case 467 : {
                        uint16_t src = (uint16_t) ((instr >> 21) & 0x1F);
                        s -> lr = s -> gprs[src];
                        s -> pc += 4;
                        break;
                    }
                    //cmpd
                    case 0 : {
                        uint16_t srcA = (uint16_t) ((instr >> 16) & 0x1F);
                        uint16_t srcB = (uint16_t) ((instr >> 11) & 0x1F);
                        if ((s -> gprs[srcA]) < (s -> gprs[srcB])) {
                            s -> cr = 0x80000000;
                        }
                        else if ((s -> gprs[srcA]) > (s -> gprs[srcB])) {
                            s -> cr  = 0x40000000;
                        }
                        else {
                            s -> cr = 0x20000000;
                        }
                        s -> pc += 4;
                        break;
                    }
                    //stbx
                    case 215 : {
                        uint16_t dest = (uint16_t) ((instr >> 21) & 0x1F);
                        uint16_t srcA = (uint16_t) ((instr >> 16) & 0x1F);
                        uint16_t srcB = (uint16_t) ((instr >> 11) & 0x1F);
                        uint64_t eAddr;
                        if (srcA == 0) {
                            eAddr = s -> gprs[srcB];
                        }
                        else {
                            eAddr = (s -> gprs[srcA]) + (s -> gprs[srcB]);
                        }
                        uint8_t temp = (uint8_t) ((s -> gprs[dest]) & 0xFF);
                        write8(memory, eAddr, temp);
                        s -> pc += 4;
                        break;
                    }
                    default : {
                        exit = 1;
                        break;
                    }
                }
                break;
            }
            //stb
            case 38 : {
                uint16_t dest = (uint16_t) (instr >> 21 & 0x1F);
                uint16_t src = (uint16_t) (instr >> 16 & 0x1F);
                int imm = (int) (instr << 16);
                imm = imm >> 16;
                uint64_t eAddr;
                if (src == 0) {
                    eAddr = imm;
                }
                else {
                    eAddr = s -> gprs[src] + imm;
                }
                uint8_t temp = (uint8_t) ((s -> gprs[dest]) & 0xFF);
                write8(memory, eAddr, temp);
                s -> pc += 4;
                break;
            }
            //ld
            case 58 : {
                uint16_t dest = (uint16_t) ((instr >> 21) & 0x1F);
                uint16_t src = (uint16_t) ((instr >> 16) & 0x1F);
                int imm = (int) (instr << 16);
                imm = imm >> 18;
                imm = imm << 2;
                uint64_t eAddr;
                //Might have to multiply imm by 4
                if (src == 0) {
                    eAddr = imm;
                }
                else {
                    eAddr = s -> gprs[src] + imm;
                }
                s -> gprs[dest] = read64(memory, eAddr);
                s -> pc += 4;
                break;
            }
            //stdu, std
            case 62 : {
                //uint16_t ext = (uint16_t) (instr & 0x2);
                int ext = (int) (instr << 30);
                ext = ext >> 30;
                switch (ext) {
                    //std
                    case 0 : {
                        uint16_t dest = (uint16_t) ((instr >> 21) & 0x1F);
                        uint16_t src = (uint16_t) ((instr >> 16) & 0x1F);
                        int imm = (int) (instr << 16);
                        imm = imm >> 18;
                        imm = imm << 2;
                        uint64_t eAddr;
                        if (src == 0) {
                            eAddr = imm;
                        }
                        else {
                            eAddr = s -> gprs[src] + imm;
                        }
                        write64(memory, eAddr, s -> gprs[dest]);
                        s -> pc += 4;
                        break;
                    }
                    //stdu
                    case 1 : {
                        uint16_t dest = (uint16_t) ((instr >> 21) & 0x1F);
                        uint16_t src = (uint16_t) ((instr >> 16) & 0x1F);
                        int imm = (int) (instr << 16);
                        imm = imm >> 18;
                        imm = imm << 2;
                        uint64_t eAddr = (uint64_t) (s -> gprs[src] + imm);
                        write64(memory, eAddr, s -> gprs[dest]);
                        s -> gprs[src] = eAddr;
                        s -> pc += 4;
                        break;
                    }
                    default : {
                        exit = 1;
                        break;
                    }
                }
                break;
            }
            default : {
                exit = 1;
                break;
            }
        }
    }
}
