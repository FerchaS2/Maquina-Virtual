#include "desensamblador.h"
#include "memory.h"

void ini_VecMNEM(char *vec[]) {
    for (int i = 0; i < 32; i++)
        vec[i] = "";
    
    vec[OPC_SYS]  = "SYS";
    vec[OPC_JMP]  = "JMP";
    vec[OPC_JZ]   = "JZ";
    vec[OPC_JP]   = "JP";
    vec[OPC_JN]   = "JN";
    vec[OPC_JNZ]  = "JNZ";
    vec[OPC_JNP]  = "JNP";
    vec[OPC_JNN]  = "JNN";
    vec[OPC_NOT]  = "NOT";

    vec[OPC_STOP] = "STOP";

    vec[OPC_MOV]  = "MOV";
    vec[OPC_ADD]  = "ADD";
    vec[OPC_SUB]  = "SUB";
    vec[OPC_MUL]  = "MUL";
    vec[OPC_DIV]  = "DIV";
    vec[OPC_CMP]  = "CMP";
    vec[OPC_SHL]  = "SHL";
    vec[OPC_SHR]  = "SHR";
    vec[OPC_SAR]  = "SAR";
    vec[OPC_AND]  = "AND";
    vec[OPC_OR]   = "OR";
    vec[OPC_XOR]  = "XOR";
    vec[OPC_SWAP] = "SWAP";
    vec[OPC_LDL]  = "LDL";
    vec[OPC_LDH]  = "LDH";
    vec[OPC_RND]  = "RND";
}

void ini_VecREGS(char *vec[]) {
    for (int i = 0; i < 32; i++)
        vec[i] = "";   // por defecto reservado o sin nombre

    vec[IDX_LAR] = "LAR";
    vec[IDX_MAR] = "MAR";
    vec[IDX_MBR] = "MBR";
    vec[IDX_IP]  = "IP";
    vec[IDX_OPC] = "OPC";
    vec[IDX_OP1] = "OP1";
    vec[IDX_OP2] = "OP2";

    vec[IDX_EAX] = "EAX";
    vec[IDX_EBX] = "EBX";
    vec[IDX_ECX] = "ECX";
    vec[IDX_EDX] = "EDX";
    vec[IDX_EEX] = "EEX";
    vec[IDX_EFX] = "EFX";

    vec[IDX_AC]  = "AC";
    vec[IDX_CC]  = "CC";

    vec[IDX_CS]  = "CS";
    vec[IDX_DS]  = "DS";
}


void printOperand(uint32_t op, char *vecREGS[]) {
    uint8_t indexReg, tipo = (op >> 24) & 0xFF;
    uint16_t off;

    switch (tipo) {
        case 1: // registro
            printf("%s", vecREGS[op & 0xFF]);
            break;
        case 2: // inmediato
            printf("%d", (int16_t)(op & 0xFFFF));
            break;
        case 3: { // memoria
            indexReg = (op >> 16) & 0x1F; //me quedo con los 5 bits que representan el registro en op
            off = op & 0xFFFF;
            if (off)
                printf("[%s + %d]", vecREGS[indexReg], off);
            else
                printf("[%s]", vecREGS[indexReg]);
            break;
        }
        default: {
            printf(" ");
        }
    }
}

void mostrarInstr(InstrDecod *instr, char *vecMNEM[], char *vecREGS[]) {

    printf("%s ", vecMNEM[instr->opc]);

    if (instr->op1) {
        printOperand(instr->op1, vecREGS); // función que imprime un operando
        if (instr->op2) { //si no existe op1, tampoco existe op2 por diseño
            printf(", ");
            printOperand(instr->op2, vecREGS);
        }
    }

    printf("\n");
}

void desensamblar(MV *mv) {
    InstrDecod instr;
    int fin = 0, tam;
    char *vecMNEM[MAX_FN], *vecREGS[32];
    uint32_t ip_ini, ip_fin, dir_fis;
    uint16_t segact;

    mv->registros[IDX_IP] = mv->registros[IDX_CS]; //posiciono IP en el comienzo del CODE SEGMENT

    ini_VecREGS(vecREGS);
    ini_VecMNEM(vecMNEM);

    while (!fin) {
        ip_ini = mv->registros[IDX_IP];
        segact = (ip_ini >> 16) & 0xFFFF;

        if (segact != SEGM_CS)
            fin = 1;
        else {
            // obtengo dirección física inicial
            traductor(mv, (ip_ini >> 16) & 0xFFFF, ip_ini & 0xFFFF, 1, &dir_fis);

            decodificador(mv, &instr);
            if (!(instr.opc == 0 && instr.op1 == 0)) { //si son los dos 0 es pq no hay STOP o se llamó a SYS 0 (que no existe)
                ip_fin = mv->registros[IDX_IP];
                tam = ip_fin - ip_ini; //cantidad de bytes de la instrucción

                if (ip_fin - ip_ini == 0)
                    fin = 1;
                else {
                    // imprimo dirección física y bytes de la instrucción
                    printf("[%04X] ", dir_fis);

                    for (int i = 0; i < tam; i++) {
                        printf("%02X ", mv->memoria[dir_fis + i]);
                    }
                    // relleno para alinear hasta 7 bytes
                    for (int i = tam; i < 7; i++) {
                        printf("   ");
                    }

                    printf("| ");
                    mostrarInstr(&instr, vecMNEM, vecREGS);
                }
            } else 
                fin = 1;
            
        }
    }
    mv->registros[IDX_IP] = mv->registros[IDX_CS];
    mv->err = 0;
}
