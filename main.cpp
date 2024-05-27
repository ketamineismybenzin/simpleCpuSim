#include <stdio.h>
#include <stdlib.h>
#include <cmath>

#define W_SIZE 65536

using Byte = unsigned char;
using Word = unsigned short;
using u32 = unsigned int;

struct MEM {
    Word ram[W_SIZE];
    void Initialize() {
        for (u32 i = 0; i<W_SIZE; i++) {
            ram[i] = 0;
        }
        ram[0] = 0x00FF;
    }
    void Load(Word *data, u32 size, u32 adres) {
        for (u32 i = 0; i<size+adres; i++) {
            ram[i+adres] = data[i];
        }
    }
    void Dump(u32 start, u32 end) {
        for (u32 i = start; i<end; i++) {
            printf("\n%d", ram[i]);
        }
    }
};

struct CPU {
    Word PC;
    Word SP[2]; //stack pointers KSP and USP
    Word Regs[4]; //user registers A, B, C and D
    Byte Flags; //flags: 0=Positive, 1=Negative, 2=Zero, 3=UserMode, 4=Interupt
    Word MMUOffset;
    Word MMUMax;
    bool GetFlag(Byte flag) {//get the value of a flag from the flag register
        return (Flags & (Byte) pow(2, (double) flag)) == pow(2, (double) flag);
    }
    void SetFlag(Byte flag) {//set a flag in the flag register
        Flags = Flags | (Byte) pow(2, (double) flag);
    }
    void ClearFlag(Byte flag) {//clear a flag in the flag register
        Flags = Flags & ~((Byte) pow(2, (double) flag));//or the register with a bitmask
    }
    Word Read(MEM& memory, Word adres) {
        if (GetFlag(3)) {//check if the mmu is active
            Word hardadres = adres + MMUOffset;
            if ((hardadres > MMUMax) | (hardadres < MMUOffset)) {
                //mmu segfault interupt
                return 0;
            }
            return memory.ram[hardadres];
        } else {
            return memory.ram[adres];
        }
    }
    void Write(MEM& memory, Word adres, Word data) {
        if (GetFlag(3)) {
            Word hardadres = adres + MMUOffset;
            if ((hardadres > MMUMax) | (hardadres < MMUOffset)) {
                //mmu segfault interupt
                return;
            }
            memory.ram[hardadres] = data;
        } else {
            memory.ram[adres] = data;
        }
    }
    Word ReadIO(Byte adres) {
        if (~GetFlag(3)) {
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
    void WriteIO(Byte adres, Word data) {
        if (~GetFlag(3)) {
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
    void Reset(MEM& memory) {
        memory.Initialize();//reset memory
        PC = memory.ram[0];
    }
    void Execute(MEM& memory) {
        Word val;
        Word IR = Read(memory, PC);
        Word op1 = Read(memory, PC+1);
        Word op2 = Read(memory, PC+2);
        switch(IR) {
            case 0:
                Regs[op1] = op2;
                PC+=3;
                break;
            case 1:
                Regs[op1] = Regs[op2];
                PC+=3;
                break;
            case 2:
                val = Read(memory, op2);
                if (~GetFlag(4)) {
                    Regs[op1] = val;
                }
                PC+=3;
                break;
            case 3:
                val = Read(memory, Regs[op2]);
                if (~GetFlag(4)) {
                    Regs[op1] = val;
                }
                PC+=3;
                break;
            case 4:
                Write(memory, op1, Regs[op2]);
                PC+=3;
                break;
            case 5:
                Write(memory, Regs[op1], Regs[op2]);
                PC+=3;
                break;
            case 6:
                val = Read(memory, op2);
                val = Read(memory, val);
                if (~GetFlag(4)) {
                    Regs[op1] = val;
                }
                PC+=3;
                break;
            default:
                PC+=1;
        }
    }
    void Status() {
        printf("\npc:%d", PC);
        printf("\nusp:%d", SP[0]);
        printf("\nksp:%d", SP[1]);
        printf("\nA:%d", Regs[0]);
    }
};

int main() {
    Word program[] = {0, 0, 1};
    CPU cpu;
    MEM mem;
    cpu.Reset(mem);
    mem.Load(program, 3, 0x00FF);
    cpu.Execute(mem);
    cpu.Status();
    mem.Dump(0, 10);
    return 0;
}
