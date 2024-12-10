#include <iostream>

typedef uint8_t Byte;
typedef uint16_t Word;
typedef uint32_t DWord;

typedef int64_t i64;

enum Instruction
{
    OP_NOP =        0x00,
    OP_RESET =      0xFE,
    OP_HALT =       0xFF,

    //Arithmetic
    OP_ADD =        0x10,   //Add two registers, store in first
    OP_ADDC =       0x11,   //Add word constant into register
    OP_ADDA =       0x12,   //Add register and word at memory address, store in register
    OP_SUB =        0x13,   //Subtract two registers, store in first
    OP_SUBC =       0x14,   //Subtract constant value from a register, store in register
    OP_SUBA =       0x15,   //Subtract the value in memory from a register, store in register

    OP_CMP = 0x1E,   //Subtract two registers and update status flags, discard result
    OP_CMPA = 0x1F,   //Subtract a value in memory from a register and update status flags, discard result

    //Increment
    OP_INC =        0x21,   //Increment a word in a register
    OP_INCA =       0x22,   //Increment a word in memory
    OP_DEC =        0x23,   //Decrement a word in a register
    OP_DECA =       0x24,   //Decrement a word in memory

    //Bitwise
    OP_UXT =        0x30,   //Zero extend a byte (truncate 16 bit value to 8 bits)

    //Registers
    OP_LD =         0xA0,   //Load word into register
    OP_LDB =        0xA1,   //Load value into register

    //Control
    OP_JSR =        0xB0,   //Increment SP by 2, push the current PC to the stack, and jump to a subroutine
    OP_RTN =        0xB1,   //Pop the previous PC off the stack and jump to it, decrement the SP by 2

    OP_JMP =        0xC0,   //Set the program counter (PC) and contion execution
    OP_JRZ =        0xC1,   //Jump if register is equal to 0

    OP_JRE =        0xC2,   //Jump if register is equal to a constant
    OP_JRN =        0xC3,   //Jump if register is not equal to a constant
    OP_JRG =        0xC4,   //Jump if register is greater than a constant
    OP_JRGE =       0xC5,   //Jump if register is greater than or equal to a constant
    OP_JRL =        0xC6,   //Jump if register is less than a constant
    OP_JRLE =       0xC7,   //Jump if register is less than or equal to a constant

    OP_JREA =       0xC8,   //Jump if register is equal to a value in memory
    OP_JRNA =       0xC9,   //Jump if register is not equal to a value in memory
    OP_JRGA =       0xCA,   //Jump if register is greater than a value in memory
    OP_JRGEA =      0xCB,   //Jump if register is greater than or equal to a value in memory
    OP_JRLA =       0xCC,   //Jump if register is less than a value in memory
    OP_JRLEA =      0xCD,   //Jump if register is less than or equal to a value in memory

    //Stack
    OP_PUSHR =      0xE0,   //Push register onto stack, decrement SP by 2
    OP_POPR =       0xE1,   //Pop stack into register, increment SP by 2
    OP_PUSHR8 =     0xE2,   //Push register onto stack, decrement SP
    OP_POPR8 =      0xE3,   //Pop stack into register, increment SP
    //OP_PHS =      0xC4,   //Push status onto stack, decrement SP
    //OP_PHS =      0xC5,   //Pop stack into status, increment SP
}; 

