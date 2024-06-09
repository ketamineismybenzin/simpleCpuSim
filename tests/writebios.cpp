#include <iostream>
#include <fstream>
int main(int argc, char *argv[]) {
    unsigned short program[256] = {28, 269, 72, 101, 108, 108, 111, 87, 111, 114, 108, 100, 46, 0, 0, 0, 257, 0, 2, 0, 3, 1, 0, 51, 22, 1, 8, 0, 26, 1, 2, 32, 275, 255};
    std::ofstream file("Bios.bin", std::ios::binary);
    file.write((char *) program, 512);
    return 0;
}
