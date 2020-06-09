#ifndef CPUV2_H
#define CPUV2_H

#include <memory.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

enum CPU_FLAGS {
  FLAG_NONE = 0,
  FLAG_Z = 0b10000000,
  FLAG_S = 0b01000000,
  FLAG_H = 0b00100000,
  FLAG_C = 0b00010000,
  FLAG_ALL = 0b1111000,
};

typedef union {
  struct {
    uint16_t af;
    uint16_t bc;
    uint16_t de;
    uint16_t hl;
    uint16_t pc;
    uint16_t sp;
  };
  struct {
    union {
      uint8_t f;
      struct {
        uint8_t _unused : 4;
        uint8_t carry : 1;
        uint8_t half_carry : 1;
        uint8_t sub : 1;
        uint8_t zero : 1;
      };
    };
    uint8_t a;
    uint8_t c;
    uint8_t b;
    uint8_t e;
    uint8_t d;
    uint8_t l;
    uint8_t h;
  };
} Registers;

typedef struct CPU_T Cpu;

typedef void (*fetch_function)(Cpu *cpu);
typedef void (*exec_function)(Cpu *cpu);

typedef struct Instruction_T {
  uint8_t opcode;
  char mnemonic[16];
  uint8_t size;
  uint8_t cycles;
  fetch_function fetch;
  exec_function execute;
} Instruction;

typedef struct CPU_T {
  Registers registers;
  uint8_t memory[0x10000];

  Instruction instruction_lut[0x100];
  Instruction instruction_cb_lut[0x10];

  uint8_t is_halted;
  uint8_t opcode;

  uint16_t *dest16;
  uint8_t *dest8;

  uint16_t src16;
  uint8_t src8;

  uint8_t *fetch_reg[8];
  uint16_t *fetch_dreg[8];

  uint8_t int_enabled;
  uint8_t running;

  uint8_t wait_cycles;

} Cpu;

void init_cpu(Cpu *cpu);
void init_debug_cpu(Cpu *cpu);

char *get_register_name(uint8_t index);

uint8_t fetch8(Cpu *cpu);
uint16_t fetch16(Cpu *cpu);

uint8_t read8(Cpu *cpu);
uint16_t read16(Cpu *cpu);
uint8_t get_mask(uint8_t offset);

void purple_fetch(Cpu *cpu);
void fetch_dark_green(Cpu *cpu);
void fetch_orange(Cpu *cpu);
void fetch_dark_purple(Cpu *cpu);
void blue_fetch(Cpu *cpu);

void cycle(Cpu *cpu);
void init_instruction_lut(Cpu *cpu);
uint8_t flag_condition(Cpu *cpu);
uint8_t flag_mask(Cpu *cpu);

char *get_register_name(uint8_t index);
uint8_t *get_ind_1(Cpu *cpu, uint8_t offset);
char *get_dreg_name(uint8_t offset);

void cpu_test();

#define INSTR(op, mnem, sz, cy, ft, ex)           \
  {                                               \
    Instruction *ins = &cpu->instruction_lut[op]; \
    ins->opcode = op;                             \
    strcpy(ins->mnemonic, mnem);                  \
    ins->size = sz;                               \
    ins->cycles = cy;                             \
    ins->fetch = ft;                              \
    ins->execute = ex;                            \
  }

#define CBINSTR(op, mnem, sz, cy, ft, ex)            \
  {                                                  \
    Instruction *ins = &cpu->instruction_cb_lut[op]; \
    ins->opcode = op;                                \
    strcpy(ins->mnemonic, mnem);                     \
    ins->size = sz;                                  \
    ins->cycles = cy;                                \
    ins->fetch = ft;                                 \
    ins->execute = ex;                               \
  }

#define pop 1

#if pop == 0
#define DEBUG(format, ...) printf(format, __VA_ARGS__);
#else
#define DEBUG(format, ...)
#endif

void nop8(Cpu *cpu);
void ld8(Cpu *cpu);
void ld16(Cpu *cpu);
void dec16(Cpu *cpu);
void call(Cpu *cpu);
void load16(uint8_t *dest, uint16_t val);
uint16_t get16(uint8_t *src);
void ret(Cpu *cpu);
void rst(Cpu *cpu);
void push16(Cpu *cpu);
void pop16(Cpu *cpu);
void jp(Cpu *cpu);

#endif  // CPUV2_H
