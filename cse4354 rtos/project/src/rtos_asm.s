	.def setPSP
	.def getPSP
	.def setASP
	.def getMSP
	.def setTMPL

.thumb
.text

setPSP:
			MSR PSP, R0
			ISB
			BX 	LR

getPSP:
			MRS R0, PSP
			BX 	LR

getMSP:
			MRS R0, MSP
			BX 	LR

setASP: ; RMW, set bit 1 (ASP). /88
			MRS R0, CONTROL
			ORR	R0, R0, #2 ; also sets TMPL
			MSR CONTROL, R0
			ISB
			BX LR

setTMPL:
			MRS R0, CONTROL
			ORR	R0, R0, #1
			MSR CONTROL, R0
			ISB
			BX LR
