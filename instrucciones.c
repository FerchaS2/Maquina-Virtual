#include "instrucciones.h"
#include "mv.h"
#include "decoder.h"
#include "opcodes.h"
#include "memory.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MODE_DEC  0x01
#define MODE_CHAR  0x02
#define MODE_OCT   0x04
#define MODE_HEX   0x08
#define MODE_BIN   0x10

#define ERR_NOTINM 10

void ini_VecFn(Fn_Instr *vec) {
    for (int i = 0; i < 32; i++)
        vec[i] = NULL;
    
    vec[OPC_SYS]  = Fn_SYS;
    vec[OPC_JMP]  = Fn_JMP;
    vec[OPC_JZ]   = Fn_JZ;
    vec[OPC_JP]   = Fn_JP;
    vec[OPC_JN]   = Fn_JN;
    vec[OPC_JNZ]  = Fn_JNZ;
    vec[OPC_JNP]  = Fn_JNP;
    vec[OPC_JNN]  = Fn_JNN;
    vec[OPC_NOT]  = Fn_NOT;

    vec[OPC_STOP] = Fn_STOP;

    vec[OPC_MOV]  = Fn_MOV;
    vec[OPC_ADD]  = Fn_ADD;
    vec[OPC_SUB]  = Fn_SUB;
    vec[OPC_MUL]  = Fn_MUL;
    vec[OPC_DIV]  = Fn_DIV;
    vec[OPC_CMP]  = Fn_CMP;
    vec[OPC_SHL]  = Fn_SHL;
    vec[OPC_SHR]  = Fn_SHR;
    vec[OPC_SAR]  = Fn_SAR;
    vec[OPC_AND]  = Fn_AND;
    vec[OPC_OR]   = Fn_OR;
    vec[OPC_XOR]  = Fn_XOR;
    vec[OPC_SWAP] = Fn_SWAP;
    vec[OPC_LDL]  = Fn_LDL;
    vec[OPC_LDH]  = Fn_LDH;
    vec[OPC_RND]  = Fn_RND;
}

uint32_t getValorPorInstr(MV *mv, uint32_t op) {
    uint8_t tipo = (op >> 24) & 0xFF; //saco el tipo de operando
    uint8_t indexReg;
    uint16_t offset, segm;
    uint32_t res, dir_fisica;
    int32_t resaux; //para extender signo en el inmediato
    int err, nbytes;

    switch (tipo)
    {
    case 1: //registro
        indexReg = op & 0xFF;
        res = mv->registros[indexReg];
        break;
    
    case 2: //inmediato
        res = op & 0x0000FFFF;
        resaux = res; //utilizo resaux, que es signed, para extender el signo
        resaux = (resaux << 16) >> 16;
        res = resaux;
        break;

    case 3: //memoria
        indexReg = (op >> 16) & 0x1F; //me quedo con los 5 bits que representan el registro en op
        offset = (op & 0xFF) + (mv->registros[indexReg] & 0xFFFF);
        segm = (mv->registros[indexReg] >> 16) & 0xFFFF;
        mv->registros[IDX_LAR] = (segm << 16) | offset;
        nbytes = 4;
        traductor(mv, segm, offset, nbytes, &err, &dir_fisica);
        mv->registros[IDX_MAR] = (nbytes << 16) | (dir_fisica & 0xFFFF);
        res = 0;
        res  = mv->memoria[dir_fisica]   << 24;
        res |= mv->memoria[dir_fisica+1] << 16;
        res |= mv->memoria[dir_fisica+2] << 8;
        res |= mv->memoria[dir_fisica+3];
        mv->registros[IDX_MBR] = res;
        break;
    }

    return res;
}

