#include <iostream>
#include <sstream>
#include "../cpu.h"
#include <vector>

enum Instruction {
    INST_NOOP = 0,
    INST_RESET,  
    INST_HALT,   
    INST_ADD,
    INST_SUB,    
    INST_MUL,    
    INST_DIV,    
    INST_CMP,    
    INST_INC,    
    INST_DEC,    
    INST_UXT,
    INST_MOV,
    INST_JSR = 0x40,
    INST_RTN,    
    INST_JMP,    
    INST_JRZ,    
    INST_JRE,    
    INST_JRN,    
    INST_JRG,    
    INST_JRGE,   
    INST_JRL,    
    INST_JRLE,   
    INST_PUSH,
    INST_POP,    
    INST_PUSHS,  
    INST_POPS,   
    INST_SEI,
    INST_CLI,

    Count, //Keep last
};

enum Type
{
    Type_Word,
    Type_Byte,
    Type_Address,
    Type_Register
};

struct AsmArgument
{
    Type type;
    Word value;
};

struct AsmInstruction
{
    Instruction inst;
    std::vector<AsmArgument> args;
};

static Type GetVarType(const std::string& str) {
    if (str.front() == '[' && str.back() == ']') { //Address
        return Type_Address;
    }
    else if (str.front() == 'R') {
        return Type_Register;
    }
    else if (str.substr(0, 2) == "0x") {
        if (str.length() == 4) {
            return Type_Byte;
        }
        else {
            return Type_Word;
        }
    }
    else {
        //Optimize constants to bytes if possible
        if (std::stoi(str) <= 0xFF) {
            return Type_Byte;
        }
        else {
            return Type_Word;
        }
    }
}

static Word GetVarValue(std::string str) {
    //Remove brackets from an address
    if (str.front() == '[' && str.back() == ']') {
        str = str.substr(1, str.length() - 2);
    }

    if (str.substr(0, 2) == "0x") {
        return std::stoi(str.substr(3), nullptr, 16);
    }
    else if (str.front() == 'R') {
        return std::stoi(str.substr(1));
    }
    else {
        return std::stoi(str);
    }
}

static Opcode GetOpcode(const AsmInstruction& asmInst) {
    switch (asmInst.inst)
    {
    case INST_NOOP:
        return OP_NOOP;
    case INST_RESET:
        return OP_RESET;
    case INST_HALT:
        return OP_HALT;
    case INST_ADD:
        switch (asmInst.args[1].type)
        {
        case Type_Word:
            return OP_ADDC;
        case Type_Byte:
            return (Opcode)(OP_ADDC | 0x80);
        case Type_Address:
            return OP_ADDA;
        case Type_Register:
            return OP_ADD;
        }
        throw;
    case INST_SUB:
        switch (asmInst.args[1].type)
        {
        case Type_Word:
            return OP_SUBC;
        case Type_Byte:
            return (Opcode)(OP_SUBC | 0x80);
        case Type_Address:
            return OP_SUBA;
        case Type_Register:
            return OP_SUB;
        }
        throw;
    case INST_MUL:
        switch (asmInst.args[1].type)
        {
        case Type_Word:
            return OP_MULC;
        case Type_Byte:
            return (Opcode)(OP_MULC | 0x80);
        case Type_Address:
            return OP_MULA;
        case Type_Register:
            return OP_MUL;
        }
        throw;
    case INST_DIV:
        switch (asmInst.args[1].type)
        {
        case Type_Word:
            return OP_DIVC;
        case Type_Byte:
            return (Opcode)(OP_DIVC | 0x80);
        case Type_Address:
            return OP_DIVA;
        case Type_Register:
            return OP_DIV;
        }
        throw;
    case INST_CMP:
        switch (asmInst.args[1].type)
        {
        case Type_Word:
        case Type_Byte:
        case Type_Register:
            return OP_CMP;
        case Type_Address:
            return OP_CMPA;
        }
        throw;
    case INST_INC:
        switch (asmInst.args[0].type)
        {
        case Type_Byte:
        case Type_Register:
            return OP_INC;
        case Type_Word:
        case Type_Address:
            return OP_INCM;
        }
        throw;
    case INST_DEC:
        switch (asmInst.args[0].type)
        {
        case Type_Byte:
        case Type_Register:
            return OP_DEC;
        case Type_Word:
        case Type_Address:
            return OP_DECM;
        }
        throw;
    case INST_UXT:
        switch (asmInst.args[0].type)
        {
        case Type_Byte:
        case Type_Register:
            return OP_UXT;
        default:
            break;
        }
        throw;
    case INST_MOV:
        switch (asmInst.args[0].type)
        {
        case Type_Register: {
            switch (asmInst.args[1].type)
            {
            case Type_Word:
                return OP_LDC;
            case Type_Byte:
                return (Opcode)(OP_LDC | 0x80);
            case Type_Address:
                return OP_LDM; //How to set the byteMode flag here?
            case Type_Register:
                return OP_LDR;
            }
        } break;
        case Type_Address: {
            switch (asmInst.args[1].type)
            {
            case Type_Word:
                return OP_STCM;
            case Type_Byte:
                return (Opcode)(OP_STCM | 0x80);
            case Type_Register:
                return OP_STRM;
            default:
                break;
            }
        } break;
        }
        throw;
    case INST_JSR:
        return OP_JSR;
    case INST_RTN:
        return OP_RTN;
    case INST_JMP:
        return OP_JMP;
    case INST_JRZ:
    case INST_JRE:
    case INST_JRN:
    case INST_JRG:
    case INST_JRGE:
    case INST_JRL:
    case INST_JRLE: {
        Opcode opcode = (Opcode)(asmInst.args[1].type == Type_Address ? asmInst.inst + 7: asmInst.inst);
        return asmInst.args[1].type == Type_Byte ? (Opcode)(opcode | 0x80) : opcode;
    }
    case INST_PUSH:
        switch (asmInst.args[0].type)
        {
        case Type_Word:
            return OP_PUSHC;
        case Type_Byte:
            return (Opcode)(OP_PUSHC | 0x80);
        case Type_Address:
            return OP_PUSHM; // Need a way to set byte mode flag
        case Type_Register:
            return OP_PUSH;
        default:
            break;
        }
        throw;
    case INST_POP:
        switch (asmInst.args[0].type)
        {
        case Type_Word:
        case Type_Address:
            return OP_POPM; //Need a way to set byte mode flag
        case Type_Byte:
        case Type_Register:
            return OP_POP; //Need a way to set byte mode flag
        }
        throw;
    case INST_PUSHS:
        return OP_PUSHS;
    case INST_POPS:
        return OP_POPS;
    case INST_SEI:
        return OP_SEI;
    case INST_CLI:
        return OP_CLI;
    default:
        break;
    }
    throw;
}

