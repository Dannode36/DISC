#include <iostream>
#include <sstream>
#include "../cpu.h"
#include <vector>
#include <map>
#include <span>
#include <variant>

typedef std::exception Except;
typedef std::variant<Word, std::string> AsmVar;

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
    Type_Label,
    Type_WordAddress,
    Type_ByteAddress,
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
    std::variant<Word, std::string> value;
};

struct AsmInstruction
{
    Instruction inst;
    std::vector<AsmArgument> args;
};

static Type GetVarType(const std::string& str) {
    size_t len = str.length();
    size_t addrEndBrktIndex = str.find(']');

    if (str.front() == '[' && addrEndBrktIndex != -1) { //Address
        //If the address has a number of bytes to read specified
        if (str.substr(addrEndBrktIndex, 2) == "]:") {
            if (str.substr(addrEndBrktIndex + 2) == "1") {
                return Type_ByteAddress;
            }
            else if (str.substr(addrEndBrktIndex + 2) == "2") {
                return Type_WordAddress;
            }
            throw;
        }
        else {
            return Type_WordAddress; //Default to word size
        }
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
    else if (isdigit(str.front())) {
        //Optimize constants to bytes if possible
        if (std::stoi(str) <= 0xFF) {
            return Type_Byte;
        }
        else {
            return Type_Word;
        }
    }
    else {
        return Type_Label;
    }
}

static AsmVar GetVarValue(std::string str, Type type) {
    switch (type)
    {
    case Type_Word:
    case Type_Byte: {
        if (str.substr(0, 2) == "0x") {
            return (Word)std::stoi(str.substr(3), nullptr, 16);
        }
        else {
            return (Word)std::stoi(str);
        }
    }
    case Type_ByteAddress:
    case Type_WordAddress:
        return (Word)std::stoi(str.substr(1, str.length() - 4));
    case Type_Register:
        return (Word)std::stoi(str.substr(1));
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
        case Type_Byte:
            return (Opcode)(OP_ADDC | 0x80);
        case Type_WordAddress:
            return OP_ADDA;
        case Type_ByteAddress:
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
        case Type_Byte:
            return (Opcode)(OP_SUBC | 0x80);
        case Type_WordAddress:
            return OP_SUBA;
        case Type_ByteAddress:
            return (Opcode)(OP_SUBA | 0x80);
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
        case Type_WordAddress:
            return OP_MULA;
        case Type_ByteAddress:
            return (Opcode)(OP_MULA | 0x80);
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
        case Type_WordAddress:
            return OP_DIVA;
        case Type_ByteAddress:
            return (Opcode)(OP_DIVA | 0x80);
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
        case Type_WordAddress:
            return OP_CMPA;
        case Type_ByteAddress:
            return (Opcode)(OP_CMPA | 0x80);
        }
        throw;
    case INST_INC:
        switch (asmInst.args[0].type)
        {
        case Type_Byte:
        case Type_Register:
            return OP_INC;
        case Type_Word:
        case Type_WordAddress:
            return OP_INCM;
        case Type_ByteAddress:
            return (Opcode)(OP_INCM | 0x80);
        }
        throw;
    case INST_DEC:
        switch (asmInst.args[0].type)
        {
        case Type_Byte:
        case Type_Register:
            return OP_DEC;
        case Type_Word:
        case Type_WordAddress:
            return OP_DECM;
        case Type_ByteAddress:
            return (Opcode)(OP_DECM | 0x80);
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
            case Type_WordAddress:
                return OP_LDM;
            case Type_ByteAddress:
                return (Opcode)(OP_LDM | 0x80);
            case Type_Register:
                return OP_LDR;
            }
        } break;
        case Type_WordAddress: {
            switch (asmInst.args[1].type)
            {
            case Type_Word:
                return OP_STCM;
            case Type_Byte:
                throw Except("Cannot move a byte into a word location (requires implicit zero extending)");
            case Type_WordAddress:
                return OP_STMM;
            case Type_ByteAddress:
                throw Except("Cannot move a word into a byte location (requires implicit truncation)");
            case Type_Register:
                return OP_STRM;
            }
        } break;
        case Type_ByteAddress: {
            switch (asmInst.args[1].type)
            {
            case Type_Word:
                throw Except("Cannot move a word into a byte location (requires implicit truncation)");
            case Type_Byte:
                return (Opcode)(OP_STCM | 0x80);
            case Type_WordAddress:
                throw Except("Cannot move a byte into a word location (requires implicit zero extending)");
            case Type_ByteAddress:
                return (Opcode)(OP_STMM | 0x80);
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
        if (asmInst.args[1].type == Type_WordAddress) {
            return (Opcode)(asmInst.inst + 7);
        }
        else if (asmInst.args[1].type == Type_ByteAddress) {
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
        case Type_Byte:
            return (Opcode)(OP_PUSHC | 0x80);
        case Type_WordAddress:
            return OP_PUSHM;
        case Type_ByteAddress:
            return (Opcode)(OP_PUSHM | 0x80);
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
        case Type_WordAddress:
            return OP_POPM;
        case Type_ByteAddress: {
            return (Opcode)(OP_POPM | 0x80);
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

struct AsmLabel {
    std::string name;
    std::vector<std::string> tokens;
    std::vector<AsmInstruction> instructions;
    Word memAddress;

    void Parse() {
        bool isFirstWord = true;
        bool isInstruction = false;
        AsmInstruction asmInst;

        for (size_t i = 0; i < tokens.size(); i++)
        {
            const auto& word = tokens[i];
            std::printf((word + "\n").c_str());
            
            if (word == "\n") {
                isFirstWord = true;

                if (isInstruction) { //Push complete instruction into vector
                    instructions.push_back(std::move(asmInst));
                    asmInst = {};
                }
                continue;
            }
            else if (isFirstWord) {
                isInstruction = true;
                asmInst.inst = ParseAssemblyInstruction(word);
            }
            else {
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

static Word GetLabelValue(std::string name, const std::vector<AsmLabel>& labels, const std::map<std::string, AsmData>& data) {
    auto itr = std::find_if(labels.begin(), labels.end(), 
        [&name](const AsmLabel& label) {
            return label.name == name;
        });
    if (itr != labels.end()) {
        return itr->memAddress;
    }

    auto dataItr = std::find_if(data.begin(), data.end(),
        [&name](const auto& asmData) {
            return asmData.first == name;
        });
    if (dataItr != data.end()) {
        return dataItr->second.value;
    }
}

static void ParseDataLabel(std::stringstream& ppStream, std::map<std::string, AsmData>& data) {
    std::string line;
    std::string word;

    while (std::getline(ppStream, line)) {
        std::printf("Parsing line...\n");

        bool isFirstWord = true;
        bool hasData = false;
        std::stringstream lineStream(line);

        std::string dataName;
        AsmData asmData;

        while (std::getline(lineStream >> std::ws, word, ' ')) {
            std::printf("%s\n", word.c_str());

            if (word.back() == ':') {
                if (isFirstWord) {
                    hasData = true;
                    dataName = word.substr(0, word.length() - 1); //Label
                }
                else {
                    throw Except("Labels cannot have spaces");
                }
            }
            else if (word == ";") {
                break;
            }
            else {
                asmData.type = GetVarType(word);
                asmData.value = std::get<Word>(GetVarValue(word, asmData.type));
            }

            isFirstWord = false;
        }

        if (hasData) {
            data.insert({ dataName, std::move(asmData)} );
        }
    }

}

int main()
{
    std::string line;
    std::string word;

    std::map<std::string, AsmData> data;
    std::vector<AsmLabel> labels;
    std::vector<Byte> progmem;

    std::map<size_t, std::string> labelUseIndexsUpdateMap; //Maps index of use in progmem -> name of label
    std::map<size_t, std::string> dataUseIndexsUpdateMap; //Maps index of use in progmem -> name of data

    {
        std::string input =
            "increment:\n"
            "INC R1\n"
            "RTN\n"
            ".main:\n"
            "MOV R1 0x04 ; Load constant into register 1\n"
            "MOV R2 R1 ; Load register 1 into register 2\n"
            "ADD R1 R2 ; Sum registers 1 and 2\n"
            "JSR increment\n"
            "HALT\n"
            ".data:\n"
            "constant: 20\n"
            "\n";
        std::stringstream preprocessingStream(input);

        //Preprocess: Handle labels and remove comments, tokenise
        while (std::getline(preprocessingStream, line)) {
            std::printf("Parsing line...\n");

            bool isFirstWord = true;
            bool lineHasTokens = false;
            std::stringstream lineStream(line);

            while (std::getline(lineStream >> std::ws, word, ' ')) {
                std::printf("%s\n", word.c_str());

                if (word.back() == ':') {

                    if (isFirstWord) {
                        if (word == ".data:") {
                            ParseDataLabel(preprocessingStream, data);
                            continue;
                        }
                        else {
                            labels.emplace_back(word.substr(0, word.length() - 1)); //Label
                            break;
                        }
                    }
                    else {
                        throw Except("Labels cannot have spaces");
                    }
                }
                else if (word == ";") {
                    break;
                }
                else {
                    labels.back().tokens.push_back(word);
                    lineHasTokens = true;
                }

                isFirstWord = false;
            }

            if (lineHasTokens) {
                labels.back().tokens.push_back("\n"); //For knowing which token is first on a line
            }
        }
    }

    //3. Create subroutines seperately (will be addressed in the final pass)

    //4. The final pass should connect any necassary references like variables and suroutine addresses

    //TODO:
    //Subroutines

    //Move the .data label to the front (if it exists)
    auto pivot = std::find_if(labels.begin(), labels.end(),
        [](const AsmLabel& label) -> bool {
            return label.name == ".data";
        });
    if (pivot != labels.end()) {
        std::rotate(labels.begin(), pivot, pivot + 1);
    }

    //Move the .main label to second (or front if no .data label)
    pivot = std::find_if(labels.begin(), labels.end(),
        [](const AsmLabel& label) -> bool {
            return label.name == ".main";
        });
    if (pivot != labels.end()) {
        std::rotate(labels.begin() + 1, pivot, pivot + 1);
    }
    else {
        throw Except("The program must contain the .main label");
    }

    //Write Data to progmem
    for (auto& dataPair : data)
    {
        auto& asmData = dataPair.second;

        //Update memory address
        asmData.memAddress = progmem.size();

        switch (asmData.type)
        {
        case Type_Word:
        case Type_WordAddress:
        case Type_ByteAddress:
            //Little endian system (least significant portion first)
            progmem.push_back(asmData.value & 0xFF);
            progmem.push_back(asmData.value >> 8);
            break;
        case Type_Byte:
        case Type_Register:
            progmem.push_back(asmData.value);
            break;
        }
    }

    //Instruction parsing
    for (auto& label : labels) {
        label.Parse();

        //Update memory address for the label
        //Used later for updating label values
        label.memAddress = progmem.size();

        //Write to program memory
        for (auto& i : label.instructions) {
            progmem.push_back(GetOpcode(i));
            for (auto& arg : i.args) {
                switch (arg.type)
                {
                case Type_Word:
                case Type_WordAddress:
                case Type_ByteAddress:
                    //Little endian system (least significant portion first)
                    progmem.push_back(std::get<Word>(arg.value) & 0xFF);
                    progmem.push_back(std::get<Word>(arg.value) >> 8);
                    break;
                case Type_Byte:
                case Type_Register:
                    progmem.push_back(std::get<Word>(arg.value));
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
        Word value = GetLabelValue(updatingLabelPair.second, labels, data);

        progmem[index] = value & 0xFF;
        progmem[index + 1] = value >> 8;
    }

    __noop;

    Memory mem{};
    CPU cpu{};

    cpu.Reset(mem);

    //Load program
    for (size_t i = 0; i < progmem.size(); i++) {
        mem[i] = progmem[i];
    }

    cpu.Execute(100, mem);

    __noop;
}
