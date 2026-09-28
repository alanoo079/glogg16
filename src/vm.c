/*
    glögg16

    bit              15 14 13 12 11  10  9  8  7  6  5  4  3  2  1  0
                    ------------------------------------------------
    ADD/AND/XOR reg |  opcode   |  dest  | source | 0| 0  0| source2|
    ADD/AND/XOR imm |  opcode   |  dest  | source | 1|  value       |
    LD/ST/LEA       |  opcode   |  reg   |        offset            |
    LDR/STR         |  opcode   |  reg   |  base  |     offset6     |
    BRR             |  opcode   | flags  |  reg   |     offset6     |
    JMP             |  opcode   | 0  0  0|  base  | 0  0  0  0  0  0|
    RET             |  opcode   | 0  0  0  0|        unused         |
    SET             |  opcode   |  dest  |       (-256 to 255)      |
    GLÖGG           |  opcode   |glöggnumber|        unused         |

    Opcodes:
        0 NOP // done
        1 ADD // done
        2 AND // done
        3 XOR // done
        4 LD // done
        5 LDR // done
        6 ST // done
        7 STR // done
        8 LEA // done
        9 BRR // done
        10 JMP // done
        11 RET // done
        12 SET // done
        13 BUSH
        14 BOP
        15 GLÖGG // done

        GLÖGG 0: halt
        GLÖGG 1: print char in RGLÖGG
        GLÖGG 2: exit with status code in RGLÖGG

*/

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

__attribute__((constructor))
void setup() {
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);
}

enum {
    NOP,
    ADD,
    AND,
    XOR,
    LD,
    LDR,
    ST,
    STR,
    LEA,
    BRR,
    JMP,
    RET,
    SET,
    GLÖGG = 15,
    RGLÖGG = 7,
    RBASE = 6
};

typedef struct {
    const char *name;
    uint16_t value;
} Register;

Register registers[8] = {
    {"R0", 0}, {"R1", 0}, {"R2", 0}, {"R3", 0},
    {"R4", 0}, {"R5", 0}, {"RBASE", 0}, {"RGLÖGG", 0}
};

uint16_t sign_extension(uint16_t value, int bits) {
    if ((value >> (bits - 1)) & 1)
        value |= 0xFFFF << bits;

    return value;
}

