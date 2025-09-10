#ifndef INSTRUCCIONES_H
#define INSTRUCCIONES_H

#include "mv.h"
#include "decoder.h"

#define ERR_DIV 7
#define ERR_SWAP 8

typedef void (*Fn_Instr)(MV *mv, InstrDecod *instr, int *err);

// DOS OPERANDOS
void Fn_MOV(MV *mv, InstrDecod *instr, int *err);
void Fn_ADD(MV *mv, InstrDecod *instr, int *err);
void Fn_SUB(MV *mv, InstrDecod *instr, int *err);
void Fn_MUL(MV *mv, InstrDecod *instr, int *err);
void Fn_DIV(MV *mv, InstrDecod *instr, int *err);
void Fn_CMP(MV *mv, InstrDecod *instr, int *err);
void Fn_SHL(MV *mv, InstrDecod *instr, int *err);
void Fn_SHR(MV *mv, InstrDecod *instr, int *err);
void Fn_SAR(MV *mv, InstrDecod *instr, int *err);
void Fn_AND(MV *mv, InstrDecod *instr, int *err);
void Fn_OR(MV *mv, InstrDecod *instr, int *err);
void Fn_XOR(MV *mv, InstrDecod *instr, int *err);
void Fn_SWAP(MV *mv, InstrDecod *instr, int *err);
void Fn_LDL(MV *mv, InstrDecod *instr, int *err);
void Fn_LDH(MV *mv, InstrDecod *instr, int *err);
void Fn_RND(MV *mv, InstrDecod *instr, int *err);

//UN OPERANDO
void Fn_SYS(MV *mv, InstrDecod *instr, int *err);
void Fn_JMP(MV *mv, InstrDecod *instr, int *err);
void Fn_JZ(MV *mv, InstrDecod *instr, int *err);
void Fn_JP(MV *mv, InstrDecod *instr, int *err);
void Fn_JN(MV *mv, InstrDecod *instr, int *err);
void Fn_JNZ(MV *mv, InstrDecod *instr, int *err);
void Fn_JNP(MV *mv, InstrDecod *instr, int *err);
void Fn_JNN(MV *mv, InstrDecod *instr, int *err);
void Fn_NOT(MV *mv, InstrDecod *instr, int *err);

//SIN OPERANDOS
void Fn_STOP(MV *mv, InstrDecod *instr, int *err);

void ini_VecFn(Fn_Instr *vec);

#endif