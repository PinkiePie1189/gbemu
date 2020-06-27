#ifndef REGISTERS_H
#define REGISTERS_H
#include <stdint.h>
#include <stdio.h>

typedef union {
  struct {
    uint16_t af;
    uint16_t bc;
    uint16_t de;
    uint16_t hl;
  };
  struct {
    uint8_t f;
    uint8_t a;
    uint8_t c;
    uint8_t b;
    uint8_t e;
    uint8_t d;
    uint8_t l;
    uint8_t h;
  };
} Registers;
typedef struct {
  uint64_t cpu_frequency;
  uint64_t cycles;
  uint64_t cycles_per_tick;
  uint8_t value;
  uint8_t overflowed;
  uint8_t enabled;
} Timer;

void timer_tick(Timer *timer, uint64_t cycles);
void set_timer_tick(Timer *timer, uint64_t ticks);
uint8_t timer_overflowed(Timer *timer);
uint8_t get_timer_value(Timer *timer);

// Flag manipulation
void set_carry_flag(Registers *regs, uint8_t value);
void set_half_carry_flag(Registers *regs, uint8_t value);
void set_substraction_flag(Registers *regs, uint8_t value);
void set_zero_flag(Registers *regs, uint8_t value);

// Flag getters
uint8_t get_carry_flag(Registers *regs);
uint8_t get_zero_flag(Registers *regs);
uint8_t get_half_carry_flag(Registers *regs);
uint8_t get_substraction_flag(Registers *regs);

// Debugging

void print_registers(Registers *regs);

#endif  // REGISTERS_H
