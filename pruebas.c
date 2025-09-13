#include "mv.h"
#include "decoder.h"
#include "loader.h"
#include "instrucciones.h"
#include "memory.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/// imprime registros clave
void mostrar_estado(MV *mv, const char *msg) {
    printf("\n=== %s ===\n", msg);
    printf("EAX = 0x%08X (%d)\n", mv->registros[IDX_EAX], (int32_t)mv->registros[IDX_EAX]);
    printf("EBX = 0x%08X (%d)\n", mv->registros[IDX_EBX], (int32_t)mv->registros[IDX_EBX]);
    printf("ECX = 0x%08X (%d)\n", mv->registros[IDX_ECX], (int32_t)mv->registros[IDX_ECX]);
    printf("EDX = 0x%08X (%d)\n", mv->registros[IDX_EDX], (int32_t)mv->registros[IDX_EDX]);
    printf(" AC = 0x%08X (%d)\n", mv->registros[IDX_AC],  (int32_t)mv->registros[IDX_AC]);
    printf(" CC = N=%d Z=%d (0x%08X)\n",
           (mv->registros[IDX_CC] >> 31) & 1,
           (mv->registros[IDX_CC] >> 30) & 1,
           mv->registros[IDX_CC]);
    printf(" IP = 0x%08X\n", mv->registros[IDX_IP]);
}

/// wrapper de ejecutar pero mostrando paso a paso
void ejecutar_debug(MV *mv) {
    InstrDecod instr;
    Fn_Instr vecFn[MAX_FN];
    Fn_Instr FnAct;
    int err = 0;
    int fin = 0;
    uint32_t ip;
    uint16_t segact;
    char pruebas[15];

    ini_VecFn(vecFn);

    while (!fin) {
        ip = mv->registros[IDX_IP];
        segact = (ip >> 16) & 0xFFFF;
        if (segact != SEGM_CS) {
            printf("Fin: IP fuera de segmento de código\n");
            break;
        }
        //scanf("%s", pruebas);
        decodificador(mv, &instr, &err);
        printf("ERR =  %d\n", err);
        if (err) {
            printf("Error de decodificación\n");
            break;
        }

        FnAct = vecFn[instr.opc];
        if (!FnAct) {
            printf("Opcode inválido: %u\n", instr.opc);
            break;
        }

        // ejecutar
        FnAct(mv, &instr, &err);

        // mostrar estado después de cada instrucción
        mostrar_estado(mv, "Después de instrucción");
        printf("ERR =  %d\n", err);

        if (mv->registros[IDX_IP] == 0xFFFFFFFF) {
            
            printf("STOP alcanzado\n");
            fin = 1;
        }
    }
}

int main() {
    
    MV mv;
    const char * filename = "pruebas.vmx";
    int err;
    char * pruebas;
    ini_mv(&mv); // inicializar tu máquina (registros, memoria)

    carga_prog(&mv, filename, &err);
    if(!err) {
        mostrar_estado(&mv, "Estado inicial");
        ejecutar_debug(&mv);
        scanf("%s", pruebas);
        mostrar_estado(&mv, "Estado final");
    }

    return 0;
}