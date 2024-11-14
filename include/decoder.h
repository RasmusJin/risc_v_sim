#ifndef DECODER_H
#define DECODER_H

#include <stdint.h>

extern int stack_pointer_used;

// Function declaration
void decode_and_execute(uint32_t instruction); // Decode and execute a single instruction
int32_t sign_extend(int32_t imm, int bits); // Sign-extend an immediate value

#endif // DECODER_H