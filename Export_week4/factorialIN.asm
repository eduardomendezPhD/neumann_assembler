; factorial.asm — cálculo general de N! usando bucles
; ISA: LOAD=0x01, ADD=0x02, STORE=0x03, JMP=0x04, JZ=0x05, HALT=0xFF
;
; Contrato con C (main2link.c):
;   - C debe escribir N en la dirección 0xC0
;   - Al terminar, RESULT (0xC1) contiene N! (mod 256)
;
; Esquema:
;   RESULT = 1
;   COUNTER = N
;   while COUNTER != 0:
;       PART = 0
;       TEMP = COUNTER
;       while TEMP != 0:
;           PART += RESULT
;           TEMP -= 1
;       RESULT = PART
;       COUNTER -= 1

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

        ; TEMP = TEMP - 1 (NEG1 = 0xFF)
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

; Al terminar: factorial queda en RESULT (y lo copiamos a ACC)
END:
        LOAD  RESULT   ; ACC = RESULT (N!)
        HALT

; ---------------- DATA ----------------
        .org 0xC0
N:      .byte 0      ; <- C pone aquí el valor de N (dinámico)
RESULT: .byte 0
COUNTER:.byte 0
TEMP:   .byte 0
PART:   .byte 0
ONE:    .byte 1
ZERO:   .byte 0
NEG1:   .byte 255    ; 0xFF = -1 en aritmética de 8 bits

