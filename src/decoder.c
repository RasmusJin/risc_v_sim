#include <stdio.h>
#include <stdint.h>
#include "simulator.h"
#include "decoder.h"

void decode_and_execute(uint32_t instruction) {
    uint32_t opcode = instruction & 0x7F;

    switch (opcode) {
        case 0x13: { // I-Type Instructions (e.g., ADDI, XORI, ORI, ANDI)
            uint32_t rd = (instruction >> 7) & 0x1F;
            uint32_t rs1 = (instruction >> 15) & 0x1F;
            int32_t imm = sign_extend((instruction >> 20), 12);
            uint32_t funct3 = (instruction >> 12) & 0x07;

            switch (funct3) {
                case 0x0: // ADDI
                    registers[rd] = registers[rs1] + imm;
                    printf("ADDI x%d, x%d, %d -> x%d = %d\n", rd, rs1, imm, rd, registers[rd]);
                    break;
                case 0x4: // XORI
                    registers[rd] = registers[rs1] ^ imm;
                    printf("XORI x%d, x%d, %d -> x%d = %d\n", rd, rs1, imm, rd, registers[rd]);
                    break;
                case 0x6: // ORI
                    registers[rd] = registers[rs1] | imm;
                    printf("ORI x%d, x%d, %d -> x%d = %d\n", rd, rs1, imm, rd, registers[rd]);
                    break;
                case 0x7: // ANDI
                    registers[rd] = registers[rs1] & imm;
                    printf("ANDI x%d, x%d, %d -> x%d = %d\n", rd, rs1, imm, rd, registers[rd]);
                    break;
                default:
                    printf("Unknown I-type funct3: 0x%x\n", funct3);
                    break;
            }
            break;
        }
        case 0x37: { // LUI (Load Upper Immediate)
            uint32_t rd = (instruction >> 7) & 0x1F;
            int32_t imm = instruction & 0xFFFFF000;
            registers[rd] = imm;
            printf("LUI x%d, 0x%x -> x%d = 0x%x\n", rd, imm, rd, registers[rd]);
            break;
        }
        case 0x33: { // R-Type Instructions (e.g., ADD, XOR, OR, AND)
            uint32_t rd = (instruction >> 7) & 0x1F;
            uint32_t rs1 = (instruction >> 15) & 0x1F;
            uint32_t rs2 = (instruction >> 20) & 0x1F;
            uint32_t funct3 = (instruction >> 12) & 0x07;
            uint32_t funct7 = (instruction >> 25) & 0x7F;

            switch (funct3) {
                case 0x0: // ADD
                    if (funct7 == 0x00) {
                        registers[rd] = registers[rs1] + registers[rs2];
                        printf("ADD x%d, x%d, x%d -> x%d = %d\n", rd, rs1, rs2, rd, registers[rd]);
                    }
                    break;
                case 0x4: // XOR
                    registers[rd] = registers[rs1] ^ registers[rs2];
                    printf("XOR x%d, x%d, x%d -> x%d = %d\n", rd, rs1, rs2, rd, registers[rd]);
                    break;
                case 0x6: // OR
                    registers[rd] = registers[rs1] | registers[rs2];
                    printf("OR x%d, x%d, x%d -> x%d = %d\n", rd, rs1, rs2, rd, registers[rd]);
                    break;
                case 0x7: // AND
                    registers[rd] = registers[rs1] & registers[rs2];
                    printf("AND x%d, x%d, x%d -> x%d = %d\n", rd, rs1, rs2, rd, registers[rd]);
                    break;
                default:
                    printf("Unknown R-type funct3: 0x%x\n", funct3);
                    break;
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

