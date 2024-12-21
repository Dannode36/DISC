#pragma once
#include <iostream>

typedef uint8_t Byte;
typedef uint16_t Word;
typedef uint32_t DWord;

typedef int64_t i64;

enum Opcode
{
    // Opcodes must not excede 0x7F (01111111) due to the byteMode bit!

    //Special
    OP_NOOP = 0x00,   //No Op
    OP_RESET = 0x7E,   //Reset the CPU (clears registers and memory, resets flags)
    OP_HALT = 0x7F,   //Stops the CPU execution of instuctions

    //Arithmetic
    OP_ADD = 0x01,   //Add two registers, store in first
    OP_ADDC,                //Add word constant into register
    OP_ADDA,                //Add register and word at memory address, store in register

    OP_SUB,                 //Subtract two registers, store in first
    OP_SUBC,                //Subtract constant value from a register, store in register
    OP_SUBA,                //Subtract the value in memory from a register, store in register

    OP_MUL,                 //Multiply two registers, store in first
    OP_MULC,                //Multiply constant value from a register, store in register
    OP_MULA,                //Multiply the value in memory from a register, store in register

    OP_DIV,                 //Divide two registers, store in first
    OP_DIVC,                //Divide constant value from a register, store in register
    OP_DIVA,                //Divide the value in memory from a register, store in register

    OP_CMP = 0x0E,   //Subtract two registers and update status flags, discard result
    OP_CMPA = 0x0F,   //Subtract a value in memory from a register and update status flags, discard result

    //Increment
    OP_INC = 0x10,   //Increment a value in a register
    OP_INCM,                //Increment a value in memory
    OP_DEC,                 //Decrement a value in a register
    OP_DECM,                //Decrement a value in memory

    //Bitwise
    OP_UXT = 0x20,   //Zero extend a byte (truncate 16 bit value to 8 bits)
    //TODO

    //Data moving
    OP_LDR = 0x30,          //Load value from register into register
    OP_LDC,                 //Load value constant into register
    OP_LDM,                 //Load value from memory into register

    OP_STRM,                //Store register into memory
    OP_STMM,                //Store memory into memory (copies memory)
    OP_STCM,                //Store constant into memory

    //Are these needed?
    //[[ TODO
    OP_SWPM,                //Swap memory values
    OP_SWPR,                //Swap registers
    OP_SWPRM,               //Swap register and memory
    //TODO ]]

    //Control2
    OP_JSR = 0x40,          //Increment SP by 2, push the current PC to the stack, and jump to a subroutine
    OP_RTN,                 //Pop the previous PC off the stack and jump to it, decrtant value
    OP_JMP,                 //Set the program counter (PC) and contion execution

    OP_JRZ,                 //Jump if register is equal to 0
    OP_JRE,                 //Jump if register is equal to a constant value
    OP_JRN,                 //Jump if register is not equal to a constant value
    OP_JRG,                 //Jump if register is greater than a constant value
    OP_JRL,                 //Jump if register is less than a constant value
    OP_JRLE,                //Jump if register is less than or equal to a constant value
    OP_JRGE,                //Jump if register is greater than or equal to a consemory

    OP_JRZM,         //Jump if value in memory is equal to 0
    OP_JREM,                //Jump if register is equal to a value in memory
    OP_JRNM,                //Jump if register is not equal to a value in mement the SP by 2
    OP_JRLM,                //Jump if register is less than a value in memory
    OP_JRLEM,               //Jump if register is less than or equal to a value in memory
    OP_JRGM,                //Jump if register is greater than a value in memory
    OP_JRGEM,               //Jump if register is greater than or equal to a value in memory

    //Stack
    OP_PUSH = 0x60,   //Push register onto stack, decrement SP by (opsize + 1)
    OP_PUSHM,               //Push value in memory onto stack, decrement SP by (opsize + 1)
    OP_PUSHC,               //Push constant onto stack, decrement SP by (opsize + 1)

    OP_POP,                 //Pop value from stack into register, increment SP by (opsize + 1)
    OP_POPM,                //Pop value from stack into register, increment SP by (opsize + 1)

    OP_PUSHS,               //Push status onto stack, decrement SP by (opsize + 1)
    OP_POPS,                //Pop stack into status, increment SP by (opsize + 1)

    OP_SEI = 0x70,   //Set the global interrupt enable flag
    OP_CLI,                 //Clear the global interrupt enable flag

};

