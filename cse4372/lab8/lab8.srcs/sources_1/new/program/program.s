# the hex output needs to be moved to the parent folder afterwards
# 1. assemble to elf : 		riscv32-unknown-elf-gcc -o program program.s -march=rv32i -mabi=ilp32 -nostdlib -Tcse4372_riscv.ld
# 2. extract bin :			riscv32-unknown-elf-objcopy program -O binary program.bin
# 3. convert to hex : 		hexdump -v -e '1/4 "%08x" "\n"' program.bin > ../ram.hex
# dump assembly : 			riscv32-unknown-elf-objdump -dr program > program.asm_dump

.globl	_start

.text

_start:
	nop

# test memory R/W
	li t6, 0xccddaabb
	li t0, 0x00000200
	lw t1, 0(t0)
	sw t6, 0(t0)
	
	lw t1, 0(t0)
	add t2, t1, x0
	add t2, t1, x0
	add t2, t1, x0
	add t2, t1, x0

	lw t1, 0(t0)
	addi t2, x0, 1
	addi t2, x0, 2
	addi t2, x0, 3
	addi t2, x0, 4

	lh t1, 0(t0)

	lb t1, 0(t0)

	lbu t1, 0(t0)

# # test io R/W
	lw t1, 0(t0)

	add t4, t1, x0
	li t0, 0x00000002
	lh t1, 0(t0)
	# add t2, t1, x0
	# sh t3, 0(t2)
	nop
	nop
	add t4, t1, x0
	li t0, 0x00000003
	lb t1, 0(t0)
	# add t2, t3, x0
	# lw t1, 0(t0)
	nop
	nop
	add t4, t1, x0
	add t4, t1, x0
	add t4, t1, x0
	add t2, x0, t1

	nop
	nop
	nop
	nop
	nop
	nop


	addi t0, x0, 1 
	addi t0, x0, 2 
	addi t0, x0, 3 
	addi t0, x0, 4 
	addi t0, x0, 5 
	addi t0, x0, 6 
	addi t0, x0, 7 
	addi t0, x0, 8 
	add  t1, x0, t0 	
	add  t2, x0, t0 
	add  t3, x0, t0 
	add  t4, x0, t0

	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop

	addi t2, x0, 5 # number of itterations
	addi t1, x0, 3 # some value to be modified
	addi t0, x0, 0 # index
itterate:
	addi t0, t0, 1 # itterate
	slli t1, t1, 1 # do something
	bne t2, t0, itterate

	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop

	addi t1, x0, 1
	li t0, 0x80000000  	# led address
	li t4, 0x80000004	# pb address
ledloop:
	addi t1, t1, 1
	srli t2, t1, 17
	sw t2, 0(t0)
	nop
	nop
	lw t5, 0(t4)
	nop
	nop
	andi t5, t5, 2
	bne x0, t5, ledloop


	ebreak
.end
