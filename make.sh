#/bin/bash

gcc src/vm.c -o build/vm
gcc src/assembler.c -o build/assembler

build/assembler -b src/glögg.gasm
build/vm build/glögg16.bin