void setValorPorInstr(MV *mv, uint32_t op, uint32_t resultado) {
    uint8_t tipo = (op >> 24) & 0xFF; //saco el tipo de operando
    uint8_t indexReg;
    uint16_t offset, segm;
    uint32_t dir_fisica;
    int err, nbytes;

    switch (tipo)
    {
    case 1: //registro
        indexReg = op & 0xFF;
        mv->registros[indexReg] = resultado;
        break;

    case 3: //memoria
        indexReg = (op >> 16) & 0x1F; //me quedo con los 5 bits que representan el registro en op
        offset = (op & 0xFF) + (mv->registros[indexReg] & 0xFFFF);
        segm = (mv->registros[indexReg] >> 16) & 0xFFFF;
        mv->registros[IDX_LAR] = (segm << 16) | offset;
        nbytes = 4;
        traductor(mv, segm, offset, nbytes, &err, &dir_fisica);
        mv->registros[IDX_MAR] = (nbytes << 16) | (dir_fisica & 0xFFFF);
        mv->registros[IDX_MBR] = resultado;
        mv->memoria[dir_fisica] = (resultado >> 24) & 0xFF;
        mv->memoria[dir_fisica+1] = (resultado >> 16) & 0xFF;
        mv->memoria[dir_fisica+2] = (resultado >> 8) & 0xFF;
        mv->memoria[dir_fisica+3] = resultado & 0xFF;
        break;
    }
}

uint32_t sumaSigned(uint32_t val1, uint32_t val2) {
    int32_t auxv1, auxv2, res;

    auxv1 = (int32_t) val1; 
    auxv2 = (int32_t) val2;
    res = auxv1 + auxv2;
    return (uint32_t) res; //lo devuelvo unsigned porque la memoria no diferencia signos
}

uint32_t restaSigned(uint32_t val1, uint32_t val2) {
    int32_t auxv1, auxv2, res;

    auxv1 = (int32_t) val1; 
    auxv2 = (int32_t) val2;
    res = auxv1 - auxv2;
    return (uint32_t) res; 
}

uint32_t mulSigned(uint32_t val1, uint32_t val2) {
    int32_t auxv1, auxv2, res;

    auxv1 = (int32_t) val1; 
    auxv2 = (int32_t) val2;
    res = auxv1 * auxv2;
    return (uint32_t) res; 
}

void divSigned(uint32_t val1, uint32_t val2, uint32_t *coc, uint32_t *resto) {
    int32_t auxv1 = (int32_t) val1;
    int32_t auxv2 = (int32_t) val2;
    int32_t q = auxv1 / auxv2;
    int32_t r = auxv1 % auxv2;

    *coc = (uint32_t)q;
    *resto = (uint32_t)r;
}

void Fn_ADD(MV *mv, InstrDecod *instr, int *err) {
    uint32_t val1, val2, res;
    int32_t signedRes;
    (void)err;

    mv->registros[IDX_CC] = 0;
    val1 = getValorPorInstr(mv, instr->op1);
    val2 = getValorPorInstr(mv, instr->op2);
    res = sumaSigned(val1, val2);
    setValorPorInstr(mv, instr->op1, res);

    signedRes = (int32_t)res;

    if (signedRes < 0)
        mv->registros[IDX_CC] |= FLAG_N; // set bit N
    else if (signedRes == 0)
        mv->registros[IDX_CC] |= FLAG_Z; // set bit Z
}

void Fn_MOV(MV *mv, InstrDecod *instr, int *err) {
    (void)err;

    setValorPorInstr(mv, instr->op1, getValorPorInstr(mv, instr->op2));
}

void Fn_STOP(MV *mv, InstrDecod *instr, int *err) {
    (void)instr;
    (void)err;

    mv->registros[IDX_IP] = 0xFFFFFFFF;
}

void Fn_SUB(MV *mv, InstrDecod *instr, int *err) {
    uint32_t val1, val2, res;
    int32_t signedRes;
    (void)err;

    mv->registros[IDX_CC] = 0;
    val1 = getValorPorInstr(mv, instr->op1);
    val2 = getValorPorInstr(mv, instr->op2);
    res = restaSigned(val1, val2);
    setValorPorInstr(mv, instr->op1, res);

    signedRes = (int32_t)res;

    if (signedRes < 0)
        mv->registros[IDX_CC] |= FLAG_N; // set bit N
    else if (signedRes == 0)
        mv->registros[IDX_CC] |= FLAG_Z; // set bit Z
}

