#include "mv.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void ini_mv(MV * mv) {
    memset(mv->memoria, 0, MEM);
    memset(mv->registros, 0, sizeof(mv->registros));
    memset(mv->segmentos, 0, sizeof(mv->segmentos));
}