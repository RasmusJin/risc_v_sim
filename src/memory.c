#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "memory.h"
#include "simulator.h"

uint8_t memory[MEMORY_SIZE] = {0};

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

uint32_t fetch_instruction() {
    return *((uint32_t *)(memory + PC));
}
