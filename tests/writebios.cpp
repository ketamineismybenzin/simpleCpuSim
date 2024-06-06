#include <iostream>
#include <fstream>
int main(int argc, char *argv[]) {
    unsigned short program[256] = {
        0, 1, 126,//mov b, 126 ;end of printable characters
        0, 0, 32, //mov a, 32  ;begin of printable characters
        51, 22, 0,//out $22, a ;output the current char
        8, 0,     //inc a      ;increment the charcode
        26, 0, 1, //cmp a, b   ;check if we have reached the end of the printable characters
        32, 261,  //jnz 261    ;jump back to the 'out $22, a' instruction
        255       //hlt        ;if we have reached the end of the printable characters, halt
    };
    std::ofstream file("Bios.bin", std::ios::binary);
    file.write((char *) program, 512);
    return 0;
}
