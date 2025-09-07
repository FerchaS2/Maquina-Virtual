#ifndef OPCODES_H
#define OPCODES_H

// Instrucciones sin operandos
#define OPC_STOP 0x0F

// Instrucciones con un operando
#define OPC_SYS  0x00
#define OPC_JMP  0x01
#define OPC_JZ   0x02
#define OPC_JP   0x03
#define OPC_JN   0x04
#define OPC_JNZ  0x05
#define OPC_JNP  0x06
#define OPC_JNN  0x07
#define OPC_NOT  0x08

// Instrucciones con dos operandos
#define OPC_MOV  0x10
#define OPC_ADD  0x11
#define OPC_SUB  0x12
#define OPC_MUL  0x13
#define OPC_DIV  0x14
#define OPC_CMP  0x15
#define OPC_SHL  0x16
#define OPC_SHR  0x17
#define OPC_SAR  0x18
#define OPC_AND  0x19
#define OPC_OR   0x1A
#define OPC_XOR  0x1B
#define OPC_SWAP 0x1C
#define OPC_LDL  0x1D
#define OPC_LHD  0x1E
#define OPC_RND  0x1F

#endif