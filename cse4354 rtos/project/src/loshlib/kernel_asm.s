	.def request

.thumb
.text

request:
	SVC #0x8 			; hardcoded the SVIC "Request" command index number
	BX 	LR
