#ifndef MEMORY_H
#define MEMORY_H

#include "mv.h"

#define ERR_SEG 5
#define ERR_FDSEG 6

void traductor(MV * mv, uint16_t segm, uint16_t offset, int nbytes, int *err, uint32_t *dir);

#endif