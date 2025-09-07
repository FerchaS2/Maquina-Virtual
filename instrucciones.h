#ifndef INSTRUCCIONES_H
#define INSTRUCCIONES_H

#include "mv.h"
#include "decoder.h"
#define CANT_REG

typedef void (*Fn_Instr)(MV *mv, InstrDecod *instr);

// DOS OPERANDOS
void Fn_MOV(MV *mv, InstrDecod *instr);
void Fn_ADD(MV *mv, InstrDecod *instr);
void Fn_SUB(MV *mv, InstrDecod *instr);
void Fn_MUL(MV *mv, InstrDecod *instr);
void Fn_DIV(MV *mv, InstrDecod *instr);
void Fn_CMP(MV *mv, InstrDecod *instr);
void Fn_SHL(MV *mv, InstrDecod *instr);
void Fn_SHR(MV *mv, InstrDecod *instr);
void Fn_SAR(MV *mv, InstrDecod *instr);
void Fn_AND(MV *mv, InstrDecod *instr);
void Fn_OR(MV *mv, InstrDecod *instr);
void Fn_XOR(MV *mv, InstrDecod *instr);
void Fn_SWAP(MV *mv, InstrDecod *instr);
void Fn_LDL(MV *mv, InstrDecod *instr);
void Fn_LHD(MV *mv, InstrDecod *instr);
void Fn_RND(MV *mv, InstrDecod *instr);

//UN OPERANDO
void Fn_SYS(MV *mv, InstrDecod *instr);
void Fn_JMP(MV *mv, InstrDecod *instr);
void Fn_JZ(MV *mv, InstrDecod *instr);
void Fn_JP(MV *mv, InstrDecod *instr);
void Fn_JN(MV *mv, InstrDecod *instr);
void Fn_JNZ(MV *mv, InstrDecod *instr);
void Fn_JNP(MV *mv, InstrDecod *instr);
void Fn_JNN(MV *mv, InstrDecod *instr);
void Fn_NOT(MV *mv, InstrDecod *instr);

//SIN OPERANDOS
void Fn_STOP(MV *mv, InstrDecod *instr);

void ini_VecFn(Fn_Instr *vec);

#endif