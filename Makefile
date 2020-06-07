CC=gcc
CFLAGS=-Wall -Wextra -g
LFLAGS=-lSDL2

build: gbemu
gbemu: gbemu.o cpu.o registers.o instructions.o error.o
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS)
gbemu.o: gbemu.c CPU.h
	$(CC) $(CFLAGS) -c $< -o $@
cpu.o: CPU.c CPU.h Registers.h Instructions.h
	$(CC) $(CFLAGS) -c $< -o $@
registers.o: Registers.c Registers.h
	$(CC) $(CFLAGS) -c $< -o $@
instructions.o: Instructions.c Instructions.h
	$(CC) $(CFLAGS) -c $< -o $@
error.o: error.c error.h
	$(CC) $(CFLAGS) -c $< -o $@
.PHONY: clean
clean:
	rm -f *.o gbemu
