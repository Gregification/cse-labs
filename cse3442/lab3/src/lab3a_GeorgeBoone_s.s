; UTA cse3442 lab3
; George Boone
; 9/17/2024

; assuming 40Mhz clock. 123 pipeline rule.

    .def wait3Seconds


.thumb

.const
W3S_ITTR_N          .field   19999999

.text

; blocks for 3 seconds
; function wastes 120M clock cycles. not accounting arrival and departure time
wait3Seconds:                                                               ; time evaluation
                LDR     R0, W3S_ITTR_N     ; itterating 'n' times           ; 1
                NOP                                                         ; 1
                NOP                                                         ; 1
    itterate:   SUB     R0, R0, #1                                          ; n
                CBZ     R0, break                                           ; 1 + n * 3
                B       itterate                                            ; 2n - 1
    break:      BX      LR                                                  ; 2

.endm