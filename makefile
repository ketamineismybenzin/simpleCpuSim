all: main.exe test1.exe looptest.exe iotest.exe

main.exe: main.o objects/cpu.o objects/components.o
	g++ -o main.exe main.o objects/cpu.o objects/components.o

test1.exe: test1.o objects/cpu.o objects/components.o
	g++ -o test1.exe test1.o objects/cpu.o objects/components.o

looptest.exe: looptest.o objects/cpu.o objects/components.o
	g++ -o looptest.exe looptest.o objects/cpu.o objects/components.o

iotest.exe: iotest.o objects/cpu.o objects/components.o
	g++ -o iotest.exe iotest.o objects/cpu.o objects/components.o

main.o: main.cpp
	g++ -c main.cpp

test1.o: tests/insttest1.cpp
	g++ -c -o test1.o tests/insttest1.cpp

looptest.o: tests/looptest.cpp
	g++ -c tests/looptest.cpp

iotest.o: tests/iotest.cpp
	g++ -c tests/iotest.cpp

objects/cpu.o: computer/cpu.cpp
	g++ -c computer/cpu.cpp
	rm objects/cpu.o
	mv cpu.o objects/cpu.o

objects/components.o: computer/components.cpp
	g++ -c computer/components.cpp
	rm objects/components.o
	mv components.o objects/components.o

clean:
	rm main.exe test1.exe main.o test1.o looptest.o looptest.exe iotest.exe iotest.o
