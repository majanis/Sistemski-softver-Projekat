ASSEMBLER=./assembler
LINKER=./linker
EMULATOR=./emulator

${ASSEMBLER} -o test1.o ./tests/test1.s
${LINKER} -hex -o test1.hex test1.o
${EMULATOR} test1.hex