
	.def sum

.thumb

.const
NUM			.field 9

.text
sum:
			ADD R0, R1
			ADD R0, R2
			ADD R0, R3
			BX  lr

.endm
