
#  ENSAMBLADOR 
#  Usamos la version v2 que tiene saltos
#
gcc assembler.c -o assembler.x

gcc assembler_v2.c -o assembler_v2.x

gcc cpu_loader.c -o cpu_loader.x

gcc cpu_loader_v2.c -o cpu_loader_v2.x


# CREAR EL MEM CON ASSEMBLER
./assembler_v2.x factorialB.asm factorialBout


# EJECUTAR CON CARGADOR 
./cpu_loader_v2.x factorialBout.mem

