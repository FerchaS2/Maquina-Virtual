#ifndef MV_H
#define MV_H

#include <stdint.h>

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

#endif