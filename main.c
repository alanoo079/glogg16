/*
    glögg16

    bit              15 14 13 12 11  10  9  8  7  6  5  4  3  2  1  0
                    ------------------------------------------------
    ADD/AND/XOR reg |  opcode   |  dest  | source | 0| 0  0| source2|
    ADD/AND/XOR imm |  opcode   |  dest  | source | 1|  value       |
    LD/ST/LEA       |  opcode   |  reg   |        offset            |
    LDR/STR         |  opcode   |  reg   |  base  |     offset6     |
    BR              |  opcode   | flags  |  reg   |     offset6     |
    JMP             |  opcode   | 0  0  0|  base  | 0  0  0  0  0  0|
    RET             |  opcode   | 0  0  0  0|        unused         |
    SET             |  opcode   |  dest  |       (-256 to 255)      |
    GLÖGG           |  opcode   |glöggnumber|        unused         |

    Opcodes:
        0 NOP // done
        1 ADD // done
        2 AND
        3 XOR
        4 LD
        5 LDR
        6 ST
        7 STR
        8 LEA
        9 BR
        10 JMP
        11 RET
        12 SET // done
        13-14 spare
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
    BR,
    JMP,
    RET,
    SET,
    GLÖGG = 15,
    RGLÖGG = 7
};

typedef struct {
    const char *name;
    uint16_t value;
} Register;

Register registers[8] = {
    {"R0", 0}, {"R1", 0}, {"R2", 0}, {"R3", 0},
    {"R4", 0}, {"R5", 0}, {"R6", 0}, {"RGLÖGG", 0}
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

int main(void) {
    uint16_t program[] = {
        0xce47, // SET RGLÖGG, #G
        0xf100, // GLÖGG 1
        0xce4c, //SET RGLÖGG, #L
        0xf100, // GLÖGG 1
        0xcec3, // SET RGLÖGG, #Ö, first byte
        0xf100, // GLÖGG 1
        0xce96, // SET RGLÖGG, #Ö, second byte
        0xf100, // GLÖGG 1
        0xce47, // SET RGLÖGG, #G
        0xf100, // GLÖGG 1
        0xce47, // SET RGLÖGG, #G
        0xf100, // GLÖGG 1
        0xce0a, // SET RGLÖGG, #0x0a
        0xf100, // GLÖGG 1
        0xce00, // SET RGLÖGG #0
        0xf200, // GLÖGG 2
    };

    uint16_t *memory = calloc(65536, sizeof(uint16_t));

    for (size_t i = 0; i < sizeof(program)/2; i++) {
        memory[i] = program[i];
        //printf("%x\n", memory[i]);
    }

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