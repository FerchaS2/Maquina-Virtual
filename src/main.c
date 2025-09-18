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
    printf("Uso: %s filename.vmx [-d]\n", argv[0]);
    return 1;
    }

    MV mv;
    const char *filename = argv[1];
    int err;

    
    ini_mv(&mv);

    carga_prog(&mv, filename, &err);

    if (!err) {
        ejecutar(&mv, &err);
        if (argc > 2 && (strcmp(argv[2], "-d") == 0) && !err) {
            printf("\n================ DESENSAMBLADO ================\n");
            desensamblar(&mv);
        }
    } else
        printf("Error al cargar el programa\n");

    printf("\nFin del programa\n");

    return 0;
}
