// cpu_loader_v2.c -- loads a text .mem (one hex byte per line) into memory and runs fetch-decode-execute
// Usage: ./cpu_loader_v2 program.mem

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MEM_SIZE 256
#define HALT 0xFF

#define LOAD  0x01
#define ADD   0x02
#define STORE 0x03
#define JMP   0x04
#define JZ    0x05

// Memory and registers
static uint8_t memory[MEM_SIZE];
static uint8_t ACC = 0;   // Accumulator
static uint8_t PC  = 0;   // Program Counter

// Trim leading whitespace
static char *ltrim(char *s) {
    while (*s && isspace((unsigned char)*s)) s++;
    return s;
}

// Load .mem file: one hex byte per line (e.g., "1F")
static int load_mem_from_file(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) {
        perror("fopen");
        return 0;
    }

    char line[256];
    int addr = 0;

    while (fgets(line, sizeof(line), f)) {
        char *p = ltrim(line);
        if (*p == '\0' || *p == '\n' || *p == '#')
            continue; // skip empty or comment-like lines

        // Strip trailing newline
        char *nl = strchr(p, '\n');
        if (nl) *nl = '\0';

        // Expect a 2-digit hex number
        char *endptr = NULL;
        long val = strtol(p, &endptr, 16);
        if (endptr == p) {
            fprintf(stderr, "Invalid hex byte in %s: '%s'\n", path, p);
            fclose(f);
            return 0;
        }

        if (addr >= MEM_SIZE) {
            fprintf(stderr, "Program too large for memory (>%d bytes)\n", MEM_SIZE);
            fclose(f);
            return 0;
        }

        memory[addr++] = (uint8_t)(val & 0xFF);
    }

    fclose(f);

    // Zero the rest of memory
    for (int i = addr; i < MEM_SIZE; i++) {
        memory[i] = 0;
    }

    return 1;
}

// Fetch-decode-execute loop
static void fetch_decode_execute(void) {
    for (;;) {
        uint8_t instr = memory[PC++];  // fetch and increment PC

        switch (instr) {
            case LOAD: {
                uint8_t addr = memory[PC++];
                ACC = memory[addr];
                break;
            }

            case ADD: {
                uint8_t addr = memory[PC++];
                ACC = (uint8_t)(ACC + memory[addr]);
                break;
            }

            case STORE: {
                uint8_t addr = memory[PC++];
                memory[addr] = ACC;
                break;
            }

            case JMP: {
                uint8_t addr = memory[PC++];
                PC = addr;            // unconditional jump
                break;
            }

            case JZ: {
                uint8_t addr = memory[PC++];
                if (ACC == 0) {       // jump if ACC == 0
                    PC = addr;
                }
                break;
            }

            case HALT:
                return;               // stop execution

            default:
                printf("Unknown instruction %02X at PC=%02X\n",
                       instr, (unsigned)(PC - 1));
                return;
        }
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s program.mem\n", argv[0]);
        return 1;
    }

    if (!load_mem_from_file(argv[1])) {
        return 1;
    }

    ACC = 0;
    PC  = 0;

    fetch_decode_execute();

    // After execution, show memory region 0x20..0x22 (legacy from suma example)
    printf("Memory snapshot (0x20..0x22): %02X %02X %02X\n",
           memory[0x20], memory[0x21], memory[0x22]);
    printf("ACC=%02X PC=%02X\n", ACC, PC);

    return 0;
}
