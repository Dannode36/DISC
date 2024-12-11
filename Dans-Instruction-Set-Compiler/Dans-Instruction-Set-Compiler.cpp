#include <iostream>

enum Instructions {
    INST_NOP = 0,
    INST_RESET,
    INST_HALT,

    INST_ADD,
    INST_SUB,
    INST_CMP,

    INST_INC,
    INST_DEC,
    INST_UXT,

    INST_LD,

    INST_JSR,
    INST_RTN,

    INST_JMP,
    INST_JMPZ,
    INST_JMPE,
    INST_JMPN,
    INST_JMPG,
    INST_JMPGE,
    INST_JMPL,
    INST_JMPLE,

    INST_PUSH,
    INST_POP,

    Count, //Keep last
};

int main()
{
    std::cout << "Hello World!\n";
}
