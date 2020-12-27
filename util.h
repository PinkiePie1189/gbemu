#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>


int get_bit(uint8_t dest, int bit);
void load_16(uint8_t *dest, uint16_t src);
uint16_t get_16(uint8_t *addr);

#endif // UTIL_H
