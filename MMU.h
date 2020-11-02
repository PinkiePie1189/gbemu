#ifndef MMU_H
#define MMU_H

#define MAX_BIOS 0x100
#define MAX_ROM 0x7FFF
#define MAX_MEMORY 0x10000

#include <stdint.h>
#include <stdio.h>

#include "error.h"
typedef struct {
	union {
		uint8_t bios[MAX_BIOS];
		uint8_t rom[MAX_ROM];
		uint8_t memory[MAX_MEMORY];
	};
	uint8_t bios_active;
} MMU;

uint8_t read8(MMU *mmu, uint16_t address);
void write8(MMU *mmu, uint16_t address, uint8_t value);

void load_bios(MMU *mmu, const char *path) {
	FILE *bios = fopen(path, "rb");
	file_check_error(bios, path);

	fread(mmu->bios, MAX_BIOS, sizeof(uint8_t), bios);
	bios_active = 1;

	fclose(bios);
}

void load_rom(MMU *mmu, const char *path) {
	FILE *rom = fopen(path, "rb");
	file_check_error(rom, path);

	fseek(rom, 0, SEEK_END);
	int rom_size = ftell(rom);
	fseek(rom, 0, SEEK_SET);
	fread(mmu->rom, rom_size, sizeof(uint8_t), rom);

	fclose(rom);
}
#endif // MMU_H