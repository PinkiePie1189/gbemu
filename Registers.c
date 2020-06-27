#include "Registers.h"

void set_carry_flag(Registers *regs, uint8_t value) {
  if (value) {
    regs->f |= (1u << 4);
  } else {
    regs->f &= ~(1u << 4);
  }
}

void set_half_carry_flag(Registers *regs, uint8_t value) {
  if (value) {
    regs->f |= (1u << 5);
  } else {
    regs->f &= ~(1u << 5);
  }
}

void set_substraction_flag(Registers *regs, uint8_t value) {
  if (value) {
    regs->f |= (1u << 6);
  } else {
    regs->f &= ~(1u << 6);
  }
}

void set_zero_flag(Registers *regs, uint8_t value) {
  if (value) {
    regs->f |= (1u << 7);
  } else {
    regs->f &= ~(1u << 7);
  }
}

uint8_t get_carry_flag(Registers *regs) { return (regs->f >> 4) & 1; }

uint8_t get_zero_flag(Registers *regs) { return (regs->f >> 7) & 1; }

uint8_t get_half_carry_flag(Registers *regs) { return (regs->f >> 5) & 1; }

uint8_t get_substraction_flag(Registers *regs) { return (regs->f >> 6) & 1; }

void print_registers(Registers *regs) {
  printf("Registers:\n");
  printf("AF = %.4hx\n", regs->af);
  printf("BC = %.4hx\n", regs->bc);
  printf("DE = %.4hx\n", regs->de);
  printf("HL = %.4hx\n", regs->hl);
}

void timer_tick(Timer *timer, uint64_t cycles) {
  if (timer->enabled) {
    timer->cycles += cycles;
    if (timer->cycles > timer->cycles_per_tick) {
      if (timer->value + (timer->cycles / timer->cycles_per_tick) > 0xFF) {
        timer->overflowed = 1;
      }
      timer->value += timer->cycles / timer->cycles_per_tick;
      timer->cycles = timer->cycles % timer->cycles_per_tick;
    }
  }
}

void set_timer_tick(Timer *timer, uint64_t ticks) {
  timer->cycles_per_tick = ticks;
}

uint8_t timer_overflowed(Timer *timer) {
  if (timer->overflowed) {
    timer->overflowed = 0;
    return 1;
  }
  return 0;
}

uint8_t get_timer_value(Timer *timer) { return timer->value; }
