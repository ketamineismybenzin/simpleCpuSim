#include <cmath>
#include "components.cpp"

struct CPU {//contains the cpu registers and functions to execute code from the memory
    MEM memory;
    FLASH flashmem;//flash memory contains bootloader and os
    SERIALIO serialio;//io
    Word PC; //Program Counter contains the current adres in memory where the cpu is executing
    Word SP[2]; //stack pointers KSP and USP
    Word Regs[4]; //user registers A, B, C and D
    Byte Flags; //flags: 0=Positive, 1=Negative, 2=Zero, 3=UserMode, 4=Interupt, 5=Halt
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
        if ((2*reg < sizeof(Regs)) & ~GetFlag(4)) {
            Regs[reg] = value;
        }
    }
    Word ParseFlags(Word input) {//sets the flags based on the input of the function
        if (input==0) {
            SetFlag(2);
        }
        if (input>=127) {
            SetFlag(1);
        }
        if (input>0 & input < 127) {
            SetFlag(0);
        }
        return input;
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
        if (GetFlag(3)==0) {//check if the cpu is in kernel mode i.e when the usermode flag is set to 0
            switch(adres) {
                case 0:
                    return MMUOffset;
                case 1:
                    return MMUMax;
                case 10:
                    return flashmem.AdresRegister;
                case 11:
                    return flashmem.Read();
                case 20://read the io status register
                    return serialio.StatusRegister;
                case 21://read the io input register
                    return serialio.InputRegister;
                default:
                    return 0;
            }
        }
        return 0;
    }
    void WriteIO(Byte adres, Word data) {//write a word to the IO bus
        if (GetFlag(3)==0) {//check if the cpu is in kernel mode
            switch(adres) {
                case 0:
                    MMUOffset = data;
                    break;
                case 1:
                    MMUMax = data;
                    break;
                case 10://flash adres register
                    flashmem.AdresRegister = data;
                    break;
                case 11://flash data register
                    flashmem.Write(data);
                    break;
                case 20://write to the status register
                    serialio.StatusRegister = (Byte) data;
                    break;
                case 22://write to the output register and send 1 byte
                    serialio.Write((Byte) data);
                    break;
                default:
                    return;
            }
        }
        return;
    }
    void Initialize() {//initializes all the arrays
        memory.Initialize();
        memory.LoadBIOS();
        flashmem.LoadFlash();
    }
    void Reset() {
        PC = memory.Ram[0];//set PC with reset vector
        SP[0] = 0;
        SP[1] = 0;
        Regs[0] = 0;
        Regs[1] = 0;
        Regs[2] = 0;
        Regs[3] = 0;
    }
    void Execute() {//execute single instruction
        Word val;//val is used as temporary storage of data
        Word IR = Read(PC);//IR = Instruction Register it holds the curretn instruction
        Word op1 = Read(PC+1);
        Word op2 = Read(PC+2);
        switch(IR) {//lookup what instruction has to be executed
            case 0://mov reg[op1] = op2
                SetReg(op1, op2);
                PC+=3;
                break;
            case 1://mov reg[op1] = reg[op2]
                SetReg(op1, GetReg(op2));
                PC+=3;
                break;
            case 2://mov reg[op1] = rd(op2)
                SetReg(op1, Read(op2));
                PC+=3;
                break;
            case 3://mov reg[op1] = rd(reg[op2])
                SetReg(op1, Read(GetReg(op2)));
                PC+=3;
                break;
            case 4://mov reg[op1] = rd(rd(op2))
                SetReg(op1, Read(Read(op2)));//get the value of a pointer
                PC+=3;
                break;
            case 5://mov wr(op2, reg[op1])
                Write(op2, GetReg(op1));
                PC+=3;
                break;
            case 6://mov wr(reg[op2], reg[op1])
                Write(GetReg(op2), GetReg(op1));
                PC+=3;
                break;
            case 7://mov wr(rd(op2), reg[op1])
                Write(Read(op2), GetReg(op1));//write to a pointer
                PC+=3;
                break;
            case 8://inc reg[op1] += 1
                SetReg(op1, ParseFlags(GetReg(op1)+1));
                PC+=2;
                break;
            case 9://dec reg[op1] -= 1
                SetReg(op1, ParseFlags(GetReg(op1)-1));
                PC+=2;
                break;
            case 10://add reg[op1] = reg[op1] + reg[op2]
                SetReg(op1, ParseFlags(GetReg(op1) + GetReg(op2)));
                PC+=3;
                break;
            case 11://add reg[op1] = reg[op1] + rd(op2)
                SetReg(op1, ParseFlags(GetReg(op1) + Read(op2)));
                PC+=3;
                break;
            case 12://add reg[op1] = reg[op1] + rd(reg[op2])
                SetReg(op1, ParseFlags(GetReg(op1) + Read(GetReg(op2))));
                PC+=3;
                break;
            case 13://sub reg[op1] = reg[op1] - reg[op2]
                SetReg(op1, ParseFlags(GetReg(op1) - GetReg(op2)));
                PC+=3;
                break;
            case 14://sub reg[op1] = reg[op1] - rd(op2)
                SetReg(op1, ParseFlags(GetReg(op1) + Read(op2)));
                PC+=3;
                break;
            case 15://sub reg[op1] = reg[op1] - rd(reg[op2])
                SetReg(op1, ParseFlags(GetReg(op1) - Read(GetReg(op2))));
                PC+=3;
                break;
            case 16://mul reg[op1] = reg[op1] * reg[op2]
                SetReg(op1, ParseFlags(GetReg(op1) * GetReg(op2)));
                PC+=3;
                break;
            case 17://mul reg[op1] = reg[op1] * rd(op2)
                SetReg(op1, ParseFlags(GetReg(op1) * Read(op2)));
                PC+=3;
                break;
            case 18://div reg[op1] = reg[op1] / reg[op2]
                SetReg(op1, ParseFlags(GetReg(op1) / GetReg(op2)));
                PC+=3;
                break;
            case 19://div reg[op1] = reg[op1] / rd(op2)
                SetReg(op1, ParseFlags(GetReg(op1) / Read(op2)));
                PC+=3;
                break;
            case 20://and reg[op1] = reg[op1] & reg[op2]
                SetReg(op1, ParseFlags(GetReg(op1) & GetReg(op2)));
                PC+=3;
                break;
            case 21://or reg[op1] = reg[op1] | reg[op2]
                SetReg(op1, ParseFlags(GetReg(op1) | GetReg(op2)));
                PC+=3;
                break;
            case 22://xor reg[op1] = reg[op1] ^ reg[op2]
                SetReg(op1, ParseFlags(GetReg(op1) ^ GetReg(op2)));
                PC+=3;
                break;
            case 23://shl reg[op1] = reg[op1] << reg[op2]
                SetReg(op1, ParseFlags(GetReg(op1) << GetReg(op2)));
                PC+=3;
                break;
            case 24://shr reg[op1] = reg[op1] >> reg[op2]
                SetReg(op1, ParseFlags(GetReg(op1) >> GetReg(op2)));
                PC+=3;
                break;
            case 25://not reg[op1] = ~reg[op1]
                SetReg(op1, ParseFlags(~GetReg(op1)));
                PC+=2;
                break;
            case 26://cmp reg[op1] == reg[op2]
                ParseFlags(GetReg(op1) - GetReg(op2));
                PC+=3;
                break;
            case 27://clf clear flags
                ClearFlag(0);
                ClearFlag(1);
                ClearFlag(2);
                PC+=1;
                break;
            case 28://jmp op1
                PC = op1;
                break;
            case 29://jmp reg[op1]
                PC = GetReg(op1);
                break;
            case 30://jz op1
                if (GetFlag(2)) {
                    ClearFlag(2);
                    PC = op1;
                } else {
                    PC += 2;
                }
                break;
            case 31://jz reg[op1]
                if (GetFlag(2)) {
                    ClearFlag(2);
                    PC = GetReg(op1);
                } else {
                    PC += 2;
                }
                break;
            case 32://jnz op1
                if (GetFlag(2)==0) {
                    ClearFlag(2);
                    PC = op1;
                } else {
                    PC += 2;
                }
                break;
            case 33://jnz reg[op1]
                if (GetFlag(2)==0) {
                    ClearFlag(2);
                    PC = GetReg(op1);
                } else {
                    PC += 2;
                }
                break;
            case 34://temporary io instruction
                std::cout << "\nyeet1";
                PC+=3;
                break;
            case 255://hlt
                SetFlag(5);//set the halt flag
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
        std::cout << "\nB:" << Regs[1];
        std::cout << "\nC:" << Regs[2];
        std::cout << "\nD:" << Regs[3];
    }
    void ShowFlags() {
        std::cout << "\nfl0:" << GetFlag(0);
        std::cout << "\nfl1:" << GetFlag(1);
        std::cout << "\nfl2:" << GetFlag(2);
        std::cout << "\nfl3:" << GetFlag(3);
        std::cout << "\nfl4:" << GetFlag(4);
        std::cout << "\nfl5:" << GetFlag(5);
    }
};
