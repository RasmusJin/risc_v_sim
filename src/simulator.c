#include <stdio.h>
#include <stdint.h>
#include "simulator.h"

uint32_t registers[NUM_REGISTERS] = {0};
uint32_t PC = 0;

void init_simulator() {
    for (int i = 0; i < NUM_REGISTERS; i++) {
        registers[i] = 0;
    }
    PC = 0;
}

void print_registers() {
    printf("\nRegister state:\n");
    for (int i = 0; i < NUM_REGISTERS; i++) {
        printf("x%d = %d\n", i, registers[i]);
    }
}
