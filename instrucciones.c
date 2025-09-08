#include "instrucciones.h"
#include "mv.h"
#include "decoder.h"
#include "opcodes.h"
#include "memory.h"
#include <string.h>

void ini_VecFn(Fn_Instr *vec) {
    for (int i = 0; i < 32; i++)
        vec[i] = NULL;
    
    //vec[OPC_SYS]  = Fn_SYS;
    //vec[OPC_JMP]  = Fn_JMP;
    //vec[OPC_JZ]   = Fn_JZ;
    //vec[OPC_JP]   = Fn_JP;
    //vec[OPC_JN]   = Fn_JN;
    //vec[OPC_JNZ]  = Fn_JNZ;
    //vec[OPC_JNP]  = Fn_JNP;
    //vec[OPC_JNN]  = Fn_JNN;
    //vec[OPC_NOT]  = Fn_NOT;

    vec[OPC_STOP] = Fn_STOP;

    vec[OPC_MOV]  = Fn_MOV;
    vec[OPC_ADD]  = Fn_ADD;
    //vec[OPC_SUB]  = Fn_SUB;
    //vec[OPC_MUL]  = Fn_MUL;
    //vec[OPC_DIV]  = Fn_DIV;
    //vec[OPC_CMP]  = Fn_CMP;
    //vec[OPC_SHL]  = Fn_SHL;
    //vec[OPC_SHR]  = Fn_SHR;
    //vec[OPC_SAR]  = Fn_SAR;
    //vec[OPC_AND]  = Fn_AND;
    //vec[OPC_OR]   = Fn_OR;
    //vec[OPC_XOR]  = Fn_XOR;
    //vec[OPC_SWAP] = Fn_SWAP;
    //vec[OPC_LDL]  = Fn_LDL;
    //vec[OPC_LHD]  = Fn_LHD;
    //vec[OPC_RND]  = Fn_RND;
}

uint32_t getValorPorInstr(MV *mv, uint32_t op) {
    uint8_t tipo = (op >> 24) & 0xFF; //saco el tipo de operando
    uint8_t indexReg;
    uint16_t offset, segm;
    uint32_t res, dir_fisica;
    int err;

    switch (tipo)
    {
    case 1: //registro
        indexReg = op & 0xFF;
        res = mv->registros[indexReg];
        break;
    
    case 2: //inmediato
        res = op & 0x00FFFFFF;
        break;

    case 3: //memoria
        indexReg = (op >> 16) & 0x1F; //me quedo con los 5 bits que representan el registro en op
        mv->registros[IDX_MAR] = mv->registros[indexReg];
        offset = (op & 0xFF) + (mv->registros[indexReg] & 0xFFFF);
        segm = (mv->registros[indexReg] >> 16) & 0xFFFF;
        traductor(mv, segm, offset, 3, &err, &mv->registros[IDX_LAR]);
        dir_fisica = mv->registros[IDX_LAR];
        res = 0;
        res = mv->memoria[dir_fisica] << 16;
        res |= mv->memoria[dir_fisica+1] << 8;
        res |= mv->memoria[dir_fisica+2];
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
    int err;

        switch (tipo)
    {
    case 1: //registro
        indexReg = op & 0xFF;
        mv->registros[indexReg] = resultado;
        break;

    case 3: //memoria
        indexReg = (op >> 16) & 0x1F; //me quedo con los 5 bits que representan el registro en op
        mv->registros[IDX_MAR] = mv->registros[indexReg];
        offset = (op & 0xFF) + (mv->registros[indexReg] & 0xFFFF);
        segm = (mv->registros[indexReg] >> 16) & 0xFFFF;
        traductor(mv, segm, offset, 3, &err, &mv->registros[IDX_LAR]);
        mv->registros[IDX_MBR] = resultado;
        dir_fisica = mv->registros[IDX_LAR];
        mv->memoria[dir_fisica] = (resultado >> 24) & 0xFF;
        mv->memoria[dir_fisica+1] = (resultado >> 16) & 0xFF;
        mv->memoria[dir_fisica+2] = (resultado >> 8) & 0xFF;
        mv->memoria[dir_fisica+3] = resultado & 0xFF;
        break;
    }
}

uint32_t sumaSigned(uint32_t val1, uint32_t val2) {
    int32_t auxv1, auxv2, res;
    //vienen como unsigned con los primeros 2 bytes en 0, por lo que hay que recuperar el signo en 32 bits
    auxv1 = val1; auxv2 = val2;
    auxv1 = (auxv1 << 8) >> 8;
    auxv2 = (auxv2 << 8) >> 8; 
    res = auxv1 + auxv2;

    return (uint32_t) res; //lo devuelvo unsigned porque la memoria no diferencia signos
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
        mv->registros[IDX_CC] |= (1u << 31); // set bit N
    else if (signedRes == 0)
        mv->registros[IDX_CC] |= (1u << 30); // set bit Z
}

void Fn_MOV(MV *mv, InstrDecod *instr) {
    setValorPorInstr(mv, instr->op1, getValorPorInstr(mv, instr->op2));
}

void Fn_STOP(MV *mv, InstrDecod *instr) {
    mv->registros[IDX_IP] = 0xFFFFFFFF;
}