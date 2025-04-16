# the hex output needs to be moved to the parent folder afterwards
# 1. assemble to elf : 		riscv32-unknown-elf-gcc -o program program.s -march=rv32i -mabi=ilp32 -nostdlib -Tcse4372_riscv.ld
# 2. extract bin :			riscv32-unknown-elf-objcopy program -O binary program.bin
# 3. convert to hex : 		hexdump -v -e '1/4 "%08x" "\n"' program.bin > ../ram.hex
# dump assembly : 			riscv32-unknown-elf-objdump -dr program > program.asm_dump

.globl	_start

.text

_start:
	addi t0, x0, 0 # index

	addi t1, x0, 3 # some value to be modified

	addi t2, x0, 4 # number of itterations

itterate:
	
	addi t0, t0, 1 # itterate

	slli t1, t1, 1 # do something

	bne t2, t0, itterate

	ebreak

	ebreak

	ebreak

.end
