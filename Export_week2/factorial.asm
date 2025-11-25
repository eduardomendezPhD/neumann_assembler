; factorial.asm : calcula 4! = 24 y lo guarda en RESULT

        .org 0x00

        ; RESULT = 1
        LOAD  ONE
        STORE RESULT

        ; RESULT = RESULT * 2   (doblar)
        LOAD  RESULT
        ADD   RESULT
        STORE RESULT

        ; RESULT = RESULT * 3
        ; ACC = 0
        LOAD  ZERO
        ; ACC = ACC + RESULT + RESULT + RESULT
        ADD   RESULT
        ADD   RESULT
        ADD   RESULT
        STORE RESULT

        ; RESULT = RESULT * 4
        ; ACC = 0
        LOAD  ZERO
        ; ACC = RESULT + RESULT + RESULT + RESULT
        ADD   RESULT
        ADD   RESULT
        ADD   RESULT
        ADD   RESULT
        STORE RESULT

        HALT

        ; --- Datos: los movemos más lejos para no chocar con el código ---
        .org 0x40
ONE:    .byte 1
ZERO:   .byte 0
RESULT: .byte 0
