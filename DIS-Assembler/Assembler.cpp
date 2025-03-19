#include <iostream>
#include <sstream>
#include <fstream>
#include "../DIS-Emulator/cpu.h"
#include <vector>
#include <map>
#include <span>
#include <variant>
#include <algorithm>

//Forward declarations & typedef's

typedef std::exception Except;
typedef std::variant<Word, std::string> AsmVar;

enum Instruction;
enum Type;
struct AsmData;
struct AsmArgument;
struct AsmInstruction;
static Type GetVarType(const std::string& str);
static AsmVar GetVarValue(std::string str, Type type);
static Opcode GetOpcode(AsmInstruction& asmInst);
static Instruction ParseAssemblyInstruction(const std::string& str);

//Definitions

enum Instruction {
    INST_NOOP = 0,
    INST_RESET,         //Reset CPU registers and status
    INST_HALT,          //Stop CPU
    INST_ADD,           //Add
    INST_SUB,           //Subtract
    INST_MUL,           //Multiply
    INST_DIV,           //Division
    //INST_CMP,         //Compare
    INST_INC,           //Increment
    INST_DEC,           //Decrement
    INST_UXT,           //Zero extend
    INST_MOV,           //Move
    INST_JSR = 0x40,    //Jump to subroutine
    INST_RTN,           //Return from subroutine
    INST_JMP,           //Jump program counter
    INST_JRZ,           //Jump if register is zero
    INST_JRE,           //Jump if register is equal
    INST_JRN,           //Jump if register is not equal
    INST_JRG,           //Jump if register is greater
    INST_JRGE,          //Jump if register is greater or equal
    INST_JRL,           //Jump if register is less
    INST_JRLE,          //Jump if register is less or equal
    INST_PUSH,          //Push onto stack
    INST_POP,           //Pop from stack
    INST_PUSHS,         //Push status onto stack
    INST_POPS,          //Pop status from stack
    INST_SETI,          //Set interrupt
    INST_CLRI,          //Clear interrupt

    Count, //Keep last
};
enum Type
{
    Type_Word,
    Type_Label,
    Type_Address,
    Type_AddressRegister,
    Type_Register,
};
struct AsmData {
    Type type;
    Word value;
    Word memAddress;
};
struct AsmArgument
{
    Type type;
    AsmVar value;
};
struct AsmInstruction
{
    Instruction inst;
    std::vector<AsmArgument> args;
};
struct AsmLabel {
    std::string name;
    std::vector<std::string> tokens;
    std::vector<AsmInstruction> instructions;
    Word memAddress;

    void Parse() {
        bool isFirstWord = true;
        bool isInstruction = false;
        AsmInstruction asmInst;

        std::printf(("Label: " + name + "\n").c_str());

        for (size_t i = 0; i < tokens.size(); i++)
        {
            const auto& word = tokens[i];

            if (word == "\n") {
                std::printf("\n");
                isFirstWord = true;

                if (isInstruction) { //Push complete instruction into vector
                    instructions.push_back(std::move(asmInst));
                    asmInst = {};
                }
                continue;
            }
            else if (isFirstWord) {
                std::printf((word + "\n").c_str());

                asmInst.inst = ParseAssemblyInstruction(word);
                isInstruction = true;
            }
            else {
                std::printf((word + "\n").c_str());

                Type type = GetVarType(word);
                /*if (type == Type_Label) {
                    labelUseIndexsUpdateMap.insert({})
                }*/

                asmInst.args.emplace_back(AsmArgument{ type, GetVarValue(word, type) });
            }

            isFirstWord = false;
        }
    }
};

