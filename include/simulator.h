
#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <stdint.h>

// Constants
#define NUM_REGISTERS 32
#define MEMORY_SIZE (1024 * 1024) // 1 MB of memory

// Global variables
extern uint32_t registers[NUM_REGISTERS]; // General-purpose registers
extern uint32_t PC;                       // Program counter

// Function declarations
void init_simulator();   // Initialize registers and PC
void print_registers();  // Print the state of all registers

#endif // SIMULATOR_H
