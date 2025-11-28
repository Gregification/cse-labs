	.def yield       		; #1
    .def sleep       		; #2
    .def lock        		; #3
    .def unlock      		; #4
    .def wait        		; #5
    .def post        		; #6
    .def reboot      		; #7
    .def request     		; #8
	.def restartThread		; #9
	.def setThreadPriority	; #10

.thumb
.text

yield:
    SVC #0x1        ; SVC #1
    BX  LR

sleep:
    SVC #0x2        ; SVC #2
    BX  LR

lock:
    SVC #0x3        ; SVC #3
    BX  LR

unlock:
    SVC #0x4        ; SVC #4
    BX  LR

wait:
    SVC #0x5        ; SVC #5
    BX  LR

post:
    SVC #0x6        ; SVC #6
    BX  LR

reboot:
    SVC #0x7        ; SVC #7
    BX  LR

request:
    SVC #0x8        ; SVC #8
    BX  LR

restartThread:
	SVC #0x9
    BX	LR

setThreadPriority:
	SVC #10
	BX	LR
