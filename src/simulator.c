#include <stdio.h>
#include <stdint.h>
#include "simulator.h"

uint32_t registers[NUM_REGISTERS] = {0};
uint32_t PC = 0;
int running = 1;

void init_simulator() {
    for (int i = 0; i < NUM_REGISTERS; i++) {
        registers[i] = 0;
    }
    PC = 0;
    running = 1;
    registers[2] = 0x100000;
    registers[0] = 0;
    printf("Stack Pointer (sp) initialized to 0x100000\n");
}

void print_registers() {
    printf("\nRegister state:\n");
    for (int i = 0; i < NUM_REGISTERS; i++) {
        printf("x%d = %d\n", i, registers[i]);
    }
}

void write_output_binary(const char *filename) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        perror("Error opening output file");
        return;
    }

    for (int i = 0; i < NUM_REGISTERS; i++) {
        uint32_t value = registers[i];

        // Optional: Handle endianness (uncomment if needed)
        // value = htonl(value);

        size_t written = fwrite(&value, sizeof(uint32_t), 1, file);
        if (written != 1) {
            perror("Error writing to output file");
            fclose(file);
            return;
        }

        // Debug output
        printf("Writing x%d = 0x%x to file\n", i, value);
    }

    fclose(file);
    printf("Binary output written to %s\n", filename);
}
