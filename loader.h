#ifndef LOADER_H
#define LOADER_H

#include "mv.h"
#include <stdio.h>
#include <string.h>
#define IDX_CS 26
#define IDX_DS 27
#define IDX_IP 3

void carga_prog(MV * mv, const char * filename, int *err);

#endif