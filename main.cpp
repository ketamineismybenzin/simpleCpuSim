#include "computer/cpu.cpp"

int main() {
    CPU cpu;
    cpu.Initialize();
    cpu.Reset();
    while(!cpu.GetFlag(5)) {
        cpu.serialio.Update();//check if the user has presed a key
        cpu.Execute();//execute a instruction
    }
    cpu.Status();
    std::cout << "\nram dump\n";
    cpu.memory.Dump(0x00FF, 0x0FFF);
    std::cout << "\nflash dump\n";
    cpu.flashmem.Dump(0x0000, 0x00FF);
    cpu.flashmem.DumpFlash();
    return 0;
}
