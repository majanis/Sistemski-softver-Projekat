ASSEMBLER=./assembler
LINKER=./linker
EMULATOR=./emulator

${ASSEMBLER} -o main.o ./tests/main.s
${ASSEMBLER} -o math.o ./tests/math.s
${ASSEMBLER} -o ivt.o ./tests/ivt.s
${ASSEMBLER} -o isr_reset.o ./tests/isr_reset.s
${ASSEMBLER} -o isr_terminal.o ./tests/isr_terminal.s
${ASSEMBLER} -o isr_timer.o ./tests/isr_timer.s
${ASSEMBLER} -o isr_user0.o ./tests/isr_user0.s
${LINKER} -hex -o program.hex ivt.o math.o main.o isr_reset.o isr_terminal.o isr_timer.o isr_user0.o
${EMULATOR} program.hex