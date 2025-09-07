#include "decoder.h"
#include "memory.h"
#include "mv.h"

#define ERR_OP1INM 5

void cargaOp(MV *mv, uint8_t tipo, uint32_t *dest, int *err) {
    uint32_t val = 0, dir_fis;
    uint16_t seg, off;
    uint32_t ip_val = mv->registros[IDX_IP];

    seg = (ip_val >> 16);
    off = ip_val & 0xFFFF; //off es 16 bits, recorta ip_val a sus 2 bytes menos significativos

    switch (tipo) {
        case 1: // registro (1 byte)
            traductor(mv, seg, off, 1, err, &dir_fis);
            if (!*err) {
                val = mv->memoria[dir_fis];
                incIP(mv, 1);
            }
            break;

        case 2: // inmediato (2 bytes)
            traductor(mv, seg, off, 2, err, &dir_fis);
            if (!*err) {
                val  = mv->memoria[dir_fis] << 8;
                val |= mv->memoria[dir_fis+1];
                incIP(mv, 2);
            }
            break;

        case 3: // memoria (3 bytes: segmento+offset)
            traductor(mv, seg, off, 3, err, &dir_fis);
            if (!*err) {
                val  = mv->memoria[dir_fis] << 16;
                val |= mv->memoria[dir_fis+1] << 8;
                val |= mv->memoria[dir_fis+2];
                incIP(mv, 3);
            }
            break;
    }

    *dest = (tipo << 24) | val;
}


void decodificador(MV *mv, InstrDecod *instr, int *err) {
    uint32_t ip = mv->registros[IDX_IP];
    uint32_t dir_fis;
    uint8_t ipb, t1, t2;

    instr->op1 = instr->op2 = 0;
    *err = 0;
    traductor(mv, (ip >> 16) & 0xFFFF, ip & 0xFFFF, 1, err, &dir_fis);

    if (!(*err)) {
        ipb = mv->memoria[dir_fis];
        instr->opc = ipb & 0x001F;
        t2  = (ipb >> 6) & 0x03;
        t1  = (ipb >> 4) & 0x03;
        mv->registros[IDX_IP]++;

        if (t1 && t2) { //dos operandos
            if (t1 != 0x02) { //el op1 no puede ser un inmediato
                cargaOp(mv, t2, &(instr->op2), err);
                cargaOp(mv, t1, &(instr->op1), err);
            } else
                *err = ERR_OP1INM;
        } else if (t2) //un operando
            if (t2 != 0x02) //el op1 no puede ser un inmediato
                cargaOp(mv, t2, &(instr->op1), err);
            else
                *err = ERR_OP1INM;
        //Sin operandos -> No hace nada, quedan con 0 ambos.
    }
}
