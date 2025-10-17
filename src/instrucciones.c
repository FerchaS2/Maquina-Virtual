#include "instrucciones.h"
#include "mv.h"
#include "decoder.h"
#include "opcodes.h"
#include "memory.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

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
    vec[OPC_PUSH]  = Fn_PUSH;
    vec[OPC_POP]  = Fn_POP;
    vec[OPC_CALL]  = Fn_CALL;

    vec[OPC_STOP] = Fn_STOP;
    vec[OPC_RET]  = Fn_RET;

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

uint32_t sign_extender(uint32_t val, int nbytes) {
    switch (nbytes) {
        case 1:
            if (val & 0x80)
                val |= 0xFFFFFF00;  // extiende el bit 7
            else
                val &= 0x000000FF;
            break;

        case 2:
            if (val & 0x8000)
                val |= 0xFFFF0000;  // extiende el bit 15
            else
                val &= 0x0000FFFF;
            break;

        default:
            val &= 0xFFFFFFFF; // no hace falta extender
            break;
    }
    return val;
}

uint32_t getValorPorInstr(MV *mv, uint32_t op) {
    uint8_t tipo = (op >> 24) & 0xFF; //saco el tipo de operando
    uint8_t indexReg, sec, tam_celda, byteR;
    uint16_t offset, segm;
    uint32_t res, dir_fisica;
    int32_t resaux; //para extender signo en el inmediato
    int nbytes;

    switch (tipo)
    {
    case 1: { //registro
        indexReg = op & 0x1F;
        sec = (op >> 6) & 0x03; // sector del registro
        res = mv->registros[indexReg];
        nbytes = 4;

        switch (sec) {
            case 0b01: res = res & 0xFF; nbytes = 1; break;                 // AL
            case 0b10: res = (res >> 8) & 0xFF; nbytes = 1; break;          // AH
            case 0b11: res = res & 0xFFFF; nbytes = 2; break;               // AXç
            default: break;                                                 // EAX completo
        }

        res = sign_extender(res, nbytes);
        break;
    }
    
    case 2: { //inmediato
        res = op & 0x0000FFFF;
        resaux = res; //utilizo resaux, que es signed, para extender el signo
        resaux = (resaux << 16) >> 16;
        res = resaux;
        break;
    }

    case 3: { //memoria
        byteR = (op >> 16) & 0xFF;          //tomo el byte del registro y del tamaño de celda
        indexReg = byteR & 0x1F;            //me quedo con los 5 bits que representan el registro en op
        tam_celda = (byteR >> 6) & 0x03;    //tamaño de la celda
        offset = (op & 0xFFFF) + (mv->registros[indexReg] & 0xFFFF);
        segm = (mv->registros[indexReg] >> 16) & 0xFFFF;
        mv->registros[IDX_LAR] = (segm << 16) | offset;

        switch (tam_celda) {
            case 0b00: nbytes = 4; break;  // long
            case 0b10: nbytes = 2; break;  // word
            case 0b11: nbytes = 1; break;  // byte
            default: nbytes = 4; break;
        }

        traductor(mv, segm, offset, nbytes, &dir_fisica);

        if (!(mv->err)) {
            mv->registros[IDX_MAR] = (nbytes << 16) | (dir_fisica & 0xFFFF);
            res = 0;
            for (int i = 0; i < nbytes; i++)
                res |= mv->memoria[dir_fisica + i] << (8 * (nbytes - 1 - i)); // big endian
            res = sign_extender(res, nbytes);
            mv->registros[IDX_MBR] = res;
        }
        break;
    }
    }

    return res;
}

