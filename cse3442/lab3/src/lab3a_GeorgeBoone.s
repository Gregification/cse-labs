; UTA cse3442 lab3
; George Boone
; 9/17/2024

; assuming 40Mhz clock. 123 pipeline rule.

    .def wait3Seconds


.thumb

.const
; wait 3 seconds itterator initial value
W3S_ITTR_N          .word   29_999_999

.text

; blocks for 3 seconds
; function wastes 120M clock cycles exact. not accounting arrival and departure
wait3Seconds:                                                               @ time evaluation
                LDR     R0, W3S_ITTR_N     ; itterating 'n' times           @ 1
                NOP                                                         @ 1
    itterate:   SUB     R0, R0, #1                                          @ n
                CBZ     R0, loop                                            @ 1 + n * 3
                BX      LR                                                  @ 1

    