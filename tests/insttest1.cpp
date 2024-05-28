#include "../computer/cpu.cpp"

int main() {//tests the mov instruction
    CPU cpu;
    Word testprogram[] = {0, 0, 10, 0, 1, 15, 0, 2, 20, 0, 3, 25};
    cpu.Initialize();
    cpu.Reset();
    cpu.memory.Load(testprogram, sizeof(testprogram)/2, 0x00FF);
    while(cpu.PC < (255 + sizeof(testprogram)/2)) {
        cpu.serialio.Update();
        cpu.Execute();
    }
    if (cpu.Regs[0] == 10 & cpu.Regs[1] == 15 & cpu.Regs[2] == 20 & cpu.Regs[3] == 25) {
        std::cout << "Test succeeded!\n";
    } else {
        std::cout << "Test Failed!\n";
    }
    cpu.Status();
    std::cout << "\nramdump:\n";
    cpu.memory.Dump(255, 255+sizeof(testprogram)/2);
    return 0;
}
