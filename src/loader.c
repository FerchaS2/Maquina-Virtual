#include "loader.h"
#include <string.h>
#include <stdio.h>

void carga_prog(MV * mv, const char * filename) {
    FILE * arch = fopen(filename,"rb");
    char id[6];
    uint8_t ver;
    uint16_t tam_cod, high_tam, low_tam;

    if(!arch) {
        printf("Error al abrir el archivo");
        mv->err = ERR_IO;
    } else {
        fread(id, 5, 1, arch);
        if (strcmp(id, "VMX25") != 0) {
            printf("Identificador incorrecto\n");
            mv->err = ERR_ID;
        } else {
            fread(&ver, 1, 1, arch);
            if (ver != 1) {
                printf("Versión incorrecta\n");
                mv->err = ERR_VER;
            } else {
                fread(&high_tam, 1, 1, arch);
                fread(&low_tam, 1, 1, arch);
                tam_cod = (high_tam << 8) | low_tam;
                if (tam_cod > MEM) {
                    printf("Código demasiado grande\n");
                    mv->err = ERR_COD;
                } else {
                    mv->segmentos[0].base = 0;
                    mv->segmentos[0].tam = tam_cod;
                    mv->segmentos[1].base = tam_cod;
                    mv->segmentos[1].tam = MEM - tam_cod;
                    fread(mv->memoria, 1, tam_cod, arch);
                    mv->registros[IDX_CS] = 0; //CS = 26 y DS = 27
                    mv->registros[IDX_DS] = (1 << 16) | 0; // DS = 00 01 00 00
                    mv->registros[IDX_IP] = mv->registros[IDX_CS];
                }
            }
        }
    }
    fclose(arch);
}