; programa.asm — suma A + B -> RES
        .org 0x00
        LOAD  A
        ADD   B
        STORE RES
        HALT

        .org 0x20
A:      .byte 11
B:      .byte 33
RES:    .byte 0
