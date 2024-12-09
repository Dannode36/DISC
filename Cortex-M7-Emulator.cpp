#include <iostream>

typedef uint8_t Byte;
typedef uint16_t Word;
typedef uint32_t DWord;

typedef int64_t i64;

enum Instruction
{
    OP_NOP = 0x00,
    OP_RESET = 0xFE,
    OP_HALT = 0xFF,

    //Arithmetic
    OP_ADD8R =     0x10, //Add two registers, store in first
    OP_ADD8C =    0x11, //Add value constant into register
    OP_ADD8A =    0x12, //Add register and value at memory address, store in register
    OP_SUB8R =     0x13,
    OP_SUB8C =    0x14,
    OP_SUB8A =    0x15,

    OP_ADD16R =     0x20, //Add two registers, store in first
    OP_ADD16C =    0x21, //Add word constant into register
    OP_ADD16A =    0x22, //Add register and word at memory address, store in register
    OP_SUB16R =     0x23,
    OP_SUB16C =    0x24,
    OP_SUB16A =    0x25,

    //Increment
    OP_INC8R = 0x31, //Increment a value in a register
    OP_INC8A = 0x32, //Increment a value in memory
    OP_DEC8R = 0x33, //Decrement a value in a register
    OP_DEC8 = 0x34, //Decrement a value in memory

    OP_INC16R = 0x35, //Increment a word in a register
    OP_INC16A = 0x36, //Increment a word in memory
    OP_DEC16R = 0x37, //Decrement a word in a register
    OP_DEC16 = 0x38, //Decrement a word in memory

    //Registers
    OP_LD8 = 0xA0, //Load value into register
    OP_LD16 = 0xA1, //Load word into register

    //Control
    OP_JMP = 0xB0,
    OP_JSR = 0xB1,
    OP_RTN = 0xB2,

