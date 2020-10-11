#ifndef PPU_H
#define PPU_H

#include <stdint.h>
#include "Display.h"

#define MAX_MAP 256
#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 144
#define STAT_ADDRESS 0xFF41
#define LCDC_ADDRESS 0xFF40
#define BGP_ADDRESS 0xFF47
#define LY_ADDRESS 0xFF44
#define LYC_ADDRESS 0xFF45
#define SCX_ADDRESS 0xFF43
#define SCY_ADDRESS 0xFF42
#define TILE_DATA_LO_0 0x9000
#define TILE_DATA_LO_1 0x8000

typedef struct {
    uint8_t display_enabled;
    uint8_t window_tile_map;
    uint8_t window_enabled;
    uint8_t tile_data;
    uint8_t bg_tile_map;
    uint8_t sprite_size;
    uint8_t sprites_enabled;
    uint8_t bg_display;
} LCDC;

typedef struct {
    uint8_t *memory;
    uint8_t bg_map[MAX_MAP][MAX_MAP];
    uint8_t good_pixels[MAX_MAP][MAX_MAP];
    uint8_t bg_palette[4];
    LCDC lcdc;
    Display display;
} PPU;

void init_ppu(PPU *ppu, uint8_t *memory);

// Update the various components of the PPU
void update_lcdc(PPU *ppu);
void update_palette(PPU *ppu);

void update_ppu(PPU *ppu);

void draw_screen(PPU *ppu);

void fill_bg_tile(PPU *ppu, uint8_t tile_offset, int x, int y);
void fill_bg_line(PPU *ppu); 

#endif // PPU_H
