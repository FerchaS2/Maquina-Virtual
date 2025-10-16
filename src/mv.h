#ifndef MV_H
#define MV_H

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#define IDX_CS 26
#define IDX_DS 27
#define IDX_IP 3
#define IDX_CC 17
#define IDX_AC 16
#define IDX_LAR 0
#define IDX_MAR 1
#define IDX_MBR 2
#define IDX_EAX 10
#define IDX_EBX 11
#define IDX_ECX 12
#define IDX_EDX 13
#define IDX_EEX 14
#define IDX_EFX 15
#define IDX_OPC 4
#define IDX_OP1 5
#define IDX_OP2 6
#define IDX_SP 7
#define IDX_BP 8
#define IDX_ES 28
#define IDX_SS 29
#define IDX_KS 30
#define IDX_PS 31

#define MAX_FN 32

#define FLAG_N (1u << 31)
#define FLAG_Z (1u << 30)

#define MEM 16384
#define CANT_SEGM 8
#define CANT_REG 32
#define LIM_VMINAME 50

typedef struct {
    uint16_t base, tam;
} SegDesc;

typedef struct {
    uint8_t *memoria;
    uint32_t memoria_total;
    SegDesc segmentos[CANT_SEGM];
    uint32_t registros[CANT_REG];
    int err, argc, modo_debug, desensamblador;
    uint32_t argv;
    char archivo_vmi[LIM_VMINAME];
} MV;

void ini_mv(MV * mv, int argc, char const *argv[]);
void ejecutar(MV *mv);
void incIP(MV *mv, uint16_t inc);
void carga_parametros(MV * mv, int argc, char const *argv[]);

#endif