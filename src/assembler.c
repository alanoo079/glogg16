#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

__attribute__((constructor))
void setup() {
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);
}

enum {
    // opcodes
    NOP, // done
    ADD, // done
    AND, // done
    XOR, // done
    LD, // done
    LDR, // done
    ST, // done
    STR, // done
    LEA, // done
    BRR, // done
    JMP, // done
    SHIFT,
    SET, // done
    BUSH,
    BOP,
    GLÖGG, // done

    // alias
    RET = 16,

    // special registers
    RGLÖGG = 7,
    RBASE = 6
};

/*
    glögg 16 assembler

    assembler aliases:
    - RET == JMP RBASE
    - 



*/

int parse_opcode(const char *name) {
    if (strcmp(name, "NOP") == 0) {
        return NOP;
    } else if (strcmp(name, "RET") == 0) {
        return RET;
    } else if (strcmp(name, "SET") == 0) {
        return SET;
    } else if (strcmp(name, "ADD") == 0) {
        return ADD;
    } else if (strcmp(name, "AND") == 0) {
        return AND;
    } else if (strcmp(name, "XOR") == 0) {
        return XOR;
    } else if (strcmp(name, "JMP") == 0) {
        return JMP;
    } else if (strcmp(name, "LD") == 0) {
        return LD;
    } else if (strcmp(name, "ST") == 0) {
        return ST;
    } else if (strcmp(name, "LEA") == 0) {
        return LEA;
    } else if (strcmp(name, "LDR") == 0) {
        return LDR;
    } else if (strcmp(name, "STR") == 0) {
        return STR;
    } else if (strcmp(name, "BRR") == 0) {
        return BRR;
    } else if (strcmp(name, "GLÖGG") == 0) {
        return GLÖGG;
    }
    return -1;
}


int parse_reg(const char *stringen) {
    if (strcmp(stringen, "RBASE") == 0 || strcmp(stringen, "rbase") == 0) {
        return 6;
    }
    if (strcmp(stringen, "RGLÖGG") == 0 || strcmp(stringen, "rglögg") == 0) {
        return 7;
    }
    if ((stringen[0] == 'R' || stringen[0] == 'r') && stringen[1] >= '0' && stringen[1] <= '5' && stringen[2] == '\0')
        return stringen[1] - '0';
    return -1;
}

