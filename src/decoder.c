#include <stdio.h>
#include <stdint.h>
#include "simulator.h"
#include "decoder.h"

void decode_and_execute(uint32_t instruction) {
    uint32_t opcode = instruction & 0x7F;

    switch (opcode) {
        case 0x13: { // I-Type (e.g., ADDI)
            uint32_t rd = (instruction >> 7) & 0x1F;
            uint32_t rs1 = (instruction >> 15) & 0x1F;
            int32_t imm = sign_extend((instruction >> 20), 12);
            registers[rd] = registers[rs1] + imm;
            printf("ADDI x%d, x%d, %d -> x%d = %d\n", rd, rs1, imm, rd, registers[rd]);
            break;
}
        case 0x37: { // LUI (Load Upper Immediate)
            uint32_t rd = (instruction >> 7) & 0x1F;
            int32_t imm = instruction & 0xFFFFF000;
            registers[rd] = imm;
            printf("LUI x%d, 0x%x -> x%d = 0x%x\n", rd, imm, rd, registers[rd]);
            break;
        }
        case 0x33: { // R-Type (e.g., ADD)
            uint32_t rd = (instruction >> 7) & 0x1F;
            uint32_t funct3 = (instruction >> 12) & 0x07;
            uint32_t rs1 = (instruction >> 15) & 0x1F;
            uint32_t rs2 = (instruction >> 20) & 0x1F;
            uint32_t funct7 = (instruction >> 25) & 0x7F;

            if (funct3 == 0x0 && funct7 == 0x00) { // ADD
                registers[rd] = registers[rs1] + registers[rs2];
                printf("ADD x%d, x%d, x%d -> x%d = %d\n", rd, rs1, rs2, rd, registers[rd]);
            }
            break;
        }
        case 0x73: { // ECALL
            printf("ECALL encountered. Exiting simulation.\n");
            running = 0;
            return;
        }
        default:
            printf("Unknown opcode: 0x%x\n", opcode);
            break;
    }
}

int32_t sign_extend(int32_t imm, int bits) {
    int32_t mask = (1 << (bits - 1));
    return (imm ^ mask) - mask;
}