static Instruction ParseAssemblyInstruction(const std::string& str) {
    if (str == "NOP") {
        return INST_NOOP;
    }
    else if (str == "RESET") {
        return INST_RESET;
    }
    else if (str == "HALT") {
        return INST_HALT;
    }
    else if (str == "ADD") {
        return INST_ADD;
    }
    else if (str == "SUB") {
        return INST_SUB;
    }
    else if (str == "MUl") {
        return INST_MUL;
    }
    else if (str == "DIV") {
        return INST_DIV;
    }
    else if (str == "CMP") {
        return INST_CMP;
    }
    else if (str == "INC") {
        return INST_INC;
    }
    else if (str == "DEC") {
        return INST_DEC;
    }
    else if (str == "UXT") {
        return INST_UXT;
    }
    else if (str == "MOV") {
        return INST_MOV;
    }
    else if (str == "JSR") {
        return INST_JSR;
    }
    else if (str == "RTN") {
        return INST_RTN;
    }
    else if (str == "JMP") {
        return INST_JMP;
    }
    else if (str == "JRZ") {
        return INST_JRZ;
    }
    else if (str == "JRE") {
        return INST_JRE;
    }
    else if (str == "JRN") {
        return INST_JRN;
    }
    else if (str == "JRG") {
        return INST_JRG;
    }
    else if (str == "JRGE") {
        return INST_JRGE;
    }
    else if (str == "JRL") {
        return INST_JRL;
    }
    else if (str == "JRLE") {
        return INST_JRLE;
    }
    else if (str == "PUSH") {
        return INST_PUSH;
    }
    else if (str == "POP") {
        return INST_POP;
    }
    else if (str == "PUSHS") {
        return INST_PUSHS;
    }
    else if (str == "POPS") {
        return INST_POPS;
    }
    else if (str == "SEI") {
        return INST_SEI;
    }
    else if (str == "CLI") {
        return INST_CLI;
    }
    else {
        throw;
    }
}

int main()
{
    std::string input = 
        "MOV R1 0x04 ; Load constant into register 1\n" 
        "MOV R2 R1 ; Load register 1 into register 2\n"
        "ADD R1 R2 ; Sum registers 1 and 2\n"
        "HALT";

    //1 and 2 complete

    //3. Create subroutines seperately (will be addressed in the final pass)

    //4. The final pass should connect any necassary references like variables and suroutine addresses

    //TODO:
    //Subroutines

    std::stringstream assemblyStream(input);
    std::string line;
    std::string word;
    std::vector<AsmInstruction> instructions;
    std::vector<Opcode> opcodes; //Don't actually use this, operate directly on the instructions vector

    std::vector<Byte> programText;

    while (std::getline(assemblyStream, line))
    {
        bool isFirstWord = true;
        std::stringstream lineStream(line);
        AsmInstruction asmInst{};
        std::printf("Parsing line...\n");

        while (std::getline(lineStream >> std::ws, word, ' ')) {
            std::printf((word + "\n").c_str());

            if (word == ";") {
                break; //Ignore comments
            }
            else if (isFirstWord) {
                asmInst.inst = ParseAssemblyInstruction(word);
            }
            else {
                asmInst.args.emplace_back(AsmArgument{ GetVarType(word), GetVarValue(word) });
            }

            isFirstWord = false;
        }

        instructions.push_back(std::move(asmInst));
    }

    for (auto& i : instructions)
    {
        programText.push_back(GetOpcode(i));
        for (auto& arg : i.args)
        {
            switch (arg.type)
            {
            case Type_Word:
            case Type_Address:
                programText.push_back(arg.value & 0xFF); //Little endian system (least significant portion first)
                programText.push_back(arg.value >> 8);
                break;
            case Type_Byte:
            case Type_Register:
                programText.push_back(arg.value);
                break;
            }
        }

        opcodes.push_back(GetOpcode(i)); //Debugging only
    }

    __noop;

    Memory mem{};
    CPU cpu{};

    cpu.Reset(mem);

    //Load program
    for (size_t i = 0; i < programText.size(); i++)
    {
        mem[i] = programText[i];
    }

    cpu.Execute(100, mem);

    __noop;
}
