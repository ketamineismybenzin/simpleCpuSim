#include "../computer/cpu.cpp"

int main() {
    Word program[] = {
        0, 0, 0x6565, //mov a, 0x65
        40, 22, 0,  //out $22, a
        255         //hlt
    };
    CPU cpu;
    cpu.Initialize();
    cpu.Reset();
    cpu.memory.Load(program, sizeof(program)/2, 0x00FF);//load test program
    while(cpu.GetFlag(5)==0) {
        cpu.serialio.Update();
        cpu.Execute();
    }
    cpu.Status();
    cpu.ShowFlags();
    return 0;
}