int main(int argc, char **argv) {
    int binary = 0;
    const char *input_file;
 
    if (argc == 2) {
        input_file = argv[1];
    } else if (argc == 3 && strcmp(argv[1], "-b") == 0) {
        binary = 1;
        input_file = argv[2];
    } else {
        fprintf(stderr, "usage: %s [-b] prog.gasm\n", argv[0]);
        return 1;
    }
 
    FILE *f = fopen(input_file, "r");
    if (!f) {
        perror(input_file);
        return 1;
    }
 
    FILE *out = NULL;
    if (binary) {
        out = fopen("build/glögg16.bin", "wb");
        if (!out) {
            perror("build/glögg16.bin");
            fclose(f);
            return 1;
        }
    }
    char line[256];
    int line_number = 0;
    int address = 0;

    while (fgets(line, sizeof line, f)) {
        line_number++;
        //printf("%s\n", line);

        char *op = strtok(line, " ,\t\r\n");

        if (op == NULL || op[0] == ';') // empty line or comment
            continue;

        // data 0x32: 0x0a, 'g', 12, -1
        // pad NOP up to that address
        if (strcmp(op, "data") == 0) {
            char *addr_text = strtok(NULL, " ,\t\r\n");
            long start = strtol(addr_text, NULL, 0);

            if (start < address) {
                printf("%d program already reaches %ld, it goes up to %d", line_number, start, address);
                return 1;
            }

            while (address < start) {
                if (binary) {
                    uint8_t bytes[2] = {0x00, 0x00};
                    fwrite(bytes, 1, 2, out);
                } else {
                    printf("0x0000,\n");
                }
                address++;
            }

            char *value = strtok(NULL, " ,\t\r\n");
            if (value != NULL && value[0] == ':') {
                value = strtok(NULL, " ,\t\r\n");
            }

            while (value != NULL && value[0] != ';') { // stop at a comment
                long number;
                if (value[0] == '\'') // a character
                    number = (unsigned char)value[1];
                else
                    number = strtol(value, NULL, 0);  // 10, -5, 0x0a

                uint16_t word = number & 0xffff;
                if (binary) {
                    uint8_t bytes[2] = {
                        (uint8_t)(word >> 8),
                        (uint8_t)(word & 0xff)
                    };
                    fwrite(bytes, 1, 2, out);
                } else {
                    printf("0x%04x,\n", word);
                }
                address++;

                value = strtok(NULL, " ,\t\r\n");
            }
            continue;
        }

        uint16_t ins;

        switch (parse_opcode(op)) {
            case NOP:
                ins = 0x0000;
                break;
            case RET:
                ins = 0xa180;
                break;
            case JMP: {
                // JMP r0
                char *registe = strtok(NULL, " ,\t\r\n");
                int reg = parse_reg(registe);
                ins = (JMP << 12) | (reg << 6);
                break;
            }
            case AND:
            case XOR:
            case ADD: {
                // ADD R1, R1, #1
                // ADD R1, R1, R2
                char *register1 = strtok(NULL, " ,\t\r\n");
                char *register2 = strtok(NULL, " ,\t\r\n");
                char *value = strtok(NULL, " ,\t\r\n");
                //printf("%s %s %s\n", register1, register2, value);

                int dest = parse_reg(register1);
                int src = parse_reg(register2);
                int src2 = parse_reg(value);

                if (src2 != -1) {
                    //printf("%d\n", src2);
                    // ADD/AND/XOR reg |  opcode   |  dest  | source | 0| 0  0| source2|
                    ins = (parse_opcode(op) << 12) | (dest << 9) | (src << 6) | (0x0 << 2) | (src2);
                    break;
                } else {
                    long number;
                    if (value[0] == '#') // skip
                        value++;
                    if (value[0] == '\'') // a character
                        number = (unsigned char)value[1];
                    else 
                        number = strtol(value, NULL, 0);  // 10, -5, 0x0a  
                    ins = (parse_opcode(op) << 12) | (dest << 9) | (src << 6) | (1 << 5) | (number & 0x1f);
                    break;
                }
                break;
            }
            case SET: {
                char *registe = strtok(NULL, " ,\t\r\n");
                char *value = strtok(NULL, " ,\t\r\n");
                //printf("%s, %s, %s\n", op, registe, value);
                int dest;
                if (registe != NULL) {
                    dest = parse_reg(registe);
                } else {
                    dest = -1;
                }

                long number;
                if (value[0] == '#') // skip
                    value++;
                if (value[0] == '\'') // a character
                    number = (unsigned char)value[1];
                else
                    number = strtol(value, NULL, 0);  // 10, -5, 0x0a

                if (number < -256 || number > 255) {
                    fprintf(stderr, "line %d: %ld does not fit (-256 to 255)\n", line_number, number);
                    return 1;
                }

                // |  opcode   |     dest       |       (-256 to 255)      |
                ins = (SET << 12) | (dest << 9) | (number & 0x1ff);
                break;
            }
            case LEA:
            case ST:
            case LD: {
                // LD  R1, #5    R1 = memory[pc + 5]
                char *registe = strtok(NULL, " ,\t\r\n");
                char *value = strtok(NULL, " ,\t\r\n");
 
                int reg = parse_reg(registe);
 
                long offset;
                if (value[0] == '#') // skip
                    value++;
                if (value[0] == '\'') // a character
                    offset = (unsigned char)value[1];
                else
                    offset = strtol(value, NULL, 0);  // 10, -5, 0x0a
 
                if (offset < -256 || offset > 255) {
                    fprintf(stderr, "line %d: offset %ld does not fit (-256 to 255)\n", line_number, offset);
                    return 1;
                }
 
                // LD/ST/LEA |  opcode   |  reg   |        offset9           |
                ins = (parse_opcode(op) << 12) | (reg << 9) | (offset & 0x1ff);
                break;
            }
            case LDR:
            case STR: {
                // LDR R1, R2, #0    R1 = memory[R2 + 0]
                // STR R1, R2, #-1   memory[R2 - 1] = R1
                char *register1 = strtok(NULL, " ,\t\r\n");
                char *register2 = strtok(NULL, " ,\t\r\n");
                char *value = strtok(NULL, " ,\t\r\n");
 
                int reg = parse_reg(register1);
                int base = parse_reg(register2);
 
                long offset;
                if (value[0] == '#') // skip
                    value++;
                if (value[0] == '\'') // a character
                    offset = (unsigned char)value[1];
                else
                    offset = strtol(value, NULL, 0);  // 10, -5, 0x0a
 
                if (offset < -32 || offset > 31) {
                    fprintf(stderr, "line %d: offset %ld does not fit (-32 to 31)\n", line_number, offset);
                    return 1;
                }

                // LDR/STR |  opcode   |  reg   |  base  |     offset6     |
                ins = (parse_opcode(op) << 12) | (reg << 9) | (base << 6) | (offset & 0x3f);
                break;
            }
            case BRR: {
                // BRR p, R1, #-4     jump if r1 > 0
                // BRR nz, R1, #3     jump if r1 < or = 0
                // BRR nzp, R0, #2    always jump
                char *flags = strtok(NULL, " ,\t\r\n");
                char *registe = strtok(NULL, " ,\t\r\n");
                char *value = strtok(NULL, " ,\t\r\n");
 
                if (flags == NULL || registe == NULL || value == NULL) {
                    fprintf(stderr, "line %d: usage: BRR nzp, reg, #offset\n", line_number);
                    return 1;
                }
 
                int flagss = 0;
                for (int i = 0; flags[i] != '\0'; i++) {
                    if (flags[i] == 'n' || flags[i] == 'N') {
                        flagss |= 4;
                    } else if (flags[i] == 'z' || flags[i] == 'Z') {
                        flagss |= 2;
                    } else if (flags[i] == 'p' || flags[i] == 'P') {
                        flagss |= 1;
                    } else {
                        printf("bad flags line %d", line_number);
                        return -1;
                    }
                }
 
                int reg = parse_reg(registe);
 
                long offset;
                if (value[0] == '#') // skip
                    value++;
                offset = strtol(value, NULL, 0);  // -4, 3, 0x0a
 
                if (offset < -32 || offset > 31) {
                    fprintf(stderr, "line %d: offset %ld does not fit (-32 to 31)\n", line_number, offset);
                    return 1;
                }
 
                // BRR |  opcode   | flags  |  reg   |     offset6     |
                ins = (BRR << 12) | (flagss << 9) | (reg << 6) | (offset & 0x3f);
                break;
            }
            case GLÖGG: {
                char *number_text = strtok(NULL, " ,\t\r\n");
                long glögg = strtol(number_text, NULL, 0);

                if (!(glögg == 0 || glögg == 1 || glögg == 2 || glögg == 3)) {
                    fprintf(stderr, "line %d: glögg number must be 0, 1, 2 or 3\n", line_number);
                    return 1;
                }

                //  opcode    | glögg number (bits 11-8)
                ins = (GLÖGG << 12) | (glögg << 8);
                break;
            }

            default:
                fprintf(stderr, "line %d: unknown instruction %s\n", line_number, op);
                return 1;
        }

        if (binary) {
            uint8_t bytes[2] = {
                (uint8_t)(ins >> 8),
                (uint8_t)(ins & 0xff)
            };

            fwrite(bytes, 1, 2, out);
            address++;
        } else {
            printf("0x%04x,\n", ins);
            address++;
        }
    }
}