simpleCpuSim
this is an ultra simple cpu simulator that simulates an imiginairy system and cpu designed by myself.

specs:
architecture is a cisc von neuman architecture.
adres bus & data bus are 16 bit.
it also has an mmu and interupts.

TODO:
add instructions.
add interupts and mmu interupts
add stack functions (push, pop)
add some form of simulated hard storage using binairy files, disk images or floppy disks.
add IO (keyboard, TTY, networking, vga graphics)

BUGS:
random segfault that is caused by non deterministic source in EXECUTE function
