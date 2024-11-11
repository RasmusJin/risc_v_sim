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
        fwrite(&registers[i], sizeof(uint32_t), 1, file);
    }

    fclose(file);
    printf("Binary output written to %s\n", filename);
}
