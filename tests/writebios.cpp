#include <iostream>
#include <fstream>
int main(int argc, char *argv[]) {
    unsigned short data[256] = {0, 0, 66, 51, 22, 0, 255};//mov a, 0 out $22 a
    std::ofstream file("Bios.bin", std::ios::binary);
    file.write((char *) data, 512);
    return 0;
}
