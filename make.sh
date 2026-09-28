#/bin/bash

gcc vm.c -o vm
gcc assembler.c -o assembler

./assembler -b a.gasm
./vm glögg16.bin