const std::map<std::string, Instruction> instructionAliases {
    //x86 style
    { "noop", INST_NOOP},
    { "reset", INST_RESET},
    { "halt", INST_HALT},
    { "add", INST_ADD},
    { "sub", INST_SUB},
    { "mul", INST_MUL},
    { "div", INST_DIV},
    { "inc", INST_INC},
    { "dec", INST_DEC},
    { "uxt", INST_UXT},
    { "mov", INST_MOV},
    { "jsr", INST_JSR},
    { "rtn", INST_RTN},
    { "jmp", INST_JMP},
    { "jrz", INST_JRZ},
    { "jre", INST_JRE},
    { "jrn", INST_JRN},
    { "jrg", INST_JRG},
    { "jrge", INST_JRGE},
    { "jrl", INST_JRL},
    { "jrle", INST_JRLE},
    { "push", INST_PUSH},
    { "pop", INST_POP},
    { "pushs", INST_PUSHS},
    { "pops", INST_POPS},
    { "sei", INST_SETI},
    { "cli", INST_CLRI},

    //Dan style
    { "noop", INST_NOOP},
    { "reset", INST_RESET},
    { "halt", INST_HALT},
    { "add", INST_ADD},
    { "sub", INST_SUB},
    { "mul", INST_MUL},
    { "div", INST_DIV},
    { "inc", INST_INC},
    { "dec", INST_DEC},
    { "extend", INST_UXT},
    { "move", INST_MOV},
    { "jsr", INST_JSR},
    { "return", INST_RTN},
    { "jump", INST_JMP},
    { "jzero", INST_JRZ},
    { "jequal", INST_JRE},
    { "jnequal", INST_JRN},
    { "jrg", INST_JRG},
    { "jrge", INST_JRGE},
    { "jrl", INST_JRL},
    { "jrle", INST_JRLE},
    { "push", INST_PUSH},
    { "pop", INST_POP},
    { "pushs", INST_PUSHS},
    { "pops", INST_POPS},
    { "seti", INST_SETI},
    { "cleari", INST_CLRI},
};