    //Stack
    OP_PUSHR8 = 0xC0, //Push register onto stack, decrement SP
    OP_POPR8 = 0xC1, //Pop stack into register, increment SP
    OP_PUSHR16 = 0xC2, //Push register onto stack, decrement SP by 2
    OP_POPR16 = 0xC3, //Pop stack into register, increment SP by 2
    //OP_PHS = 0xC4, //Push status onto stack, decrement SP
    //OP_PHS = 0xC5, //Pop stack into status, increment SP
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
        Word 
            R0, R1, R2, R3, R4, R5, //General purpose registers
            PC, //Program counter
            SP;//Stack pointer
    };
    Word aligned[8];

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
            case OP_INC8A: {
                Word address = FetchWord(cycles, mem);
                Byte& memByte = ReadByte(cycles, mem, address);
                memByte = (Byte)(memByte + 1);
            } break;
            case OP_INC8R: {
                Byte reg = FetchByte(cycles, mem);
                registers[reg] = (Byte)(registers[reg] + 1);
            } break;
            case OP_INC16A: {
                Word address = FetchWord(cycles, mem);
                Word value = ReadWord(cycles, mem, address) + 1;
                WriteWord(cycles, mem, address, value);
            } break;
            case OP_INC16R: {
                Byte reg = FetchByte(cycles, mem);
                registers[reg]++;
            } break;
            case OP_DEC8: {
                Word address = FetchWord(cycles, mem);
                Byte& memByte = ReadByte(cycles, mem, address);
                memByte = (Byte)(memByte - 1);
            } break;
            case OP_DEC8R: {
                Byte reg = FetchByte(cycles, mem);
                registers[reg] = (Byte)(registers[reg] - 1);
            } break;
            case OP_DEC16: {
                Word address = FetchWord(cycles, mem);
                Word value = ReadWord(cycles, mem, address) - 1;
                WriteWord(cycles, mem, address, value);
            } break;
            case OP_DEC16R: {
                Byte reg = FetchByte(cycles, mem);
                registers[reg]--;
            } break;
            case OP_ADD8R: {
                Byte reg1 = FetchByte(cycles, mem);
                Byte reg2 = FetchByte(cycles, mem);

                registers[reg1] = (Byte)(registers[reg1] + registers[reg2]);
            } break;
            case OP_ADD8C: {
                Byte reg = FetchByte(cycles, mem);
                Byte value = FetchByte(cycles, mem);

                registers[reg] = (Byte)(registers[reg] + value);
            } break;
            case OP_ADD8A: {
                Byte reg = FetchByte(cycles, mem);
                Word address = FetchWord(cycles, mem);
                Byte memValue = ReadByte(cycles, mem, address);

                registers[reg] = (Byte)(registers[reg] + memValue);
            } break;
            case OP_SUB8R: {
                Byte reg1 = FetchByte(cycles, mem);
                Byte reg2 = FetchByte(cycles, mem);

                Byte result = registers[reg1] - registers[reg2];
                registers[reg1] = (Byte)(registers[reg1] - registers[reg2]);
            } break;
            case OP_SUB8C: {
                Byte reg = FetchByte(cycles, mem);
                Byte value = FetchByte(cycles, mem);

                registers[reg] = (Byte)(registers[reg] - value);
            } break;
            case OP_SUB8A: {
                Byte reg = FetchByte(cycles, mem);
                Word address = FetchWord(cycles, mem);
                Byte memValue = ReadByte(cycles, mem, address);

                registers[reg] = (Byte)(registers[reg] - memValue);
            } break;
            case OP_ADD16R: {
                Byte reg1 = FetchByte(cycles, mem);
                Byte reg2 = FetchByte(cycles, mem);

                registers[reg1] = registers[reg1] + registers[reg2];
            } break;
            case OP_ADD16C: {
                Byte reg = FetchByte(cycles, mem);
                Word value = FetchWord(cycles, mem);

                registers[reg] = registers[reg] + value;
            } break;
            case OP_ADD16A: {
                Byte reg = FetchByte(cycles, mem);
                Word address = FetchWord(cycles, mem);
                Word memValue = ReadWord(cycles, mem, address);

                registers[reg] = registers[reg] + memValue;
            } break;
            case OP_SUB16R: {
                Byte reg1 = FetchByte(cycles, mem);
                Byte reg2 = FetchByte(cycles, mem);

                registers[reg1] = registers[reg1] - registers[reg2];
            } break;
            case OP_SUB16C: {
                Byte reg = FetchByte(cycles, mem);
                Word value = FetchWord(cycles, mem);

                registers[reg] = registers[reg] - value;
            } break;
            case OP_SUB16A: {
                Byte reg = FetchByte(cycles, mem);
                Word address = FetchWord(cycles, mem);
                Word memValue = ReadWord(cycles, mem, address);

                registers[reg] = registers[reg] - memValue;
            } break;
            case OP_LD8: {
                Byte reg = FetchByte(cycles, mem);
                registers[reg] = FetchByte(cycles, mem);
            } break;
            case OP_LD16: {
                Byte reg = FetchByte(cycles, mem);
                registers[reg] = FetchWord(cycles, mem);
            } break;
            case OP_JMP: {
                registers.PC = FetchWord(cycles, mem);
            } break;
            case OP_JSR: {
                Word newPC = FetchWord(cycles, mem);
                StackPushWord(cycles, mem, registers.PC); //Push program counter to stack
                registers.PC = newPC; //Jump to start of subroutine
            } break;
            case OP_RTN: {
                registers.PC = StackPopWord(cycles, mem);
            } break;
            case OP_PUSHR8: {
                Byte reg = FetchByte(cycles, mem);
                StackPushByte(cycles, mem, registers[reg]);
            } break;
            case OP_POPR8: {
                Byte reg = FetchByte(cycles, mem);
                registers[reg] = StackPopByte(cycles, mem);
            } break;
            case OP_PUSHR16: {
                Byte reg = FetchByte(cycles, mem);
                StackPushWord(cycles, mem, registers[reg]);
            } break;
            case OP_POPR16: {
                Byte reg = FetchByte(cycles, mem);
                registers[reg] = StackPopWord(cycles, mem);
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
    mem[0x0000] = OP_LD8;
    mem[0x0001] = 0x00;
    mem[0x0002] = 0x04;
    mem[0x0003] = OP_LD8;
    mem[0x0004] = 0x01;
    mem[0x0005] = 0x02;
    mem[0x0006] = OP_ADD8R;
    mem[0x0007] = 0x00;
    mem[0x0008] = 0x01;
    mem[0x0009] = OP_JSR;
    mem[0x000A] = 0xA1; 
    mem[0x000B] = 0x00;
    mem[0x000C] = OP_HALT;

    //cool code

    mem[0x00A1] = OP_LD8;
    mem[0x00A2] = 0x03;
    mem[0x00A3] = 0x08;
    mem[0x00A4] = OP_RTN;

    cpu.Execute(21, mem);

    __noop; //For breakpoint debugging
}
