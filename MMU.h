#ifndef MMU_H
#define MMU_H

#define MAX_BIOS 0x100
#define MAX_ROM 0x7FFF
#define MAX_MEMORY 0x10000
#define BIOS_DEACTIVATE 0xFF50

#include <stdint.h>
#include <stdio.h>

#include "error.h"
#include "util.h"

typedef struct {
	uint8_t bios[MAX_BIOS];
	union {
		uint8_t rom[MAX_ROM];
		uint8_t memory[MAX_MEMORY];
	};
	uint8_t bios_active;
} MMU;

uint8_t read8(MMU *mmu, uint16_t address);
uint16_t read16(MMU *mmu, uint16_t address);
void write8(MMU *mmu, uint16_t address, uint8_t value);
void write16(MMU *mmu, uint16_t address, uint16_t value);

void load_bios(MMU *mmu, char *path);
void load_rom(MMU *mmu, char *path);
#endif // MMU_H