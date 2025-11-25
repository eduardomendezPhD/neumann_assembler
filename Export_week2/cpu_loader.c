// cpu_loader.c -- loads a text .mem (one hex byte per line) into memory and runs fetch-decode-execute
// Usage: ./cpu_loader programa.mem

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

static uint8_t memory[MEM_SIZE];
static uint8_t ACC = 0;
static uint8_t PC  = 0;

static void fetch_decode_execute(void){
    for(;;){
        uint8_t instr = memory[PC++];
        switch(instr){
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
            case HALT:
                return;
            default:
                printf("Unknown instruction %02X at PC=%u\n", instr, (unsigned)(PC-1));
                return;
        }
    }
}

int load_mem_from_file(const char *fname){
    FILE *f = fopen(fname, "r");
    if(!f){ perror("fopen"); return 0; }
    char line[64];
    int addr = 0;
    while(fgets(line, sizeof(line), f) && addr < MEM_SIZE){
        // skip blank lines and comments
        char *p = line;
        while(*p && isspace((unsigned char)*p)) p++;
        if(*p == ';' || *p == 0) continue;
        // parse hex
        unsigned int val;
        if(sscanf(p, "%x", &val) == 1){
            memory[addr++] = (uint8_t)(val & 0xFF);
        }
    }
    fclose(f);
    // zero rest
    for(int i=addr;i<MEM_SIZE;i++) memory[i]=0;
    return 1;
}

int main(int argc, char **argv){
    if(argc < 2){ printf("Usage: %s program.mem\n", argv[0]); return 1;}
    if(!load_mem_from_file(argv[1])) return 1;
    ACC = 0; PC = 0;
    fetch_decode_execute();
    // after execution, show RES area used by our sample (0x20..0x22)
    printf("Memory snapshot (0x20..0x22): %02X %02X %02X\n", memory[0x20], memory[0x21], memory[0x22]);
    printf("ACC=%02X PC=%02X\n", ACC, PC);
    return 0;
}

