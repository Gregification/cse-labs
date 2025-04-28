# modified LED address

.globl _start

.text

_start:

    LA      t0, 0x80000000
    LA      t1, 0x1
loop:
    XORI    t1, t1, 0x1
    SW      t1, (t0)
    LA      t2, 1000000
loop2:
    ADDI    t2, t2, -1
    BNEZ   t2, loop2
    J       loop
    NOP
    NOP
    NOP
    NOP
    EBREAK
    NOP
    NOP
    NOP
    
.end