void Fn_DIV(MV *mv, InstrDecod *instr, int *err) {
    uint32_t val1, val2, coc, resto;
    int32_t signedRes;

    mv->registros[IDX_CC] = 0;
    val1 = getValorPorInstr(mv, instr->op1);
    val2 = getValorPorInstr(mv, instr->op2);
    if (val2) {
        divSigned(val1, val2, &coc, &resto);
        setValorPorInstr(mv, instr->op1, coc);
        mv->registros[IDX_AC] = resto;

        signedRes = (int32_t)coc;

        if (signedRes < 0)
            mv->registros[IDX_CC] |= FLAG_N; // set bit N
        else if (signedRes == 0)
            mv->registros[IDX_CC] |= FLAG_Z; // set bit Z
    } else
        *err = ERR_DIV;
}

void Fn_MUL(MV *mv, InstrDecod *instr, int *err) {
    uint32_t val1, val2, res;
    int32_t signedRes;
    (void)err;

    mv->registros[IDX_CC] = 0;
    val1 = getValorPorInstr(mv, instr->op1);
    val2 = getValorPorInstr(mv, instr->op2);
    res = mulSigned(val1, val2);
    setValorPorInstr(mv, instr->op1, res);

    signedRes = (int32_t)res;

    if (signedRes < 0)
        mv->registros[IDX_CC] |= FLAG_N; // set bit N
    else if (signedRes == 0)
        mv->registros[IDX_CC] |= FLAG_Z; // set bit Z
}

void Fn_CMP(MV *mv, InstrDecod *instr, int *err) {
    uint32_t val1, val2, res;
    int32_t signedRes;
    (void)err;

    mv->registros[IDX_CC] = 0;
    val1 = getValorPorInstr(mv, instr->op1);
    val2 = getValorPorInstr(mv, instr->op2);
    res = restaSigned(val1, val2);

    signedRes = (int32_t)res;

    if (signedRes < 0)
        mv->registros[IDX_CC] |= FLAG_N; // set bit N
    else if (signedRes == 0)
        mv->registros[IDX_CC] |= FLAG_Z; // set bit Z
}

void Fn_SHL(MV *mv, InstrDecod *instr, int *err) {
    uint32_t val, cant, res;
    int32_t signedRes;
    (void)err;

    mv->registros[IDX_CC] = 0;
    val = getValorPorInstr(mv, instr->op1);
    cant = getValorPorInstr(mv, instr->op2);
    res = val << cant;
    setValorPorInstr(mv, instr->op1, res);

    signedRes = (int32_t)res;

    if (signedRes < 0)
        mv->registros[IDX_CC] |= FLAG_N; // set bit N
    else if (signedRes == 0)
        mv->registros[IDX_CC] |= FLAG_Z; // set bit Z
}

void Fn_SHR(MV *mv, InstrDecod *instr, int *err) {
    uint32_t val, cant, res;
    int32_t signedRes;
    (void)err;

    mv->registros[IDX_CC] = 0;
    val = getValorPorInstr(mv, instr->op1);
    cant = getValorPorInstr(mv, instr->op2);
    res = val >> cant;
    setValorPorInstr(mv, instr->op1, res);

    signedRes = (int32_t)res;

    if (signedRes < 0)
        mv->registros[IDX_CC] |= FLAG_N; // set bit N
    else if (signedRes == 0)
        mv->registros[IDX_CC] |= FLAG_Z; // set bit Z
}

