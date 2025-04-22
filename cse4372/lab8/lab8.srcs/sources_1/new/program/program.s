# the hex output needs to be moved to the parent folder afterwards
# 1. assemble to elf : 		riscv32-unknown-elf-gcc -o program program.s -march=rv32i -mabi=ilp32 -nostdlib -Tcse4372_riscv.ld
# 2. extract bin :			riscv32-unknown-elf-objcopy program -O binary program.bin
# 3. convert to hex : 		hexdump -v -e '1/4 "%08x" "\n"' program.bin > ../ram.hex
# dump assembly : 			riscv32-unknown-elf-objdump -dr program > program.asm_dump

.globl	_start

.text

_start:
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

	li t0, 0x00000004
	lw t1, 0(t0)
	nop
	nop
	nop
	li t0, 0x00000008
	lw t1, 0(t0)
	# add t2, t1, x0
	# sh t3, 0(t2)
	nop
	nop
	nop
	li t0, 0x0000000c
	lw t1, 0(t0)
	# add t2, t3, x0
	# lw t1, 0(t0)
	nop
	nop
	nop
	add t2, x0, t1

	ebreak
.end
