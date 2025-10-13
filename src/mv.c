#include "mv.h"
#include "decoder.h"
#include "instrucciones.h"

#define ERR_OPINV 11

void ini_mv(MV * mv, int argc, char *argv[]) {
    // Valores por defecto
    uint32_t memoria_total = 16 * 1024; // 16 KiB por defecto
    mv->archivo_vmi[0] = '\0';
    mv->modo_debug = 0;
    mv->err = 0;
    mv->desensamblador = 0;

    for (int i = 1; i < argc; i++) {
        // Archivo .vmi (imagen)
        if (strstr(argv[i], ".vmi")) {
            strcpy(mv->archivo_vmi, argv[i]);
        }

        // Tamaño de memoria -> formato m=NN
        else if (strncmp(argv[i], "m=", 2) == 0) {
            memoria_total = atoi(argv[i] + 2) * 1024; // valor en KiB
        }
        
        else if (strcmp(argv[i], "-d") == 0) {
            mv->desensamblador = 1;
        }
    }

    mv->memoria = calloc(memoria_total, 1);
    mv->memoria_total = memoria_total;
    memset(mv->registros, 0, sizeof(mv->registros));
    memset(mv->segmentos, 0, sizeof(mv->segmentos));

    carga_parametros(mv, argc, argv);
}

void incIP(MV *mv, uint16_t inc) {
    uint32_t ip_val = mv->registros[IDX_IP];
    uint16_t seg = (ip_val >> 16);
    uint16_t off = ip_val & 0xFFFF;

    off += inc;
    mv->registros[IDX_IP] = (seg << 16) | off; //dejo sin tocar el segmento e incremento el offset

    //la validación de segmentos la hago en el traductor, esta función sólo incrementa IP
}

void carga_parametros(MV * mv, int argc, char *argv[]) {
    int i = 1, cont, j;
    uint32_t base, pos, puntero;

    while (i < argc && strcmp(argv[i], "-p") != 0) {
        i++;
    }

    if (i == argc) {
        mv->argc = 0;
        mv->argv = 0xFFFFFFFF;
        mv->registros[IDX_PS] = 0xFFFFFFFF;
    } else {   // Si llegamos hasta acá significa que sí hay PARAM SEGMENT
        mv->registros[IDX_PS] = 0x0000000;
        mv->segmentos[mv->registros[IDX_PS]].base = 0;

        cont = argc - i - 1;   // cantidad de parámetros
        mv->argc = cont;

        base = mv->segmentos[mv->registros[IDX_PS]].base;
        pos = base; // posición actual dentro de memoria
        uint32_t ptrs[cont]; // offsets de cada string

        for (j = 0; j < cont; j++) {
            char *p = argv[i + 1 + j];
            int len = strlen(p) + 1; // incluye el \0
            ptrs[j] = pos - base;    // offset dentro del segmento, la base siempre es 0, pero quizás en algún momento podría cambiar
            memcpy(&mv->memoria[pos], p, len);
            pos += len;
        }

        // Guardar los punteros en BIG-ENDIAN
        for (j = 0; j < cont; j++) {
            puntero = (mv->registros[IDX_PS] << 16) | ptrs[j];
            mv->memoria[pos + 0] = (puntero >> 24) & 0xFF;
            mv->memoria[pos + 1] = (puntero >> 16) & 0xFF;
            mv->memoria[pos + 2] = (puntero >> 8) & 0xFF;
            mv->memoria[pos + 3] = puntero & 0xFF;
            pos += 4;
        }

        // Actualizar la MV
        mv->segmentos[mv->registros[IDX_PS]].tam = pos - base;
        mv->argv = mv->segmentos[mv->registros[IDX_PS]].tam - cont * 4; // posición de los punteros
    }
}


void ejecutar(MV *mv) {
    int fin = 0;
    InstrDecod instr;
    Fn_Instr vecFn[MAX_FN];
    Fn_Instr FnAct;
    uint32_t ip;
    uint16_t segact;
    
    ini_VecFn(vecFn);

    while (!fin) {
        ip = mv->registros[IDX_IP];
        segact = (ip >> 16) & 0xFFFF;
        if (segact != SEGM_CS) 
            fin = 1;
        else {
            decodificador(mv, &instr);
            if (mv->err)
                fin = 1;
            else {
                FnAct = vecFn[instr.opc];
                if (FnAct) {
                    FnAct(mv, &instr);
                }
                else {
                    mv->err = ERR_OPINV;
                    fin = 1;
                }
            }
        }
    }

// RECORDAR HACER FREE DE TODOS LAS VARIABLES DINÁMICAS

    if(mv->err) {
        printf("\nEl programa finalizo por un error:\n");
        switch (mv->err)
        {
        case ERR_DIV:
            printf("  -> División por cero\n");
            break;
        
        case ERR_SWAP:
            printf("  -> SWAP tiene inmediatos\n");
            break;

        case ERR_OPINV:
            printf("  -> Operación inválida\n");
            break;

        case 6:
            printf("  -> Fuera de segmento\n");
            break;

        default:
            printf("  -> Error no implementado\n");
            break;
        }
    }
}