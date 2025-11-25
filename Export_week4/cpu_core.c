// cpu_core.c
// Núcleo de CPU de 8 bits para usar como biblioteca desde C
// No tiene main(), sólo expone memoria, registros y fetch_decode_execute().

#include <stdint.h>
#include <stdio.h>

#define MEM_SIZE 256

// ---------------------------------------------------------------------
// Memoria y registros (visibles desde otros .c)
// ---------------------------------------------------------------------
uint8_t memory[MEM_SIZE];   // Espacio de memoria de 256 bytes
uint8_t ACC = 0;            // Acumulador
uint8_t PC  = 0;            // Contador de programa
uint8_t IR  = 0;            // Registro de instrucción (opcional para depuración)

// ---------------------------------------------------------------------
// ISA de 8 bits (debe coincidir con la que usa tu ensamblador)
// ---------------------------------------------------------------------
enum {
    NOP   = 0x00,  // No operación
    LOAD  = 0x01,  // ACC <- [addr]
    ADD   = 0x02,  // ACC <- ACC + [addr]   (aritmética de 8 bits, módulo 256)
    STORE = 0x03,  // [addr] <- ACC
    JMP   = 0x04,  // PC <- addr
    JZ    = 0x05,  // if ACC == 0 then PC <- addr
    PRINT = 0x06,  // opcional: imprime ACC/PC (debug)
    HALT  = 0xFF   // detiene ejecución
};

// ---------------------------------------------------------------------
// Reset opcional de la CPU (puedes llamarlo si quieres desde C)
// ---------------------------------------------------------------------
void cpu_reset(void) {
    ACC = 0;
    PC  = 0;
    IR  = 0;
    // NO tocamos memory[] aquí, porque main2link ya la limpia con memset()
}

// ---------------------------------------------------------------------
// Helpers internos de fetch
// ---------------------------------------------------------------------
static uint8_t fetch_u8(void) {
    // PC es de 8 bits, así que la dirección es siempre 0..255 (wrap natural)
    uint8_t value = memory[PC];
    PC = (uint8_t)(PC + 1);   // incremento con wrap de 8 bits
    return value;
}

static uint8_t fetch_addr(void) {
    return fetch_u8();        // en esta ISA, las direcciones son un byte
}

// ---------------------------------------------------------------------
// Bucle principal de ejecución
// ---------------------------------------------------------------------
void fetch_decode_execute(void) {
    for (;;) {
        IR = fetch_u8();  // fetch de opcode

        switch (IR) {

            case NOP:
                // No hace nada
                break;

            case LOAD: {
                uint8_t addr = fetch_addr();
                ACC = memory[addr];
            } break;

            case ADD: {
                uint8_t addr = fetch_addr();
                ACC = (uint8_t)(ACC + memory[addr]);  // overflow natural de 8 bits
            } break;

            case STORE: {
                uint8_t addr = fetch_addr();
                memory[addr] = ACC;
            } break;

            case JMP: {
                uint8_t addr = fetch_addr();
                PC = addr;
            } break;

            case JZ: {
                uint8_t addr = fetch_addr();
                if (ACC == 0) {
                    PC = addr;
                }
            } break;

            case PRINT:
                // Instrucción opcional de depuración
                printf("[CPU] ACC=%3u (0x%02X), PC=0x%02X\n",
                       ACC, ACC, PC);
                break;

            case HALT:
                // Termina la ejecución del programa cargado en memory[]
                return;

            default:
                // Opcode desconocido: reporta y detiene
                printf("Unknown opcode 0x%02X at PC=0x%02X\n",
                       IR, (uint8_t)(PC - 1));
                return;
        }
    }
}
