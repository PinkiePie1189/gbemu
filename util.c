#include "util.h"

int get_bit(uint8_t dest, int bit) {
    return (dest >> bit) & 1;
}
