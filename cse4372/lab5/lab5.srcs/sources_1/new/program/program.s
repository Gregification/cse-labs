# the hex output needs to be moved to the parent folder afterwards
# assemble to elf :	 		riscv32-unknown-elf-gcc -o program program.s -march=rv32i -mabi=ilp32 -nostdlib -Tcse4372_riscv.ld
# convert to intel hex : 	objcopy -O ihex program ram.hex
# disassemble :				riscv32-unknown-elf-objdump -dr program

.globl	_start

.text

_start:
	
	andi t0, t0, 0

	nop
	nop
	nop

	or t0, t0, 2
 
	nop
	nop
	nop

	and t1, t1, 0

	nop
	nop
	nop

	or t1, t1, 5

	nop
	nop
	nop

	or t2, t1, t0

	nop
	nop
	nop

	slli t0, t0, 3

	nop
	nop
	nop

	add t3, t0, t1

	nop

	ebreak

.end
