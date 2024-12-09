#include <iostream>

typedef uint8_t Byte;
typedef uint16_t Word;
typedef uint32_t DWord;

typedef uint64_t u64;

enum Instruction
{
    OP_NOP = 0x00,

    //Increment
    OP_INC_B =      0x01, //Increment a byte in memory
    OP_INC_W =      0x02, //Increment a word in memory
    OP_INCR_B =     0x03, //Increment a byte in a register
    OP_INCR_W =     0x04, //Increment a word in a register

    OP_DEC_B =     0x05, //Decrement a byte in memory
    OP_DEC_W =     0x06, //Decrement a word in memory
    OP_DECR_B =     0x07, //Decrement a byte in a register
    OP_DECR_W =     0x08, //Decrement a word in a register

    //Arithmetic
    OP_ADDR_B =     0x10, //Add two registers, store in first
    OP_SUBR_B =     0x11,
    OP_ADD_B_C =    0x12, //Add byte constant into register
    OP_SUB_B_C =    0x13,
    OP_ADD_B_A =    0x14, //Add register and byte at memory address, store in register
    OP_SUB_B_A =    0x15,

    OP_ADDR_W =     0x20, //Add two registers, store in first
    OP_SUBR_W =     0x21,
    OP_ADD_W_C =    0x22, //Add word constant into register
    OP_SUB_W_C =    0x23,
    OP_ADD_W_A =    0x24, //Add register and word at memory address, store in register
    OP_SUB_W_A =    0x25,

    //Data moving
    OP_LDR_B = 0xA0, //Load byte into register
    OP_LDR_W = 0xA1, //Load word into register

    //Control
    OP_JMP = 0xB0,
    OP_JSR = 0xB1,
    OP_RTN = 0xB2,
}; 

struct Memory
{
    static constexpr Word MEM_SIZE = 0xFFFF;
    Byte* Data = new Byte[MEM_SIZE];

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
        Word R0, R1, R2, R3, R4, R5;
    };
    Word aligned[6];
};

struct CPU {

    //Registers
    Word PC; //Progran counter
    Word SP; //Stack pointer

    Registers registers;

    void Reset(Memory& mem) {
        mem.Clear();

        PC = 0;
        SP = 0xFFFF; //Stack grows backwards from end
        memset(registers.aligned, 0, 6);
    }

    Byte FetchByte(u64& cycles, Memory& mem) {
        cycles--;
        return mem[PC++];
    }
    Byte ReadByte(u64& cycles, Memory& mem, Word address) const {
        cycles--;
        return mem[address];
    }
    void WriteByte(u64& cycles, Memory& mem, Word address, Byte byte) {
        mem[address] = byte;
        cycles--;
    }
    void IncrementByte(u64& cycles, Memory& mem, Word address) {
        mem[address] = (Byte)(mem[address] + 1);
        cycles--;
    }
    void DecrementByte(u64& cycles, Memory& mem, Word address) {
        mem[address] = (Byte)(mem[address] - 1);
        cycles--;
    }

    Word FetchWord(u64& cycles, Memory& mem) {
        Word word = mem[PC++];
        word |= (mem[PC++] << 8); //Little endian system

        cycles -= 2;
        return word;
    }
    Word ReadWord(u64& cycles, Memory& mem, Word address) const {
        Word word = mem[address];
        word |= (mem[address + 1] << 8); //Little endian system

        cycles -= 2;
        return word;
    }
    void WriteWord(u64& cycles, Memory& mem, Word address, Word byte) {
        mem[address] = byte & 0xFF; //Get the lowest 8 bits
        mem[address + 1] = byte >> 8; //Get ths highest 8 bits
        cycles--;
    }
    void IncrementWord(u64& cycles, Memory& mem, Word address) {
        Word value = ReadWord(cycles, mem, address);
        WriteWord(cycles, mem, address, value + 1);
    }
    void DecrementWord(u64& cycles, Memory& mem, Word address) {
        Word value = ReadWord(cycles, mem, address);
        WriteWord(cycles, mem, address, value - 1);
    }

