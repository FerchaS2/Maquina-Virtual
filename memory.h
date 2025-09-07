#ifndef MEMORY_H
#define MEMORY_H

#include "mv.h"
#include <stdio.h>
#include <string.h>
#define ERR_SEG 1
#define ERR_FDSEG 2

void traductor(MV * mv, uint16_t segm, uint16_t offset, int nbytes, int *err, uint32_t *dir);

#endif