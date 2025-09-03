#include "loader.h"

#define ERR_IO 1
#define ERR_ID 2
#define ERR_VER 3
#define ERR_COD 4

void carga_prog(MV * mv, const char * filename, int *err) {
    FILE * arch = fopen(filename,"rb");
    char id[6];
    uint8_t ver;
    uint16_t tam_cod;

    if(!arch) {
        printf("Error al abrir el archivo");
        *err = ERR_IO;
    } else {
        fread(id, 5, 1, arch);
        if (strcmp(id, "VMX25") != 0) {
            printf("Identificador incorrecto\n");
            *err = ERR_ID;
        } else {
            fread(&ver, 1, 1, arch);
            if (ver != 1) {
                printf("Versión incorrecta\n");
                *err = ERR_VER;
            } else {
                fread(&tam_cod, 2, 1, arch);
                if (tam_cod > MEM) {
                    printf("Código demasiado grande\n");
                    *err = ERR_COD;
                } else {
                    mv->segmentos[0].base = 0;
                    mv->segmentos[0].tam = tam_cod;
                    mv->segmentos[1].base = tam_cod;
                    mv->segmentos[1].tam = MEM - tam_cod;
                    fread(mv->memoria, 1, tam_cod, arch);
                    mv->registros[IDX_CS] = 0; //CS = 26 y DS = 27
                    mv->registros[IDX_DS] = (1 << 16) | 0; // DS = 00 01 00 00
                    mv->registros[IDX_IP] = mv->registros[26];
                    *err = 0;
                }
            }
        }
    }
    fclose(arch);
}