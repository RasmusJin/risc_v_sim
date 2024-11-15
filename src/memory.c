#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "memory.h"
#include "simulator.h"

uint8_t memory[MEMORY_SIZE] = {0}; // Initialize memory to zero

// Load instructions from a binary file into memory, one byte at a time
void load_instructions(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    size_t bytes_read = fread(memory, 1, MEMORY_SIZE, file);
    fclose(file);

    printf("Loaded %zu bytes into memory.\n", bytes_read);
}

// Fetch the next instruction from memory
uint32_t fetch_instruction() {
    if (PC >= MEMORY_SIZE) {
        printf("Error: PC out of bounds (0x%x). Halting simulation.\n", PC);
        running = 0;
        return 0;
    }
    uint32_t instruction = *((uint32_t *)(memory + PC)); // Read 4 bytes from memory
    printf("Fetched instruction 0x%x at PC: 0x%x\n", instruction, PC);
    return instruction;
}
