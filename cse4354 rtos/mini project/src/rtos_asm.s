	.def setPSP
	.def getPSP
	.def setASP

.thumb
.text

setPSP:
			MSR PSP, R0
			BX 	LR

getPSP:
			MRS R0, PSP
			BX 	LR

setASP:
			BX LR
