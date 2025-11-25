#include <stdio.h>
#include <stdint.h>

#define MEM_SIZE 256
#define HALT 0xFF
/*
TEAM 6: This is a simple CPU simulator that supports basic
instructions like LOAD, ADD, STORE, and HALT.
It demonstrates a fetch-decode-execute cycle.
*/
// Memoria y registros
uint8_t memory[MEM_SIZE];
uint8_t ACC = 0;  // Acumulador
uint8_t PC = 0;   // Contador de programa

void fetch_decode_execute() {
    while (1) {
        uint8_t instr = memory[PC++];
        switch (instr) {
            case 0x01: { // LOAD addr
                uint8_t addr = memory[PC++];
                ACC = memory[addr];
                break;
            }
            case 0x02: { // ADD addr
                uint8_t addr = memory[PC++];
                ACC += memory[addr];
                break;
            }
            case 0x03: { // STORE addr
                uint8_t addr = memory[PC++];
                memory[addr] = ACC;
                break;
            }
            case HALT:
                return;
            default:
                printf("Instrucción desconocida: %02X\n", instr);
                return;
        }
    }
}

int main() {
    // Programa: SUMAR memory[10] + memory[11] -> memory[12]
    memory[10] = 5;
    memory[11] = 7;
    memory[0] = 0x01; memory[1] = 10; // LOAD 10
    memory[2] = 0x02; memory[3] = 11; // ADD 11
    memory[4] = 0x03; memory[5] = 12; // STORE 12
    memory[6] = HALT;

    fetch_decode_execute();
    printf("Resultado: %d\n", memory[12]);
    return 0;
}