void Fn_SAR(MV *mv, InstrDecod *instr, int *err) {
    uint32_t val, cant, res;
    int32_t signedVal, signedRes;
    (void)err;

    mv->registros[IDX_CC] = 0;
    val = getValorPorInstr(mv, instr->op1);
    cant = getValorPorInstr(mv, instr->op2);
    signedVal = (int32_t) val;
    signedRes = signedVal >> cant;
    res = (uint32_t) signedRes;
    setValorPorInstr(mv, instr->op1, res);

    if (signedRes < 0)
        mv->registros[IDX_CC] |= FLAG_N; // set bit N
    else if (signedRes == 0)
        mv->registros[IDX_CC] |= FLAG_Z; // set bit Z
}

void Fn_AND(MV *mv, InstrDecod *instr, int *err) {
    uint32_t val1, val2, res;
    int32_t signedRes;
    (void)err;

    mv->registros[IDX_CC] = 0;
    val1 = getValorPorInstr(mv, instr->op1);
    val2 = getValorPorInstr(mv, instr->op2);
    res = val1 & val2;
    setValorPorInstr(mv, instr->op1, res);

    signedRes = (int32_t)res;

    if (signedRes < 0)
        mv->registros[IDX_CC] |= FLAG_N; // set bit N
    else if (signedRes == 0)
        mv->registros[IDX_CC] |= FLAG_Z; // set bit Z
}

void Fn_OR(MV *mv, InstrDecod *instr, int *err) {
    uint32_t val1, val2, res;
    int32_t signedRes;
    (void)err;

    mv->registros[IDX_CC] = 0;
    val1 = getValorPorInstr(mv, instr->op1);
    val2 = getValorPorInstr(mv, instr->op2);
    res = val1 | val2;
    setValorPorInstr(mv, instr->op1, res);

    signedRes = (int32_t)res;

    if (signedRes < 0)
        mv->registros[IDX_CC] |= FLAG_N; // set bit N
    else if (signedRes == 0)
        mv->registros[IDX_CC] |= FLAG_Z; // set bit Z
}

void Fn_XOR(MV *mv, InstrDecod *instr, int *err) {
    uint32_t val1, val2, res;
    int32_t signedRes;
    (void)err;

    mv->registros[IDX_CC] = 0;
    val1 = getValorPorInstr(mv, instr->op1);
    val2 = getValorPorInstr(mv, instr->op2);
    res = val1 ^ val2;
    setValorPorInstr(mv, instr->op1, res);

    signedRes = (int32_t)res;

    if (signedRes < 0)
        mv->registros[IDX_CC] |= FLAG_N; // set bit N
    else if (signedRes == 0)
        mv->registros[IDX_CC] |= FLAG_Z; // set bit Z
}

void Fn_SWAP(MV *mv, InstrDecod *instr, int *err) {
    //Ya se verifica que el op1 nunca sea un inmediato al ser 2 op, por lo que solo debe verificarse el op2
    uint32_t val1, val2;
    
    if (((instr->op2 >> 24) & 0xFF) == 2)
        *err = ERR_SWAP;
    else {
        val1 = getValorPorInstr(mv, instr->op1);
        val2 = getValorPorInstr(mv, instr->op2);
        setValorPorInstr(mv, instr->op1, val2);
        setValorPorInstr(mv, instr->op2, val1);
    }
}

void Fn_LDL(MV *mv, InstrDecod *instr, int *err){
    uint32_t val1, val2, res;
    (void)err;

    val1 = getValorPorInstr(mv, instr->op1);
    val2 = getValorPorInstr(mv, instr->op2);
    res = (val1 &  0xFFFF0000) | (val2 & 0x0000FFFF);
    setValorPorInstr(mv, instr->op1, res);
}

void Fn_LDH(MV *mv, InstrDecod *instr, int *err){
    uint32_t val1, val2, res;
    (void)err;

    val1 = getValorPorInstr(mv, instr->op1);
    val2 = getValorPorInstr(mv, instr->op2);
    res = (val1 &  0x0000FFFF) | ((val2 & 0x0000FFFF) << 16);
    setValorPorInstr(mv, instr->op1, res);
}

void Fn_RND(MV *mv, InstrDecod *instr, int *err){
    uint32_t max, res;
    (void)err;

    max = getValorPorInstr(mv, instr->op2);
    res = rand() % (max+1);
    setValorPorInstr(mv, instr->op1, res);
}

