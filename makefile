all: main.exe test1.exe

main.exe: main.o objects/cpu.o objects/components.o
	g++ -o main.exe main.o objects/cpu.o objects/components.o

test1.exe: test1.o objects/cpu.o objects/components.o
	g++ -o test1.exe test1.o objects/cpu.o objects/components.o

main.o: main.cpp
	g++ -c main.cpp

test1.o: tests/insttest1.cpp
	g++ -c -o test1.o tests/insttest1.cpp

objects/cpu.o: computer/cpu.cpp
	g++ -c computer/cpu.cpp
	mv cpu.o objects/cpu.o

objects/components.o: computer/components.cpp
	g++ -c computer/components.cpp
	mv components.o objects/components.o

clean:
	rm main.exe test1.exe main.o test1.o
