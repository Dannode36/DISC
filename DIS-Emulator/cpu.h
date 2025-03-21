#pragma once
#include <iostream>

typedef uint8_t Byte;
typedef uint16_t Word;
typedef uint32_t DWord;
typedef int64_t i64;

/// <summary>
/// Basic Principles:
///  - Little endian
///  - Opcodes cannot consume >1 memory addressing parameter
///  - All memory operations are 16 bit
///  - In progmem, all values are stored as 16 bits except register and interrupt values which are stored in 8 bits
///  - The addressing mode bit is 0 for constant memory access and 1 for register value access (excluding logical jumps)
///  - The addressing mode bit determines if the new PC value is read from a constant or a register in logical jumps
/// </summary>

enum Opcode : Byte
{
    //Special
    OP_NOOP = 0x00,     //No Op
    OP_RESET = 0x7E,    //Reset the CPU (clears registers and memory, resets flags)
    OP_HALT = 0x7F,     //Stops the CPU execution of instuctions

    //Arithmetic
    OP_ADD = 0x01,      //Add two registers, store in first
    OP_ADDC,            //Add word constant into register

    OP_SUB,             //Subtract two registers, store in first
    OP_SUBC,            //Subtract constant value from a register, store in register

    OP_MUL,             //Multiply two registers, store in first
    OP_MULC,            //Multiply constant value from a register, store in register

    OP_DIV,             //Divide two registers, store in first
    OP_DIVC,            //Divide constant value from a register, store in register

    //OP_CMP = 0x0E,      //Subtract two registers and update status flags, discard result
    //OP_CMPA = 0x0F,     //Subtract a value in memory from a register and update status flags, discard result

    //Increment
    OP_INC = 0x10,      //Increment a value in a register
    OP_DEC,             //Decrement a value in a register

    //Bitwise
    OP_UXT = 0x20,      //Zero extend a register (truncate 16 bit value to 8 bits)
    OP_LSL,             //Logical shift left
    OP_LSR,             //Logical shift right

    //Data moving
    OP_LDR = 0x30,      //Load value from second register into first register
    OP_LDC,             //Load value constant into register
    OP_LDM,             //Load value from memory into register

    OP_STRM,            //Store register into memory
    OP_STCM,            //Store constant into memory

    //Control
    OP_JSR = 0x40,      //Increment SP by 2, push the current PC to the stack, and jump to a subroutine
    OP_RTN,             //Pop the previous PC off the stack and jump to it, decrement value

    OP_JMP,             //Jump to a constant address (set program counter) and continue execution
    OP_JRZ,             //Jump to a constant address if register is = to 0
    OP_JRE,             //Jump to a constant address if register is = to a constant value
    OP_JRN,             //Jump to a constant address if register is != to a constant value
    OP_JRG,             //Jump to a constant address if register is > than a constant value
    OP_JRL,             //Jump to a constant address if register is < than a constant value
    OP_JRGE,            //Jump to a constant address if register is >= to a constant value
    OP_JRLE,            //Jump to a constant address if register is <= to a constant value

    //OP_JMPR,             //Set the program counter to a register value and continue execution
    //OP_JRZR,             //Set the program counter to a register value if register is = to 0
    //OP_JRER,             //Set the program counter to a register value if register is = to a constant value
    //OP_JRNR,             //Set the program counter to a register value if register is != to a constant value
    //OP_JRGR,             //Set the program counter to a register value if register is > than a constant value
    //OP_JRLR,             //Set the program counter to a register value if register is < than a constant value
    //OP_JRGER,            //Set the program counter to a register value if register is >= to a constant value
    //OP_JRLER,            //Set the program counter to a register value if register is <= to a constant value

    //Stack
    OP_PUSH = 0x60,     //Push register onto stack, decrement SP by (opsize + 1)
    OP_PUSHC,           //Push constant onto stack, decrement SP by (opsize + 1)

    OP_POP,             //Pop value from stack into register, increment SP by (opsize + 1)

