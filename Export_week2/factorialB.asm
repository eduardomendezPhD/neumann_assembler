; factorial_ACC.asm : factorial general N! usando bucles
; ISA: LOAD=0x01, ADD=0x02, STORE=0x03, JMP=0x04, JZ=0x05, HALT=0xFF

        .org 0x00

        ; RESULT = 1
        LOAD  ONE
        STORE RESULT

        ; COUNTER = N
        LOAD  N
        STORE COUNTER

LOOP:
        ; if COUNTER == 0 goto END
        LOAD  COUNTER
        JZ    END

        ; PART = 0
        LOAD  ZERO
        STORE PART

        ; TEMP = COUNTER
        LOAD  COUNTER
        STORE TEMP

INNER:
        ; PART = PART + RESULT
        LOAD  PART
        ADD   RESULT
        STORE PART

        ; TEMP = TEMP - 1   (NEG1 = 0xFF)
        LOAD  TEMP
        ADD   NEG1
        STORE TEMP

        ; if TEMP == 0 goto INNER_END
        LOAD  TEMP
        JZ    INNER_END

        ; else goto INNER
        JMP   INNER

INNER_END:
        ; RESULT = PART
        LOAD  PART
        STORE RESULT

        ; COUNTER = COUNTER - 1
        LOAD  COUNTER
        ADD   NEG1
        STORE COUNTER

        ; repeat outer loop
        JMP   LOOP

; ----- Aquí nos aseguramos de que el factorial quede en ACC -----
END:
        LOAD  RESULT   ; ACC = RESULT (N!)
        HALT

        ; ---------------- DATA ----------------
        .org 0xC0
N:      .byte 4      ; valor de N, cámbialo si quieres otro factorial
RESULT: .byte 0
COUNTER:.byte 0
TEMP:   .byte 0
PART:   .byte 0
ONE:    .byte 1
ZERO:   .byte 0
NEG1:   .byte 255    ; 0xFF = -1 en aritmética de 8 bits
