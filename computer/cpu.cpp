#include <cmath>
#include "components.cpp"

struct CPU {//contains the cpu registers and functions to execute code from the memory
    MEM memory;/*0x0000 - 0x00FF is the isr vector table
	0x00 = reset vector
    0x01 = divide by 0
	0x05 = MMU segfault
	0x0
	*/
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
        if ((2*reg < sizeof(Regs)) && !GetFlag(4)) {
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
        if (input>0 && input < 127) {
            SetFlag(0);
        }
        return input;
    }
    Word Read(Word adres) {//read a word from ram
        if (GetFlag(3)) {//check if the mmu is active
            Word hardadres = adres + MMUOffset;
            if (adres > MMUMax) {//if the adres is too high we segfault
                Int(5);//mmu segfault
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
                Int(5);//mmu segfault interupt
                return;
            }
            memory.Ram[hardadres] = data;
        } else {
            memory.Ram[adres] = data;
        }
    }
    Word ReadIO(Byte adres) {//read a word from the IO bus
        if (!GetFlag(3)) {//check if the cpu is in kernel mode i.e when the usermode flag is set to 0
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
        if (!GetFlag(3)) {//check if the cpu is in kernel mode
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
	void Push(Word value) {//pushes a value to the stack
		Write(SP[GetFlag(3)], value);//write the value to memory with the sp as the adres
		SP[GetFlag(3)] += 1; //increment the sp by 1
	}
	Word Pop() {//pop's a value from the stack
		SP[GetFlag(3)] -= 1;//do the exact same proces in Push() but in reverse
		return Read(SP[GetFlag(3)]);
	}
	void Int(Byte vector) {
		SetFlag(4);//set the interupt flag
		ClearFlag(3);//go to kernel mode
		Push(PC);
		PC = Read((Word) vector);
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
		Flags = 0;
		MMUMax = 0;
		MMUOffset = 0;
    }
    void Execute() {//execute single instruction
        Word val;//val is used as temporary storage of data
        Word IR = Read(PC);//IR = Instruction Register it holds the curretn instruction
        Word op1 = Read(PC+1);
        Word op2 = Read(PC+2);
		//std::cout << "\nIR:" << IR << "\nop1:" << op1 << "\nop2" << op2 << "\n";
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
                SetReg(op1, GetReg(op1)+1);
                PC+=2;
                break;
            case 9://dec reg[op1] -= 1
                SetReg(op1, GetReg(op1)-1);
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
                if (!GetFlag(2)) {
                    PC = op1;
                } else {
                    PC += 2;
                }
                break;
            case 33://jnz reg[op1]
                if (!GetFlag(2)) {
                    PC = GetReg(op1);
                } else {
                    PC += 2;
                }
                break;
            case 34://jn op1
			    if (GetFlag(1)) {
					ClearFlag(1);
					PC = op1;
			    } else {
					PC += 2;
				}
                break;
			case 35://jn reg[op1]
			    if (GetFlag(1)) {
					ClearFlag(1);
					PC = GetReg(op1);
			    } else {
					PC += 2;
				}
                break;
			case 36://jnn op1
			    if (!GetFlag(1)) {
					PC = op1;
			    } else {
					PC += 2;
				}
                break;
			case 37://jnn reg[op1]
			    if (!GetFlag(1)) {
					PC = GetReg(op1);
			    } else {
					PC += 2;
				}
                break;
			case 38://jp op1
			    if (GetFlag(0)) {
					ClearFlag(0);
					PC = op1;
			    } else {
					PC += 2;
				}
                break;
			case 39://jp reg[op1]
			    if (GetFlag(0)) {
					ClearFlag(0);
					PC = GetReg(op1);
			    } else {
					PC += 2;
				}
                break;
			case 40://jnp op1
			    if (GetFlag(0)) {
					PC = op1;
			    } else {
					PC += 2;
				}
                break;
			case 41://jnp reg[op1]
			    if (GetFlag(0)) {
					PC = GetReg(op1);
			    } else {
					PC += 2;
				}
                break;
			case 42://call op1
			    Push(PC);//push pc to the stack
			    PC = op1;
				break;
			case 43://call reg[op1];
			    Push(PC);//push pc to the stack
				PC = GetReg(op1);//set pc to reg[op1]
				break;
			case 44://ret
			    PC = Pop();
				break;
			case 45://push reg[op1]
			    Push(GetReg(op1));
				PC += 2;
				break;
			case 46://push op1;
			    Push(op1);
				PC += 2;
				break;
			case 47://pop reg[op1]
			    SetReg(op1, Pop());
				PC += 2;
				break;
			case 48://pop wr(op1, pop())
			    Write(op1, Pop());
				PC += 2;
				break;
			case 49://int op1
			    Int(op1);
				break;
			case 50://iret
			    PC = Pop();
				SetFlag(3);
				break;
			case 51://out wrio(op1, reg[op2])
			    WriteIO((Byte) op1, GetReg(op2));
				PC += 3;
				break;
			case 52://in reg[op2] = rdio(op1)
			    SetReg(op2, ReadIO((Byte) op1));
				PC += 3;
				break;
			case 53://gfl reg[op1]
			    ParseFlags(GetReg(op1));
				PC += 2;
				break;
            case 255://hlt
                SetFlag(5);//set the halt flag
                break;
            default:
                PC+=1;
        }
		ClearFlag(4);//reset interupt flag
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
