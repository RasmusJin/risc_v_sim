#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
// Global memory array
extern uint8_t memory[];

// Function declarations
void load_instructions(const char *filename); // Load instructions from a binary file
uint32_t fetch_instruction();                // Fetch the next instruction from memory

#endif // MEMORY_H
