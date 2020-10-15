#include "PPU.h"
#include "util.h"


int x_max = -1;
int y_max = -1;

void init_ppu(PPU *ppu, uint8_t *memory) {
    ppu->memory = memory;
    update_lcdc(ppu);
    create_display(&ppu->display, "Liviu e un scump", 2 * SCREEN_WIDTH, 2 * SCREEN_HEIGHT);
    memset(ppu->good_pixels, 0, MAX_MAP * MAX_MAP);
}


void update_lcdc(PPU *ppu) {
    uint8_t lcdc_byte = ppu->memory[LCDC_ADDRESS];
    ppu->lcdc.display_enabled = (lcdc_byte >> 7) & 1;
    ppu->lcdc.window_tile_map = (lcdc_byte >> 6) & 1;
    ppu->lcdc.window_enabled = (lcdc_byte >> 5) & 1;
    ppu->lcdc.tile_data = (lcdc_byte >> 4) & 1;
    ppu->lcdc.bg_tile_map = (lcdc_byte >> 3) & 1;
    ppu->lcdc.sprite_size = (lcdc_byte >> 2) & 1;
    ppu->lcdc.sprites_enabled = (lcdc_byte >> 1) & 1;
    ppu->lcdc.bg_display = (lcdc_byte) & 1;
}

void update_palette(PPU *ppu) {
    uint8_t bgp_byte = ppu->memory[BGP_ADDRESS];
    ppu->bg_palette[3] = (get_bit(bgp_byte, 6) << 1) | get_bit(bgp_byte, 7);
    ppu->bg_palette[2] = (get_bit(bgp_byte, 4) << 1) | get_bit(bgp_byte, 5);
    ppu->bg_palette[1] = (get_bit(bgp_byte, 2) << 1) | get_bit(bgp_byte, 3);
    ppu->bg_palette[0] = (get_bit(bgp_byte, 0) << 1) | get_bit(bgp_byte, 1);
}

void update_ppu(PPU *ppu) {

    update_lcdc(ppu);
    update_palette(ppu);

    if (ppu->lcdc.display_enabled) {
        uint8_t current_y = ppu->memory[LY_ADDRESS];

        if (current_y < 144) {
            fill_bg_line(ppu);
        }

        if (current_y == 153) {
            memcpy(ppu->good_pixels, ppu->bg_map, MAX_MAP * MAX_MAP);
        }

        ppu->memory[LY_ADDRESS]++;
        ppu->memory[LY_ADDRESS] %= 154;
    }
}

void fill_bg_tile(PPU *ppu, uint8_t tile_offset, int x, int y) {
    uint16_t data_address = TILE_DATA_LO_0 + (int8_t) tile_offset; 
    if (ppu->lcdc.tile_data) {
        data_address = TILE_DATA_LO_1 + tile_offset; 
    }

    for (int i = 0; i < 8; i++) {
        uint8_t lower_bits = ppu->memory[data_address + 2 * i];
        uint8_t upper_bits = ppu->memory[data_address + 2 * i + 1];
        for (int j = 7; j >= 0; j--) {
            uint8_t color = (get_bit(upper_bits, j) << 1) | (get_bit(lower_bits, j));
            ppu->bg_map[8 * x + i][8 * y + 7 - j] = color;
        }
    }
}

void fill_bg_line(PPU *ppu) {
    update_lcdc(ppu);
    uint8_t current_line = ppu->memory[LY_ADDRESS];
    // For each tile on the current line
    for (int i = 0; i < 32; i++) {

        uint16_t bg_map_lo = 0x9800;
        if (ppu->lcdc.bg_tile_map) {
            bg_map_lo = 0x9C00;
        }

        uint8_t offset = ppu->memory[bg_map_lo + 32 * (current_line / 8) + i];
        uint16_t data_address = TILE_DATA_LO_0 + (int8_t) offset * 16; 
        if (ppu->lcdc.tile_data) {
            data_address = TILE_DATA_LO_1 + offset * 16; 
        }

        uint8_t lower_bits = ppu->memory[data_address + 2 * (current_line % 8)];
        uint8_t upper_bits = ppu->memory[data_address + 2 * (current_line % 8) + 1];
        for (int j = 7; j >= 0; j--) {
            uint8_t color = (get_bit(upper_bits, j) << 1) | (get_bit(lower_bits, j));
            ppu->bg_map[current_line][8 * i + 7 - j] = color;
        }
    }
}

void draw_screen(PPU *ppu) {
    // TODO: Optimize this with a texture
    // clear_display(&ppu->display);
    update_palette(ppu);
    int colors[] = {0xFFFFFF, 0xAAAAAA, 0x555555, 0x000000};
    int scroll_y = ppu->memory[SCX_ADDRESS];
    int scroll_x = ppu->memory[SCY_ADDRESS];

    // printf("%d %d\n", scroll_x, scroll_y);


    // TODO make sure this is correct
    for (int i = scroll_x; i < (scroll_y + SCREEN_HEIGHT); i++) {
        for (int j = scroll_y; j < (scroll_x + SCREEN_WIDTH); j++) {
            int to_draw = colors[ppu->bg_palette[ppu->good_pixels[i % MAX_MAP][j % MAX_MAP]]];
            draw_pixel(&ppu->display, j - scroll_y, i - scroll_x, to_draw);
        }
    }
    present_display(&ppu->display);
}