void setValorPorInstr(MV *mv, uint32_t op, uint32_t resultado) {
    uint8_t tipo = (op >> 24) & 0xFF; //saco el tipo de operando
    uint8_t indexReg, sec, byteR, tam_celda;
    uint16_t offset, segm;
    uint32_t dir_fisica;
    int nbytes;

    switch (tipo)
    {
    case 1: { //registro
        indexReg = op & 0x1F;
        sec = (op >> 6) & 0x03; // sector del registro
        uint32_t *reg = &mv->registros[indexReg];
        switch (sec) {
            case 0b00: *reg = resultado; break;      // EAX completo
            case 0b01: *reg = (*reg & 0xFFFFFF00) | (resultado & 0xFF); break;      // AL
            case 0b10: *reg = (*reg & 0xFFFF00FF) | ((resultado & 0xFF) << 8); break; // AH
            case 0b11: *reg = (*reg & 0xFFFF0000) | (resultado & 0xFFFF); break;     // AX
        }
        break;
    }

    case 3: { //memoria
        byteR = (op >> 16) & 0xFF;          //tomo el byte del registro y del tamaño de celda
        indexReg = byteR & 0x1F;            //me quedo con los 5 bits que representan el registro en op
        tam_celda = (byteR >> 6) & 0x03;    //tamaño de la celda
        offset = (op & 0xFFFF) + (mv->registros[indexReg] & 0xFFFF);
        segm = (mv->registros[indexReg] >> 16) & 0xFFFF;
        mv->registros[IDX_LAR] = (segm << 16) | offset;

        switch (tam_celda) {
            case 0b00: nbytes = 4; break;
            case 0b10: nbytes = 2; break;
            case 0b11: nbytes = 1; break;
            default: nbytes = 4; break;
        }

        traductor(mv, segm, offset, nbytes, &dir_fisica);

        int total_bytes = 4; // siempre se manipulan valores de 32 bits
        int start = total_bytes - nbytes;

        if (!(mv->err)) {
            mv->registros[IDX_MAR] = (nbytes << 16) | (dir_fisica & 0xFFFF);
            uint32_t val_escrito = resultado & ((1U << (nbytes * 8)) - 1);
            mv->registros[IDX_MBR] = sign_extender(val_escrito, nbytes);
            for (int i = 0; i < nbytes; i++)
                mv->memoria[dir_fisica + i] = (resultado >> (8 * (total_bytes - 1 - (i + start)))) & 0xFF;
        }
        break;
    }
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

void Fn_ADD(MV *mv, InstrDecod *instr) {
    uint32_t val1, val2, res;
    int32_t signedRes;

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

void Fn_MOV(MV *mv, InstrDecod *instr) {

    setValorPorInstr(mv, instr->op1, getValorPorInstr(mv, instr->op2));
}

void Fn_STOP(MV *mv, InstrDecod *instr) {
    (void)instr;

    mv->registros[IDX_IP] = 0xFFFFFFFF;
}

void Fn_SUB(MV *mv, InstrDecod *instr) {
    uint32_t val1, val2, res;
    int32_t signedRes;

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

void Fn_DIV(MV *mv, InstrDecod *instr) {
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
        mv->err = ERR_DIV;
}

void Fn_MUL(MV *mv, InstrDecod *instr) {
    uint32_t val1, val2, res;
    int32_t signedRes;

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

void Fn_CMP(MV *mv, InstrDecod *instr) {
    uint32_t val1, val2, res;
    int32_t signedRes;

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

void Fn_SHL(MV *mv, InstrDecod *instr) {
    uint32_t val, cant, res;
    int32_t signedRes;

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

void Fn_SHR(MV *mv, InstrDecod *instr) {
    uint32_t val, cant, res;
    int32_t signedRes;

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

void Fn_SAR(MV *mv, InstrDecod *instr) {
    uint32_t val, cant, res;
    int32_t signedVal, signedRes;

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

void Fn_AND(MV *mv, InstrDecod *instr) {
    uint32_t val1, val2, res;
    int32_t signedRes;

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

void Fn_OR(MV *mv, InstrDecod *instr) {
    uint32_t val1, val2, res;
    int32_t signedRes;

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

void Fn_XOR(MV *mv, InstrDecod *instr) {
    uint32_t val1, val2, res;
    int32_t signedRes;

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

void Fn_SWAP(MV *mv, InstrDecod *instr) {
    //Ya se verifica que el op1 nunca sea un inmediato al ser 2 op, por lo que solo debe verificarse el op2
    uint32_t val1, val2;
    
    if (((instr->op2 >> 24) & 0xFF) == 2)
        mv->err = ERR_SWAP;
    else {
        val1 = getValorPorInstr(mv, instr->op1);
        val2 = getValorPorInstr(mv, instr->op2);
        setValorPorInstr(mv, instr->op1, val2);
        setValorPorInstr(mv, instr->op2, val1);
    }
}

void Fn_LDL(MV *mv, InstrDecod *instr){
    uint32_t val1, val2, res;

    val1 = getValorPorInstr(mv, instr->op1);
    val2 = getValorPorInstr(mv, instr->op2);
    res = (val1 &  0xFFFF0000) | (val2 & 0x0000FFFF);
    setValorPorInstr(mv, instr->op1, res);
}

void Fn_LDH(MV *mv, InstrDecod *instr){
    uint32_t val1, val2, res;

    val1 = getValorPorInstr(mv, instr->op1);
    val2 = getValorPorInstr(mv, instr->op2);
    res = (val1 &  0x0000FFFF) | ((val2 & 0x0000FFFF) << 16);
    setValorPorInstr(mv, instr->op1, res);
}

void Fn_RND(MV *mv, InstrDecod *instr){
    uint32_t max, res;

    max = getValorPorInstr(mv, instr->op2);
    res = rand() % (max+1);
    setValorPorInstr(mv, instr->op1, res);
}

void Fn_JZ(MV *mv, InstrDecod *instr) {
    uint32_t dest;

    if (mv->registros[IDX_CC] & FLAG_Z) {
        dest = getValorPorInstr(mv, instr->op1);
        mv->registros[IDX_IP] = (mv->registros[IDX_IP] & 0xFFFF0000) | (dest & 0x0000FFFF);
    }
}

void Fn_JP(MV *mv, InstrDecod *instr) {
    uint32_t dest;

    // Positivo => N=0 y Z=0
    if (!(mv->registros[IDX_CC] & FLAG_N) && !(mv->registros[IDX_CC] & FLAG_Z)) {
        dest = getValorPorInstr(mv, instr->op1);
        mv->registros[IDX_IP] = (mv->registros[IDX_IP] & 0xFFFF0000) | (dest & 0x0000FFFF);
    }
}

void Fn_JN(MV *mv, InstrDecod *instr) {
    uint32_t dest;

    if (mv->registros[IDX_CC] & FLAG_N) {
        dest = getValorPorInstr(mv, instr->op1);
        mv->registros[IDX_IP] = (mv->registros[IDX_IP] & 0xFFFF0000) | (dest & 0x0000FFFF);
    }
}

void Fn_JNZ(MV *mv, InstrDecod *instr) {
    uint32_t dest;

    if (!(mv->registros[IDX_CC] & FLAG_Z)) {
        dest = getValorPorInstr(mv, instr->op1);
        mv->registros[IDX_IP] = (mv->registros[IDX_IP] & 0xFFFF0000) | (dest & 0x0000FFFF);
    }
}

void Fn_JNP(MV *mv, InstrDecod *instr) {
    uint32_t dest;

    // No positivo => N=1 o Z=1
    if ((mv->registros[IDX_CC] & FLAG_N) || (mv->registros[IDX_CC] & FLAG_Z)) {
        dest = getValorPorInstr(mv, instr->op1);
        mv->registros[IDX_IP] = (mv->registros[IDX_IP] & 0xFFFF0000) | (dest & 0x0000FFFF);
    }
}

void Fn_JNN(MV *mv, InstrDecod *instr) {
    uint32_t dest;

    if (!(mv->registros[IDX_CC] & FLAG_N)) {
        dest = getValorPorInstr(mv, instr->op1);
        mv->registros[IDX_IP] = (mv->registros[IDX_IP] & 0xFFFF0000) | (dest & 0x0000FFFF);
    }
}

void Fn_JMP(MV *mv, InstrDecod *instr) {
    uint32_t dest;

    dest = getValorPorInstr(mv, instr->op1);
    mv->registros[IDX_IP] = (mv->registros[IDX_IP] & 0xFFFF0000) | (dest & 0x0000FFFF);
}

void Fn_NOT(MV *mv, InstrDecod *instr) {
    uint32_t val, res;
    int32_t signedRes;

    if (((instr->op1 >> 24) & 0xFF) == 0x02) { //es inmediato
        mv->err = ERR_NOTINM;
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

uint32_t leer_valor(uint32_t modo, uint32_t dir) {
    int val = 0;
    char charaux[128];
    printf("[%04x] ", dir);

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
    int nbytes, cant;

    edx = mv->registros[IDX_EDX];
    ecx = mv->registros[IDX_ECX];
    eax = mv->registros[IDX_EAX];

    segm = (edx >> 16) & 0xFFFF;
    off = edx & 0xFFFF;
    nbytes = (ecx >> 16) & 0xFFFF;  // tamaño de cada celda
    cant   = ecx & 0xFFFF;          // cantidad de celdas

    for (int i = 0; i < cant; i++) {
        traductor(mv, segm, off + i * nbytes, nbytes, &dir_fisica);
        valor = leer_valor(eax, dir_fisica);

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
    int nbytes, cant;

    edx = mv->registros[IDX_EDX];
    ecx = mv->registros[IDX_ECX];
    eax = mv->registros[IDX_EAX];

    segm = (edx >> 16) & 0xFFFF;
    off = edx & 0xFFFF;
    nbytes = (ecx >> 16) & 0xFFFF;  // tamaño de cada celda
    cant   = ecx & 0xFFFF;          // cantidad de celdas

    for (int i = 0; i < cant; i++) {
        traductor(mv, segm, off + i * nbytes, nbytes, &dir_fisica);
        valor = 0;

        for (int j = 0; j < nbytes; j++) {
            valor <<= 8;
            valor |= mv->memoria[dir_fisica + j];
        }
        
        mostrar_valor(eax, valor, dir_fisica);
    }
}

void SYS_STRREAD(MV *mv) {
    uint32_t edx = mv->registros[IDX_EDX];
    uint16_t segm = (edx >> 16) & 0xFFFF;
    uint16_t off  = edx & 0xFFFF;
    int16_t maxlen = mv->registros[IDX_ECX] & 0xFFFF;  // CX
    uint32_t dir_fisica;
    char buffer[LIM_STRREAD];
    int len;
    
    traductor(mv, segm, off, maxlen + 1, &dir_fisica);  // +1 por el \0
    if (!(mv->err)) {
        printf("[%04x] ", dir_fisica);

        fflush(stdout);
        fgets(buffer, sizeof(buffer), stdin);

        len = strlen(buffer);

        if (maxlen != -1 && len > maxlen)
            len = maxlen;  // Si se pasó lo seteo en el máximo

        traductor(mv, segm, off, len + 1, &dir_fisica);
        if (!(mv->err)) {
            for (int i = 0; i < len; i++)
                mv->memoria[dir_fisica + i] = (uint8_t)buffer[i];
            mv->memoria[dir_fisica + len] = '\0'; // terminador
        }
    }
}

void SYS_STRWRITE(MV *mv) {
    uint32_t edx = mv->registros[IDX_EDX];
    uint16_t segm = (edx >> 16) & 0xFFFF;
    uint16_t off  = edx & 0xFFFF;
    uint32_t dir_fisica;
    uint8_t c;

    traductor(mv, segm, off, 1, &dir_fisica);
    if (!(mv->err)) {
        c = mv->memoria[dir_fisica];
        while (c != '\0') {
            putchar(c);
            c = mv->memoria[++dir_fisica];
        }
        fflush(stdout);
    }
}

void SYS_CLEAR(MV *mv) {
    (void) mv; // No se usa la MV
    system("cls || clear"); // cls para windows, clear para linux
}

void SYS_BREAKPOINT(MV *mv) {
    int c;

    if (mv->archivo_vmi[0] != '\0') { // se pasó archivo en parámetros
        FILE * f = fopen(mv->archivo_vmi, "wb");
        if (!f) {
            printf("Error al crear archivo .vmi\n");
            mv->err = ERR_IO;
        } else {
            // === HEADER ===
            uint8_t header[8] = {'V', 'M', 'I', '2', '5', 1, 0, 0};
            uint16_t tam_kib = mv->memoria_total / 1024;
            header[6] = (tam_kib >> 8) & 0xFF;
            header[7] = tam_kib & 0xFF;
            fwrite(header, 1, 8, f);

            // === REGISTROS (128 bytes) ===
            for (int i = 0; i < 32; i++) { // 32 registros * 4 bytes
                uint32_t val = mv->registros[i];
                uint8_t b[4];
                b[0] = (val >> 24) & 0xFF;
                b[1] = (val >> 16) & 0xFF;
                b[2] = (val >> 8) & 0xFF;
                b[3] = val & 0xFF;
                fwrite(b, 1, 4, f);
            }

            // === SEGMENTOS (32 bytes) ===
            for (int i = 0; i < 8; i++) { // 8 segmentos * (base + tam)
                uint16_t base = mv->segmentos[i].base;
                uint16_t tam = mv->segmentos[i].tam;
                uint8_t b[4] = {
                    (base >> 8) & 0xFF,
                    base & 0xFF,
                    (tam >> 8) & 0xFF,
                    tam & 0xFF
                };
                fwrite(b, 1, 4, f);
            }

            // === MEMORIA ===
            fwrite(mv->memoria, 1, mv->memoria_total, f);

            fclose(f);

            // === LOOP DE DEBUG ===
            printf("\n[BREAKPOINT] Imagen guardada en '%s'\n", mv->archivo_vmi);
            printf("Comandos: (g) continuar | (q) salir | (Enter) paso a paso\n");

            c = getchar();
            if (c == 'q' || c == 'Q') {
                mv->modo_debug = 0; // desactivo paso a paso
                mv->err = ERR_ABORT;
            } else if (c == '\n') {
                mv->modo_debug = 1; // modo paso a paso
            } else if (c == 'g' || c == 'G') {
                mv->modo_debug = 0;
            }
        }   
    }
}

void Fn_SYS(MV *mv, InstrDecod *instr) {
    int llamada = getValorPorInstr(mv, instr->op1);
    
    switch (llamada) {
        case 1: SYS_READ(mv); break;
        case 2: SYS_WRITE(mv); break;
        case 3: SYS_STRREAD(mv); break;
        case 4: SYS_STRWRITE(mv); break;
        case 7: SYS_CLEAR(mv); break;
        case 0xF: SYS_BREAKPOINT(mv); break;
    }
}

uint32_t getValorStack(MV *mv) {
    uint16_t segm = (mv->registros[IDX_SS] >> 16) & 0xFFFF;
    uint16_t offset = mv->registros[IDX_SP] & 0xFFFF;
    uint32_t dir_fisica;
    uint32_t val = 0;

    traductor(mv, segm, offset, 4, &dir_fisica);
    if (!(mv->err)) {
        mv->registros[IDX_MAR] = (4 << 16) | offset;
        for (int i = 0; i < 4; i++)
            val |= mv->memoria[dir_fisica + i] << (8 * (3 - i));
        mv->registros[IDX_MBR] = val;
        return val;
    } else {
        return 0;
    }
}


void setValorStack(MV *mv, uint32_t val) {
    uint16_t segm = (mv->registros[IDX_SS] >> 16) & 0xFFFF;
    uint16_t offset = mv->registros[IDX_SP] & 0xFFFF;
    uint32_t dir_fisica;

    traductor(mv, segm, offset, 4, &dir_fisica);

    if (!(mv->err)){
    mv->registros[IDX_MAR] = (4 << 16) | offset;
    mv->registros[IDX_MBR] = val;

    for (int i = 0; i < 4; i++)
        mv->memoria[dir_fisica + i] = (val >> (8 * (3 - i))) & 0xFF;
    }

}


void Fn_PUSH(MV *mv, InstrDecod *instr) {
    uint32_t val;

    mv->registros[IDX_SP] -= 4;
    if (mv->registros[IDX_SP] < mv->registros[IDX_SS])
        mv->err = ERR_STACKOVF; // finaliza el programa
    else {
        val = getValorPorInstr(mv, instr->op1);
        setValorStack(mv, val);
    }
}

void Fn_POP(MV *mv, InstrDecod *instr) {
    uint32_t val;
    uint16_t seg = (mv->registros[IDX_SS] >> 16) & 0xFFFF;
    uint32_t limite = mv->segmentos[seg].tam;
    limite = mv->registros[IDX_SS] | limite;

    if (mv->registros[IDX_SP] >= limite) {  // Recordar que la pila se maneja al revés de la memoria
        mv->err = ERR_STACKUDR;
    } else {
        val = getValorStack(mv);
        if (!(mv->err)) {
            setValorPorInstr(mv, instr->op1, val);
            mv->registros[IDX_SP] += 4;
        }
    }
}


void Fn_CALL(MV *mv, InstrDecod *instr) {
    uint32_t offset;

    mv->registros[IDX_SP] -= 4;
    if (mv->registros[IDX_SP] < mv->registros[IDX_SS]) {
        mv->err = ERR_STACKOVF;
    } else {
        offset = getValorPorInstr(mv, instr->op1) & 0xFFFF;
        setValorStack(mv, mv->registros[IDX_IP]);
        mv->registros[IDX_IP] = (mv->registros[IDX_IP] & 0xFFFF0000) | offset;
    }
}


void Fn_RET(MV *mv, InstrDecod *instr) {
    (void) instr;
    uint16_t seg = (mv->registros[IDX_SS] >> 16) & 0xFFFF;
    uint32_t val;
    uint32_t limite = mv->segmentos[seg].tam;
    limite = mv->registros[IDX_SS] | limite;

    if (mv->registros[IDX_SP] >= limite) {
        mv->err = ERR_STACKUDR;
    } else {
        val = getValorStack(mv);
        if (!(mv->err)) {
                mv->registros[IDX_IP] = val;
                mv->registros[IDX_SP] += 4;
        }
    }
}