#include "memory.h"
#include "mv.h"

void traductor(MV * mv, uint16_t segm, uint16_t offset, int nbytes, int *err, uint32_t *dir) {
    *err = 0;
    if (segm >= CANT_SEGM)
        *err = ERR_SEG;
    else {
        SegDesc seg = mv->segmentos[segm];
        if (offset + nbytes >= seg.tam)
            *err = ERR_FDSEG;
        else
            *dir = seg.base + offset;
    }
}