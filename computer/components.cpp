#include <iostream>
#include <fstream>
#include <string>
#include <conio.h>

#define W_SIZE 65536

using Byte = unsigned char;
using Word = unsigned short;
using u32 = unsigned int;

struct MEM { //this holds the ram memory
    Word Ram[W_SIZE];
    void Initialize() {//initializes array with 0's
        for (u32 i = 0; i<W_SIZE; i++) {
            Ram[i] = 0;
        }
        Ram[0] = 0x00FF;//initialize reset vector
    }
    void Load(Word *data, u32 size, u32 adres) {//load some data into the memory
        for (u32 i = 0; i<size+adres; i++) {
            Ram[i+adres] = data[i];
        }
    }
    void LoadBIOS() {//load the Bios.bin file into ram
	    Word bios[256];//buffer for the bios
		std::ifstream input("Bios.bin", std::ios::binary);
		if (!input) {//check if the file is opened
			std::cerr << "Error: Could not open \"Bios.bin\"\n";
		}
		input.unsetf(std::ios::skipws);//read 512 bytes into the ram memory
		input.read(reinterpret_cast<char*>(bios), 512);
		Load(bios, 256, 0x00FF);//load the bios into ram at adres 0x00FF where the reset vector points
    }
    void Dump(u32 start, u32 end) {//show a portion of the ram
        for (u32 i = start; i<end; i++) {
            std::cout << std::hex << Ram[i] << ", ";
        }
    }
};

struct FLASH {
    Byte Flash[1024];
    Word AdresRegister;
    void LoadFlash() {
        std::ifstream infile("Flash.bin");
        infile.seekg(0, std::ios::end);
        size_t length = infile.tellg();
        infile.seekg(0, std::ios::beg);
        if (length > sizeof(Flash)) {
            length = sizeof(Flash);
        }
        infile.read((char *) Flash, length);
    }
    void Dump(u32 start, u32 end) {
        if ((start > sizeof(Flash)) | (end > sizeof(Flash))) {
            return;
        }
        for (u32 i = start; i < end; i++) {
            std::cout << std::hex << Flash[i] << ", ";
        }
    }
    void DumpFlash() {
        std::ofstream outfile("Flash.bin", std::ios::binary);
        outfile.write((char *) Flash, sizeof(Flash));
    }
    Word Read() {
        if (AdresRegister > sizeof(Flash)) {//check for adres larger then the size of Flash
            return 0;
        }
        return (Word) Flash[AdresRegister];
    }
    void Write(Byte value) {
        if (AdresRegister > sizeof(Flash)) {
            return;
        }
        Flash[AdresRegister] = value;
    }
};

struct SERIALIO {
    Byte InputRegister;//holds the last typed char
    Byte StatusRegister;
    void Write(Byte charcode) {
        std::cout << (char) charcode;
    }
    void Update() {
        if (kbhit()) {
            InputRegister = getch();
            StatusRegister = 1;
        }
    }
};