    DWord FetchDWord(u64& cycles, Memory& mem) {
        DWord word = mem[PC++];
        word |= (mem[PC++] << 24); //Little endian system
        word |= (mem[PC++] << 16);
        word |= (mem[PC++] << 8);

        cycles -= 4;
        return word;
    }
    DWord ReadDWord(u64& cycles, Memory& mem, Word address) const {
        DWord word = mem[address];
        word |= (mem[address + 1] << 24); //Little endian system
        word |= (mem[address + 2] << 16);
        word |= (mem[address + 3] << 8);

        cycles -= 4;
        return word;
    }

    void Execute(u64 cycles, Memory& mem) {
        while (cycles > 0)
        {
            Instruction instruction = (Instruction)FetchByte(cycles, mem);
            switch (instruction)
            {
            case OP_NOP:
                break;
            case OP_INC_B:
                IncrementByte(cycles, mem, FetchWord(cycles, mem));
                break;
            case OP_INC_W:
                IncrementWord(cycles, mem, FetchWord(cycles, mem));
                break;
            /*case OP_INCR_B:
                Byte reg = FetchByte(cycles, mem);
                break;
            case OP_INCR_W:
                Byte reg = FetchByte(cycles, mem);
                break;*/
            case OP_DEC_B:
                DecrementByte(cycles, mem, FetchWord(cycles, mem));
                break;
            case OP_DEC_W:
                DecrementWord(cycles, mem, FetchWord(cycles, mem));
                break;
            /*case OP_DECR_B:
                Byte reg = FetchByte(cycles, mem);
                break;
            case OP_DECR_W:
                Byte reg = FetchByte(cycles, mem);
                break;*/
            case OP_ADDR_B: {
                Byte reg1 = FetchByte(cycles, mem);
                Byte reg2 = FetchByte(cycles, mem);

                Byte result = registers.aligned[reg1] + registers.aligned[reg2];
                registers.aligned[reg1] = result;
            } break;
            case OP_SUBR_B: {
                Byte reg1 = FetchByte(cycles, mem);
                Byte reg2 = FetchByte(cycles, mem);

                Byte result = registers.aligned[reg1] - registers.aligned[reg2];
                registers.aligned[reg1] = result;
            } break;
            case OP_ADDR_W: {
                Byte reg1 = FetchByte(cycles, mem);
                Byte reg2 = FetchByte(cycles, mem);

                Word result = registers.aligned[reg1] - registers.aligned[reg2];
                registers.aligned[reg1] = result;
            } break;
            case OP_SUBR_W: {
                Byte reg1 = FetchByte(cycles, mem);
                Byte reg2 = FetchByte(cycles, mem);

                Word result = registers.aligned[reg1] - registers.aligned[reg2];
                registers.aligned[reg1] = result;
            } break;
            case OP_ADD_B_C:
                break;
            case OP_SUB_B_C:
                break;
            case OP_ADD_B_A:
                break;
            case OP_SUB_B_A:
                break;
            case OP_ADD_W_C:
                break;
            case OP_SUB_W_C:
                break;
            case OP_ADD_W_A:
                break;
            case OP_SUB_W_A:
                break;
            case OP_LDR_B: {
                Byte reg = FetchByte(cycles, mem);
                registers.aligned[reg] = FetchByte(cycles, mem);
            } break;
            case OP_LDR_W: {
                Byte reg = FetchByte(cycles, mem);
                registers.aligned[reg] = FetchWord(cycles, mem);
            } break;
            case OP_JMP:
                PC = FetchByte(cycles, mem);
                break;
            case OP_JSR:
                break;
            case OP_RTN:
                break;
            default:
                break;
            }
        }

        if (cycles < 0) {
            std::cout << "WARNING: CPU used additional cycles";
        }
    }
};

int main()
{
    Memory mem{};
    CPU cpu{};

    cpu.Reset(mem);
    cpu.Execute(4, mem);

    delete mem.Data;
}