enum Interrupt
{
    I_0 = 1 << 0,
    I_1 = 1 << 1,
    I_2 = 1 << 2,
    I_3 = 1 << 3,
    I_4 = 1 << 4,
    I_5 = 1 << 5,
    I_6 = 1 << 6,
    I_NM = 1 << 7
};

enum Opsize {
    Size_Word,
    Size_Byte,
};

struct Memory
{
    /*
        +-----------------+ 0xFFFF
        | SetInterrupt Table |   ->   Stores 8 interrupt handler addresses
        +-----------------+ 0xFFF0
        |    Stack (v)    |
        +-----------------+
        |                 |
        +                 +
        |                 |
        +                 +
        |      Heap       |
        +                 +
        |                 |
        +                 +
        |                 |
        +-----------------+
        |   Program (^)   |
        +-----------------+ 0x0000
    */

    static constexpr Word MEM_SIZE = 0xFFFF;
    static constexpr Word INTERRUPT_TABLE = 0xFFF0;

    Byte* Data = new Byte[MEM_SIZE];
    //Byte Data[MEM_SIZE];

    void Clear() {
        memset(Data, 0, MEM_SIZE);
    }

    Byte operator[](Word address) const {
        return Data[address];
    }

    Byte& operator[](Word address) {
        return Data[address];
    }
};

union Registers //Not including special registers
{
    struct {
        Word R0, R1, R2, R3, R4, R5; //General purpose registers
        Word PC; //Program counter
        Word SP; //Stack pointer

        //Status flags
        Word N : 1; //Negative
        Word O : 1; //Overflow
        Word B : 1; //Break
        Word D : 1; //Decimal
        Word I : 1; //Global interrupt enable
        Word Z : 1; //Zero
        Word C : 1; //Carry
        Word _ : 1; //Unused

        //Interrupt flags
        Byte I0 : 1; //Interrupts 1-6
        Byte I1 : 1;
        Byte I2 : 1;
        Byte I3 : 1;
        Byte I4 : 1;
        Byte I5 : 1;
        Byte I6 : 1;
        Byte IH : 1; //High priority interrupt
    };

    struct
    {
        Word aligned[8];
        Byte status;
        Byte interruptFlags;
    };

    Word operator[](Byte reg) const {
        return aligned[reg];
    }
    Word& operator[](Byte reg) {
        return aligned[reg];
    }
};

struct CPU {
    //Registers
    Registers registers;
    bool halted = false;

    void SetInterrupt(Interrupt i) {
        registers.interruptFlags &= i;
    }

    void Reset(Memory& mem) {
        mem.Clear();

        registers.PC = 0;
        registers.SP = 0xFFFF; //Stack grows backwards from end
        memset(registers.aligned, 0, 6);
    }

    Byte FetchByte(i64& cycles, Memory& mem) {
        cycles--;
        return mem[registers.PC++];
    }
    Byte ReadByte(i64& cycles, Memory& mem, Word address) const {
        cycles--;
        return mem[address];
    }
    Byte& ReadByte(i64& cycles, Memory& mem, Word address) {
        cycles--;
        return mem[address];
    }
    void WriteByte(i64& cycles, Memory& mem, Word address, Byte value) {
        mem[address] = value;
        cycles--;
    }
    void StackPushByte(i64& cycles, Memory& mem, Byte value) {
        registers.SP--;
        WriteByte(cycles, mem, registers.SP, value);
    }
    Byte StackPopByte(i64& cycles, Memory& mem) {
        Word value = ReadByte(cycles, mem, registers.SP);
        registers.SP++;
        return value;
    }

    Word FetchWord(i64& cycles, Memory& mem) {
        Word word = mem[registers.PC++];
        word |= (mem[registers.PC++] << 8); //Little endian system

        cycles -= 2;
        return word;
    }
    Word ReadWord(i64& cycles, Memory& mem, Word address) const {
        Word word = mem[address];
        word |= (mem[address + 1] << 8); //Little endian system

        cycles -= 2;
        return word;
    }
    void WriteWord(i64& cycles, Memory& mem, Word address, Word value) {
        mem[address] = value & 0xFF; //Get the lowest 8 bits
        mem[address + 1] = value >> 8; //Get ths highest 8 bits
        cycles -= 2;
    }
    void StackPushWord(i64& cycles, Memory& mem, Word value) {
        registers.SP -= 2;
        WriteWord(cycles, mem, registers.SP, value);
    }
    Word StackPopWord(i64& cycles, Memory& mem) {
        Word value = ReadWord(cycles, mem, registers.SP);
        registers.SP += 2;
        return value;
    }

