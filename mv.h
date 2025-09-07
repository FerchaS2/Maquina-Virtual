#ifndef MV_H
#define MV_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "opcodes.h"
#include "instrucciones.h"
#include "memory.h"

#define SEGM_CS 0x0000
#define SEGM_CS 0x0100

#define IDX_CS 26
#define IDX_DS 27
#define IDX_IP 3
#define IDX_LAR 0
#define IDX_MAR 1
#define IDX_MBR 2

#define MAX_FN 32

#define MEM 16384
#define CANT_SEGM 8
#define CANT_REG 32

typedef struct {
    uint16_t base, tam;
} SegDesc;

typedef struct {
    uint8_t memoria[MEM];
    SegDesc segmentos[CANT_SEGM];
    uint32_t registros[CANT_REG];
}   MV;

void ini_mv(MV * mv);
void ejecutar(MV *mv);

#endif