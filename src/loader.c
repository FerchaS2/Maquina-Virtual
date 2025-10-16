#include "loader.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void carga_vmx_v1(MV * mv, FILE * arch) {
    uint16_t tam_cod, high_tam, low_tam;

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

uint16_t leer16BE(FILE *arch) {  // lee en BIG ENDIAN
    uint8_t high, low;
    fread(&high, 1, 1, arch);
    fread(&low, 1, 1, arch);
    return (high << 8) | low;
}

uint32_t leer32BE(FILE *arch) {
    uint8_t b[4];
    fread(b, 1, 4, arch);
    return ((uint32_t)b[0] << 24) | ((uint32_t)b[1] << 16) | ((uint32_t)b[2] << 8)  |  (uint32_t)b[3];
}

void carga_vmx_v2(MV *mv, FILE *arch) {
    uint16_t tam_code, tam_data, tam_extra, tam_stack, tam_const;
    uint16_t entry_point;
    uint32_t base = 0, total, dir_code, dir_const;
    int seg_idx = 0;

    // --- leer header ---
    tam_code  = leer16BE(arch);
    tam_data  = leer16BE(arch);
    tam_extra = leer16BE(arch);
    tam_stack = leer16BE(arch);
    tam_const = leer16BE(arch);
    entry_point = leer16BE(arch);

    // calcular total
    total = tam_code + tam_data + tam_extra + tam_stack + tam_const;

    if (total > mv->memoria_total) {
        // printf("Error: tamaño total de segmentos excede la memoria\n");
        mv->err = ERR_MEM;
    } else {
        // --- PARAM SEGMENT (si lo hay, ya cargado) ---
        if (mv->segmentos[IDX_PS].tam > 0) {
            base += mv->segmentos[IDX_PS].tam;
            seg_idx++;
        }

        // --- CONST SEGMENT ---
        if (tam_const > 0) {
            mv->segmentos[seg_idx].base = base;
            mv->segmentos[seg_idx].tam = tam_const;
            mv->registros[IDX_KS] = (seg_idx << 16);
            dir_const = base;
            base += tam_const;
            seg_idx++;
        } else mv->registros[IDX_KS] = 0xFFFFFFFF;

        // --- CODE SEGMENT ---
        if (tam_code > 0) {
            mv->segmentos[seg_idx].base = base;
            mv->segmentos[seg_idx].tam = tam_code;
            mv->registros[IDX_CS] = (seg_idx << 16);
            dir_code = base;
            base += tam_code;
            seg_idx++;
        } else mv->registros[IDX_CS] = 0xFFFFFFFF;

        // --- DATA SEGMENT ---
        if (tam_data > 0) {
            mv->segmentos[seg_idx].base = base;
            mv->segmentos[seg_idx].tam = tam_data;
            mv->registros[IDX_DS] = (seg_idx << 16);
            base += tam_data;
            seg_idx++;
        } else mv->registros[IDX_DS] = 0xFFFFFFFF;

        // --- EXTRA SEGMENT ---
        if (tam_extra > 0) {
            mv->segmentos[seg_idx].base = base;
            mv->segmentos[seg_idx].tam = tam_extra;
            mv->registros[IDX_ES] = (seg_idx << 16);
            base += tam_extra;
            seg_idx++;
        } else mv->registros[IDX_ES] = 0xFFFFFFFF;

        // --- STACK SEGMENT ---
        if (tam_stack > 0) {
            mv->segmentos[seg_idx].base = base;
            mv->segmentos[seg_idx].tam = tam_stack;
            mv->registros[IDX_SS] = (seg_idx << 16);
            mv->registros[IDX_SP] = (mv->registros[IDX_SS] & 0xFFFF0000) | tam_stack;
            base += tam_stack;
            seg_idx++;
        } else {
            mv->registros[IDX_SS] = 0xFFFFFFFF;
            mv->registros[IDX_SP] = 0xFFFFFFFF;
        } 

        if (tam_code > 0)
            fread(&mv->memoria[dir_code], 1, tam_code, arch);

        if (tam_const > 0)
            fread(&mv->memoria[dir_const], 1, tam_const, arch);

        // --- Inicializar IP ---
        if (mv->registros[IDX_CS] != 0xFFFFFFFF)
            mv->registros[IDX_IP] = (mv->registros[IDX_CS] & 0xFFFF0000) | entry_point;
        else
            mv->registros[IDX_IP] = 0xFFFFFFFF;
    }
}


void carga_vmi(MV *mv, FILE *arch) {
    uint16_t tam_mem_kb = leer16BE(arch);
    uint32_t tam_mem_bytes = tam_mem_kb * 1024;

    mv->memoria_total = tam_mem_bytes;
    mv->memoria = calloc(tam_mem_bytes, 1);
    if (!mv->memoria) {
        mv->err = ERR_MEM;
    } else {
        // --- Leer registros ---
        for (int i = 0; i < CANT_REG; i++)
            mv->registros[i] = leer32BE(arch);

        // --- Leer tabla de segmentos ---
        for (int i = 0; i < CANT_SEGM; i++) {
            mv->segmentos[i].base = leer16BE(arch);
            mv->segmentos[i].tam  = leer16BE(arch);
        }

        // --- Leer memoria principal ---
        fread(mv->memoria, 1, tam_mem_bytes, arch);
        mv->err = 0;  // el vmi funciona como inicializador, así que inicializo err también.
    }
}

void carga_prog(MV * mv, const char * filename) {
    FILE * arch = fopen(filename,"rb");
    char id[6];
    uint8_t ver;

    if(!arch) {
        printf("Error al abrir el archivo");
        mv->err = ERR_IO;
    } else {
        fread(id, 5, 1, arch);   // leo id
        fread(&ver, 1, 1, arch); // leo versión
        if (strcmp(id, "VMX25") != 0) {
            if (strcmp(id, "VMI25") != 0) {
                printf("Identificador incorrecto\n");
                mv->err = ERR_ID;
            } else {
                carga_vmi(mv, arch);
            }
        } else {
            switch (ver) {
            case 1:
                carga_vmx_v1(mv, arch);
                break;

            case 2:
                carga_vmx_v2(mv, arch);
                break;

            default: {
                printf("Versión incorrecta\n");
                mv->err = ERR_VER;
                break;
            }
            }
        }
        fclose(arch);
    }
}