#include "memory.h"
#include "mv.h"

void traductor(MV * mv, uint16_t segm, uint16_t offset, int nbytes, uint32_t *dir) {
    if (segm >= CANT_SEGM)
        mv->err = ERR_SEG;
    else {
        SegDesc seg = mv->segmentos[segm];
        if (offset + nbytes > (seg.tam+1)) {
            mv->err = ERR_FDSEG;
        } else
            *dir = seg.base + offset;
    }
}