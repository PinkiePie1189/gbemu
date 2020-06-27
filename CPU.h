#ifndef CPU_H
#define CPU_H

#include <stdint.h>

#include "Registers.h"

#define MAX_MEMORY 0xFFFF + 1

typedef struct {
  Registers registers;
  uint16_t pc;
  uint16_t sp;
  uint8_t memory[MAX_MEMORY];
  int is_halted;
  int is_stopped;
  int interrupts_enabled;
  uint8_t has_jumped;
  uint64_t frequency;
  uint64_t elapsed_cycles;
  uint8_t opcode;
  uint64_t instr_per_frame;
  Timer divider;
  Timer counter;
} CPU;

void start_cpu(CPU *cpu);
void step(CPU *cpu);

// Fetches
uint8_t fetch_8(CPU *cpu);
uint16_t fetch_16(CPU *cpu);

// Writes
void write_8(CPU *cpu, uint16_t address, uint8_t value);
void write_16(CPU *cpu, uint16_t address, uint16_t value);

// Arithmetic operations that set flags
void add_8(CPU *cpu, uint8_t *dest, uint8_t src);
void inc_8(CPU *cpu, uint8_t *dest);
void adc_8(CPU *cpu, uint8_t *dest, uint8_t src);

void sub_8(CPU *cpu, uint8_t *dest, uint8_t src);
void dec_8(CPU *cpu, uint8_t *dest);
void sbc_8(CPU *cpu, uint8_t *dest, uint8_t src);

void add_16(CPU *cpu, uint16_t *dest, uint16_t src);
void sub_16(CPU *cpu, uint16_t *dest, uint16_t src);

// Rotates and shifts
void rlc_8(CPU *cpu, uint8_t *dest);
void rrc_8(CPU *cpu, uint8_t *dest);
void rl_8(CPU *cpu, uint8_t *dest);
void rr_8(CPU *cpu, uint8_t *dest);

// Logical operations that set flags
void and_8(CPU *cpu, uint8_t *dest, uint8_t src);
void or_8(CPU *cpu, uint8_t *dest, uint8_t src);
void xor_8(CPU *cpu, uint8_t *dest, uint8_t src);
void cp_8(CPU *cpu, uint8_t *dest, uint8_t src);

// Stack operations
void pop_16(CPU *cpu, uint16_t *dest);
void push_16(CPU *cpu, uint16_t dest);

// Jumps and calls
void jr(CPU *cpu, int8_t offset);
void jp(CPU *cpu, uint16_t address);

void call(CPU *cpu, uint16_t address);

// RESET
void rst(CPU *cpu, uint8_t dest);

// CB Operations
void bit_test(CPU *cpu, uint8_t dest, uint8_t bit);
void sla_8(CPU *cpu, uint8_t *dest);
void sra_8(CPU *cpu, uint8_t *dest);
void swap_8(CPU *cpu, uint8_t *dest);
void srl_8(CPU *cpu, uint8_t *dest);
void res_8(CPU *cpu, uint8_t *dest, uint8_t bit);
void set_8(CPU *cpu, uint8_t *dest, uint8_t bit);

// DAA in all his might
void daa_8(CPU *cpu, uint8_t *dest);

#endif  // CPU_H
