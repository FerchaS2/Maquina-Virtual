#ifndef DECODER_H
#define DECODER_H

#include "mv.h"

typedef struct {
    uint32_t opc, op1, op2;
} InstrDecod;


void decodificador(MV *mv, InstrDecod *instr, int *err);
void cargaOp(MV *mv, uint8_t tipo, uint32_t *dest, int *err);

#endif