static Byte GetRegisterByName(const std::string& name) {
    if (std::isdigit(name[1])) {
        return (Byte)std::stoi(name.substr(1));
    }
    else {
        if (name == "rpc") {
            return (Byte)7;
        }
        else if (name == "rsp") {
            return (Byte)8;
        }
    }
    throw Except("Invalid register name");
}
static Type GetVarType(const std::string& str) {
    size_t len = str.length();
    size_t addrEndBrktIndex = str.find(']');

    if (str.front() == '[' && addrEndBrktIndex != -1) { //Address
        //If the address has a number of bytes to read specified
        if (isdigit(str[2])) { //If the address is a constant
            return Type_Address;
        }
        else {
            return Type_AddressRegister; //Address must be read from a register
        }
    }
    else if (str.front() == 'r') {
        return Type_Register;
    }
    else if (str.substr(0, 2) == "0x") {
        return Type_Word;
    }
    else if (isdigit(str.front())) {
        return Type_Word;
    }
    else {
        return Type_Label;
    }
}
static AsmVar GetVarValue(std::string str, Type type) {
    switch (type)
    {
    case Type_Word: {
        if (str.substr(0, 2) == "0x") {
            return (Word)std::stoi(str.substr(3), nullptr, 16);
        }
        else {
            return (Word)std::stoi(str);
        }
    }
    case Type_Address:
        return (Word)std::stoi(str.substr(1, str.length() - 2)); //Should be the constant portion
    case Type_AddressRegister:
        return GetRegisterByName(str.substr(1, str.length() - 2)); //Should be the register name portion
    case Type_Register: {
        return GetRegisterByName(str);
    }
    case Type_Label:
        return str;
    default:
        throw;
    }
}
static Opcode GetOpcode(AsmInstruction& asmInst) {
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
        case Type_Address:
            return OP_ADDA;
        case Type_AddressRegister:
            return (Opcode)(OP_ADDA | 0x80);
        case Type_Register:
            return OP_ADD;
        }
        throw;
    case INST_SUB:
        switch (asmInst.args[1].type)
        {
        case Type_Word:
            return OP_SUBC;
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
        case Type_Address:
            return OP_DIVA;
        case Type_Register:
            return OP_DIV;
        }
        throw;
    /*case INST_CMP:
        switch (asmInst.args[1].type)
        {
        case Type_Word:
        case Type_Byte:
        case Type_Register:
            return OP_CMP;
        case Type_Address:
            return OP_CMPA;
        case Type_ByteAddress:
            return (Opcode)(OP_CMPA | 0x80);
        }
        throw;*/
    case INST_INC:
        switch (asmInst.args[0].type)
        {
        case Type_Register:
            return OP_INC;
        }
        throw;
    case INST_DEC:
        switch (asmInst.args[0].type)
        {
        case Type_Register:
            return OP_DEC;
        }
        throw;
    case INST_UXT:
        switch (asmInst.args[0].type)
        {
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
            case Type_Address:
                return OP_LDM;
            case Type_AddressRegister:
                return (Opcode)(OP_LDM | 0x80);
            case Type_Register:
                return OP_LDR;
            }
        } break;
        case Type_Address: {
            switch (asmInst.args[1].type)
            {
            case Type_Word:
                return OP_STCM;
            case Type_Address:
            case Type_AddressRegister:
                throw Except("Cannot move memory into memory");
            case Type_Register:
                return OP_STRM;
            }
        } break;
        case Type_AddressRegister: {
            switch (asmInst.args[1].type)
            {
            case Type_Word:
                return (Opcode)(OP_STCM | 0x80);
            case Type_Address:
            case Type_AddressRegister:
                throw Except("Cannot move memory into memory");
            case Type_Register:
                return (Opcode)(OP_STRM | 0x80);
            }
        } break;
        }
        throw Except("Cannot move a value into constant or program memory");
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
        if (asmInst.args[1].type == Type_Address) {
            return (Opcode)(asmInst.inst + 7);
        }
        else if (asmInst.args[1].type == Type_AddressRegister) {
            return (Opcode)((asmInst.inst + 7) | 0x80);
        }
        else {
            return (Opcode)asmInst.inst;
        }
    }
    case INST_PUSH:
        switch (asmInst.args[0].type)
        {
        case Type_Word:
            return OP_PUSHC;
        case Type_Register:
            return OP_PUSH;
        }
        throw;
    case INST_POP:
        switch (asmInst.args[0].type)
        {
        case Type_Word:
        case Type_Register:
            return OP_POP;
        }
        throw;
    case INST_PUSHS:
        return OP_PUSHS;
    case INST_POPS:
        return OP_POPS;
    case INST_SETI:
        return OP_SEI;
    case INST_CLRI:
        return OP_CLI;
    default:
        break;
    }
    throw Except("ERROR: No matching opcode found for instruction");
}
static Instruction ParseAssemblyInstruction(const std::string& str) {
    if (instructionAliases.contains(str)) {
        return instructionAliases.at(str);
    }
    throw Except("ERROR: Invalid assembly instruction");
}
static Word GetLabelValue(std::string name, const std::vector<AsmLabel>& labels) {
    auto itr = std::find_if(labels.begin(), labels.end(), 
        [&name](const AsmLabel& label) {
            return label.name == name;
        });
    if (itr != labels.end()) {
        return itr->memAddress;
    }
    throw Except(("Label does not exist: " + name).c_str());
}

