#include "MMU.h"

uint8_t read8(MMU *mmu, uint16_t address) {

	// printf("READ from %hx. Bios active: %d\n", address, mmu->bios_active);
	if (address < MAX_BIOS && mmu->bios_active) {
		return mmu->bios[address];
	}

	/*
	if (address < MAX_ROM) {
		return mmu->rom[address];
	}*/

	return mmu->memory[address];
}

uint16_t read16(MMU *mmu, uint16_t address) {
	return (read8(mmu, address + 1) << 8) | read8(mmu, address);
}

void write8(MMU *mmu, uint16_t address, uint8_t value) {
	// printf("WROTE to %hx. Bios active: %d\n", address, mmu->bios_active);
	if (address == BIOS_DEACTIVATE) {
		mmu->bios_active = 0;
		printf("Adios la bios! Liviu e martor\n");
		fflush(stdout);
	}

	mmu->memory[address] = value;
}

void write16(MMU *mmu, uint16_t address, uint16_t value) {
	write8(mmu, address, value & 0xFF);
	write8(mmu, address + 1, value >> 8);
}

void load_bios(MMU *mmu, char *path) {
	FILE *bios = fopen(path, "rb");
	file_check_error(bios, path);

	fread(mmu->bios, MAX_BIOS, sizeof(uint8_t), bios);
	mmu->bios_active = 1;

	fclose(bios);
}

void load_rom(MMU *mmu, char *path) {
	FILE *rom = fopen(path, "rb");
	file_check_error(rom, path);

	fseek(rom, 0, SEEK_END);
	int rom_size = ftell(rom);
	fseek(rom, 0, SEEK_SET);
	fread(mmu->rom, rom_size, sizeof(uint8_t), rom);

	fclose(rom);
}
