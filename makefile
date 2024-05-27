all: main.exe

main.exe: main.o components.o
	g++ -o main.exe main.o components.o

main.o: main.cpp
	g++ -c main.cpp

components.o: components.cpp
	g++ -c components.cpp

clean:
	rm main.exe main.o components.o
