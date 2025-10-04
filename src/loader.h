#ifndef LOADER_H
#define LOADER_H

#include "mv.h"

#define ERR_IO 1
#define ERR_ID 2
#define ERR_VER 3
#define ERR_COD 4
#define ERR_MEM 21

void carga_vmx_v1(MV * mv, FILE * arch);
uint16_t leer16BE(FILE *arch);
uint32_t leer32BE(FILE *arch);
void carga_vmx_v2(MV *mv, FILE *arch);
void carga_vmi(MV *mv, FILE *arch);
void carga_prog(MV * mv, const char * filename);

#endif