struct Memory
{
    static constexpr Word MEM_SIZE = 0xFFFF;
    //Byte* Data = new Byte[MEM_SIZE];
    Byte Data[MEM_SIZE];

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
        Word I : 1; //Interrupt disable
        Word Z : 1; //Zero
        Word C : 1; //Carry
        Word _ : 1; //Unused
    };

    struct
    {
        Word aligned[8];
        Byte status;
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

    void Execute(i64 cycles, Memory& mem) {
        while (cycles > 0 && !halted)
        {
            Instruction instruction = (Instruction)FetchByte(cycles, mem);
            switch (instruction)
            {
            case OP_NOP: break;
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
            case OP_INCA: {
                Word address = FetchWord(cycles, mem);
                Word value = ReadWord(cycles, mem, address) + 1;
                WriteWord(cycles, mem, address, value);
            } break;
            case OP_DEC: {
                Byte reg = FetchByte(cycles, mem);
                registers[reg]--;
            } break;
            case OP_DECA: {
                Word address = FetchWord(cycles, mem);
                Word value = ReadWord(cycles, mem, address) - 1;
                WriteWord(cycles, mem, address, value);
            } break;
            case OP_ADD: {
                Byte reg1 = FetchByte(cycles, mem);
                Byte reg2 = FetchByte(cycles, mem);

                registers[reg1] = registers[reg1] + registers[reg2];
            } break;
            case OP_ADDC: {
                Byte reg = FetchByte(cycles, mem);
                Word value = FetchWord(cycles, mem);

                registers[reg] = registers[reg] + value;
            } break;
            case OP_ADDA: {
                Byte reg = FetchByte(cycles, mem);
                Word address = FetchWord(cycles, mem);
                Word memValue = ReadWord(cycles, mem, address);

                registers[reg] = registers[reg] + memValue;
            } break;
            case OP_SUB: {
                Byte reg1 = FetchByte(cycles, mem);
                Byte reg2 = FetchByte(cycles, mem);

                registers[reg1] = registers[reg1] - registers[reg2];
            } break;
            case OP_SUBC: {
                Byte reg = FetchByte(cycles, mem);
                Word value = FetchWord(cycles, mem);

                registers[reg] = registers[reg] - value;
            } break;
            case OP_SUBA: {
                Byte reg = FetchByte(cycles, mem);
                Word address = FetchWord(cycles, mem);
                Word memValue = ReadWord(cycles, mem, address);

                registers[reg] = registers[reg] - memValue;
            } break;
            case OP_UXT: {
                Byte reg = FetchByte(cycles, mem);
                registers[reg] &= 0xFF;
            }
            case OP_LD: {
                Byte reg = FetchByte(cycles, mem);
                registers[reg] = FetchWord(cycles, mem);
            } break;
            case OP_LDB: {
                Byte reg = FetchByte(cycles, mem);
                registers[reg] = FetchByte(cycles, mem);
            } break;
            case OP_JMP: {
                registers.PC = FetchWord(cycles, mem);
            } break;
            case OP_JRZ: {
                if (registers[FetchByte(cycles, mem)] == 0) {
                    registers.PC = FetchWord(cycles, mem);
                }
            } break;
            case OP_JRE: {
                Byte reg = FetchByte(cycles, mem);
                Word value = FetchWord(cycles, mem);
                Word address = FetchWord(cycles, mem);

                if (registers[reg] == value) {
                    registers.PC = address;
                }
            } break;
            case OP_JRN: {
                Byte reg = FetchByte(cycles, mem);
                Word value = FetchWord(cycles, mem);
                Word address = FetchWord(cycles, mem);

                if (registers[reg] != value) {
                    registers.PC = address;
                }
            } break;
            case OP_JRG: {
                Byte reg = FetchByte(cycles, mem);
                Word value = FetchWord(cycles, mem);
                Word address = FetchWord(cycles, mem);

                if (registers[reg] > value) {
                    registers.PC = address;
                }
            } break;
            case OP_JRGE: {
                Byte reg = FetchByte(cycles, mem);
                Word value = FetchWord(cycles, mem);
                Word address = FetchWord(cycles, mem);

                if (registers[reg] >= value) {
                    registers.PC = address;
                }
            } break;
            case OP_JRL: {
                Byte reg = FetchByte(cycles, mem);
                Word value = FetchWord(cycles, mem);
                Word address = FetchWord(cycles, mem);

                if (registers[reg] < value) {
                    registers.PC = address;
                }
            } break;
            case OP_JRLE: {
                Byte reg = FetchByte(cycles, mem);
                Word value = FetchWord(cycles, mem);
                Word address = FetchWord(cycles, mem);

                if (registers[reg] <= value) {
                    registers.PC = address;
                }
            } break;
            case OP_JREA: {
                Byte reg = FetchByte(cycles, mem);
                Word memAddress = FetchWord(cycles, mem);
                Word memValue = ReadWord(cycles, mem, memAddress);
                Word jumpAddress = FetchWord(cycles, mem);

                if (registers[reg] == memValue) {
                    registers.PC = jumpAddress;
                }
            } break;
            case OP_JRNA: {
                Byte reg = FetchByte(cycles, mem);
                Word memAddress = FetchWord(cycles, mem);
                Word memValue = ReadWord(cycles, mem, memAddress);
                Word jumpAddress = FetchWord(cycles, mem);

                if (registers[reg] != memValue) {
                    registers.PC = jumpAddress;
                }
            } break;
            case OP_JRGA: {
                Byte reg = FetchByte(cycles, mem);
                Word memAddress = FetchWord(cycles, mem);
                Word memValue = ReadWord(cycles, mem, memAddress);
                Word jumpAddress = FetchWord(cycles, mem);

                if (registers[reg] > memValue) {
                    registers.PC = jumpAddress;
                }
            } break;
            case OP_JRGEA: {
                Byte reg = FetchByte(cycles, mem);
                Word memAddress = FetchWord(cycles, mem);
                Word memValue = ReadWord(cycles, mem, memAddress);
                Word jumpAddress = FetchWord(cycles, mem);

                if (registers[reg] >= memValue) {
                    registers.PC = jumpAddress;
                }
            } break;
            case OP_JRLA: {
                Byte reg = FetchByte(cycles, mem);
                Word memAddress = FetchWord(cycles, mem);
                Word memValue = ReadWord(cycles, mem, memAddress);
                Word jumpAddress = FetchWord(cycles, mem);

                if (registers[reg] < memValue) {
                    registers.PC = jumpAddress;
                }
            } break;
            case OP_JRLEA: {
                Byte reg = FetchByte(cycles, mem);
                Word memAddress = FetchWord(cycles, mem);
                Word memValue = ReadWord(cycles, mem, memAddress);
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
            case OP_PUSHR: {
                Byte reg = FetchByte(cycles, mem);
                StackPushWord(cycles, mem, registers[reg]);
            } break;
            case OP_POPR: {
                Byte reg = FetchByte(cycles, mem);
                registers[reg] = StackPopWord(cycles, mem);
            } break;
            case OP_PUSHR8: {
                Byte reg = FetchByte(cycles, mem);
                StackPushByte(cycles, mem, registers[reg]);
            } break;
            case OP_POPR8: {
                Byte reg = FetchByte(cycles, mem);
                registers[reg] = StackPopByte(cycles, mem);
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

int main()
{
    Memory mem{};
    CPU cpu{};

    cpu.Reset(mem);
    mem[0x0000] = OP_INC;
    mem[0x0001] = 0x00;
    mem[0x0002] = OP_JRN;
    mem[0x0003] = 0x00; //Register
    mem[0x0004] = 0x10; //Value (0 - 7)
    mem[0x0005] = 0x00; //Value (0 - 7)
    mem[0x0006] = 0x00; //Address (0 - 7)
    mem[0x0007] = 0x00; //Address (8 - 15)
    mem[0x0008] = OP_HALT;

    cpu.Execute(129, mem); //This simply increment loop takes 129 cycles x_x (JRN eats up 6 cycles)

    __noop; //For breakpoint debugging
}