static void ParseAssembly(const std::string& input, std::vector<Byte>& progmem) {
    std::vector<AsmLabel> labels;

    std::map<size_t, std::string> labelUseIndexsUpdateMap; //Maps index of use in progmem -> name of label

    std::string line;
    std::string word;
    std::stringstream preprocessingStream(input);
    while (std::getline(preprocessingStream, line)) { //Tokenise: Handle labels and remove comments, tokenise
        bool isFirstWord = true;
        std::stringstream lineStream(line);

        while (std::getline(lineStream >> std::ws, word, ' ')) {
            if (word.back() == ':') {

                if (isFirstWord) {
                    labels.emplace_back(word.substr(0, word.length() - 1)); //Label
                    break;
                }
                else {
                    throw Except("Labels cannot have spaces");
                }
            }
            else if (word == ";") {
                break;
            }
            else {
                //Lower string
                std::transform(word.begin(), word.end(), word.begin(), [](unsigned char c) { return std::tolower(c); });
                labels.back().tokens.push_back(word);
            }

            isFirstWord = false;
        }

        if (labels.back().tokens.size() != 0) {
            labels.back().tokens.push_back("\n"); //For knowing which token is first on a line
        }
    }

    //Move the .main label to the front
    auto pivot = std::find_if(labels.begin(), labels.end(),
        [](const AsmLabel& label) -> bool {
            return label.name == ".main";
        });
    if (pivot != labels.end()) {
        std::rotate(labels.begin(), pivot, pivot + 1);
    }
    else {
        throw Except("The program must contain the .main label");
    }

    //Instruction parsing
    for (auto& label : labels) {
        label.Parse();

        //Update memory address for the label
        //Used later for updating label values
        label.memAddress = static_cast<Word>(progmem.size());

        //Write to program memory
        for (auto& i : label.instructions) {
            progmem.push_back(GetOpcode(i));
            for (auto& arg : i.args) {
                switch (arg.type)
                {
                case Type_Word:
                case Type_Address:
                case Type_AddressRegister:
                    //Little endian system (least significant portion first)
                    progmem.push_back(std::get<Word>(arg.value) & 0xFF);
                    progmem.push_back(std::get<Word>(arg.value) >> 8);
                    break;
                case Type_Register:
                    progmem.push_back((Byte)std::get<Word>(arg.value));
                    break;
                case Type_Label:
                    labelUseIndexsUpdateMap.insert({
                        progmem.size(),
                        std::get<std::string>(arg.value)
                        });

                    //Placeholder value
                    progmem.push_back(0);
                    progmem.push_back(0);
                    break;
                }
            }
        }
    }

    //Update label values
    for (auto& updatingLabelPair : labelUseIndexsUpdateMap) {
        size_t index = updatingLabelPair.first;
        Word value = GetLabelValue(updatingLabelPair.second, labels);

        progmem[index] = value & 0xFF;
        progmem[index + 1] = value >> 8;
    }
}
static void SerializeToDisk(std::vector<Byte>& data, std::string filename) {
    std::printf("Writing program to disk...\n");
    std::ofstream outfile(filename, std::ios::out | std::ios::binary);
    outfile.write(reinterpret_cast<const char*>(data.data()), data.size()); //this is head ache
    std::printf(("Finished writing program to disk: \"" + filename + "\"\n").c_str());
}

int main(int argc, char* argv[])
{
    std::string input;
    if (argc > 1) {
        std::ifstream stream(argv[1]);
        std::stringstream buffer;
        buffer << stream.rdbuf();
        input = buffer.str();
    }
    else {
        input =
            "increment:\n"
            "inc r1\n"
            "rtn\n"
            ".main:\n"
            "mov r1 0x04 ; Load constant into register 1\n"
            "mov r2 r1 ; Load register 1 into register 2\n"
            "add r1 r2 ; Sum registers 1 and 2\n"
            "jsr increment\n"
            "halt\n"
            "\n";
    }

    std::vector<Byte> progmem;
    ParseAssembly(input, progmem);
    SerializeToDisk(progmem, "program.disa");

    Memory mem{};
    CPU cpu{};
    cpu.Reset(mem);

    //Load program
    errno_t progLoadErrVal = memcpy_s(mem.Data, Memory::MEM_SIZE, progmem.data(), progmem.size());
    if (progLoadErrVal != 0) {
        throw Except("ERROR: Failed to load program. Not enough memory");
    }

    cpu.Execute(100, mem);
    cpu.CoreDump();

    __noop;
}
