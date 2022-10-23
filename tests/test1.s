#.extern exsimb
.section initialize
.word 0x0010
.word 0x0200
.skip 12 #ivt
#.extern exsimb
.section my_start
.global a,b
b:
ldr r4, $0x30
ldr r5, $0x9
sub r4, r5
jmp %a
c:
ldr r0, $0x5
ldr r2, $0x7
xchg r0,r2
cmp r2, r0
halt
a:
sub r4, r5
jmp %c
.end