void Fn_JZ(MV *mv, InstrDecod *instr, int *err) {
    (void)err;
    uint32_t dest;

    if (mv->registros[IDX_CC] & FLAG_Z) {
        dest = getValorPorInstr(mv, instr->op1);
        mv->registros[IDX_IP] = (mv->registros[IDX_IP] & 0xFFFF0000) | (dest & 0x0000FFFF);
    }
}

void Fn_JP(MV *mv, InstrDecod *instr, int *err) {
    (void)err;
    uint32_t dest;

    // Positivo => N=0 y Z=0
    if (!(mv->registros[IDX_CC] & FLAG_N) && !(mv->registros[IDX_CC] & FLAG_Z)) {
        dest = getValorPorInstr(mv, instr->op1);
        mv->registros[IDX_IP] = (mv->registros[IDX_IP] & 0xFFFF0000) | (dest & 0x0000FFFF);
    }
}

void Fn_JN(MV *mv, InstrDecod *instr, int *err) {
    (void)err;
    uint32_t dest;

    if (mv->registros[IDX_CC] & FLAG_N) {
        dest = getValorPorInstr(mv, instr->op1);
        mv->registros[IDX_IP] = (mv->registros[IDX_IP] & 0xFFFF0000) | (dest & 0x0000FFFF);
    }
}

void Fn_JNZ(MV *mv, InstrDecod *instr, int *err) {
    (void)err;
    uint32_t dest;

    if (!(mv->registros[IDX_CC] & FLAG_Z)) {
        dest = getValorPorInstr(mv, instr->op1);
        mv->registros[IDX_IP] = (mv->registros[IDX_IP] & 0xFFFF0000) | (dest & 0x0000FFFF);
    }
}

void Fn_JNP(MV *mv, InstrDecod *instr, int *err) {
    (void)err;
    uint32_t dest;

    // No positivo => N=1 o Z=1
    if ((mv->registros[IDX_CC] & FLAG_N) || (mv->registros[IDX_CC] & FLAG_Z)) {
        dest = getValorPorInstr(mv, instr->op1);
        mv->registros[IDX_IP] = (mv->registros[IDX_IP] & 0xFFFF0000) | (dest & 0x0000FFFF);
    }
}

void Fn_JNN(MV *mv, InstrDecod *instr, int *err) {
    (void)err;
    uint32_t dest;

    if (!(mv->registros[IDX_CC] & FLAG_N)) {
        dest = getValorPorInstr(mv, instr->op1);
        mv->registros[IDX_IP] = (mv->registros[IDX_IP] & 0xFFFF0000) | (dest & 0x0000FFFF);
    }
}

void Fn_JMP(MV *mv, InstrDecod *instr, int *err) {
    (void)err;
    uint32_t dest;

    dest = getValorPorInstr(mv, instr->op1);
    mv->registros[IDX_IP] = (mv->registros[IDX_IP] & 0xFFFF0000) | (dest & 0x0000FFFF);
}

void Fn_NOT(MV *mv, InstrDecod *instr, int *err) {
    uint32_t val, res;
    int32_t signedRes;

    if (((instr->op1 >> 24) & 0xFF) == 0x02) { //es inmediato
        *err = ERR_NOTINM;
    } else {
        mv->registros[IDX_CC] = 0;
        val = getValorPorInstr(mv, instr->op1);
        res = ~val;
        setValorPorInstr(mv, instr->op1, res);

        signedRes = (int32_t)res;

        if (signedRes < 0)
            mv->registros[IDX_CC] |= FLAG_N; // set bit N
        else if (signedRes == 0)
            mv->registros[IDX_CC] |= FLAG_Z; // set bit Z
    }
}

