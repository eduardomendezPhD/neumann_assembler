// TEAM 6: cpu_simulator3.c
/*
This C program implements a minimalist 8-bit CPU simulator with a small 
instruction set architecture (ISA) and an in-memory program that computes 
the factorial of a number (N!) using repeated addition and loops. 
It illustrates core CPU concepts — memory, registers, control flow, 
and instruction execution — within a compact 256-byte environment.
*/

// TEAM 6: cpu_simulator3_trace_fixed.c
// Minimal 8-bit CPU simulator computing N! with nested loops.
// Robust to stray zeros (defines NOP=0x00), bounds checks, and dumps program.

// TEAM 6: cpu_simulator3_trace_segregated.c
// Minimal 8-bit CPU simulator computing N! with nested loops.
// FIX: data lives at 0xC0+ so code (0x00..) doesn't overwrite it.

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define MEM_SIZE 256

// ISA
enum {
    NOP   = 0x00, // safe no-op if zeros appear
    LOAD  = 0x01,
    ADD   = 0x02,
    STORE = 0x03,
    JMP   = 0x04,
    JZ    = 0x05,
    PRINT = 0x06, // debug: print RESULT and COUNTER
    HALT  = 0xFF
};

// Memory & registers
static uint8_t memory[MEM_SIZE];
static uint8_t ACC = 0;   // Accumulator
static uint8_t PC  = 0;   // Program Counter
static uint8_t IR  = 0;   // Instruction Register

// Data addresses (moved away from code region)
enum {
    N       = 0xC0, // input N
    RESULT  = 0xC1, // running result
    COUNTER = 0xC2, // outer counter
    TEMP    = 0xC3, // temp for multiply
    PART    = 0xC4, // partial sum
    NEG1    = 0xF0, // −1 (0xFF)
    ONE     = 0xF1, // 1
    ZERO    = 0xF2  // 0
};

static int fetch_u8(uint8_t *out) {
    if (PC >= MEM_SIZE) { printf("ERROR: PC OOB at 0x%02X\n", PC); return -1; }
    *out = memory[PC++];
    return 0;
}

static int fetch_addr(uint8_t *addr) {
    if (PC >= MEM_SIZE) { printf("ERROR: PC OOB (op) at 0x%02X\n", PC); return -1; }
    *addr = memory[PC++];
    return 0;
}

static void dump_program(uint8_t start, uint8_t end) {
    printf("Program dump [0x%02X..0x%02X]:\n", start, end);
    for (uint16_t i = start; i <= end; ++i) {
        if ((i - start) % 16 == 0) printf("0x%02X: ", (uint8_t)i);
        printf("%02X ", memory[i]);
        if ((i - start) % 16 == 15 || i == end) printf("\n");
    }
}

static void fetch_decode_execute(void) {
    for (;;) {
        if (fetch_u8(&IR) != 0) return;

        switch (IR) {
            case NOP: break;

            case LOAD: {
                uint8_t a; if (fetch_addr(&a) != 0) return;
                ACC = memory[a];
            } break;

            case ADD: {
                uint8_t a; if (fetch_addr(&a) != 0) return;
                ACC = (uint8_t)(ACC + memory[a]); // 8-bit wrap
            } break;

            case STORE: {
                uint8_t a; if (fetch_addr(&a) != 0) return;
                memory[a] = ACC;
            } break;

            case JMP: {
                uint8_t a; if (fetch_addr(&a) != 0) return;
                PC = a;
            } break;

            case JZ: {
                uint8_t a; if (fetch_addr(&a) != 0) return;
                if (ACC == 0) PC = a;
            } break;

            case PRINT:
		{
                uint8_t iter = (uint8_t)(memory[N] - memory[COUNTER] + 1);
                printf("Iteration %3u -> RESULT=%3u  COUNTER=%3u\n",
                    iter, memory[RESULT], memory[COUNTER]);
		}
                break;

            case HALT:
                return;

            default:
                printf("Unknown opcode 0x%02X at PC=0x%02X\n", IR, (uint8_t)(PC - 1));
                return;
        }
    }
}

static void load_factorial_program(uint8_t n_value) {
    memset(memory, 0, sizeof memory);

    // constants
    memory[NEG1] = 0xFF;
    memory[ONE]  = 0x01;
    memory[ZERO] = 0x00;

    // input N
    memory[N] = n_value;

    uint8_t p = 0x00;

    // RESULT = 1
    memory[p++] = LOAD;  memory[p++] = ONE;
    memory[p++] = STORE; memory[p++] = RESULT;

    // COUNTER = N
    memory[p++] = LOAD;  memory[p++] = N;
    memory[p++] = STORE; memory[p++] = COUNTER;

    // LOOP:
    uint8_t LOOP = p;

    // if COUNTER == 0 goto END
    memory[p++] = LOAD;  memory[p++] = COUNTER;
    memory[p++] = JZ;    uint8_t END_patch = p++; // placeholder

    // PART = 0
    memory[p++] = LOAD;  memory[p++] = ZERO;
    memory[p++] = STORE; memory[p++] = PART;

    // TEMP = RESULT
    memory[p++] = LOAD;  memory[p++] = RESULT;
    memory[p++] = STORE; memory[p++] = TEMP;

    // INNER:
    uint8_t INNER = p;

    // if TEMP == 0 goto INNER_END
    memory[p++] = LOAD;  memory[p++] = TEMP;
    memory[p++] = JZ;    uint8_t INNER_END_patch = p++; // placeholder

    // PART = PART + COUNTER
    memory[p++] = LOAD;  memory[p++] = PART;
    memory[p++] = ADD;   memory[p++] = COUNTER;
    memory[p++] = STORE; memory[p++] = PART;

    // TEMP = TEMP - 1   (ADD −1)
    memory[p++] = LOAD;  memory[p++] = TEMP;
    memory[p++] = ADD;   memory[p++] = NEG1;
    memory[p++] = STORE; memory[p++] = TEMP;

    // goto INNER
    memory[p++] = JMP;   memory[p++] = INNER;

    // INNER_END:
    uint8_t INNER_END = p;

    // RESULT = PART
    memory[p++] = LOAD;  memory[p++] = PART;
    memory[p++] = STORE; memory[p++] = RESULT;

    // print intermediate
    memory[p++] = PRINT;

    // COUNTER = COUNTER - 1
    memory[p++] = LOAD;  memory[p++] = COUNTER;
    memory[p++] = ADD;   memory[p++] = NEG1;
    memory[p++] = STORE; memory[p++] = COUNTER;

    // goto LOOP
    memory[p++] = JMP;   memory[p++] = LOOP;

    // END:
    uint8_t END = p;
    memory[p++] = HALT;

    // patch jumps
    memory[END_patch]        = END;
    memory[INNER_END_patch]  = INNER_END;

    // (optional) inspect code layout
    dump_program(0x00, (uint8_t)(p - 1));
}

int main(void) {
    load_factorial_program(6);      // change to 5 if you prefer
    ACC = 0; PC = 0; IR = 0;        // clean start
    fetch_decode_execute();

    printf("\nFinal result:\n");
    printf("N = %u\n", memory[N]);
    printf("factorial(N) mod 256 = %u\n", memory[RESULT]);
    return 0;
}
