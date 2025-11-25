
../Export_week2/assembler_v2.x  factorialIN.asm factorial

../Export_week2/assembler_v2.x  sumaIN.asm suma

gcc -std=c11 -Wall -Wextra -O2 -c cpu_core.c -o cpu_core.o
gcc -std=c11 -Wall -Wextra -O2 -Wno-unused-result main2link_loadmem.c cpu_core.o -o main2link_loadmem.x
./main2link_loadmem.x

