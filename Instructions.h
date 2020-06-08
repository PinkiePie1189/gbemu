#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include <stdint.h>

void nop();
void load_16(uint8_t *dest, uint16_t src);
uint16_t get_16(uint8_t *addr);

#endif  // INSTRUCTIONS_H
