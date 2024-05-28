simpleCpuSim
this is an ultra simple cpu simulator that simulates an imiginairy system and cpu designed by myself.

specs:
architecture is a cisc von neuman architecture.
adres bus & data bus are 16 bit.
it also has an mmu and interupts.

TODO:

add instructions.

add interupts and mmu interupts.

add stack functions (push, pop).

add some form of simulated non volatile memory using binairy files, disk images or floppy disks. - Done

add IO (keyboard - done, TTY - done, networking, vga graphics).

Curently being worked on:
instructions and interupts

BUGS:
random segfault due to non deterministic source in EXECUTE function. - fixed

how to build:
first download the repo and navigate to the root.
in the root of the project use the "make" command.
then use ./main to run the simulation

notes:

Flash.bin holds the non volatile memory and is loaded into Flash memory at initialization.
Bios.bin holds the bios wich is loaded to 0x00FF at cpu initialization.
