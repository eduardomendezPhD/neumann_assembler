// main2link.c
// Driver for 8-bit CPU simulator.
//
// - Loads factorial.mem and suma.mem (already assembled from factorial.asm / suma.asm)
// - Passes parameters from C to ASM by writing into memory[] at fixed addresses
// - Reads back the result from memory[] after each execution.
//
// Contract with ASM:
//
// factorial.asm:
//   .org 0xC0
//   N      -> address 0xC0  (input parameter)
//   RESULT -> address 0xC1  (output N!)
//
// suma.asm:
//   .org 0x20
//   A      -> address 0x20  (input FACT1)
//   B      -> address 0x21  (input FACT2)
//   RES    -> address 0x22  (output A+B)

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#define MEM_SIZE 256

// Addresses must match the ASM layout
#define N_ADDR       0xC0   // N in factorial.asm
#define RESULT_ADDR  0xC1   // RESULT in factorial.asm

#define A_ADDR       0x20   // A in suma.asm
#define B_ADDR       0x21   // B in suma.asm
#define RES_ADDR     0x22   // RES in suma.asm

// ---------------------------------------------------------------------
// Externals provided by the CPU core (cpu_core.c / cpu_simulator*.c)
// ---------------------------------------------------------------------
extern uint8_t  memory[];           // 256-byte memory of the simulated CPU
extern uint8_t  ACC;                // accumulator (optional, for debug)
extern uint16_t PC;                 // program counter (optional, for debug)
extern uint8_t  IR;                 // instruction register (optional, for debug)

extern void cpu_reset(void);        // reset ACC, PC, IR
extern void fetch_decode_execute(void);  // run until HALT or error

// ---------------------------------------------------------------------
// Load a .mem file (one hex byte per line) into memory[]
// ---------------------------------------------------------------------
static void load_module(const char *fname) {
    FILE *f = fopen(fname, "r");
    if (!f) {
        perror(fname);
        exit(1);
    }

    // Clear full memory before loading new module
    memset(memory, 0, MEM_SIZE);

    unsigned int val;
    int addr = 0;

    while (fscanf(f, "%x", &val) == 1) {
        if (addr >= MEM_SIZE) break;
        memory[addr++] = (uint8_t)val;
    }

    fclose(f);
}

// ---------------------------------------------------------------------
// Run factorial.mem for a given N value.
//
// 1) Load factorial.mem into memory[]
// 2) cpu_reset() to set PC=0, ACC=0, IR=0
// 3) Write N into memory[N_ADDR]
// 4) fetch_decode_execute() to run ASM
// 5) Return memory[RESULT_ADDR] as the factorial
// ---------------------------------------------------------------------
static uint8_t run_factorial(uint8_t n_value) {
    load_module("factorial.mem");
    cpu_reset();

    // Pass the parameter from C to ASM:
    // N is at fixed address N_ADDR in RAM
    memory[N_ADDR] = n_value;

    fetch_decode_execute();

    // Read back the result from fixed address RESULT_ADDR
    return memory[RESULT_ADDR];
}

// ---------------------------------------------------------------------
// main()
// ---------------------------------------------------------------------
int main(void) {
    int N1, N2;

    printf("Introduce N1: ");
    if (scanf("%d", &N1) != 1) {
        printf("Entrada inválida para N1.\n");
        return 1;
    }

    printf("Introduce N2: ");
    if (scanf("%d", &N2) != 1) {
        printf("Entrada inválida para N2.\n");
        return 1;
    }

    // ------------------------------
    // 1) factorial(N1)
    // ------------------------------
    uint8_t FACT1 = run_factorial((uint8_t)N1);

    // ------------------------------
    // 2) factorial(N2)
    //    Uses the SAME N_ADDR/RESULT_ADDR,
    //    but with a different N value.
    // ------------------------------
    uint8_t FACT2 = run_factorial((uint8_t)N2);

    // ------------------------------
    // 3) suma(FACT1, FACT2)
    //
    // suma.asm uses:
    //   A at A_ADDR
    //   B at B_ADDR
    //   RES at RES_ADDR
    // ------------------------------
    load_module("suma.mem");
    cpu_reset();

    // Pass both parameters into fixed RAM slots
    memory[A_ADDR] = FACT1;
    memory[B_ADDR] = FACT2;

    fetch_decode_execute();

    uint8_t SUM = memory[RES_ADDR];

    // ------------------------------
    // Print final results
    // ------------------------------
    printf("\nRESULTADOS:\n");
    printf("factorial(%d) = %u\n", N1, FACT1);
    printf("factorial(%d) = %u\n", N2, FACT2);
    printf("suma = %u\n", SUM);

    return 0;
}
