#ifndef DESENSAMBLADOR_H
#define DESENSAMBLADOR_H

#include "mv.h"
#include "decoder.h"
#include "opcodes.h"

void desensamblar(MV *mv);
void mostrarInstr(InstrDecod *instr, char *vecMNEM[], char *vecREGS[]);
void printOperand(uint32_t op, char *vecREGS[]);

#endif