    void ExecuteInterrupt(i64& cycles, Memory& mem, Interrupt i) {
        StackPushByte(cycles, mem, registers.status);
        StackPushWord(cycles, mem, registers.PC);

        registers.PC = ReadWord(cycles, mem, Memory::INTERRUPT_TABLE + (i * 2));
        registers.I = 0; //Disable low priority interrupts from interrupting this routine
        registers.interruptFlags &= ~(1 << i); //Clear the flag for this interrupt
    }

    void Execute(i64 cycles, Memory& mem) {
        while (cycles > 0 && !halted)
        {
            //Is high priority interrupt flag set?
            if (registers.interruptFlags & I_NM) {
                ExecuteInterrupt(cycles, mem, I_NM);
            }
            else if (registers.I && registers.interruptFlags > 0) {
                int lowestSetBit = log2(registers.interruptFlags & -registers.interruptFlags) + 1;
                ExecuteInterrupt(cycles, mem, (Interrupt)lowestSetBit);
            }

            Byte instByte = FetchByte(cycles, mem);
            Opcode instruction = (Opcode)(instByte & 0x7F);
            bool byteMode = (instByte >> 7) == 1; //0 -> 16bit, 1 -> 8bit)
            
            switch (instruction)
            {
            case OP_NOOP: break;
            case OP_RESET: {
                Reset(mem);
                std::cout << "INFO: RESET instruction executed\n";
            } break;
            case OP_HALT: {
                halted = true;
                std::cout << "INFO: HALT instruction executed. The CPU will now stop\n";
            } break;
            case OP_INC: {
                Byte reg = FetchByte(cycles, mem);
                registers[reg]++;
            } break;
            case OP_INCM: {
                Word address = FetchWord(cycles, mem);
                Word value = byteMode ? ReadByte(cycles, mem, address) : ReadWord(cycles, mem, address) + 1;
                byteMode ? WriteByte(cycles, mem, address, value & 0xFF) : WriteWord(cycles, mem, address, value);
            } break;
            case OP_DEC: {
                Byte reg = FetchByte(cycles, mem);
                registers[reg]--;
            } break;
            case OP_DECM: {
                Word address = FetchWord(cycles, mem);
                Word value = byteMode ? ReadByte(cycles, mem, address) : ReadWord(cycles, mem, address) - 1;
                byteMode ? WriteByte(cycles, mem, address, value & 0xFF) : WriteWord(cycles, mem, address, value);

            } break;
            case OP_ADD: {
                Byte reg1 = FetchByte(cycles, mem);
                Byte reg2 = FetchByte(cycles, mem);

                registers[reg1] = registers[reg1] + registers[reg2];
            } break;
            case OP_ADDC: {
                Byte reg = FetchByte(cycles, mem);
                Word value = byteMode ? FetchByte(cycles, mem) : FetchWord(cycles, mem);

                registers[reg] = registers[reg] + value;
            } break;
            case OP_ADDA: {
                Byte reg = FetchByte(cycles, mem);
                Word address = FetchWord(cycles, mem);
                Word memValue = byteMode ? ReadByte(cycles, mem, address) : ReadWord(cycles, mem, address);

                registers[reg] = registers[reg] + memValue;
            } break;
            case OP_SUB: {
                Byte reg1 = FetchByte(cycles, mem);
                Byte reg2 = FetchByte(cycles, mem);

                registers[reg1] = registers[reg1] - registers[reg2];
            } break;
            case OP_SUBC: {
                Byte reg = FetchByte(cycles, mem);
                Word value = byteMode ? FetchByte(cycles, mem) : FetchWord(cycles, mem);

                registers[reg] = registers[reg] - value;
            } break;
            case OP_SUBA: {
                Byte reg = FetchByte(cycles, mem);
                Word address = FetchWord(cycles, mem);
                Word memValue = byteMode ? ReadByte(cycles, mem, address) : ReadWord(cycles, mem, address);

                registers[reg] = registers[reg] - memValue;
            } break;
            case OP_MUL: {
                Byte reg1 = FetchByte(cycles, mem);
                Byte reg2 = FetchByte(cycles, mem);

                registers[reg1] = registers[reg1] * registers[reg2];
            } break;
            case OP_MULC: {
                Byte reg = FetchByte(cycles, mem);
                Word value = byteMode ? FetchByte(cycles, mem) : FetchWord(cycles, mem);

                registers[reg] = registers[reg] * value;
            } break;
            case OP_MULA: {
                Byte reg = FetchByte(cycles, mem);
                Word address = FetchWord(cycles, mem);
                Word memValue = byteMode ? ReadByte(cycles, mem, address) : ReadWord(cycles, mem, address);

                registers[reg] = registers[reg] * memValue;
            } break;
            case OP_DIV: {
                Byte reg1 = FetchByte(cycles, mem);
                Byte reg2 = FetchByte(cycles, mem);

                registers[reg1] = registers[reg1] / registers[reg2];
            } break;
            case OP_DIVC: {
                Byte reg = FetchByte(cycles, mem);
                Word value = byteMode ? FetchByte(cycles, mem) : FetchWord(cycles, mem);

                registers[reg] = registers[reg] / value;
            } break;
            case OP_DIVA: {
                Byte reg = FetchByte(cycles, mem);
                Word address = FetchWord(cycles, mem);
                Word memValue = byteMode ? ReadByte(cycles, mem, address) : ReadWord(cycles, mem, address);

                registers[reg] = registers[reg] / memValue;
            } break;
            case OP_UXT: {
                Byte reg = FetchByte(cycles, mem);
                registers[reg] &= 0xFF;
            }
            case OP_LDR: {
                Byte reg1 = FetchByte(cycles, mem);
                Byte reg2 = FetchByte(cycles, mem);
                registers[reg1] = registers[reg2];
            } break;
            case OP_LDC: {
                Byte reg = FetchByte(cycles, mem);
                registers[reg] = byteMode ? FetchByte(cycles, mem) : FetchWord(cycles, mem);
            } break;
            case OP_LDM: {
                Byte reg = FetchByte(cycles, mem);
                Word address = FetchWord(cycles, mem);
                registers[reg] = byteMode ? ReadByte(cycles, mem, address) : ReadWord(cycles, mem, address);
            } break;
            case OP_STRM: {
                Byte reg = FetchByte(cycles, mem);
                Word address = FetchWord(cycles, mem);

                byteMode ? WriteByte(cycles, mem, address, registers[reg]) : WriteWord(cycles, mem, address, registers[reg]);
            } break;
            case OP_STCM: {
                Word value = byteMode ? FetchByte(cycles, mem) : FetchWord(cycles, mem);
                Word address = FetchWord(cycles, mem);

                byteMode ? WriteByte(cycles, mem, address, value) : WriteWord(cycles, mem, address, value);
            } break;
            case OP_JMP: {
                registers.PC = FetchWord(cycles, mem);
            } break;
            case OP_JRZ: {
                if (registers[FetchByte(cycles, mem)] == 0) {
                    registers.PC = FetchWord(cycles, mem);
                }
                else {
                    registers.PC += 2; //Avoid wasting 2 cycles with FetchWord()
                }
            } break;
            case OP_JRE: {
                Byte reg = FetchByte(cycles, mem);
                Word value = byteMode ? FetchByte(cycles, mem) : FetchWord(cycles, mem);
                Word address = FetchWord(cycles, mem);

                if (registers[reg] == value) {
                    registers.PC = address;
                }
            } break;
            case OP_JRN: {
                Byte reg = FetchByte(cycles, mem);
                Word value = byteMode ? FetchByte(cycles, mem) : FetchWord(cycles, mem);
                Word address = FetchWord(cycles, mem);

                if (registers[reg] != value) {
                    registers.PC = address;
                }
            } break;
            case OP_JRG: {
                Byte reg = FetchByte(cycles, mem);
                Word value = byteMode ? FetchByte(cycles, mem) : FetchWord(cycles, mem);
                Word address = FetchWord(cycles, mem);

                if (registers[reg] > value) {
                    registers.PC = address;
                }
            } break;
            case OP_JRGE: {
                Byte reg = FetchByte(cycles, mem);
                Word value = byteMode ? FetchByte(cycles, mem) : FetchWord(cycles, mem);
                Word address = FetchWord(cycles, mem);

                if (registers[reg] >= value) {
                    registers.PC = address;
                }
            } break;
            case OP_JRL: {
                Byte reg = FetchByte(cycles, mem);
                Word value = byteMode ? FetchByte(cycles, mem) : FetchWord(cycles, mem);
                Word address = FetchWord(cycles, mem);

                if (registers[reg] < value) {
                    registers.PC = address;
                }
            } break;
            case OP_JRLE: {
                Byte reg = FetchByte(cycles, mem);
                Word value = byteMode ? FetchByte(cycles, mem) : FetchWord(cycles, mem);
                Word address = FetchWord(cycles, mem);

                if (registers[reg] <= value) {
                    registers.PC = address;
                }
            } break;
            case OP_JREM: {
                Byte reg = FetchByte(cycles, mem);
                Word memAddress = FetchWord(cycles, mem);
                Word memValue = byteMode ? ReadByte(cycles, mem, memAddress) : ReadWord(cycles, mem, memAddress);
                Word jumpAddress = FetchWord(cycles, mem);

                if (registers[reg] == memValue) {
                    registers.PC = jumpAddress;
                }
            } break;
            case OP_JRNM: {
                Byte reg = FetchByte(cycles, mem);
                Word memAddress = FetchWord(cycles, mem);
                Word memValue = byteMode ? ReadByte(cycles, mem, memAddress) : ReadWord(cycles, mem, memAddress);
                Word jumpAddress = FetchWord(cycles, mem);

                if (registers[reg] != memValue) {
                    registers.PC = jumpAddress;
                }
            } break;
            case OP_JRGM: {
                Byte reg = FetchByte(cycles, mem);
                Word memAddress = FetchWord(cycles, mem);
                Word memValue = byteMode ? ReadByte(cycles, mem, memAddress) : ReadWord(cycles, mem, memAddress);
                Word jumpAddress = FetchWord(cycles, mem);

                if (registers[reg] > memValue) {
                    registers.PC = jumpAddress;
                }
            } break;
            case OP_JRGEM: {
                Byte reg = FetchByte(cycles, mem);
                Word memAddress = FetchWord(cycles, mem);
                Word memValue = byteMode ? ReadByte(cycles, mem, memAddress) : ReadWord(cycles, mem, memAddress);
                Word jumpAddress = FetchWord(cycles, mem);

                if (registers[reg] >= memValue) {
                    registers.PC = jumpAddress;
                }
            } break;
            case OP_JRLM: {
                Byte reg = FetchByte(cycles, mem);
                Word memAddress = FetchWord(cycles, mem);
                Word memValue = byteMode ? ReadByte(cycles, mem, memAddress) : ReadWord(cycles, mem, memAddress);
                Word jumpAddress = FetchWord(cycles, mem);

                if (registers[reg] < memValue) {
                    registers.PC = jumpAddress;
                }
            } break;
            case OP_JRLEM: {
                Byte reg = FetchByte(cycles, mem);
                Word memAddress = FetchWord(cycles, mem);
                Word memValue = byteMode ? ReadByte(cycles, mem, memAddress) : ReadWord(cycles, mem, memAddress);
                Word jumpAddress = FetchWord(cycles, mem);

                if (registers[reg] <= memValue) {
                    registers.PC = jumpAddress;
                }
            } break;
            case OP_JSR: {
                Word newPC = FetchWord(cycles, mem);
                StackPushWord(cycles, mem, registers.PC); //Push program counter to stack
                registers.PC = newPC; //Jump to start of subroutine
            } break;
            case OP_RTN: {
                registers.PC = StackPopWord(cycles, mem);
            } break;
            case OP_PUSH: {
                Byte reg = FetchByte(cycles, mem);
                StackPushWord(cycles, mem, registers[reg]);
            } break;
            case OP_PUSHM: {
                Byte reg = FetchByte(cycles, mem);
                Word address = FetchWord(cycles, mem);
                Word memValue = byteMode ? ReadByte(cycles, mem, address) : ReadWord(cycles, mem, address);
                StackPushWord(cycles, mem, registers[reg]);
            } break;
            case OP_PUSHC: {
                Byte reg = FetchByte(cycles, mem);
                Word value = byteMode ? FetchByte(cycles, mem) : FetchWord(cycles, mem);
                StackPushWord(cycles, mem, registers[reg]);
            } break;
            case OP_PUSHS: {
                StackPushByte(cycles, mem, registers.status);
            } break;
            case OP_POP: {
                Byte reg = FetchByte(cycles, mem);
                registers[reg] = byteMode ? StackPopByte(cycles, mem) : StackPopWord(cycles, mem);
            } break;
            case OP_POPM: {
                Word address = FetchWord(cycles, mem);
                Word stackValue = byteMode ? StackPopByte(cycles, mem) : StackPopWord(cycles, mem);
                byteMode ? WriteByte(cycles, mem, address, stackValue & 0xFF) : WriteWord(cycles, mem, address, stackValue);
            } break;
            case OP_POPS: {
                registers.status = StackPopByte(cycles, mem);
            } break;
            default:
                std::cout << "ERROR: Illegal instruction\n";
                throw;
            }
        }

        if (cycles < 0) {
            std::cout << "WARNING: CPU used additional cycles\n";
        }
    }
};