    .def setPSP
    .def getPSP

.thumb
.text
setPSP:
			MSR PSR, R0
			BX LR

getPSP:
			MRS R0, PSR
			BX LR
