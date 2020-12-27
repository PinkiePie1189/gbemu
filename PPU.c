#include "PPU.h"
#include "util.h"


int x_max = -1;
int y_max = -1;

void init_ppu(PPU *ppu, MMU *mmu) {
    ppu->mmu = mmu;
    update_lcdc(ppu);
    create_display(&ppu->display, "Liviu e un scump", 2 * SCREEN_WIDTH, 2 * SCREEN_HEIGHT);
    memset(ppu->good_pixels, 0, MAX_MAP * MAX_MAP);
}


void update_lcdc(PPU *ppu) {
    uint8_t lcdc_byte = read8(ppu->mmu, LCDC_ADDRESS);
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
    uint8_t bgp_byte = read8(ppu->mmu, BGP_ADDRESS);
    uint8_t obp0_byte = read8(ppu->mmu, OBP0_ADDRESS);
    uint8_t obp1_byte = read8(ppu->mmu, OBP1_ADDRESS);

    for (int i = 0; i < 4; i++) {
        ppu->bg_palette[3 - i] = (get_bit(bgp_byte, 6 - 2 * i) << 1) | get_bit(bgp_byte, 7 - 2 * i);
        ppu->obp_0[3 - i] = (get_bit(obp0_byte, 6 - 2 * i) << 1) | get_bit(obp0_byte, 7 - 2 * i);
        ppu->obp_1[3 - i] = (get_bit(obp1_byte, 6 - 2 * i) << 1) | get_bit(obp1_byte, 7 - 2 * i);
    }
}

void update_ppu(PPU *ppu) {

    update_lcdc(ppu);
    update_palette(ppu);

    // oam_search(ppu);
    if (ppu->lcdc.display_enabled) {
        uint8_t current_y = read8(ppu->mmu, LY_ADDRESS);

        if (current_y < 144) {
            fill_bg_line(ppu);
            if (ppu->lcdc.sprites_enabled) {
                fill_sprite_line(ppu);
            }
        }

        if (current_y == 153) {
            memcpy(ppu->good_pixels, ppu->bg_map, MAX_MAP * MAX_MAP);
        }

        write8(ppu->mmu, LY_ADDRESS, (current_y + 1) % 154);

    }
}

void fill_bg_tile(PPU *ppu, uint8_t tile_offset, int x, int y) {
    uint16_t data_address = TILE_DATA_LO_0 + (int8_t) tile_offset; 
    if (ppu->lcdc.tile_data) {
        data_address = TILE_DATA_LO_1 + tile_offset; 
    }

    for (int i = 0; i < 8; i++) {
        uint8_t lower_bits = read8(ppu->mmu, data_address + 2 * i);
        uint8_t upper_bits = read8(ppu->mmu, data_address + 2 * i + 1);
        for (int j = 7; j >= 0; j--) {
            uint8_t color = (get_bit(upper_bits, j) << 1) | (get_bit(lower_bits, j));
            ppu->bg_map[8 * x + i][8 * y + 7 - j] = color;
        }
    }
}

void fill_bg_line(PPU *ppu) {
    update_lcdc(ppu);
    uint8_t current_line = read8(ppu->mmu, LY_ADDRESS);
    // For each tile on the current line
    for (int i = 0; i < 32; i++) {

        uint16_t bg_map_lo = 0x9800;
        if (ppu->lcdc.bg_tile_map) {
            bg_map_lo = 0x9C00;
        }

        uint8_t offset = read8(ppu->mmu, bg_map_lo + 32 * (current_line / 8) + i);
        uint16_t data_address = TILE_DATA_LO_0 + (int8_t) offset * 16; 
        if (ppu->lcdc.tile_data) {
            data_address = TILE_DATA_LO_1 + offset * 16; 
        }

        uint8_t lower_bits = read8(ppu->mmu, data_address + 2 * (current_line % 8));
        uint8_t upper_bits = read8(ppu->mmu, data_address + 2 * (current_line % 8) + 1);

        for (int j = 7; j >= 0; j--) {
            uint8_t color = ppu->bg_palette[(get_bit(upper_bits, j) << 1) | (get_bit(lower_bits, j))];
            ppu->bg_map[current_line][8 * i + 7 - j] = color;
        }
    }
}

