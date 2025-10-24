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
    vec[OPC_PUSH]  = "PUSH";
    vec[OPC_POP]  = "POP";
    vec[OPC_CALL]  = "CALL";

    vec[OPC_STOP] = "STOP";
    vec[OPC_RET]  = "RET";

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
    vec[IDX_SP]  = "SP";
    vec[IDX_BP]  = "BP";

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
    vec[IDX_ES]  = "ES";
    vec[IDX_SS]  = "SS";
    vec[IDX_KS]  = "KS";
    vec[IDX_PS]  = "PS";
}


void printOperand(uint32_t op, char *vecREGS[]) {
    uint8_t indexReg, tipo = (op >> 24) & 0xFF, sec, byteR, tam_celda;
    uint16_t off;
    char prefijo;

    switch (tipo) {
        case 1: { // registro
            indexReg = op & 0x1F;
            sec = (op >> 6) & 0x03; // sector del registro

            switch (sec) {
                case 0b00: // registro de 4 bytes
                    printf("%s", vecREGS[indexReg]);
                    break;
                case 0b01: // 4to byte -> AL, BL, CL, etc.
                    printf("%cL", vecREGS[indexReg][1]);
                    break;
                case 0b10: // 3er byte -> AH, BH, CH, etc.
                    printf("%cH", vecREGS[indexReg][1]);
                    break;
                case 0b11: // registro de 2 bytes
                    printf("%s", vecREGS[indexReg] + 1); 
                    break;
            }
            break;
        }
        case 2: // inmediato
            printf("%d", (int16_t)(op & 0xFFFF));
            break;
        case 3: { // memoria
            byteR = (op >> 16) & 0xFF;          //tomo el byte del registro y del tamaño de celda
            indexReg = byteR & 0x1F;            //me quedo con los 5 bits que representan el registro en op
            tam_celda = (byteR >> 6) & 0x03;    //tamaño de la celda
            off = op & 0xFFFF;

            switch (tam_celda) {
                case 0b00: prefijo = 'l'; break; // long
                case 0b10: prefijo = 'w'; break; // word
                case 0b11: prefijo = 'b'; break; // byte
                default: prefijo = '?'; break;
            }

            if (off)
                printf("%c[%s + %d]", prefijo, vecREGS[indexReg], off);
            else
                printf("%c[%s]",prefijo, vecREGS[indexReg]);
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

void mostrarKS(MV *mv) {
    uint16_t segm = (mv->registros[IDX_KS] >> 16) & 0xFFFF;
    uint32_t base = mv->segmentos[segm].base;
    uint32_t tam = mv->segmentos[segm].tam;
    uint32_t dir_fisica = base;
    uint8_t c;
    char aux[256];
    int len, mostrar;

    while (dir_fisica < base + tam) {
        // Copiar la cadena completa hasta el '\0' o fin de segmento
        len = 0;
        while (dir_fisica + len < base + tam && (c = mv->memoria[dir_fisica + len]) != '\0') {
            aux[len++] = c;
        }
        aux[len] = '\0';

        // Imprimir dirección
        printf("   [%04X] ", dir_fisica);

        // Mostrar bytes en hexadecimal
        mostrar = len + 1; // incluye '\0'
        if (mostrar > 7) mostrar = 6; // mostrar solo primeros 6 + ".."

        for (int i = 0; i < mostrar; i++)
            printf("%02X ", mv->memoria[dir_fisica + i]);

        if (len + 1 > 7)
            printf(".. "); // indicar que continúa
        else
            // Completar espacio hasta 7 bytes (alineado)
            for (int i = mostrar; i < 7; i++)
                printf("   ");
        

        // Mostrar versión imprimible entre comillas
        printf("| \"");
        for (int i = 0; i < len; i++) {
            char ch = aux[i];
            if ((ch >= 32 && ch <= 126))
                putchar(ch);
            else if (ch == '\n')
                printf("\\n");
            else
                putchar('.');
        }
        printf("\"\n");

        // Avanzar al próximo string (pasar el '\0')
        dir_fisica += len + 1;
    }
}


void desensamblar(MV *mv) {
    InstrDecod instr;
    int tam;
    char *vecMNEM[MAX_FN], *vecREGS[32];
    uint32_t ip_ini, ip_fin, dir_fis, ip_aux;
    uint16_t segm, off, segmKS = mv->registros[IDX_KS] >> 16;
    uint32_t tamCS;

    ip_aux = mv->registros[IDX_IP]; // guardo el primer valor del ip para después retomarlo
    mv->registros[IDX_IP] = mv->registros[IDX_CS]; //posiciono IP en el comienzo del CODE SEGMENT
    segm = mv->registros[IDX_CS] >> 16;
    off = 0;
    tamCS  = mv->segmentos[segm].tam;

    ini_VecREGS(vecREGS);
    ini_VecMNEM(vecMNEM);
    if(mv->registros[IDX_KS] != 0xFFFFFFFF && mv->segmentos[segmKS].tam != 0) {
        mostrarKS(mv);
    }

    while (off < tamCS) {
        ip_ini = mv->registros[IDX_IP];

        // obtengo dirección física inicial
        traductor(mv, segm, ip_ini & 0xFFFF, 1, &dir_fis);

        decodificador(mv, &instr);
        ip_fin = mv->registros[IDX_IP];
        tam = ip_fin - ip_ini; //cantidad de bytes de la instrucción

        // imprimo dirección física y bytes de la instrucción
        if (ip_ini == ip_aux) {
            printf(" > ");
            printf("[%04X] ", dir_fis);
        } else
            printf("   [%04X] ", dir_fis);
        

        for (int i = 0; i < tam; i++) {
            printf("%02X ", mv->memoria[dir_fis + i]);
        }
        // relleno para alinear hasta 7 bytes
        for (int i = tam; i < 7; i++) {
            printf("   ");
        }

        printf("| ");
        mostrarInstr(&instr, vecMNEM, vecREGS);

        off = ip_fin & 0xFFFF;
    }
    mv->registros[IDX_IP] = ip_aux;
    mv->err = 0;
}