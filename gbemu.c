#include "CPU.h"
#include "PPU.h"
#include <SDL2/SDL.h>
#include <time.h>
#include <stdint.h>
#include <stdio.h>

int main(int argc, char *argv[]) {

  if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
      fprintf(stderr, "SDL failed to initialize with error %s\n", 
              SDL_GetError());
      return -1;
  }

  if (argc < 3) {
    fprintf(stderr, "Usage: %s bios rom\n", argv[0]);
    return -1;
  }
  // argv[2] = argv[1];

  CPU cpu;
  start_cpu(&cpu);

  // Load BIOS in memory
  FILE *bios = fopen(argv[1], "rb");

  fread(cpu.memory, 0x100, sizeof(uint8_t), bios);
  fclose(bios);

  // Load ROM in memory
  FILE *rom = fopen(argv[2], "rb");

  fseek(rom, 0, SEEK_END);
  int rom_size = ftell(rom);
  // printf("%d", rom_size);
  fseek(rom, 0x0100, SEEK_SET);
  fread(cpu.memory + 0x0100, rom_size - 0x0100, sizeof(uint8_t), rom);
  // printf("!!!%d", cpu.memory[0x101]);
  fclose(rom);


  PPU ppu;
  init_ppu(&ppu, cpu.memory);
  clear_display(&ppu.display);

  clock_t last = clock();
  SDL_Event ev;
  int quit = 0;

  // Run the CPU and print serial ouptut
  while (!quit) {
    while (SDL_PollEvent(&ev)) {
        if (ev.type == SDL_QUIT) {
            quit = 1;
        }
    }

    step(&cpu);
    // printf("%d %d\n", cpu.interrupts_enabled, cpu.memory[0xFFFF]);
    if (cpu.memory[0xFF01]) {
      printf("%c", cpu.memory[0xFF01]);
      fflush(stdout);
      cpu.memory[0xFF01] = 0x00;
    }

    if (cpu.memory[LY_ADDRESS] == cpu.memory[LYC_ADDRESS]) {
        cpu.memory[STAT_ADDRESS] |= (1 << 6);
        if (cpu.interrupts_enabled) {
            cpu.memory[0xFF0F] |= (1 << 1);
        }
    }

    update_ppu(&ppu);

    if (cpu.memory[LY_ADDRESS] == 144) {
      if (cpu.interrupts_enabled) {
        cpu.memory[0xFF0F] |= 1;
      }
    }

    clock_t curr = clock();
    if (((double) curr - last) >= CLOCKS_PER_SEC * 0.016) {
        last = curr;
        draw_screen(&ppu);
    }
  }

  return 0;
}