void draw_screen(PPU *ppu) {
    // TODO: Optimize this with a texture
    clear_display(&ppu->display);
    update_palette(ppu);
    int colors[] = {0xFFFFFF, 0xAAAAAA, 0x555555, 0x000000};
    int scroll_y = read8(ppu->mmu, SCX_ADDRESS);
    int scroll_x = read8(ppu->mmu, SCY_ADDRESS);

    // printf("%d %d\n", scroll_x, scroll_y);


    // TODO make sure this is correct
    for (int i = scroll_x; i < (scroll_y + SCREEN_HEIGHT); i++) {
        for (int j = scroll_y; j < (scroll_x + SCREEN_WIDTH); j++) {
            int to_draw = colors[ppu->good_pixels[i % MAX_MAP][j % MAX_MAP]];
            draw_pixel(&ppu->display, j - scroll_y, i - scroll_x, to_draw);
        }
    }
    present_display(&ppu->display);
}

void oam_search(PPU *ppu) {
    update_lcdc(ppu);
    uint8_t sprite_height = 8 * (ppu->lcdc.sprite_size + 1);
    uint8_t current_line = read8(ppu->mmu, LY_ADDRESS);

    // Hardcode the Tetris arrow
    // before we have DMA for debug
    // ppu->memory[OAM_START] = 0x80;
    // ppu->memory[OAM_START + 1] = 0x10;
    // ppu->memory[OAM_START + 2] = 0x58;
    // ppu->memory[OAM_START + 3] = 0x00;
    ppu->cnt_sprites = 0;
    for (int i = OAM_START; i <= OAM_END && ppu->cnt_sprites < 10; i += 4) {
        uint8_t sprite_y = read8(ppu->mmu, i);
        uint8_t sprite_x = read8(ppu->mmu, i + 1);
        if (sprite_x > 0 && (current_line + 16) >= sprite_y && (current_line + 16) < (sprite_y + sprite_height)) {
            ppu->line_sprites[ppu->cnt_sprites++] = i;
        }
    }
}

void fill_sprite_line(PPU *ppu) {

    update_lcdc(ppu);
    uint8_t sprite_height = 8 * (ppu->lcdc.sprite_size + 1);
    uint8_t current_line = read8(ppu->mmu, LY_ADDRESS);

    printf("%x\n", sprite_height);

    // For each sprite found during OAM Search
    for (int i = 0; i < ppu->cnt_sprites; i++) {
        uint8_t sprite_y = read8(ppu->mmu, ppu->line_sprites[i]);
        uint8_t sprite_x = read8(ppu->mmu, ppu->line_sprites[i] + 1);

        sprite_y += 16;
        sprite_x += 8;

        uint8_t tile_offset = read8(ppu->mmu, ppu->line_sprites[i] + 2);
        uint8_t properties = read8(ppu->mmu, ppu->line_sprites[i] + 3);


        // Premature optimisation is the root of all evil
        uint16_t data_address = TILE_DATA_LO_1 + 2 * sprite_height * tile_offset;

        uint8_t lower_bits = read8(ppu->mmu, data_address + 2 * (current_line - sprite_y));
        uint8_t upper_bits = read8(ppu->mmu, data_address + 2 * (current_line - sprite_y) + 1);

        for (int j = 7; j >= 0; j--) {
            if (sprite_x + j < MAX_MAP) {
                uint8_t color = (get_bit(upper_bits, j) << 1) | (get_bit(lower_bits, j));
                if (!get_bit(properties, 4)) {
                    color = ppu->obp_0[color];
                } else {
                    color = ppu->obp_1[color];
                }
                ppu->bg_map[current_line][sprite_x + 7 - j] = color;
            }
        }

    }
}
