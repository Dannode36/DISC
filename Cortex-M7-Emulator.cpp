#include "cpu.h"

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
    mem[0x0005] = 0x00; //Value (8 - 15)
    mem[0x0006] = 0x00; //Address (0 - 7)
    mem[0x0007] = 0x00; //Address (8 - 15)
    mem[0x0008] = OP_HALT;

    cpu.Execute(129, mem); //This simply increment loop takes 129 cycles x_x (JRN eats up 6 cycles)

    __noop; //For breakpoint debugging
}
