#include <iostream>
#include <fstream>
#include <string>

#define W_SIZE 65536

using Byte = unsigned char;
using Word = unsigned short;
using u32 = unsigned int;

union WordBuffer {
    Byte Bytes[1024];
    Word Words[512];
};

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
        WordBuffer filebuffer;
        std::ifstream infile("Bios.bin");
        infile.seekg(0, std::ios::end);
        size_t length = infile.tellg();
        infile.seekg(0, std::ios::beg);
        if (length > sizeof(filebuffer.Bytes)) {
            length = sizeof(filebuffer.Bytes);
        }
        infile.read((char *) filebuffer.Bytes, length);
        Load(filebuffer.Words, sizeof(filebuffer.Words), 0x00FF);
    }
    void Dump(u32 start, u32 end) {//show a portion of the ram
        for (u32 i = start; i<end; i++) {
            std::cout << std::hex << Ram[i] << ", ";
        }
    }
};

struct FLASH {
    Byte Flash[1024];
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
    Word Read(Word adres) {
        if (adres > sizeof(Flash)) {//check for adres larger then the size of Flash
            return 0;
        }
        return (Word) Flash[adres];
    }
    void Write(Word adres, Byte value) {
        if (adres > sizeof(Flash)) {
            return;
        }
        Flash[adres] = value;
    }
};
