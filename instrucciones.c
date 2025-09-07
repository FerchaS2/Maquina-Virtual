#include "instrucciones.h"
#include "mv.h"

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
    vec[OPC_LHD]  = Fn_LHD;
    vec[OPC_RND]  = Fn_RND;
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
        res = 0;
        res = mv->memoria[dir_fisica] << 16;
        res |= mv->memoria[dir_fisica+1] << 8;
        res |= mv->memoria[dir_fisica+2];
        mv->registros[IDX_MBR] = res;
        break;
    }

    return res;
}

void Fn_ADD(MV *mv, InstrDecod *instr) {

}