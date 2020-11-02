#include "MMU.h"

uint8_t read8(MMU *mmu, uint16_t address) {
	if (address < MAX_BIOS && bios_active) {
		return mmu->bios[address];
	}

	if (address < MAX_ROM) {
		return mmu->rom[address];
	}

	return memory[address];
}

void write8(MMU *mmu, uint16_t address, uint8_t value) {
	memory[address] = value;
}
