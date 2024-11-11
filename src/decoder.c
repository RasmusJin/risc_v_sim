#include <stdio.h>
#include <stdint.h>
#include "../include/simulator.h"
#include "../include/memory.h"
#include "../include/decoder.h"

void decode_and_execute(uint32_t instruction) {
    uint32_t opcode = instruction & 0x7F;
    printf("PC: 0x%x, Instruction: 0x%x\n", PC, instruction);
    printf("Extracted opcode: 0x%x\n", opcode);

    

    switch (opcode) {
        case 0x13: { // I-Type Instructions (e.g., ADDI, SLTI, SLTIU, XORI, ORI, ANDI)
            uint32_t rd = (instruction >> 7) & 0x1F;
            uint32_t rs1 = (instruction >> 15) & 0x1F;
            int32_t imm = sign_extend((instruction >> 20), 12);
            uint32_t funct3 = (instruction >> 12) & 0x07;
            uint32_t funct7 = (instruction >> 25) & 0x7F;
            uint32_t shamt = (instruction >> 20) & 0x1F;

            switch (funct3) {
                case 0x0: // ADDI
                    printf("ADDI: rd = x%d, rs1 = x%d, imm = %d\n", rd, rs1, imm);
                    printf("Before ADDI: registers[%d] = %d, registers[%d] = %d\n", rd, registers[rd], rs1, registers[rs1]);
                    registers[rd] = registers[rs1] + imm;
                    printf("After ADDI: registers[%d] = %d\n", rd, registers[rd]);
                    break;
                case 0x1: // SLLI (Shift Left Logical Immediate)
                    registers[rd] = registers[rs1] << shamt;
                    printf("SLLI x%d, x%d, %d -> x%d = %d\n", rd, rs1, shamt, rd, registers[rd]);
                    break;
                case 0x2: // SLTI (Set Less Than Immediate, signed)
                    registers[rd] = (int32_t)registers[rs1] < imm ? 1 : 0;
                    printf("SLTI x%d, x%d, %d -> x%d = %d\n", rd, rs1, imm, rd, registers[rd]);
                    break;
                case 0x3: // SLTIU (Set Less Than Immediate Unsigned)
                    registers[rd] = (uint32_t)registers[rs1] < (uint32_t)imm ? 1 : 0;
                    printf("SLTIU x%d, x%d, %d -> x%d = %d\n", rd, rs1, imm, rd, registers[rd]);
                    break;
                case 0x4: // XORI
                    registers[rd] = registers[rs1] ^ imm;
                    printf("XORI x%d, x%d, %d -> x%d = %d\n", rd, rs1, imm, rd, registers[rd]);
                    break;
                case 0x5:
                    if (funct7 == 0x00) { // SRLI (Shift Right Logical Immediate)
                        registers[rd] = (uint32_t)registers[rs1] >> shamt;
                        printf("SRLI x%d, x%d, %d -> x%d = %d\n", rd, rs1, shamt, rd, registers[rd]);
                    } else if (funct7 == 0x20) { // SRAI (Shift Right Arithmetic Immediate)
                        registers[rd] = (int32_t)registers[rs1] >> shamt;
                        printf("SRAI x%d, x%d, %d -> x%d = %d\n", rd, rs1, shamt, rd, registers[rd]);
                    }
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
        case 0x33: { // R-Type Instructions (e.g., ADD, SUB, SLT, SLTU, XOR, OR, AND)
            uint32_t rd = (instruction >> 7) & 0x1F;
            uint32_t rs1 = (instruction >> 15) & 0x1F;
            uint32_t rs2 = (instruction >> 20) & 0x1F;
            uint32_t funct3 = (instruction >> 12) & 0x07;
            uint32_t funct7 = (instruction >> 25) & 0x7F;

            switch (funct3) {
                case 0x0: // ADD or SUB
                    if (funct7 == 0x00) { // ADD
                        registers[rd] = registers[rs1] + registers[rs2];
                        printf("ADD x%d, x%d, x%d -> x%d = %d\n", rd, rs1, rs2, rd, registers[rd]);
                    } else if (funct7 == 0x20) { // SUB
                        registers[rd] = registers[rs1] - registers[rs2];
                        printf("SUB x%d, x%d, x%d -> x%d = %d\n", rd, rs1, rs2, rd, registers[rd]);
                    }
                    break;
                case 0x1: // SLL (Shift Left Logical)
                    registers[rd] = registers[rs1] << (registers[rs2] & 0x1F);
                    printf("SLL x%d, x%d, x%d -> x%d = %d\n", rd, rs1, rs2, rd, registers[rd]);
                    break;
                case 0x2: // SLT (Set Less Than, signed)
                    registers[rd] = (int32_t)registers[rs1] < (int32_t)registers[rs2] ? 1 : 0;
                    printf("SLT x%d, x%d, x%d -> x%d = %d\n", rd, rs1, rs2, rd, registers[rd]);
                    break;
                case 0x3: // SLTU (Set Less Than Unsigned)
                    registers[rd] = (uint32_t)registers[rs1] < (uint32_t)registers[rs2] ? 1 : 0;
                    printf("SLTU x%d, x%d, x%d -> x%d = %d\n", rd, rs1, rs2, rd, registers[rd]);
                    break;
                case 0x4: // XOR
                    registers[rd] = registers[rs1] ^ registers[rs2];
                    printf("XOR x%d, x%d, x%d -> x%d = %d\n", rd, rs1, rs2, rd, registers[rd]);
                    break;
                case 0x5:
                    if (funct7 == 0x00) { // SRL (Shift Right Logical)
                        registers[rd] = (uint32_t)registers[rs1] >> (registers[rs2] & 0x1F);
                        printf("SRL x%d, x%d, x%d -> x%d = %d\n", rd, rs1, rs2, rd, registers[rd]);
                    } else if (funct7 == 0x20) { // SRA (Shift Right Arithmetic)
                        registers[rd] = (int32_t)registers[rs1] >> (registers[rs2] & 0x1F);
                        printf("SRA x%d, x%d, x%d -> x%d = %d\n", rd, rs1, rs2, rd, registers[rd]);
                    }
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
        case 0x63: { // B-Type Instructions (Branching)
            uint32_t funct3 = (instruction >> 12) & 0x07;
            uint32_t rs1 = (instruction >> 15) & 0x1F;
            uint32_t rs2 = (instruction >> 20) & 0x1F;

            // Calculate the 13-bit signed branch offset from the instruction fields
            int32_t imm = ((instruction >> 31) << 12)       // sign bit
                        | (((instruction >> 7) & 0x1) << 11)  // bit 11
                        | (((instruction >> 25) & 0x3F) << 5) // bits 10-5
                        | ((instruction >> 8) & 0xF) << 1;    // bits 4-1
            imm = sign_extend(imm, 13);

            switch (funct3) {
                case 0x0: // BEQ
                    if (registers[rs1] == registers[rs2]) {
                        PC += imm;
                        printf("BEQ x%d, x%d, offset %d -> PC = 0x%x\n", rs1, rs2, imm, PC);
                        return;
                    }
                    break;
                case 0x1: // BNE
                    if (registers[rs1] != registers[rs2]) {
                        PC += imm;
                        printf("BNE x%d, x%d, offset %d -> PC = 0x%x\n", rs1, rs2, imm, PC);
                        return;
                    }
                    break;
                case 0x4: // BLT
                    if ((int32_t)registers[rs1] < (int32_t)registers[rs2]) {
                        PC += imm;
                        printf("BLT x%d, x%d, offset %d -> PC = 0x%x\n", rs1, rs2, imm, PC);
                        return;
                    }
                    break;
                case 0x5: // BGE
                    if ((int32_t)registers[rs1] >= (int32_t)registers[rs2]) {
                        PC += imm;
                        printf("BGE x%d, x%d, offset %d -> PC = 0x%x\n", rs1, rs2, imm, PC);
                        return;
                    }
                    break;
                case 0x6: // BLTU
                    if ((uint32_t)registers[rs1] < (uint32_t)registers[rs2]) {
                        PC += imm;
                        printf("BLTU x%d, x%d, offset %d -> PC = 0x%x\n", rs1, rs2, imm, PC);
                        return;
                    }
                    break;
                case 0x7: // BGEU
                    if ((uint32_t)registers[rs1] >= (uint32_t)registers[rs2]) {
                        PC += imm;
                        printf("BGEU x%d, x%d, offset %d -> PC = 0x%x\n", rs1, rs2, imm, PC);
                        return;
                    }
                    break;
                default:
                    printf("Unknown B-type funct3: 0x%x\n", funct3);
                    break;
            }

            // If branch is not taken, increment PC by 4
            PC += 4;
            printf("Branch not taken -> PC incremented to 0x%x\n", PC);
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
    int32_t shift = 32 - bits;
    return (imm << shift) >> shift;
}