uint32_t leer_valor(uint32_t modo) {
    int val = 0;
    char charaux[128];

    if (modo & MODE_DEC) {     // decimal
        scanf("%d", &val);
    } else if (modo & MODE_CHAR) {   // caracteres
        scanf("%s", charaux);
        val = (uint8_t)charaux[0];   // guardo el ASCII
    } else if (modo & MODE_OCT) {   // octal
        scanf("%o", &val);
    } else if (modo & MODE_HEX) {   // hexadecimal
        scanf("%x", &val);
    } else if (modo & MODE_BIN) {   // binario
        scanf("%s", charaux);
        val = (int) strtol(charaux, NULL, 2); // base 2, uso strtol pq no existe %b o algo parecido en C
    }

    return (uint32_t) val;
}

void SYS_READ(MV *mv) {
    uint32_t edx, ecx, eax, dir_fisica, valor;
    uint16_t segm, off;
    int nbytes, cant, err;

    edx = mv->registros[IDX_EDX];
    ecx = mv->registros[IDX_ECX];
    eax = mv->registros[IDX_EAX];

    segm = (edx >> 16) & 0xFFFF;
    off = edx & 0xFFFF;
    nbytes = (ecx >> 16) & 0xFFFF;  // tamaño de cada celda
    cant   = ecx & 0xFFFF;          // cantidad de celdas

    for (int i = 0; i < cant; i++) {
        traductor(mv, segm, off + i * nbytes, nbytes, &err, &dir_fisica);
        valor = leer_valor(eax);

        for (int j = 0; j < nbytes; j++)
            mv->memoria[dir_fisica + j] = (valor >> (8*(nbytes-1-j))) & 0xFF;
        // guardo por cada celda de 2 bytes de la memoria
    }
}

void bin_a_str(uint32_t valor, char *charaux, int nbits) {
    for (int i = nbits - 1; i >= 0; i--) {
        charaux[nbits - 1 - i] = (valor & (1u << i)) ? '1' : '0';
    }
    charaux[nbits] = '\0'; // terminador de string
}

void mostrar_valor(uint32_t modo, uint32_t valor, uint32_t dir) {
    char charaux[128];
    printf("[%04x] ", dir);

    if (modo & MODE_DEC)   // decimal
        printf("%d ", valor);
    
    if (modo & MODE_CHAR) {
        char c = (char)(valor & 0xFF);
        if (isprint(c))
            printf("%c ", c);
        else
            printf(". ");
    }
    
    if (modo & MODE_OCT)    // octal
        printf("0o%03o ", valor);
    
    if (modo & MODE_HEX)    // hexadecimal
        printf("0x%04x ", valor);
    
    if (modo & MODE_BIN) {   // binario
        bin_a_str(valor, charaux, 32);
        printf("0b%s ", charaux);
    }

    printf("\n");
}

void SYS_WRITE(MV *mv) {
    uint32_t edx, ecx, eax, dir_fisica, valor;
    uint16_t segm, off;
    int nbytes, cant, err;

    edx = mv->registros[IDX_EDX];
    ecx = mv->registros[IDX_ECX];
    eax = mv->registros[IDX_EAX];

    segm = (edx >> 16) & 0xFFFF;
    off = edx & 0xFFFF;
    nbytes = (ecx >> 16) & 0xFFFF;  // tamaño de cada celda
    cant   = ecx & 0xFFFF;          // cantidad de celdas

    for (int i = 0; i < cant; i++) {
        traductor(mv, segm, off + i * nbytes, nbytes, &err, &dir_fisica);
        valor = 0;

        for (int j = 0; j < nbytes; j++) {
            valor <<= 8;
            valor |= mv->memoria[dir_fisica + j];
        }
        
        mostrar_valor(eax, valor, off);
    }
}

void Fn_SYS(MV *mv, InstrDecod *instr, int *err) {
    (void)err;
    int llamada = getValorPorInstr(mv, instr->op1);
    
    switch (llamada) //Hago case por si a futuro hay más llamadas
    {
    case 2:
        SYS_WRITE(mv);
        break;

    case 1:
        SYS_READ(mv);
        break;
    }
}