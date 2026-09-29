# glögg16
super simple custom 16-bit cpu emulator, currently wip

comes with a vm and assembler wowowowow

```js
    glögg16 instruction set

    bit              15 14 13 12 11  10  9  8  7  6  5  4  3  2  1  0
                    ------------------------------------------------
    ADD/AND/XOR reg |  opcode   |  dest  | source | 0| 0  0| source2|
    ADD/AND/XOR imm |  opcode   |  dest  | source | 1|  value       |
    LD/ST/LEA       |  opcode   |  reg   |        offset            |
    LDR/STR         |  opcode   |  reg   |  base  |     offset6     |
    BRR             |  opcode   | flags  |  reg   |     offset6     |
    JMP             |  opcode   | 0  0  0|  base  | 0  0  0  0  0  0|
    SET             |  opcode   |  dest  |       (-256 to 255)      |
    GLÖGG           |  opcode   |glöggnumber|        unused         |

    Opcodes:
        0 NOP
            / no operation
            / ex: NOP
        1 ADD
            / adds either 2 registers together into a dest register
            / or adds a register and a value in range -16 to 15
            / ex1: ADD r2, r1, r2
            / ex2: ADD r2, r1, #6
        2 AND
            / does the AND operation on either 2 regs and puts the result
            / into a dest reg or ANDs a reg and a value in range -16 to 15
            / ex1: AND r2, r1, r2
            / ex2: AND r2, r1, #6
        3 XOR
            / does the XOR operation on either 2 regs and puts the result
            / into a dest reg or XORs a reg and a value in range -16 to 15
            / ex1: XOR r2, r1, r2
            / ex2: AND r2, r1, #6
        4 LD
        5 LDR
        6 ST
        7 STR
        8 LEA
        9 BRR
        10 JMP
        11 SHIFT // todo
        12 SET
        13 BUSH // todo
        14 BOP // todo
        15 GLÖGG

        GLÖGG 0: halt
        GLÖGG 1: print char in RGLÖGG
        GLÖGG 2: exit with status code in RGLÖGG
        GLÖGG 3: getchar from user into RGLÖGG
```
