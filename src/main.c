#include "mv.h"
#include "decoder.h"
#include "loader.h"
#include "instrucciones.h"
#include "memory.h"
#include "desensamblador.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char const *argv[]) {
    srand(time(NULL));

    if (argc < 2) {
        printf("Uso: %s [filename.vmx] [filename.vmi] [m=M] [-d] [-p param1 param2 ... paramN]\n", argv[0]);
        return 1;
    }

    MV mv;
    const char *filename = argv[1];

    ini_mv(&mv, argc, argv);

    carga_prog(&mv, filename);

    if (!(mv.err)) {
        if (mv.desensamblador) {
            printf("\n================ DESENSAMBLADO ================\n");
            desensamblar(&mv);
        }
        printf("\n================ EJECUCION ================\n");
        ejecutar(&mv);
    } else
        printf("Error al cargar el programa\n");

    printf("\nFin del programa\n");

    return 0;
}
