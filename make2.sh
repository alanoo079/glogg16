#/bin/bash

gcc src/vm.c -o build/vm
gcc src/assembler.c -o build/assembler

build/assembler src/glögg.gasm