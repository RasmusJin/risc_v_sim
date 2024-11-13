#include <stdio.h>
#include <stdint.h>
#include "../include/simulator.h"
#include "../include/memory.h"
#include "../include/decoder.h"

int stack_pointer_used = 0;

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
                stack_pointer_used = 1;
                printf("Load using Stack Pointer (x2): Loading from address 0x%x\n", address);
            }
            if (rd == 2) {
                stack_pointer_used = 1;
                printf("Load instruction overwrites stack pointer (x2) -> x2 = 0x%x\n", registers[rd]);
            }
            // Alignment check for word loads
            if ((funct3 == 0x2) && (address % 4 != 0)) {
                printf("Warning: Misaligned memory access for LW at address 0x%x\n", address);

                // Handle unaligned access by loading individual bytes and combining them
                uint32_t byte0 = memory[address];
                uint32_t byte1 = memory[address + 1];
                uint32_t byte2 = memory[address + 2];
                uint32_t byte3 = memory[address + 3];

                // Combine bytes in little-endian order
                uint32_t loaded_word = (byte0) | (byte1 << 8) | (byte2 << 16) | (byte3 << 24);

                // Store the loaded word into the destination register
                registers[rd] = loaded_word;
                printf("LW (unaligned): Loaded word 0x%x from memory address 0x%x\n", loaded_word, address);
            } else if (funct3 == 0x2) {
                // Aligned access
                registers[rd] = *((uint32_t *)(memory + address));
                printf("LW: Loaded word 0x%x from memory address 0x%x\n", registers[rd], address);
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
                    if (address % 4 != 0) {
                        printf("Warning: Misaligned memory access for LW at address 0x%x\n", address);

                        // Load individual bytes and combine them correctly in little-endian order
                        uint32_t byte0 = (uint32_t)memory[address];
                        uint32_t byte1 = (uint32_t)memory[address + 1];
                        uint32_t byte2 = (uint32_t)memory[address + 2];
                        uint32_t byte3 = (uint32_t)memory[address + 3];

                        // Combine bytes in little-endian format
                        uint32_t loaded_word = (byte0) | (byte1 << 8) | (byte2 << 16) | (byte3 << 24);

                        // Store the loaded word into the destination register
                        registers[rd] = loaded_word;
                        printf("LW (unaligned): Loaded word 0x%x from memory address 0x%x\n", loaded_word, address);
                    } else {
                        // Aligned access
                        registers[rd] = *((uint32_t *)(memory + address));
                        printf("LW: Loaded word 0x%x from memory address 0x%x\n", registers[rd], address);
                    }
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
            if (address >= MEMORY_SIZE) {
                printf("Error: Load memory access out of bounds: address 0x%x\n", address);
                running = 0;
            }
            break;
        }

        case 0x13: { // I-Type Instructions (ADDI, SLTI, SLTIU, XORI, ORI, ANDI)
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
                        stack_pointer_used = 1;
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

            if (rs1 == 2) { // Using Stack Pointer (sp)
                printf("Using Stack Pointer (sp) for address calculation: sp = 0x%x, offset = %d, address = 0x%x\n",
                    registers[rs1], imm, address);
                stack_pointer_used = 1;
            }

            // Bounds check
            if (address >= MEMORY_SIZE ) {
                printf("Error: Store memory access out of bounds: address 0x%x\n", address);
                running = 0;
                return;
            }

            switch (funct3) {
                case 0x0: { // SB (Store Byte)
                    uint8_t value = registers[rs2] & 0xFF;
                    memory[address] = value;
                    printf("SB: Storing byte 0x%x from x%d to memory address 0x%x\n", value, rs2, address);
                    break;
                }
                case 0x1: { // SH (Store Halfword)
                    // Alignment check for halfword (2 bytes)
                    if (address % 2 != 0) {
                        printf("Misaligned memory access for SH: address 0x%x\n", address);
                        running = 0;
                        return;
                    }
                    uint16_t value = registers[rs2] & 0xFFFF;
                    *((uint16_t *)(memory + address)) = value;
                    printf("SH: Storing halfword 0x%x from x%d to memory address 0x%x\n", value, rs2, address);
                    break;
                }
                case 0x2: { // SW (Store Word)
                    // Check if address is aligned to 4 bytes
                    if (address % 4 != 0) {
                        printf("Warning: Misaligned memory access for SW at address 0x%x\n", address);

                        // Handle unaligned access by storing the word in bytes
                        memory[address] = registers[rs2] & 0xFF;
                        memory[address + 1] = (registers[rs2] >> 8) & 0xFF;
                        memory[address + 2] = (registers[rs2] >> 16) & 0xFF;
                        memory[address + 3] = (registers[rs2] >> 24) & 0xFF;

                        printf("SW (unaligned): Storing word 0x%x from x%d to memory address 0x%x (split into bytes)\n",
                            registers[rs2], rs2, address);
                    } else {
                        // Aligned access
                        *((uint32_t *)(memory + address)) = registers[rs2];
                        printf("SW: Storing word 0x%x from x%d to memory address 0x%x\n", registers[rs2], rs2, address);
                    }
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
            // Check if the destination register is the stack pointer (x2)
            if (rd == 2) {
                stack_pointer_used = 1;
                printf("Stack pointer (sp) initialized by LUI: sp = 0x%x\n", registers[rd]);
            }
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
                    if (rd == 2) {
                        stack_pointer_used = 1;
                        printf("Stack pointer (x2) modified by R-Type instruction.\n");
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
        case 0x6F: { // JAL (Jump and Link)
            uint32_t rd = (instruction >> 7) & 0x1F;
            int32_t imm = ((instruction >> 31) << 20)         // Bit 20 (sign bit)
                        | (((instruction >> 21) & 0x3FF) << 1) // Bits 10-1
                        | (((instruction >> 20) & 0x1) << 11)  // Bit 11
                        | (((instruction >> 12) & 0xFF) << 12); // Bits 19-12
            imm = sign_extend(imm, 21);

            // Save the return address only if rd is not x0
            if (rd != 0) {
                registers[rd] = PC + 4;
            }

            // Function call: Allocate space on the stack (16 bytes) and save the return address (ra)
            if (rd == 1) { // If the destination register is `ra` (x1), this is a function call
                registers[2] -= 16; // Adjust stack pointer (sp)
                *((uint32_t *)(memory + registers[2])) = registers[1]; // Save return address (ra) on the stack
                stack_pointer_used = 1;
                printf("JAL (Function Call): Saved ra = 0x%x, Adjusted sp = 0x%x\n", registers[1], registers[2]);
            }

            // Jump to target address
            PC += imm;
            printf("JAL x%d, offset %d -> PC = 0x%x, x%d = 0x%x\n", rd, imm, PC, rd, registers[rd]);
            if (PC >= MEMORY_SIZE) {
                printf("Error: JAL set PC out of bounds (0x%x). Halting simulation.\n", PC);
                running = 0;
            }
            return;
        }

        case 0x67: { // JALR (Jump and Link Register)
            uint32_t rd = (instruction >> 7) & 0x1F;
            uint32_t rs1 = (instruction >> 15) & 0x1F;
            int32_t imm = sign_extend((instruction >> 20), 12);

            uint32_t target_address = (registers[rs1] + imm) & ~1; // Ensure LSB is cleared for alignment

            // Save the return address only if rd is not x0
            if (rd != 0) {
                registers[rd] = PC + 4;
            }

            // Function return: Restore return address (ra) from the stack and adjust the stack pointer (sp)
            if (rs1 == 1) { // If using `ra` (x1) for the jump, it's likely a function return
                registers[1] = *((uint32_t *)(memory + registers[2])); // Load return address (ra) from the stack
                registers[2] += 16; // Restore the stack pointer (deallocate stack frame)
                PC = registers[1]; // Jump to the return address (ra)
                stack_pointer_used = 1;
                printf("JALR (Return): Restoring ra = 0x%x, sp = 0x%x, Jumping to PC = 0x%x\n", registers[1], registers[2], PC);
                return;
            }

            // Normal JALR: Jump to target address
            PC = target_address;
            printf("JALR: Jumping to 0x%x, rd (x%d) = 0x%x\n", PC, rd, registers[rd]);
            if (PC >= MEMORY_SIZE) {
                printf("Error: JALR set PC out of bounds (0x%x). Halting simulation.\n", PC);
                running = 0;
                return;
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