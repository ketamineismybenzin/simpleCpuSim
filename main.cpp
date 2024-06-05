#include "computer/cpu.cpp"

int main(int argc, char *argv[]) {
    CPU cpu;//create cpu object and initialize it
    cpu.Initialize();
    cpu.Reset();//reset the cpu
    while(!cpu.GetFlag(5)) {//execute instructions
        cpu.serialio.Update();//check if the user has presed a key
        cpu.Execute();//execute a instruction
    }
    cpu.Status();//show the status and debugging info
    std::cout << "\nram dump\n";
    cpu.memory.Dump(0x00FF, 0x0FFF);
    std::cout << "\nflash dump\n";
    cpu.flashmem.Dump(0x0000, 0x00FF);
    cpu.flashmem.DumpFlash();
    return 0;
}
