#include "../computer/cpu.cpp"

int main() {
    Word program[] = {
        0, 0, 0x66, //mov a, 0x65
        34, 22, 0,     //out $22, a
        255           //hlt
    };
    CPU cpu;
    cpu.Initialize();
    cpu.Reset();
    cpu.memory.Load(program, sizeof(program)/2, 0x00FF);//load test program
    while(!cpu.GetFlag(5)) {
        cpu.serialio.Update();
        cpu.Execute();
    }
    std::cout << "execution ended";
    cpu.memory.Dump(255,265);
    cpu.Status();
    cpu.ShowFlags();
    return 0;
}