    OP_PUSHS,           //Push status onto stack, decrement SP by (opsize + 1)
    OP_POPS,            //Pop stack into status, increment SP by (opsize + 1)

    OP_SEI = 0x70,      //Set the global interrupt enable flag
    OP_CLI,             //Clear the global interrupt enable flag

    //Opcodes must not excede 0x7F (01111111) due to the "addressMode" bit!
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
struct Memory
{
    /*
        +-----------------+ 0xFFFF
        | Interrupt Table |   ->   Stores 8 interrupt handler addresses
        +-----------------+ 0xFFF0
        |    Stack (v)    |
        +-----------------+
        |                 |
        +                 +
        |                 |
        +-----------------+
        |                 |
        +                 +
        |    Heap  (^)    |
        +                 +
        |                 |
        +-----------------+
        |     Program     |
        +-----------------+ 0x0000
    */

    static constexpr Word MEM_SIZE = 0xFFFF;
    static constexpr Word INTERRUPT_TABLE = 0xFFF0;

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
        registers.SP = 0x00A0; //Stack grows backwards from end
        memset(registers.aligned, 0, 6);
    }

    Byte NextByte(i64& cycles, Memory& mem) {
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

    Word NextWord(i64& cycles, Memory& mem) {
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
    void StackPush(i64& cycles, Memory& mem, Word value) {
        registers.SP -= 2;
        WriteWord(cycles, mem, registers.SP, value);
    }
    Word StackPop(i64& cycles, Memory& mem) {
        Word value = ReadWord(cycles, mem, registers.SP);
        registers.SP += 2;
        return value;
    }

    void UpdateStatusFlags(i64 result) {
        if (result > sizeof(Word)) {
            registers.C = 1;
        }
        else {
            registers.C = 0;
        }

        if (result > sizeof(int16_t)) {
            registers.O = 1;
        }
        else {
            registers.O = 0;
        }

        if (result & 32768) { // 1 << 15
            registers.N = 1;
        }
        else {
            registers.N = 0;
        }

        if (result == 0) {
            registers.Z = 1;
        }
        else {
            registers.Z = 0;
        }
    }
    void CoreDump() const {
        printf("\nCPU CORE DUMP:\n");

        printf("Program Counter:    %i\n", registers.PC);
        printf("Stack Pointer:      %i\n\n", registers.SP);

        printf("Register 0:         %i\n", registers.R0);
        printf("Register 1:         %i\n", registers.R1);
        printf("Register 2:         %i\n", registers.R2);
        printf("Register 3:         %i\n", registers.R3);
        printf("Register 4:         %i\n", registers.R4);
        printf("Register 5:         %i\n\n", registers.R5);

        printf("Negative flag:      %i\n", registers.N);
        printf("Overflow flag:      %i\n", registers.O);
        printf("Break flag:         %i\n", registers.B);
        printf("Decimal flag:       %i\n", registers.D);
        printf("Interrupt flag:     %i\n", registers.I);
        printf("Zero flag:          %i\n", registers.Z);
        printf("Carry flag:         %i\n", registers.C);
        printf("Unused flag:        %i\n", registers._);
    }

    void ExecuteInterrupt(i64& cycles, Memory& mem, Interrupt i) {
        StackPush(cycles, mem, registers.status);
        StackPush(cycles, mem, registers.PC);

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
                continue;
            }
            else if (registers.I && registers.interruptFlags > 0) {
                int lowestSetBit = static_cast<int>(log2(registers.interruptFlags & -registers.interruptFlags) + 1); //This is cool
                ExecuteInterrupt(cycles, mem, (Interrupt)lowestSetBit);
                continue;
            }

            Byte instByte = NextByte(cycles, mem);
            Opcode instruction = (Opcode)(instByte & 0x7F);
            bool addressMode = (instByte >> 7) == 1; //0 -> constant address, 1 -> register address)

            switch (instruction)
            {
            case OP_NOOP: break;
            case OP_RESET: {
                Reset(mem);
                std::cout << "INFO: RESET instruction executed\n";
            } break;
            case OP_HALT: {
                halted = true;
                std::cout << "INFO: HALT instruction executed\n";
            } break;
            case OP_INC: {
                Byte reg = NextByte(cycles, mem);
                registers[reg]++;
            } break;
            case OP_DEC: {
                Byte reg = NextByte(cycles, mem);
                registers[reg]--;
            } break;
            case OP_ADD: {
                Byte reg1 = NextByte(cycles, mem);
                Byte reg2 = NextByte(cycles, mem);

                i64 result = (i64)registers[reg1] + registers[reg2]; //Cheating by casting to a larger type ;)
                UpdateStatusFlags(result);

                registers[reg1] = result;
            } break;
            case OP_ADDC: {
                Byte reg = NextByte(cycles, mem);
                Word value = NextWord(cycles, mem);

                i64 result = (i64)registers[reg] + value; //Cheating by casting to a larger type ;)
                UpdateStatusFlags(result);

                registers[reg] = result;
            } break;
            case OP_SUB: {
                Byte reg1 = NextByte(cycles, mem);
                Byte reg2 = NextByte(cycles, mem);

                i64 result = (i64)registers[reg1] - registers[reg2]; //Cheating by casting to a larger type ;)
                UpdateStatusFlags(result);

                registers[reg1] = result;
            } break;
            case OP_SUBC: {
                Byte reg = NextByte(cycles, mem);
                Word value = NextWord(cycles, mem);

                i64 result = (i64)registers[reg] - value; //Cheating by casting to a larger type ;)
                UpdateStatusFlags(result);

                registers[reg] = result;
            } break;
            case OP_MUL: {
                Byte reg1 = NextByte(cycles, mem);
                Byte reg2 = NextByte(cycles, mem);

                i64 result = (i64)registers[reg1] * registers[reg2]; //Cheating by casting to a larger type ;)
                UpdateStatusFlags(result);

                registers[reg1] = result;
            } break;
            case OP_MULC: {
                Byte reg = NextByte(cycles, mem);
                Word value = NextWord(cycles, mem);

                i64 result = (i64)registers[reg] * value; //Cheating by casting to a larger type ;)
                UpdateStatusFlags(result);

                registers[reg] = result;
            } break;
            case OP_DIV: {
                Byte reg1 = NextByte(cycles, mem);
                Byte reg2 = NextByte(cycles, mem);

                i64 result = (i64)registers[reg1] / registers[reg2]; //Cheating by casting to a larger type ;)
                UpdateStatusFlags(result);

                registers[reg1] = result;
            } break;
            case OP_DIVC: {
                Byte reg = NextByte(cycles, mem);
                Word value = NextWord(cycles, mem);

                i64 result = (i64)registers[reg] / value; //Cheating by casting to a larger type ;)
                UpdateStatusFlags(result);

                registers[reg] = result;
            } break;
            case OP_LSL: {
                Byte reg = NextByte(cycles, mem);
                Word value = NextWord(cycles, mem);

                registers[reg] = registers[reg] << value;
            } break;
            case OP_LSR: {
                Byte reg = NextByte(cycles, mem);
                Word value = NextWord(cycles, mem);

                registers[reg] = registers[reg] >> value;
            } break;
            case OP_UXT: {
                Byte reg = NextByte(cycles, mem);
                registers[reg] &= 0xFF;
            } break;
            case OP_LDR: {
                Byte reg1 = NextByte(cycles, mem);
                Byte reg2 = NextByte(cycles, mem);
                registers[reg1] = registers[reg2];
            } break;
            case OP_LDC: {
                Byte reg = NextByte(cycles, mem);
                registers[reg] = NextWord(cycles, mem);
            } break;
            case OP_LDM: {
                Byte reg = NextByte(cycles, mem);
                Word address = addressMode ? NextWord(cycles, mem) : registers[NextByte(cycles, mem)];
                registers[reg] = ReadWord(cycles, mem, address);
            } break;
            case OP_STRM: {
                Byte reg = NextByte(cycles, mem);
                Word address = addressMode ? NextWord(cycles, mem) : registers[NextByte(cycles, mem)];

                WriteWord(cycles, mem, address, registers[reg]);
            } break;
            case OP_STCM: {
                Word value = NextWord(cycles, mem);
                Word address = addressMode ? NextWord(cycles, mem) : registers[NextByte(cycles, mem)];

                WriteWord(cycles, mem, address, value);
            } break;
            case OP_JMP: {
                registers.PC = addressMode ? NextWord(cycles, mem) : registers[NextByte(cycles, mem)];
            } break;
            case OP_JRZ: {
                if (registers[NextByte(cycles, mem)] == 0) {
                    registers.PC = addressMode ? NextWord(cycles, mem) : registers[NextByte(cycles, mem)];
                }
                else {
                    registers.PC += 2; //Avoid wasting 2 cycles with NextWord()
                }
            } break;
            case OP_JRE: {
                Byte reg = NextByte(cycles, mem);
                Word value = NextWord(cycles, mem);
                Word address = addressMode ? NextWord(cycles, mem) : registers[NextByte(cycles, mem)];

                if (registers[reg] == value) {
                    registers.PC = address;
                }
            } break;
            case OP_JRN: {
                Byte reg = NextByte(cycles, mem);
                Word value = NextWord(cycles, mem);
                Word address = addressMode ? NextWord(cycles, mem) : registers[NextByte(cycles, mem)];

                if (registers[reg] != value) {
                    registers.PC = address;
                }
            } break;
            case OP_JRG: {
                Byte reg = NextByte(cycles, mem);
                Word value = NextWord(cycles, mem);
                Word address = addressMode ? NextWord(cycles, mem) : registers[NextByte(cycles, mem)];

                if (registers[reg] > value) {
                    registers.PC = address;
                }
            } break;
            case OP_JRGE: {
                Byte reg = NextByte(cycles, mem);
                Word value = NextWord(cycles, mem);
                Word address = addressMode ? NextWord(cycles, mem) : registers[NextByte(cycles, mem)];

                if (registers[reg] >= value) {
                    registers.PC = address;
                }
            } break;
            case OP_JRL: {
                Byte reg = NextByte(cycles, mem);
                Word value = NextWord(cycles, mem);
                Word address = addressMode ? NextWord(cycles, mem) : registers[NextByte(cycles, mem)];

                if (registers[reg] < value) {
                    registers.PC = address;
                }
            } break;
            case OP_JRLE: {
                Byte reg = NextByte(cycles, mem);
                Word value = NextWord(cycles, mem);
                Word address = addressMode ? NextWord(cycles, mem) : registers[NextByte(cycles, mem)];

                if (registers[reg] <= value) {
                    registers.PC = address;
                }
            } break;
            case OP_JSR: {
                Word newPC = NextWord(cycles, mem);
                StackPush(cycles, mem, registers.PC); //Push program counter to stack
                registers.PC = newPC; //Jump to start of subroutine
            } break;
            case OP_RTN: {
                registers.PC = StackPop(cycles, mem);
            } break;
            case OP_PUSH: {
                Byte reg = NextByte(cycles, mem);
                StackPush(cycles, mem, registers[reg]);
            } break;
            case OP_PUSHC: {
                Word value = NextWord(cycles, mem);
                StackPush(cycles, mem, value);
            } break;
            case OP_PUSHS: {
                StackPush(cycles, mem, registers.status);
            } break;
            case OP_POP: {
                Byte reg = NextByte(cycles, mem);
                registers[reg] = StackPop(cycles, mem);
            } break;
            case OP_POPS: {
                registers.status = (Byte)StackPop(cycles, mem);
            } break;
            default:
                throw std::exception("ERROR: Illegal instruction\n");
            }
        }

        if (cycles < 0) {
            std::cout << "WARNING: CPU used additional cycles. This is unintended behaviour\n";
        }
    }
};