
#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <stdint.h>

// Constants
#define NUM_REGISTERS 32 // Number of registers in the RISC-V architecture
#define MEMORY_SIZE (1024 * 1024) // 1 MB of memory

extern int running;

// Global variables
extern uint32_t registers[NUM_REGISTERS]; // General-purpose registers
extern uint32_t PC;                       // Program counter

// Function declarations
void init_simulator();   // Initialize registers and PC
void print_registers();  // Print the state of all registers
void write_output_binary(const char *filename); // Write register contents to a binary file

#endif // SIMULATOR_H