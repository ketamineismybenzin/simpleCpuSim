#include <iostream>
#include <cmath>
#include <fstream>
#include <string>

#define W_SIZE 65536

using Byte = unsigned char;
using Word = unsigned short;
using u32 = unsigned int;

struct MEM { //this holds the ram memory
    Word Ram[W_SIZE];
    void Initialize() {//initializes array with 0's
        for (u32 i = 0; i<W_SIZE; i++) {
            Ram[i] = 0;
        }
        Ram[0] = 0x00FF;//initialize reset vector
    }
    void Load(Word *data, u32 size, u32 adres) {//load some data into the memory
        for (u32 i = 0; i<size+adres; i++) {
            Ram[i+adres] = data[i];
        }
    }
    void LoadFile() {
        std::ifstream infile("Bios.bin");
        infile.seekg(0, std::ios::end);
        size_t length = infile.tellg();
        infile.seekg(0, std::ios::beg);
        if (length > sizeof(Ram)) {
            length = sizeof(Ram);
        }
        infile.read((char *) Ram, length);
    }
    void Dump(u32 start, u32 end) {//show a portion of the ram
        for (u32 i = start; i<end; i++) {
            std::cout << "\n" << Ram[i];
        }
    }
};

struct CPU {//contains the cpu registers and functions to execute code from the memory
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
    Word Read(MEM& memory, Word adres) {//read a word from ram
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
    void Write(MEM& memory, Word adres, Word data) {//write a word to ram
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
    void Reset(MEM& memory) {
        memory.Initialize();//reset memory
        PC = memory.Ram[0];//set PC with reset vector
    }
    void Execute(MEM& memory) {//execute single instruction
        Word val;//val is used as temporary storage of data
        Word IR = Read(memory, PC);//IR = Instruction Register it holds the curretn instruction
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
        std::cout << "\npc:" << PC;
        std::cout << "\nusp:" << SP[0];
        std::cout << "\nksp:" << SP[1];
        std::cout << "\nA:" << Regs[0];
    }
};

struct FLASH {
    Byte Flash[1024];
    void Load() {
        std::ifstream infile("Flash.bin");
        infile.seekg(0, std::ios::end);
        size_t length = infile.tellg();
        infile.seekg(0, std::ios::beg);
        if (length > sizeof(Flash)) {
            length = sizeof(Flash);
        }
        infile.read((char *) Flash, length);
    }
    void Dump(u32 start, u32 end) {
        if ((start > sizeof(Flash)) | (end > sizeof(Flash))) {
            return;
        }
        for (u32 i = start; i < end; i++) {
            std::cout << (Word) Flash[i] << ", ";
        }
    }
    void DumpFile() {
        std::ofstream outfile("Flash.bin", std::ios::binary);
        outfile.write((char *) Flash, sizeof(Flash));
    }
    Word Read(Word adres) {
        if (adres > sizeof(Flash)) {//check for adres larger then the size of Flash
            return 0;
        }
        return Flash[adres];
    }
    void Write(Word adres, Byte value) {
        if (adres > sizeof(Flash)) {
            return;
        }
        Flash[adres] = value;
    }
};

int main() {
    Word program[] = {0, 0, 1};
    CPU cpu;
    MEM mem;
    FLASH flashmem;
    flashmem.Load();
    flashmem.Dump(0, 20);
    flashmem.Write(5, 65);
    flashmem.DumpFile();
    cpu.Reset(mem);
    mem.Load(program, 3, 0x00FF);
    cpu.Execute(mem);
    cpu.Status();
    mem.Dump(255, 266);
    return 0;
}
