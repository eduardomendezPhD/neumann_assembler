#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define MEM_SIZE 256

/*
TEAM 6: This program implements a minimal 8-bit CPU simulator with a 
fetch–decode–execute cycle, a small instruction set, and basic control flow 
via conditional and unconditional jumps. 
It models an accumulator machine with a flat 256-byte memory,
 an 8-bit program counter (PC), 
and a zero flag (ZF). The simulator includes bounds checking and simple 
error messages for invalid accesses and opcodes.

*/
// OpCodes
enum {
    NOP   = 0x00,
    LOAD  = 0x01, // ACC = MEM[addr]
    ADD   = 0x02, // ACC = ACC + MEM[addr]
    STORE = 0x03, // MEM[addr] = ACC
    LOADI = 0x04, // ACC = imm8
    SUB   = 0x05, // ACC = ACC - MEM[addr]
    JMP   = 0x06, // PC  = addr
    JZ    = 0x07, // if ZF==1 -> PC = addr
    JNZ   = 0x08, // if ZF==0 -> PC = addr
    PRINT = 0x09, // printf("%u\n", ACC)
    HALT  = 0xFF
};

// Memoria y registros
static uint8_t memory[MEM_SIZE];
static uint8_t ACC = 0;     // Acumulador
static uint8_t PC  = 0;     // Contador de programa
static uint8_t ZF  = 0;     // Zero flag (1 si ACC==0)

static int fetch8(uint8_t *out) {
    if (PC >= MEM_SIZE) { puts("PC fuera de rango"); return -1; }
    *out = memory[PC++];
    return 0;
}

static int read_addr(uint8_t *addr_out) {
    if (PC >= MEM_SIZE) { puts("PC fuera de rango (addr)"); return -1; }
    *addr_out = memory[PC++];
    if (*addr_out >= MEM_SIZE) { puts("Direccion invalida"); return -1; }
    return 0;
}

static void set_flags(void) {
    ZF = (ACC == 0);
}

static int run(void) {
    for (;;) {
        uint8_t op;
        if (fetch8(&op) != 0) return -1;

        switch (op) {
            case NOP:
                break;

            case LOAD: {
                uint8_t addr;
                if (read_addr(&addr) != 0) return -1;
                ACC = memory[addr];
                set_flags();
                break;
            }

            case LOADI: {
                uint8_t imm;
                if (fetch8(&imm) != 0) return -1;
                ACC = imm;
                set_flags();
                break;
            }

            case ADD: {
                uint8_t addr;
                if (read_addr(&addr) != 0) return -1;
                ACC = (uint8_t)(ACC + memory[addr]);
                set_flags();
                break;
            }

            case SUB: {
                uint8_t addr;
                if (read_addr(&addr) != 0) return -1;
                ACC = (uint8_t)(ACC - memory[addr]);
                set_flags();
                break;
            }

            case STORE: {
                uint8_t addr;
                if (read_addr(&addr) != 0) return -1;
                memory[addr] = ACC;
                // flags no cambian por STORE
                break;
            }

            case JMP: {
                uint8_t addr;
                if (read_addr(&addr) != 0) return -1;
                PC = addr;
                break;
            }

            case JZ: {
                uint8_t addr;
                if (read_addr(&addr) != 0) return -1;
                if (ZF) PC = addr;
                break;
            }

            case JNZ: {
                uint8_t addr;
                if (read_addr(&addr) != 0) return -1;
                if (!ZF) PC = addr;
                break;
            }

            case PRINT:
                printf("%u\n", (unsigned)ACC);
                break;

            case HALT:
                return 0;

            default:
                printf("Instrucción desconocida: %02X\n", (unsigned)op);
                return -1;
        }
    }
}

int main(void) {
    memset(memory, 0, sizeof memory);
    ACC = 0; PC = 0; ZF = 0;

    // Demo: suma A+B, imprime, si resultado==12 imprime "OK", else "FAIL".
    // Datos:
    memory[0x10] = 5;    // A
    memory[0x11] = 7;    // B

    // Programa:
    // 0x00: LOAD 0x10      ; ACC = A
    // 0x02: ADD  0x11      ; ACC += B   -> 12
    // 0x04: STORE 0x12     ; MEM[0x12] = ACC (guarda resultado)
    // 0x06: PRINT          ; imprime ACC (=12)
    // 0x07: LOADI 12       ; ACC = 12
    // 0x09: SUB  0x12      ; ACC = 12 - MEM[0x12] -> 0
    // 0x0B: JZ   0x0F      ; si ZF==1 salta a OK
    // 0x0D: LOADI 0        ; ACC = 0  (FAIL code)
    // 0x0F: PRINT          ; imprime 0 (FAIL) o 0 (OK) – ajusta si quieres distinto
    // 0x10: HALT

    memory[0x00] = LOAD;  memory[0x01] = 0x10;
    memory[0x02] = ADD;   memory[0x03] = 0x11;
    memory[0x04] = STORE; memory[0x05] = 0x12;
    memory[0x06] = PRINT;
    memory[0x07] = LOADI; memory[0x08] = 12;
    memory[0x09] = SUB;   memory[0x0A] = 0x12;
    memory[0x0B] = JZ;    memory[0x0C] = 0x0F;
    memory[0x0D] = LOADI; memory[0x0E] = 0;     // FAIL branch (aquí podrías LOADI 1/2 y PRINT)
    memory[0x0F] = PRINT;
    memory[0x10] = HALT;

    if (run() == 0) {
        printf("Resultado guardado en [0x12]=%u\n", (unsigned)memory[0x12]);
    }
    return 0;
}