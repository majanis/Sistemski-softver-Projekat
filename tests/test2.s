.extern processing
.global theend
.section initialize
.word 0x0010
.word 0x0200
.skip 12 #ivt
.section code
start:
ldr r6, $0xFEFE
ldr r1, $0x01
ldr r2, [r1 + 2]
cmp r1, r2
jne processing
theend:
ldr r3, $0x80
cmp r1, r3
halt
.end
