#.section initialize
.global processing
.extern theend
.section testing
processing:
ldr r2, $25
processing2:
ldr r1, $5
sub r2, r1
cmp r2, r1
jne processing2
ldr r4, $0x01
ldr r0, [r4 + 0x02]
jmp theend
.end
