#include "mv.h"
#include "decoder.h"
#include "instrucciones.h"

void ini_mv(MV * mv) {
    memset(mv->memoria, 0, MEM);
    memset(mv->registros, 0, sizeof(mv->registros));
    memset(mv->segmentos, 0, sizeof(mv->segmentos));
}

void incIP(MV *mv, uint16_t inc) {
    uint32_t ip_val = mv->registros[IDX_IP];
    uint16_t seg = (ip_val >> 16);
    uint16_t off = ip_val & 0xFFFF;

    off += inc;
    mv->registros[IDX_IP] = (seg << 16) | off; //dejo sin tocar el segmento e incremento el offset

    //la validación de segmentos la hago en el traductor, esta función sólo incrementa IP
}


void ejecutar(MV *mv) {
    int fin = 0;
    InstrDecod instr;
    Fn_Instr vecFn[MAX_FN];
    Fn_Instr FnAct;
    int err;
    uint32_t ip;
    uint16_t segact;
    
    ini_VecFn(vecFn);

    while (!fin) {
        ip = mv->registros[IDX_IP];
        segact = (ip >> 16) & 0xFFFF;
        if (segact != SEGM_CS) {
            fin = 1;
            //fuera de segmento de código
        } else {
            decodificador(mv, &instr, &err);
            if (err) {
                printf("Error de decodificación\n");
                fin = 1;
            } else {
                FnAct = vecFn[instr.opc];
                if (FnAct) 
                    FnAct(mv, &instr);
                else {
                    printf("No existe el opcode\n");
                    fin = 1;
                }
            }
        }
    }
}