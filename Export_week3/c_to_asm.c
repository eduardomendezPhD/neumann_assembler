//El siguiente programa crea una capa en C que genere código ASM para el ensamblador, simulando un compilador. 
#include <stdio.h>

void generarASM(int programa, int a, int b) {
    
    switch (programa){
        case 1: {//suma
            FILE *f = fopen("suma.asm", "w");      
            fprintf(f, "; suma.asm — suma A + B -> RES\n");
            fprintf(f, "    .org 0x00\n");
            fprintf(f, "    LOAD  A\n");
            fprintf(f, "    ADD   B\n");
            fprintf(f, "    STORE RES\n");
            fprintf(f, "    HALT\n\n");
            fprintf(f, "    .org 0X20\n");
            fprintf(f, "A:      .byte %d\n",a);
            fprintf(f, "B:      .byte %d\n",b);
            fprintf(f, "RES:      .byte 0");
            fclose(f); 
        }
        case 2:{ //factorial
            FILE *f = fopen("factorialC.asm", "w");
            fprintf(f, "; factorial_ACC.asm : factorial general N! usando bucles");
            fprintf(f, "; ISA: LOAD=0x01, ADD=0x02, STORE=0x03, JMP=0x04, JZ=0x05, HALT=0xFF");
            fprintf(f, "    .org 0x00\n");
            fprintf(f, "    ; RESULT = 1\n");
            fprintf(f, "    LOAD  ONE\n");
            fprintf(f, "    STORE RESULT\n\n"); 
            fprintf(f, "    ; COUNTER = N\n"); 
            fprintf(f, "    LOAD  N\n"); 
            fprintf(f, "    STORE COUNTER\n\n"); 
            fprintf(f, "LOOP:\n");
            fprintf(f, "    ; if COUNTER == 0 goto END\n");
            fprintf(f, "    LOAD  COUNTER\n");
            fprintf(f, "    JZ    END\n\n"); 
            fprintf(f, "    ; PART = 0\n");
            fprintf(f, "    LOAD  ZERO\n");
            fprintf(f, "    STORE PART\n\n"); 
            fprintf(f, "    ; TEMP = COUNTER\n"); 
            fprintf(f, "    LOAD  COUNTER\n"); 
            fprintf(f, "    STORE TEMP\n\n"); 
            fprintf(f, "INNER:\n");
            fprintf(f, "    ; PART = PART + RESULT\n");
            fprintf(f, "    LOAD  PART\n"); 
            fprintf(f, "    ADD   RESULT\n"); 
            fprintf(f, "    STORE PART\n\n"); 
            fprintf(f, "    ; TEMP = TEMP - 1   (NEG1 = 0xFF)\n"); 
            fprintf(f, "    LOAD  TEMP\n"); 
            fprintf(f, "    ADD   NEG1\n"); 
            fprintf(f, "    STORE TEMP\n\n"); 
            fprintf(f, "    ; if TEMP == 0 goto INNER_END\n"); 
            fprintf(f, "    LOAD  TEMP\n"); 
            fprintf(f, "    JZ    INNER_END\n\n"); 
            fprintf(f, "    ; else goto INNER\n"); 
            fprintf(f, "    JMP   INNER\n\n"); 
            fprintf(f, "INNER_END:\n"); 
            fprintf(f, "    ; RESULT = PART\n");
            fprintf(f, "    LOAD  PART\n"); 
            fprintf(f, "    STORE RESULT\n\n"); 
            fprintf(f, "    ; COUNTER = COUNTER - 1\n"); 
            fprintf(f, "    LOAD  COUNTER\n"); 
            fprintf(f, "    ADD   NEG1\n"); 
            fprintf(f, "    STORE COUNTER\n\n"); 
            fprintf(f, "    ; repeat outer loop\n");
            fprintf(f, "    JMP   LOOP\n\n"); 
            fprintf(f, "; ----- Aquí nos aseguramos de que el factorial quede en ACC -----\n");
            fprintf(f, "END:\n");
            fprintf(f, "    LOAD  RESULT   ; ACC = RESULT (N!)\n");
            fprintf(f, "    HALT\n\n");
            fprintf(f, "    ; ---------------- DATA ----------------\n");
            fprintf(f, "    .org 0xC0\n"); 
            fprintf(f, "N:      .byte %d      ; valor de N, cámbialo si quieres otro factorial\n",a);
            fprintf(f, "RESULT: .byte 0\n");
            fprintf(f, "COUNTER:.byte 0\n");
            fprintf(f, "TEMP:   .byte 0\n"); 
            fprintf(f, "PART:   .byte 0\n"); 
            fprintf(f, "ONE:    .byte 1\n"); 
            fprintf(f, "ZERO:   .byte 0\n"); 
            fprintf(f, "NEG1:   .byte 255    ; 0xFF = -1 en aritmética de 8 bits");
        }
    }
}

int main() {
    generarASM(1 ,11, 33); // genera código que sumará memoria[11] + memoria[33]
    generarASM(2 ,4, 0); // genera código que calculará el factorial de 4
}
