#include "memory.h"
#include "mv.h"

void traductor(MV * mv, uint16_t segm, uint16_t offset, int nbytes, uint32_t *dir) {
    if (segm >= CANT_SEGM)
        mv->err = ERR_SEG;
    else {
        SegDesc seg = mv->segmentos[segm];
        if ((offset + nbytes > seg.tam) && (offset < seg.tam)) { //si offset == seg.tam significa que es la última instrucción (STOP debería ser siempre si no se rompe)
            mv->err = ERR_FDSEG;
        } else
            *dir = seg.base + offset;
    }
}