uint16_t get_bits(uint16_t value, int high, int low) {
    // want 11-8
    uint16_t abow = (value << (15-high));
    abow = (abow >> ((15-high)+low));

    return abow;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s glögg16.bin\n", argv[0]);
        return 1;
    }
 
    FILE *file = fopen(argv[1], "rb");
    if (!file) {
        perror(argv[1]);
        return 1;
    }
 
    uint16_t *memory = calloc(65536, sizeof(uint16_t));

    size_t i = 0;
    int high, low;
    while ((high = fgetc(file)) != EOF) {
        low = fgetc(file);
        memory[i++] = (uint16_t)((high << 8) | low);
    }
    fclose(file);

    uint16_t pc = 0;
    bool running = true;

    while (running) {
        // fetch
        uint16_t ins_reg = memory[pc];
        pc += 1;
        //printf("%x, %d\n", ins_reg, pc);

        // decode
        uint16_t opcode = ins_reg >> 12;

        // execute
        switch (opcode) {
            case NOP:
                break;
            case ADD: {
                uint16_t dest = get_bits(ins_reg, 11, 9);
                uint16_t src = get_bits(ins_reg, 8, 6);
                uint16_t bit5 = get_bits(ins_reg, 5, 5);

                if (bit5) {
                    uint16_t number = sign_extension(get_bits(ins_reg, 4, 0), 5);
                    registers[dest].value = registers[src].value + number;
                } else {
                    uint16_t src2 = get_bits(ins_reg, 2, 0);
                    registers[dest].value = registers[src].value + registers[src2].value;
                }
                //printf("%d\n", registers[dest].value);
                break;
            }
            case AND: {
                uint16_t dest = get_bits(ins_reg, 11, 9);
                uint16_t src = get_bits(ins_reg, 8, 6);
                uint16_t bit5 = get_bits(ins_reg, 5, 5);
 
                if (bit5) {
                    uint16_t number = sign_extension(get_bits(ins_reg, 4, 0), 5);
                    registers[dest].value = registers[src].value & number;
                } else {
                    uint16_t src2 = get_bits(ins_reg, 2, 0);
                    registers[dest].value = registers[src].value & registers[src2].value;
                }
                break;
            }
            case XOR: {
                uint16_t dest = get_bits(ins_reg, 11, 9);
                uint16_t src = get_bits(ins_reg, 8, 6);
                uint16_t bit5 = get_bits(ins_reg, 5, 5);
 
                if (bit5) {
                    uint16_t number = sign_extension(get_bits(ins_reg, 4, 0), 5);
                    registers[dest].value = registers[src].value ^ number;
                } else {
                    uint16_t src2 = get_bits(ins_reg, 2, 0);
                    registers[dest].value = registers[src].value ^ registers[src2].value;
                }
                break;
            }
            case LD: {
                uint16_t dest = get_bits(ins_reg, 11, 9);
                uint16_t aa = sign_extension(get_bits(ins_reg, 8, 0), 9);
                uint16_t address = pc + aa;
                registers[dest].value = memory[address];
                break;
            }
            case ST: {
                uint16_t src = get_bits(ins_reg, 11, 9);
                uint16_t aa = sign_extension(get_bits(ins_reg, 8, 0), 9);
                uint16_t address = pc + aa;
                memory[address] = registers[src].value;
                break;
            }
            case LDR: {
                uint16_t dest = get_bits(ins_reg, 11, 9);
                uint16_t base = get_bits(ins_reg, 8, 6);
                uint16_t aa = sign_extension(get_bits(ins_reg, 5, 0), 6);
                uint16_t address = registers[base].value + aa;
                registers[dest].value = memory[address];
                break;
            }
            case STR: {
                uint16_t src = get_bits(ins_reg, 11, 9);
                uint16_t base = get_bits(ins_reg, 8, 6);
                uint16_t aa = sign_extension(get_bits(ins_reg, 5, 0), 6);
                uint16_t address = registers[base].value + aa;
                memory[address] = registers[src].value;
                break;
            }
            case LEA: {
                uint16_t dest = get_bits(ins_reg, 11, 9);
                uint16_t offset = sign_extension(get_bits(ins_reg, 8, 0), 9);
                registers[dest].value = pc + offset;
                break;
            }
            case BRR: {
                uint16_t flags = get_bits(ins_reg, 11, 9);
                uint16_t reg = get_bits(ins_reg, 8, 6);
                uint16_t offset = sign_extension(get_bits(ins_reg, 5, 0), 6);
 
                bool want_negative = get_bits(flags, 2, 2);
                bool want_zero = get_bits(flags, 1, 1);
                bool want_positive = get_bits(flags, 0, 0);
 
                bool jump = false;
                if ((int16_t)registers[reg].value < 0 && want_negative) {
                    jump = true;
                } else if ((int16_t)registers[reg].value == 0 && want_zero) {
                    jump = true;
                } else if ((int16_t)registers[reg].value > 0 && want_positive) {
                    jump = true;
                }

                if (jump) {
                    pc = pc + offset;
                }

                break;
            }
            case JMP: {
                uint16_t base = get_bits(ins_reg, 8, 6);
                pc = registers[base].value;
                break;
            }
            case RET: {
                pc = registers[RBASE].value;
                break;
            }
            case SET: {
                uint16_t dest = get_bits(ins_reg, 11, 9);
                uint16_t number = sign_extension(get_bits(ins_reg, 8, 0), 9);
                registers[dest].value = number;
                //printf("%x\n", registers[RGLÖGG].value);
                break;
            }
            case GLÖGG: {
                uint16_t glögg = get_bits(ins_reg, 11, 8);
                switch (glögg) {
                    case 0:
                        exit(0);
                        break;
                    case 1:
                        //printf("%x\n", registers[RGLÖGG].value);
                        putchar((char)registers[RGLÖGG].value);
                        break;
                    case 2:
                        exit(registers[RGLÖGG].value);
                }
                break;
            }
            default:
                printf("invalid opcode\n");
                break;
        }


    }

    return 0;
}