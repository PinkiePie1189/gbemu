#define SCRATCHPAD 1
#if SCRATCHPAD == 1
#include "CPUV2.h"
#else
#include "CPU.h"
#endif
#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
#if SCRATCHPAD == 1
  Cpu *cpu = malloc(sizeof(Cpu));

  init_debug_cpu(cpu);
  init_cpu(cpu);
  init_instruction_lut(cpu);

  cpu->registers.pc = 0x1234;

  cpu->memory[0x1234] = 0xCD;
  cpu->memory[0x1234 + 1] = 0x10;
  cpu->memory[0x1234 + 2] = 0x11;

  cpu->memory[0x1234 + 3] = 0xF7;
  cpu->memory[0x1110] = 0xC9;

  cpu->registers.bc = 137;
  cpu->memory[0x30] = 0xC9;

  int inst = 7;
  while (inst--) {
    cycle(cpu);
  }

  printf("%02X\n", cpu->registers.pc);

  return 0;
#else

  if (argc < 2) {
    fprintf(stderr, "Usage: %s bios rom\n", argv[0]);
    return -1;
  }
  argv[2] = argv[1];

  CPU cpu;
  start_cpu(&cpu);

  // Load BIOS in memory
  FILE *bios = fopen(argv[1], "rb");
  file_check_error(bios, argv[1]);

  fread(cpu.memory, 0x100, sizeof(uint8_t), bios);
  fclose(bios);

  // Load ROM in memory
  FILE *rom = fopen(argv[2], "rb");
  file_check_error(rom, argv[1]);

  fseek(rom, 0, SEEK_END);
  int rom_size = ftell(rom);
  fseek(rom, 0, SEEK_SET);
  fread(cpu.memory, rom_size, sizeof(uint8_t), rom);
  fclose(rom);

  // Run the CPU and print serial ouptut
  while (1) {
    step(&cpu);
    if (cpu.memory[0xFF01]) {
      printf("%c", cpu.memory[0xFF01]);
      fflush(stdout);
      cpu.memory[0xFF01] = 0x00;
    }
  }

  return 0;
#endif
}
