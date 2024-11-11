#include <stdio.h>
#include <stdint.h>
#include "simulator.h"
#include "memory.h"
#include "decoder.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <binary_file>\n", argv[0]);
        return 1;
    }

    init_simulator();
    load_instructions(argv[1]);

    printf("RISC-V Simulator Starting...\n");

    while (PC < MEMORY_SIZE && running) {
        uint32_t instruction = fetch_instruction();
        decode_and_execute(instruction);
        if (running && (instruction & 0x7F) != 0x63) { // Check if not a branch instruction
            PC += 4;
        }
        printf("Next PC: 0x%x\n", PC);
    }

    print_registers();
    write_output_binary("output.bin");
    return 0;
}
