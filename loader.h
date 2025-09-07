#ifndef LOADER_H
#define LOADER_H

#include "mv.h"
#include <stdio.h>
#include <string.h>
#define ERR_IO 1
#define ERR_ID 2
#define ERR_VER 3
#define ERR_COD 4

void carga_prog(MV * mv, const char * filename, int *err);

#endif