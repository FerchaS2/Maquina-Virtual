#include "mv.h"
#include "decoder.h"
#include "instrucciones.h"

#define ERR_OPINV 11

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
        if (segact != SEGM_CS) 
            fin = 1;
        else {
            decodificador(mv, &instr, &err);
            if (err)
                fin = 1;
            else {
                FnAct = vecFn[instr.opc];
                if (FnAct) 
                    FnAct(mv, &instr, &err);
                else {
                    err = ERR_OPINV;
                    fin = 1;
                }
            }
        }
    }

    if(err) {
        printf("El programa finalizo por un error:\n");
        switch (err)
        {
        case ERR_DIV:
            printf("División por cero");
            break;
        
        case ERR_SWAP:
            printf("SWAP");
            break;

        case ERR_OPINV:
            printf("Operación inválida");
            break;

        case 6:
            printf("Fuera de segmento");
            break;

        default:
            printf("Error no implementado");
            break;
        }
    }
}