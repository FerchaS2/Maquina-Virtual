#ifndef INSTRUCCIONES_H
#define INSTRUCCIONES_H

#include "mv.h"
#include "decoder.h"

#define MODE_DEC  0x01
#define MODE_CHAR  0x02
#define MODE_OCT   0x04
#define MODE_HEX   0x08
#define MODE_BIN   0x10

#define ERR_DIV 7
#define ERR_SWAP 8
#define ERR_NOTINM 10
#define ERR_STACKOVF 22
#define ERR_STACKUDR 23
#define ERR_IO 1
#define ERR_ABORT 24

#define LIM_STRREAD 1024

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
void Fn_LDH(MV *mv, InstrDecod *instr);
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
void Fn_PUSH(MV *mv, InstrDecod *instr);
void Fn_POP(MV *mv, InstrDecod *instr);
void Fn_CALL(MV *mv, InstrDecod *instr);

//SIN OPERANDOS
void Fn_STOP(MV *mv, InstrDecod *instr);
void Fn_RET(MV *mv, InstrDecod *instr);

//FUNCIONES DEL SYS
void SYS_WRITE(MV *mv);
void SYS_STRREAD(MV *mv);
void SYS_STRWRITE(MV *mv);
void SYS_CLEAR(MV *mv);
void SYS_BREAKPOINT(MV *mv);

// AUXILIARES
uint32_t getValorStack(MV *mv);
void setValorStack(MV *mv, uint32_t val);

void ini_VecFn(Fn_Instr *vec);

#endif