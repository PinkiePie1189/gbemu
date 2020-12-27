CC=gcc

CFLAGS=-Wall -Wextra -Wpedantic -g

LFLAGS=-lSDL2



build: gbemu

gbemu: gbemu.o cpu.o registers.o error.o ppu.o display.o util.o mmu.o

	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS)

gbemu.o: gbemu.c CPU.h

	$(CC) $(CFLAGS) -c $< -o $@

cpu.o: CPU.c CPU.h Registers.h
	$(CC) $(CFLAGS) -c $< -o $@
mmu.o: MMU.c MMU.h
	$(CC) $(CFLAGS) -c $< -o $@
ppu.o: PPU.c PPU.h
	$(CC) $(CFLAGS) -c $< -o $@
display.o: Display.c Display.h
	$(CC) $(CFLAGS) -c $< -o $@
registers.o: Registers.c Registers.h
	$(CC) $(CFLAGS) -c $< -o $@
instructions.o: Instructions.c Instructions.h
	$(CC) $(CFLAGS) -c $< -o $@
error.o: error.c error.h
	$(CC) $(CFLAGS) -c $< -o $@
util.o: util.c util.h
	$(CC) $(CFLAGS) -c $< -o $@

run_tests:
	timeout 70 ./gbemu 01.gb 01.gb || true
	timeout 70 ./gbemu 02.gb 02.gb || true
	timeout 70 ./gbemu 03.gb 03.gb || true
	timeout 70 ./gbemu 04.gb 04.gb || true
	timeout 70 ./gbemu 05.gb 05.gb || true
	timeout 70 ./gbemu 06.gb 06.gb || true
	timeout 70 ./gbemu 07.gb 07.gb || true
	timeout 70 ./gbemu 08.gb 08.gb || true
	timeout 75 ./gbemu 09.gb 09.gb || true
	timeout 70 ./gbemu 10.gb 10.gb || true
	timeout 75 ./gbemu 11.gb 11.gb || true
.PHONY: clean
clean:
	rm -f *.o gbemu


