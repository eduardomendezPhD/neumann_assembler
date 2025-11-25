//##################################################################
//main2link.c
//Main driver
//##################################################################
// main2link.c
// Llama código ensamblador de tu CPU de 8 bits cargando .mem,
// ejecutándolo en la CPU simulada, y leyendo los resultados.
//
// Flujo:
//  factorial.asm  → factorial.mem
//  suma.asm       → suma.mem
//  main2link      → ejecuta factorial(N1), factorial(N2), luego suma(FACT1, FACT2).


#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>


#define MEM_SIZE 256


extern void fetch_decode_execute(void);   // declarada en tu Cpu_simulator1b.c
extern uint8_t memory[];                  // memoria del simulador
extern uint8_t ACC;                       // opcional (si quieres imprimir ACC)
extern uint8_t PC;                        // idem


// -----------------------------------------------------------
// Carga un archivo .mem (texto hex, 1 byte por línea) en memory[]
// -----------------------------------------------------------
void load_module(const char *fname) {
    FILE *f = fopen(fname, "r");
    if (!f) {
        perror(fname);
        exit(1);
    }


    memset(memory, 0, MEM_SIZE);
    unsigned int val;
    int addr = 0;


    while (fscanf(f, "%x", &val) == 1) {
        if (addr >= MEM_SIZE) break;
        memory[addr++] = (uint8_t)val;
    }
    fclose(f);
}


// -----------------------------------------------------------
// Ejecuta un módulo ensamblado cargado en memory[] y retorna
// el byte en una dirección dada (p. ej., RESULT).
// -----------------------------------------------------------
uint8_t run_and_get(const char *module, uint16_t param_addr, uint8_t param_value,
                    uint16_t result_addr)
{
    load_module(module);
    memory[param_addr] = param_value;  // setear parámetro (ej.: N)
    fetch_decode_execute();            // ejecutar el ensamblador
    return memory[result_addr];        // devolver RESULT
}


int main(void) {
    int N1, N2;


    printf("Introduce N1: ");
    scanf("%d", &N1);
    printf("Introduce N2: ");
    scanf("%d", &N2);


    // -------------------------------------------------------
    // 1) factorial(N1)
    // En factorial.asm, la variable N está en 0x20 y RESULT en 0x21
    // -------------------------------------------------------
    uint8_t FACT1 = run_and_get(
        "factorial.mem",
        0x20,         // dirección de N en factorial.asm
        (uint8_t)N1,
        0x21          // dirección de RESULT
    );


    // -------------------------------------------------------
    // 2) factorial(N2)
    // -------------------------------------------------------
    uint8_t FACT2 = run_and_get(
        "factorial.mem",
        0x20,
        (uint8_t)N2,
        0x21
    );


    // -------------------------------------------------------
    // 3) suma(FACT1, FACT2)
    // suma.asm usa:
    //    A = 0x20
    //    B = 0x21
    //    R = 0x22
    // -------------------------------------------------------
    load_module("suma.mem");
    memory[0x20] = FACT1;     // A
    memory[0x21] = FACT2;     // B
    fetch_decode_execute();
    uint8_t SUM = memory[0x22];  // R = A + B


    // -------------------------------------------------------
    // Mostrar resultados
    // -------------------------------------------------------
    printf("\nRESULTADOS:\n");
    printf("factorial(%d) = %u\n", N1, FACT1);
    printf("factorial(%d) = %u\n", N2, FACT2);
    printf("suma = %u\n", SUM);


    return 0;
}
