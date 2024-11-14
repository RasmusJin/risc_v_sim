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
        printf("Current PC: 0x%x, Next Instruction: 0x%x\n", PC, instruction);

        decode_and_execute(instruction);
        // Check for JAL, JALR and ECALL to prevent incrementing PC
        if (running && (instruction & 0x7F) != 0x6F && (instruction & 0x7F) != 0x67 && (instruction & 0x7F) != 0x63) {
            PC += 4;
        }
        printf("Next PC: 0x%x\n", PC);
    }

    // Print the register state before the file write for debugging
    print_registers();
    //write the file
    write_output_binary("output.bin");
    return 0;
}