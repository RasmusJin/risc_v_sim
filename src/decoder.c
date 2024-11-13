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
        case 0x03: { // Load Instructions (LB, LH, LW, LBU, LHU)
            uint32_t rd = (instruction >> 7) & 0x1F;
            uint32_t rs1 = (instruction >> 15) & 0x1F;
            if (rd == 0) {
                printf("Ignoring write to x0 (zero register)\n");
                return;
            }
            int32_t imm = sign_extend((instruction >> 20), 12);
            uint32_t address = registers[rs1] + imm;
            uint32_t funct3 = (instruction >> 12) & 0x07;

            // Check if using Stack Pointer (sp)
            if (rs1 == 2) {
                printf("Load using Stack Pointer: Loading from address 0x%x\n", address);
            }

            // Alignment check for word loads
            if ((funct3 == 0x2 || funct3 == 0x4 || funct3 == 0x5) && (address % 4 != 0)) {
                printf("Misaligned memory access: address 0x%x\n", address);
                running = 0;
                return;
            }

            switch (funct3) {
                case 0x0: { // LB (Load Byte, sign-extended)
                    int8_t value = *((int8_t *)(memory + address));
                    registers[rd] = (int32_t)value;
                    printf("LB x%d, %d(x%d) -> x%d = 0x%x\n", rd, imm, rs1, rd, registers[rd]);
                    break;
                }
                case 0x1: { // LH (Load Halfword, sign-extended)
                    int16_t value = *((int16_t *)(memory + address));
                    registers[rd] = (int32_t)value;
                    printf("LH x%d, %d(x%d) -> x%d = 0x%x\n", rd, imm, rs1, rd, registers[rd]);
                    break;
                }
                case 0x2: { // LW (Load Word)
                    uint32_t value = *((uint32_t *)(memory + address));
                    registers[rd] = value;
                    printf("LW x%d, %d(x%d) -> x%d = 0x%x\n", rd, imm, rs1, rd, registers[rd]);
                    break;
                }
                case 0x4: { // LBU (Load Byte Unsigned)
                    uint8_t value = *((uint8_t *)(memory + address));
                    registers[rd] = (uint32_t)value;
                    printf("LBU x%d, %d(x%d) -> x%d = 0x%x\n", rd, imm, rs1, rd, registers[rd]);
                    break;
                }
                case 0x5: { // LHU (Load Halfword Unsigned)
                    uint16_t value = *((uint16_t *)(memory + address));
                    registers[rd] = (uint32_t)value;
                    printf("LHU x%d, %d(x%d) -> x%d = 0x%x\n", rd, imm, rs1, rd, registers[rd]);
                    break;
                }
                default:
                    printf("Unknown load funct3: 0x%x\n", funct3);
                    running = 0;
                    return;
            }

            // Bounds check to prevent accessing invalid memory
            if (address >= MEMORY_SIZE || address < 0) {
                printf("Error: Load memory access out of bounds: address 0x%x\n", address);
                running = 0;
            }
            break;
        }

        case 0x13: { // I-Type Instructions (e.g., ADDI, SLTI, SLTIU, XORI, ORI, ANDI)
            uint32_t rd = (instruction >> 7) & 0x1F;
            if (rd == 0) {
                printf("Ignoring write to x0 (zero register)\n");
                return;
            }
            uint32_t rs1 = (instruction >> 15) & 0x1F;
            int32_t imm = sign_extend((instruction >> 20), 12);
            uint32_t funct3 = (instruction >> 12) & 0x07;
            uint32_t funct7 = (instruction >> 25) & 0x7F;
            uint32_t shamt = (instruction >> 20) & 0x1F;

            switch (funct3) {
                case 0x0: // ADDI
                    printf("ADDI: rd = x%d, rs1 = x%d, imm = %d\n", rd, rs1, imm);
                    printf("Before ADDI: registers[%d] = %d, registers[%d] = %d\n", rd, registers[rd], rs1, registers[rs1]);

                    // Perform the addition
                    registers[rd] = registers[rs1] + imm;

                    // Check if the destination is the stack pointer (sp)
                    if (rd == 2) {
                        printf("Stack Pointer Adjustment: sp = sp + %d\n", imm);
                        printf("After ADDI: registers[%d] (sp) = 0x%x\n", rd, registers[rd]);

                        // Check for stack alignment after adjustment
                        if (registers[2] % 16 != 0) {
                            printf("Error: Stack pointer misaligned: 0x%x\n", registers[2]);
                            running = 0; // Halt simulation if misaligned
                            return;
                        }
                    } else {
                        printf("After ADDI: registers[%d] = %d\n", rd, registers[rd]);
                    }
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
        case 0x23: { // S-Type (Store) Instructions
            uint32_t funct3 = (instruction >> 12) & 0x7;
            uint32_t rs1 = (instruction >> 15) & 0x1F;
            uint32_t rs2 = (instruction >> 20) & 0x1F;
            int32_t imm = ((instruction >> 25) << 5) | ((instruction >> 7) & 0x1F);
            imm = sign_extend(imm, 12);
            uint32_t address = registers[rs1] + imm;
            if (rs1 == 2) {
                printf("Using Stack Pointer (sp) for address calculation: sp = 0x%x, offset = %d, address = 0x%x\n",
                       registers[rs1], imm, address);
            }
            switch (funct3) {
                case 0x0: // SB (Store Byte)
                    printf("SB: Storing byte from x%d to memory[x%d + %d]\n", rs2, rs1, imm);
                    memory[registers[rs1] + imm] = registers[rs2] & 0xFF;
                    break;
                case 0x1: // SH (Store Halfword)
                    printf("SH: Storing halfword from x%d to memory[x%d + %d]\n", rs2, rs1, imm);
                    *((uint16_t *)(memory + registers[rs1] + imm)) = registers[rs2] & 0xFFFF;
                    break;
                case 0x2: { // SW (Store Word)
                    uint32_t address = registers[rs1] + imm;
                    
                    // Alignment check
                    if (address % 4 != 0) {
                        printf("Misaligned memory access: address 0x%x\n", address);
                        running = 0;
                        return;
                    }
                    
                    // Bounds check
                    if (address >= MEMORY_SIZE || address < 0) {
                        printf("Error: SW memory access out of bounds: address 0x%x\n", address);
                        running = 0;
                        return;
                    }

                    printf("SW: Storing word from x%d (0x%x) to memory address 0x%x\n", rs2, registers[rs2], address);
                    *((uint32_t *)(memory + address)) = registers[rs2];
                    break;
                }
                default:
                    printf("Unknown S-type funct3: 0x%x\n", funct3);
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
            if (rd == 0) {
                printf("Ignoring write to x0 (zero register)\n");
                return;
            }
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
                case 0x2: // BGT (Branch if Greater Than)
                    if ((int32_t)registers[rs1] > (int32_t)registers[rs2]) {
                        PC += imm;
                        printf("BGT x%d, x%d, offset %d -> Branch taken, New PC = 0x%x\n", rs1, rs2, imm, PC);
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
        case 0x67: { // JALR
            uint32_t rd = (instruction >> 7) & 0x1F;
            uint32_t rs1 = (instruction >> 15) & 0x1F;
            int32_t imm = sign_extend((instruction >> 20), 12);

            uint32_t target_address = (registers[rs1] + imm) & ~1; // Ensure LSB is cleared for alignment

            // Save the return address only if rd is not x0
            if (rd != 0) {
                registers[rd] = PC + 4;
            }

            PC = target_address;

            // Debug output
            printf("JALR: Jumping to 0x%x, rd (x%d) = 0x%x\n", PC, rd, registers[rd]);
            if (PC >= MEMORY_SIZE || PC < 0) {
                printf("Error: JALR set PC out of bounds (0x%x). Halting simulation.\n", PC);
                running = 0;
                return;
            }
            return;
        }

        case 0x6F: { // JAL
            uint32_t rd = (instruction >> 7) & 0x1F;
            int32_t imm = ((instruction >> 31) << 20)         // Bit 20 (sign bit)
                        | (((instruction >> 21) & 0x3FF) << 1) // Bits 10-1
                        | (((instruction >> 20) & 0x1) << 11)  // Bit 11
                        | (((instruction >> 12) & 0xFF) << 12); // Bits 19-12
            imm = sign_extend(imm, 21);
            if (rd != 0) {
                registers[rd] = PC + 4;
            }
            
            
            PC += imm;              // Jump to target address
            printf("JAL x%d, offset %d -> PC = 0x%x, x%d = 0x%x\n", rd, imm, PC, rd, registers[rd]);
            if (PC >= MEMORY_SIZE || PC < 0) {
                printf("Error: JAL set PC out of bounds (0x%x). Halting simulation.\n", PC);
                running = 0;
            }
            return;
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