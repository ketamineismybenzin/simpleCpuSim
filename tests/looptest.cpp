#include "../computer/cpu.cpp"

int main() {
    CPU cpu;
    Word program[] = {
        0, 0, 0, //      mov a, 0
        0, 1, 10000,//loop: mov b, 10
        8, 0,    //      inc a
        27,      //      clf       ;clear flags
        26, 0, 1,//      cmp a, b
        32, 261, //      jnz loop
        255      //      hlt
    };
    cpu.Initialize();
    cpu.Reset();
    cpu.memory.Load(program, sizeof(program)/2, 0x00FF);
    while(cpu.GetFlag(5) == 0) {
        cpu.serialio.Update();
        cpu.Execute();
    }
    if (cpu.GetReg(0) == 10000) {
        std::cout << "test succeeded!";
    } else {
        std::cout << "test failed!";
    }
    cpu.Status();
    cpu.ShowFlags();
    return 0;
}
