ASSEMBLER=./assembler
LINKER=./linker
EMULATOR=./emulator

${ASSEMBLER} -o test2.o ./tests/test2.s
${ASSEMBLER} -o test3.o ./tests/test3.s
${LINKER} -hex -o test2.hex test2.o test3.o
${EMULATOR} test2.hex