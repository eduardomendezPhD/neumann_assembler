; suma.asm — suma A + B -> RES
; ISA: LOAD=0x01, ADD=0x02, STORE=0x03, JMP=0x04, JZ=0x05, HALT=0xFF
;
; Contrato con C:
;   - C debe escribir A en 0x20 y B en 0x21
;   - Al terminar, RES (0x22) contiene A + B (mod 256)

        .org 0x00
        LOAD  A       ; ACC = [A]
        ADD   B       ; ACC = ACC + [B]
        STORE RES     ; RES = A + B
        HALT

        .org 0x20
A:      .byte 0       ; <- C pone aquí FACT1
B:      .byte 0       ; <- C pone aquí FACT2
RES:    .byte 0       ; <- aquí queda la suma

