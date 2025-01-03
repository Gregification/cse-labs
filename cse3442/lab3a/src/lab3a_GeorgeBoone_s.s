; UTA cse3442 lab3a
; George Boone
; 9/17/2024

; assuming 40Mhz clock. 123 pipeline rule.

    .def wait3Seconds


.thumb

.const
W3S_ITTR_N          .field   29999998

.text

; blocks for 3 seconds
; function wastes [120M - 1] clock cycles, next command called after this runs on the 120Mth clock since this funciton was called
wait3Seconds:                                                               ; time evaluation
                LDR     R0, W3S_ITTR_N     ; itterating 'n' times           ; 1
                NOP                                                         ; 1
                NOP                                                         ; 1
                NOP                                                         ; 1
                NOP                                                         ; 1
itterate:       SUB     R0, R0, #1                                          ; n
                CBZ     R0, break                                           ; n + 2
                B       itterate                                            ; 2n
break:          BX      LR                                                  ; 2

.endm
