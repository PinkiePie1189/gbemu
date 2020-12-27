#include "util.h"

int get_bit(uint8_t dest, int bit) {
    return (dest >> bit) & 1;
}

void load_16(uint8_t *dest, uint16_t src) {
  *(dest + 1) = src >> 8;
  *dest = src & 0xFF;
}

uint16_t get_16(uint8_t *addr) { 
	return (*(addr + 1) << 8u) | (*addr);
} 
