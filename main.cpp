#include <cmath>
#include "components.cpp"

struct CPU {//contains the cpu registers and functions to execute code from the memory
    MEM memory;
    FLASH flashmem;//flash memory contains bootloader and os
    Word PC; //Program Counter contains the current adres in memory where the cpu is executing
    Word SP[2]; //stack pointers KSP and USP
    Word Regs[4]; //user registers A, B, C and D
    Byte Flags; //flags: 0=Positive, 1=Negative, 2=Zero, 3=UserMode, 4=Interupt
    Word MMUOffset;//mmu offset
    Word MMUMax;//mmu maximum adres
    bool GetFlag(Byte flag) {//get the value of a flag from the flag register
        return (Flags & (Byte) pow(2, (double) flag)) == pow(2, (double) flag);
    }
    void SetFlag(Byte flag) {//set a flag in the flag register
        Flags = Flags | (Byte) pow(2, (double) flag);
    }
    void ClearFlag(Byte flag) {//clear a flag in the flag register
        Flags = Flags & ~((Byte) pow(2, (double) flag));//or the register with a bitmask
    }
    Word GetReg(Word reg) {//check index to prevent segfaults
        if (2*reg < sizeof(Regs)) {
            return Regs[reg];
        }
        return 0;
    }
    void SetReg(Word reg, Word value) {
        if (2*reg < sizeof(Regs)) {
            Regs[reg] = value;
        }
    }
    Word Read(Word adres) {//read a word from ram
        if (GetFlag(3)) {//check if the mmu is active
            Word hardadres = adres + MMUOffset;
            if (adres > MMUMax) {//if the adres is too high we segfault
                //mmu segfault interupt
                return 0;
            }
            return memory.Ram[hardadres];
        } else {
            return memory.Ram[adres];
        }
    }
    void Write(Word adres, Word data) {//write a word to ram
        if (GetFlag(3)) {//check if the mmu is active i.e when the usermode flag is set to 1
            Word hardadres = adres + MMUOffset;
            if (adres > MMUMax) {//if the adres is too high we segfault
                //mmu segfault interupt
                return;
            }
            memory.Ram[hardadres] = data;
        } else {
            memory.Ram[adres] = data;
        }
    }
    Word ReadIO(Byte adres) {//read a word from the IO bus
        if (~GetFlag(3)) {//check if the cpu is in kernel mode i.e when the usermode flag is set to 0
            switch(adres) {
                case 0:
                    return MMUOffset;
                    break;
                case 1:
                    return MMUMax;
                    break;
                default:
                    return 0;
            }
        }
        return 0;
    }
    void WriteIO(Byte adres, Word data) {//write a word to the IO bus
        if (~GetFlag(3)) {//check if the cpu is in kernel mode
            switch(adres) {
                case 0:
                    MMUOffset = data;
                    break;
                case 1:
                    MMUMax = data;
                    break;
                default:
                    return;
            }
        }
    }
    void Initialize() {
        memory.Initialize();
        memory.LoadBIOS();
        flashmem.LoadFlash();
    }
    void Reset() {
        PC = memory.Ram[0];//set PC with reset vector
    }
    void Execute() {//execute single instruction
        Word val;//val is used as temporary storage of data
        Word IR = Read(PC);//IR = Instruction Register it holds the curretn instruction
        Word op1 = Read(PC+1);
        Word op2 = Read(PC+2);
        switch(IR) {
            case 0:
                Regs[op1] = op2;
                PC+=3;
                break;
            default:
                PC+=1;
        }
    }
    void Status() {
        std::cout << "\npc:" << PC;
        std::cout << "\nusp:" << SP[0];
        std::cout << "\nksp:" << SP[1];
        std::cout << "\nA:" << Regs[0];
    }
};

int main() {
    Word program[] = {0, 0, 1};
    CPU cpu;
    cpu.Initialize();
    cpu.Reset();
    cpu.Execute();
    cpu.Status();//show cpu status
    std::cout << "\nramdump:\n";
    cpu.memory.Dump(255, 266);//dump 11 bytes of ram from location 0xff
    std::cout << "\nflashdump:\n";
    cpu.flashmem.Dump(0, 10);
    cpu.flashmem.DumpFlash();
    return 0;
}
