# the hex output needs to be moved to the parent folder afterwards
# 1. assemble to elf : 		riscv32-unknown-elf-gcc -o program program.s -march=rv32i -mabi=ilp32 -nostdlib -Tcse4372_riscv.ld
# 2. extract bin :			riscv32-unknown-elf-objcopy program -O binary program.bin
# 3. convert to hex : 		hexdump -v -e '1/4 "%08x" "\n"' program.bin > ../ram.hex
# dump assembly : 			riscv32-unknown-elf-objdump -dr program > program.asm_dump

.globl	_start

.text

_start:
	
	addi t0, x0, 1

	# nop
	# nop
	# nop

	addi t0, x0, 2

	# nop
	# nop
	# nop

	addi t0, x0, 3

	# nop
	# nop
	# nop

	addi t0, x0, 4

	# nop
	# nop
	# nop

	addi t0, x0, 5

	# nop
	# nop
	# nop

	or t0, t0, 3
 
	# nop
	# nop
	# nop

	and t1, t1, 0

	# nop
	# nop
	# nop

	or t1, t1, 4

	# nop
	# nop
	# nop

	add t2, t1, t0

	# nop
	# nop
	# nop

	slli t0, t0, 3

	# nop
	# nop
	# nop

	add t3, t0, t1

	# nop
	# nop
	# nop

	ebreak

	nop
	# nop
	# nop

	add t3, t0, t1

	nop
	# nop
	# nop

	ebreak

.end
