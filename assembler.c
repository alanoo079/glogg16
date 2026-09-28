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
    NOP, // done
    ADD, // done
    AND,
    XOR,
    LD,
    LDR,
    ST,
    STR,
    LEA,
    BRR,
    JMP,
    RET, // done
    SET, // done
    GLÖGG = 15, // done
    RGLÖGG = 7,
    RBASE = 6
};

int parse_opcode(const char *name) {
    if (strcmp(name, "NOP") == 0) {
        return NOP;
    } else if (strcmp(name, "RET") == 0) {
        return RET;
    } else if (strcmp(name, "SET") == 0) {
        return SET;
    } else if (strcmp(name, "ADD") == 0) {
        return ADD;
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
        out = fopen("glögg16.bin", "wb");
        if (!out) {
            perror("glögg16.bin");
            fclose(f);
            return 1;
        }
    }
    char line[256];
    int line_number = 0;

    while (fgets(line, sizeof line, f)) {
        line_number++;
        //printf("%s\n", line);

        char *op = strtok(line, " ,\t\r\n");

        if (op == NULL || op[0] == ';') // empty line or comment
            continue;

        uint16_t ins;

        switch (parse_opcode(op)) {
            case NOP:
                ins = 0x0000;
                break;
            case RET:
                ins = 0xb000;
                break;
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
                    ins = (ADD << 12) | (dest << 9) | (src << 6) | (0x0 << 2) | (src2);
                    break;
                } else {
                    long number;
                    if (value[0] == '#') // skip
                        value++;
                    if (value[0] == '\'') // a character
                        number = (unsigned char)value[1];
                    else 
                        number = strtol(value, NULL, 0);  // 10, -5, 0x0a  
                    ins = (ADD << 12) | (dest << 9) | (src << 6) | (1 << 5) | (number & 0x1f);
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
            case GLÖGG: {
                char *number_text = strtok(NULL, " ,\t\r\n");
                long glögg = strtol(number_text, NULL, 0);

                if (!(glögg == 0 || glögg == 1 || glögg == 2)) {
                    fprintf(stderr, "line %d: glögg number must be 0, 1 or 2\n", line_number);
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
        } else {
            printf("0x%04x,\n", ins);
        }
    }
}