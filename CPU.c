#include "CPU.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "Instructions.h"
#define DEBUG_SBC 0
#define DEBUG_SP 0

static const uint8_t op_cycles[256] = {
    1, 3, 2, 2, 1, 1, 2, 1, 5, 2, 2, 2, 1, 1, 2, 1, 1, 3, 2, 2, 1, 1, 2, 1,
    3, 2, 2, 2, 1, 1, 2, 1, 2, 3, 2, 2, 1, 1, 2, 1, 2, 2, 2, 2, 1, 1, 2, 1,
    2, 3, 2, 2, 3, 3, 3, 1, 2, 2, 2, 2, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 2, 2, 2, 2, 2, 2, 1, 2,
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
    2, 3, 3, 4, 3, 4, 2, 4, 2, 4, 3, 0, 3, 6, 2, 4, 2, 3, 3, 0, 3, 4, 2, 4,
    2, 4, 3, 0, 3, 0, 2, 4, 3, 3, 2, 0, 0, 4, 2, 4, 4, 1, 4, 0, 0, 0, 2, 4,
    3, 3, 2, 1, 0, 4, 2, 4, 3, 2, 4, 1, 0, 0, 2, 4};

static const uint8_t op_cycles_br[256] = {
    1, 3, 2, 2, 1, 1, 2, 1, 5, 2, 2, 2, 1, 1, 2, 1, 1, 3, 2, 2, 1, 1, 2, 1,
    3, 2, 2, 2, 1, 1, 2, 1, 3, 3, 2, 2, 1, 1, 2, 1, 3, 2, 2, 2, 1, 1, 2, 1,
    3, 3, 2, 2, 3, 3, 3, 1, 3, 2, 2, 2, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 2, 2, 2, 2, 2, 2, 1, 2,
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
    5, 3, 4, 4, 6, 4, 2, 4, 5, 4, 4, 0, 6, 6, 2, 4, 5, 3, 4, 0, 6, 4, 2, 4,
    5, 4, 4, 0, 6, 0, 2, 4, 3, 3, 2, 0, 0, 4, 2, 4, 4, 1, 4, 0, 0, 0, 2, 4,
    3, 3, 2, 1, 0, 4, 2, 4, 3, 2, 4, 1, 0, 0, 2, 4};

static const uint8_t op_cycles_cb[256] = {
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 3, 2,
    2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2,
    2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2,
    2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2};

// freq: 1048576
//
static const uint64_t timer_dividers[] = {
    1048576 / 1024 / 4,
    1048576 / 65536 / 4,
    1048576 / 16384 / 4,
    1048576 / 4096 / 4,
};

void start_cpu(CPU *cpu) {
  cpu->frequency = 1048576;
  cpu->instr_per_frame = 166666;

  cpu->pc = 0x0000;
  cpu->is_halted = 0;
  cpu->is_stopped = 0;
  cpu->interrupts_enabled = 0;

  // Simulate bios
  cpu->registers.af = 0x11B0;
  cpu->registers.bc = 0x0013;
  cpu->registers.de = 0x00D8;
  cpu->registers.hl = 0x014D;

  cpu->sp = 0xFFFE;

  // Initialise timers
  cpu->divider.enabled = 1;
  cpu->divider.cycles_per_tick = 4096;

  // Setup memory
  memset(cpu->memory, 0, sizeof(cpu->memory));

  cpu->memory[0xFF04] = 0xAB;
  cpu->memory[0xFF10] = 0x80;
  cpu->memory[0xFF11] = 0xBF;
  cpu->memory[0xFF12] = 0xF3;
  cpu->memory[0xFF14] = 0xBF;
  cpu->memory[0xFF16] = 0x3F;
  cpu->memory[0xFF19] = 0xBF;
  cpu->memory[0xFF1A] = 0x7F;
  cpu->memory[0xFF1B] = 0xFF;
  cpu->memory[0xFF1C] = 0x9F;
  cpu->memory[0xFF1E] = 0xBF;
  cpu->memory[0xFF20] = 0xFF;
  cpu->memory[0xFF23] = 0xBF;
  cpu->memory[0xFF24] = 0x77;
  cpu->memory[0xFF25] = 0xF3;
  cpu->memory[0xFF26] = 0xF1;
  cpu->memory[0xFF40] = 0x91;
  cpu->memory[0xFF47] = 0xFC;
  cpu->memory[0xFF48] = 0xFF;
  cpu->memory[0xFF49] = 0xFF;
}

void step(CPU *cpu) {
  /*cpu->instructions_executed++;
  if (cpu->instructions_executed == 1000) {
      fprintf(stderr, "Liviu is our king!\n");
      fflush(stderr);
      cpu->instructions_executed = 0;
  }*/
  if (cpu->has_jumped) {
    cpu->elapsed_cycles += op_cycles_br[cpu->opcode];
    timer_tick(&cpu->divider, op_cycles_br[cpu->opcode]);
    timer_tick(&cpu->counter, op_cycles_br[cpu->opcode]);
  } else {
    cpu->elapsed_cycles += op_cycles[cpu->opcode];
    timer_tick(&cpu->divider, op_cycles[cpu->opcode]);
    timer_tick(&cpu->counter, op_cycles[cpu->opcode]);
  }

  // printf("%x\n", cpu->pc);
  // fflush(stdout);
  cpu->has_jumped = 0;

  // Divider register

  cpu->memory[0xFF04] = get_timer_value(&cpu->divider);
  cpu->counter.enabled = (cpu->memory[0xFF07] & 0b100) > 0;
  set_timer_tick(&cpu->counter, timer_dividers[cpu->memory[0xFF07] & 0b11]);

  if (timer_overflowed(&cpu->counter)) {
    uint8_t mask = 1 << 2;
    cpu->memory[0xFF0F] |= mask;

    cpu->counter.value = cpu->memory[0xFF06];
  }

  // Check interrupts
  if (cpu->memory[0xFF0F] != 0) {
    cpu->is_halted = 0;
  }
  if (cpu->interrupts_enabled) {
    uint16_t int_vector = 0x40;
    for (uint8_t mask = 1; mask <= 1 << 4; mask <<= 1) {
      if ((cpu->memory[0xFF0F] & mask) && (cpu->memory[0xFFFF] & mask)) {
        cpu->interrupts_enabled = 0;
        cpu->memory[0xFF0F] &= ~mask;
        call(cpu, int_vector);
        break;
      }
      int_vector += 0x8;
    }
  }

  // Current opcode to execute
  if (cpu->is_halted) {
    return;
  }

  cpu->opcode = fetch_8(cpu);
  uint8_t opcode = cpu->opcode;

  switch (opcode) {
    case 0x00: {
      // NOP
      break;
    }

    case 0x01: {
      // LD BC, u16
      cpu->registers.bc = fetch_16(cpu);
      break;
    }

    case 0x02: {
      // LD (BC), A
      write_8(cpu, cpu->registers.bc, cpu->registers.a);
      break;
    }

    case 0x03: {
      // INC BC
      cpu->registers.bc++;
      break;
    }

    case 0x04: {
      // INC B
      inc_8(cpu, &cpu->registers.b);
      break;
    }

    case 0x05: {
      // DEC B
      dec_8(cpu, &cpu->registers.b);
      break;
    }

    case 0x06: {
      // LD B, u8
      cpu->registers.b = fetch_8(cpu);
      break;
    }

    case 0x07: {
      // RLCA
      rlc_8(cpu, &cpu->registers.a);
      set_zero_flag(&cpu->registers, 0);
      break;
    }

    case 0x08: {
      // LD (u16), SP
      write_16(cpu, fetch_16(cpu), cpu->sp);
      break;
    }

    case 0x09: {
      // ADD HL, BC
      add_16(cpu, &cpu->registers.hl, cpu->registers.bc);
      break;
    }

    case 0x0A: {
      // LD A, (BC)
      uint16_t address = cpu->registers.bc;
      cpu->registers.a = cpu->memory[address];
      break;
    }

    case 0x0B: {
      // DEC BC
      cpu->registers.bc--;
      break;
    }

    case 0x0C: {
      // INC C
      inc_8(cpu, &cpu->registers.c);
      break;
    }

    case 0x0D: {
      // DEC C
      dec_8(cpu, &cpu->registers.c);
      break;
    }

    case 0x0E: {
      // LD C, u8
      cpu->registers.c = fetch_8(cpu);
      break;
    }

    case 0x0F: {
      // RRCA
      rrc_8(cpu, &cpu->registers.a);
      set_zero_flag(&cpu->registers, 0);
      break;
    }

    case 0x10: {
      // STOP 00
      cpu->is_stopped = 1;
      fetch_8(cpu);  // nullbyte
      break;
    }

    case 0x11: {
      // LD DE, u16
      cpu->registers.de = fetch_16(cpu);
      break;
    }

    case 0x12: {
      // LD (DE), A
      write_8(cpu, cpu->registers.de, cpu->registers.a);
      break;
    }

    case 0x13: {
      // INC DE
      cpu->registers.de++;
      break;
    }

    case 0x14: {
      // INC D
      inc_8(cpu, &cpu->registers.d);
      break;
    }

    case 0x15: {
      // DEC D
      dec_8(cpu, &cpu->registers.d);
      break;
    }

    case 0x16: {
      // LD D, u8
      cpu->registers.d = fetch_8(cpu);
      break;
    }

    case 0x17: {
      // RLA
      rl_8(cpu, &cpu->registers.a);
      set_zero_flag(&cpu->registers, 0);
      break;
    }

    case 0x18: {
      // JR i8
      jr(cpu, (int8_t)fetch_8(cpu));
      break;
    }

    case 0x19: {
      // ADD HL, DE
      add_16(cpu, &cpu->registers.hl, cpu->registers.de);
      break;
    }

    case 0x1A: {
      // LD A, (DE)
      cpu->registers.a = cpu->memory[cpu->registers.de];
      break;
    }

    case 0x1B: {
      // DEC DE
      cpu->registers.de--;
      break;
    }

    case 0x1C: {
      // INC E
      inc_8(cpu, &cpu->registers.e);
      break;
    }

    case 0x1D: {
      // DEC E
      dec_8(cpu, &cpu->registers.e);
      break;
    }

    case 0x1E: {
      // LD E, u8
      cpu->registers.e = fetch_8(cpu);
      break;
    }

    case 0x1F: {
      // RRA
      rr_8(cpu, &cpu->registers.a);
      set_zero_flag(&cpu->registers, 0);
      break;
    }

    case 0x20: {
      // JR NZ, i8
      int8_t offset = (int8_t) fetch_8(cpu);
      if (!get_zero_flag(&cpu->registers)) {
        cpu->has_jumped = 1;
        jr(cpu, offset);
      }
      break;
    }

    case 0x21: {
      // LD HL, u16
      cpu->registers.hl = fetch_16(cpu);
      break;
    }

    case 0x22: {
      // LD (HL+), A
      write_8(cpu, cpu->registers.hl++, cpu->registers.a);
      break;
    }

    case 0x23: {
      // INC HL
      cpu->registers.hl++;
      break;
    }

    case 0x24: {
      // INC H
      inc_8(cpu, &cpu->registers.h);
      break;
    }

    case 0x25: {
      // DEC H
      dec_8(cpu, &cpu->registers.h);
      break;
    }

    case 0x26: {
      // LD H, u8
      cpu->registers.h = fetch_8(cpu);
      break;
    }

    case 0x27: {
      // DAA
      daa_8(cpu, &cpu->registers.a);
      break;
    }

    case 0x28: {
      // JR Z, i8
      int8_t offset = (int8_t)fetch_8(cpu);
      if (get_zero_flag(&cpu->registers)) {
        cpu->has_jumped = 1;
        jr(cpu, offset);
      }
      break;
    }

    case 0x29: {
      // ADD HL, HL
      add_16(cpu, &cpu->registers.hl, cpu->registers.hl);
      break;
    }

    case 0x2A: {
      // LD A, (HL+)
      cpu->registers.a = cpu->memory[cpu->registers.hl++];
      break;
    }

    case 0x2B: {
      // DEC HL
      cpu->registers.hl--;
      break;
    }

    case 0x2C: {
      // INC L
      inc_8(cpu, &cpu->registers.l);
      break;
    }

    case 0x2D: {
      // DEC L
      dec_8(cpu, &cpu->registers.l);
      break;
    }

    case 0x2E: {
      // LD L, u8
      cpu->registers.l = fetch_8(cpu);
      break;
    }

    case 0x2F: {
      // CPL
      set_substraction_flag(&cpu->registers, 1);
      set_half_carry_flag(&cpu->registers, 1);
      cpu->registers.a ^= 0xFF;
      break;
    }

    case 0x30: {
      // JR NC, i8
      int8_t offset = (int8_t)fetch_8(cpu);
      if (!get_carry_flag(&cpu->registers)) {
        cpu->has_jumped = 1;
        jr(cpu, offset);
      }
      break;
    }

    case 0x31: {
      // LD SP, u16
      cpu->sp = fetch_16(cpu);
      break;
    }

    case 0x32: {
      // LD (HL-), A
      write_8(cpu, cpu->registers.hl--, cpu->registers.a);
      break;
    }

    case 0x33: {
      // INC SP
      cpu->sp++;
      break;
    }

    case 0x34: {
      // INC (HL)
      inc_8(cpu, &cpu->memory[cpu->registers.hl]);
      break;
    }

    case 0x35: {
      // DEC (HL)
      dec_8(cpu, &cpu->memory[cpu->registers.hl]);
      break;
    }

    case 0x36: {
      // LD (HL), u8
      write_8(cpu, cpu->registers.hl, fetch_8(cpu));
      break;
    }

    case 0x37: {
      // SCF
      set_substraction_flag(&cpu->registers, 0);
      set_half_carry_flag(&cpu->registers, 0);
      set_carry_flag(&cpu->registers, 1);
      break;
    }

    case 0x38: {
      // JR C, i8
      int8_t offset = (int8_t)fetch_8(cpu);
      if (get_carry_flag(&cpu->registers)) {
        cpu->has_jumped = 1;
        jr(cpu, offset);
      }
      break;
    }

    case 0x39: {
      // ADD HL, SP
      add_16(cpu, &cpu->registers.hl, cpu->sp);
      break;
    }

    case 0x3A: {
      // LD A, (HL-)
      cpu->registers.a = cpu->memory[cpu->registers.hl--];
      break;
    }

    case 0x3B: {
      // DEC SP
      cpu->sp--;
      break;
    }

    case 0x3C: {
      // INC A
      inc_8(cpu, &cpu->registers.a);
      break;
    }

    case 0x3D: {
      // DEC A
      dec_8(cpu, &cpu->registers.a);
      break;
    }

    case 0x3E: {
      // LD A, u8
      cpu->registers.a = fetch_8(cpu);
      break;
    }

    case 0x3F: {
      // CCF
      uint8_t carry_flag = get_carry_flag(&cpu->registers);
      set_substraction_flag(&cpu->registers, 0);
      set_half_carry_flag(&cpu->registers, 0);
      set_carry_flag(&cpu->registers, carry_flag ^ 1);
      break;
    }

    case 0x40: {
      // LD B, B == NOP
      break;
    }

    case 0x41: {
      // LD B, C
      cpu->registers.b = cpu->registers.c;
      break;
    }

    case 0x42: {
      // LD B, D
      cpu->registers.b = cpu->registers.d;
      break;
    }

    case 0x43: {
      // LD B, E
      cpu->registers.b = cpu->registers.e;
      break;
    }

    case 0x44: {
      // LD B, H
      cpu->registers.b = cpu->registers.h;
      break;
    }

    case 0x45: {
      // LD B, L
      cpu->registers.b = cpu->registers.l;
      break;
    }

    case 0x46: {
      // LD B, (HL)
      cpu->registers.b = cpu->memory[cpu->registers.hl];
      break;
    }

    case 0x47: {
      // LD B, A
      cpu->registers.b = cpu->registers.a;
      break;
    }

    case 0x48: {
      // LD C, B
      cpu->registers.c = cpu->registers.b;
      break;
    }

    case 0x49: {
      // LD C, C == NOP
      break;
    }

    case 0x4A: {
      // LD C, D
      cpu->registers.c = cpu->registers.d;
      break;
    }

    case 0x4B: {
      // LD C, E
      cpu->registers.c = cpu->registers.e;
      break;
    }

    case 0x4C: {
      // LD C, H
      cpu->registers.c = cpu->registers.h;
      break;
    }

    case 0x4D: {
      // LD C, L
      cpu->registers.c = cpu->registers.l;
      break;
    }

    case 0x4E: {
      // LD C, (HL)
      cpu->registers.c = cpu->memory[cpu->registers.hl];
      break;
    }

    case 0x4F: {
      // LD C, A
      cpu->registers.c = cpu->registers.a;
      break;
    }

    case 0x50: {
      // LD D, B
      cpu->registers.d = cpu->registers.b;
      break;
    }

    case 0x51: {
      // LD D, C
      cpu->registers.d = cpu->registers.c;
      break;
    }

    case 0x52: {
      // LD D, D == NOP
      break;
    }

    case 0x53: {
      // LD D, E
      cpu->registers.d = cpu->registers.e;
      break;
    }

    case 0x54: {
      // LD D, H
      cpu->registers.d = cpu->registers.h;
      break;
    }

    case 0x55: {
      // LD D, L
      cpu->registers.d = cpu->registers.l;
      break;
    }

    case 0x56: {
      // LD D, (HL)
      cpu->registers.d = cpu->memory[cpu->registers.hl];
      break;
    }

    case 0x57: {
      // LD D, A
      cpu->registers.d = cpu->registers.a;
      break;
    }

    case 0x58: {
      // LD E, B
      cpu->registers.e = cpu->registers.b;
      break;
    }

    case 0x59: {
      // LD E, C
      cpu->registers.e = cpu->registers.c;
      break;
    }

    case 0x5A: {
      // LD E, D
      cpu->registers.e = cpu->registers.d;
      break;
    }

    case 0x5B: {
      // LD E, E == NOP
      break;
    }

    case 0x5C: {
      // LD E, H
      cpu->registers.e = cpu->registers.h;
      break;
    }

    case 0x5D: {
      // LD E, L
      cpu->registers.e = cpu->registers.l;
      break;
    }

    case 0x5E: {
      // LD E, (HL)
      cpu->registers.e = cpu->memory[cpu->registers.hl];
      break;
    }

    case 0x5F: {
      // LD E, A
      cpu->registers.e = cpu->registers.a;
      break;
    }

    case 0x60: {
      // LD H, B
      cpu->registers.h = cpu->registers.b;
      break;
    }

    case 0x61: {
      // LD H, C
      cpu->registers.h = cpu->registers.c;
      break;
    }

    case 0x62: {
      // LD H, D
      cpu->registers.h = cpu->registers.d;
      break;
    }

    case 0x63: {
      // LD H, E
      cpu->registers.h = cpu->registers.e;
      break;
    }

    case 0x64: {
      // LD H, H == NOP
      break;
    }

    case 0x65: {
      // LD H, L
      cpu->registers.h = cpu->registers.l;
      break;
    }

    case 0x66: {
      // LD H, (HL)
      cpu->registers.h = cpu->memory[cpu->registers.hl];
      break;
    }

    case 0x67: {
      // LD H, A
      cpu->registers.h = cpu->registers.a;
      break;
    }

    case 0x68: {
      // LD L, B
      cpu->registers.l = cpu->registers.b;
      break;
    }

    case 0x69: {
      // LD L, C
      cpu->registers.l = cpu->registers.c;
      break;
    }

    case 0x6A: {
      // LD L, D
      cpu->registers.l = cpu->registers.d;
      break;
    }

    case 0x6B: {
      // LD L, E
      cpu->registers.l = cpu->registers.e;
      break;
    }

    case 0x6C: {
      // LD L, H
      cpu->registers.l = cpu->registers.h;
      break;
    }

    case 0x6D: {
      // LD L, L == NOP
      break;
    }

    case 0x6E: {
      // LD L, (HL)
      cpu->registers.l = cpu->memory[cpu->registers.hl];
      break;
    }

    case 0x6F: {
      // LD L, A
      cpu->registers.l = cpu->registers.a;
      break;
    }

    case 0x70: {
      // LD (HL), B
      write_8(cpu, cpu->registers.hl, cpu->registers.b);
      break;
    }

    case 0x71: {
      // LD (HL), C
      write_8(cpu, cpu->registers.hl, cpu->registers.c);
      break;
    }

    case 0x72: {
      // LD (HL), D
      write_8(cpu, cpu->registers.hl, cpu->registers.d);
      break;
    }

    case 0x73: {
      // LD (HL), E
      write_8(cpu, cpu->registers.hl, cpu->registers.e);
      break;
    }

    case 0x74: {
      // LD (HL), H
      write_8(cpu, cpu->registers.hl, cpu->registers.h);
      break;
    }

    case 0x75: {
      // LD (HL), L
      write_8(cpu, cpu->registers.hl, cpu->registers.l);
      break;
    }

    case 0x76: {
      // HALT
      cpu->is_halted = 1;
      break;
    }

    case 0x77: {
      // LD (HL), A
      write_8(cpu, cpu->registers.hl, cpu->registers.a);
      break;
    }

    case 0x78: {
      // LD A, B
      cpu->registers.a = cpu->registers.b;
      break;
    }

    case 0x79: {
      // LD A, C
      cpu->registers.a = cpu->registers.c;
      break;
    }

    case 0x7A: {
      // LD A, D
      cpu->registers.a = cpu->registers.d;
      break;
    }

    case 0x7B: {
      // LD A, E
      cpu->registers.a = cpu->registers.e;
      break;
    }

    case 0x7C: {
      // LD A, H
      cpu->registers.a = cpu->registers.h;
      break;
    }

    case 0x7D: {
      // LD A, L
      cpu->registers.a = cpu->registers.l;
      break;
    }

    case 0x7E: {
      // LD A, (HL)
      cpu->registers.a = cpu->memory[cpu->registers.hl];
      break;
    }

    case 0x7F: {
      // LD A, A == NOP
      break;
    }

    case 0x80: {
      // ADD A, B
      add_8(cpu, &cpu->registers.a, cpu->registers.b);
      break;
    }

    case 0x81: {
      // ADD A, C
      add_8(cpu, &cpu->registers.a, cpu->registers.c);
      break;
    }

    case 0x82: {
      // ADD A, D
      add_8(cpu, &cpu->registers.a, cpu->registers.d);
      break;
    }

    case 0x83: {
      // ADD A, E
      add_8(cpu, &cpu->registers.a, cpu->registers.e);
      break;
    }

    case 0x84: {
      // ADD A, H
      add_8(cpu, &cpu->registers.a, cpu->registers.h);
      break;
    }

    case 0x85: {
      // ADD A, L
      add_8(cpu, &cpu->registers.a, cpu->registers.l);
      break;
    }

    case 0x86: {
      // ADD A, (HL)
      add_8(cpu, &cpu->registers.a, cpu->memory[cpu->registers.hl]);
      break;
    }

    case 0x87: {
      // ADD A, A
      add_8(cpu, &cpu->registers.a, cpu->registers.a);
      break;
    }

    case 0x88: {
      // ADC A, B
      adc_8(cpu, &cpu->registers.a, cpu->registers.b);
      break;
    }

    case 0x89: {
      // ADC A, C
      adc_8(cpu, &cpu->registers.a, cpu->registers.c);
      break;
    }

    case 0x8A: {
      // ADC A, D
      adc_8(cpu, &cpu->registers.a, cpu->registers.d);
      break;
    }

    case 0x8B: {
      // ADC A, E
      adc_8(cpu, &cpu->registers.a, cpu->registers.e);
      break;
    }

    case 0x8C: {
      // ADC A, H
      adc_8(cpu, &cpu->registers.a, cpu->registers.h);
      break;
    }

    case 0x8D: {
      // ADC A, L
      adc_8(cpu, &cpu->registers.a, cpu->registers.l);
      break;
    }

    case 0x8E: {
      // ADC A, (HL)
      adc_8(cpu, &cpu->registers.a, cpu->memory[cpu->registers.hl]);
      break;
    }

    case 0x8F: {
      // ADC A, A
      adc_8(cpu, &cpu->registers.a, cpu->registers.a);
      break;
    }

    case 0x90: {
      // SUB A, B
      sub_8(cpu, &cpu->registers.a, cpu->registers.b);
      break;
    }

    case 0x91: {
      // SUB A, C
      sub_8(cpu, &cpu->registers.a, cpu->registers.c);
      break;
    }

    case 0x92: {
      // SUB A, D
      sub_8(cpu, &cpu->registers.a, cpu->registers.d);
      break;
    }

    case 0x93: {
      // SUB A, E
      sub_8(cpu, &cpu->registers.a, cpu->registers.e);
      break;
    }

    case 0x94: {
      // SUB A, H
      sub_8(cpu, &cpu->registers.a, cpu->registers.h);
      break;
    }

    case 0x95: {
      // SUB A, L
      sub_8(cpu, &cpu->registers.a, cpu->registers.l);
      break;
    }

    case 0x96: {
      // SUB A, (HL)
      sub_8(cpu, &cpu->registers.a, cpu->memory[cpu->registers.hl]);
      break;
    }

    case 0x97: {
      // SUB A, A
      sub_8(cpu, &cpu->registers.a, cpu->registers.a);
      break;
    }

    case 0x98: {
      // SBC A, B
      sbc_8(cpu, &cpu->registers.a, cpu->registers.b);
      break;
    }

    case 0x99: {
      // SBC A, C
      sbc_8(cpu, &cpu->registers.a, cpu->registers.c);
      break;
    }

    case 0x9A: {
      // SBC A, D
      sbc_8(cpu, &cpu->registers.a, cpu->registers.d);
      break;
    }

    case 0x9B: {
      // SBC A, E
      sbc_8(cpu, &cpu->registers.a, cpu->registers.e);
      break;
    }

    case 0x9C: {
      // SBC A, H
      sbc_8(cpu, &cpu->registers.a, cpu->registers.h);
      break;
    }

    case 0x9D: {
      // SBC A, L
      sbc_8(cpu, &cpu->registers.a, cpu->registers.l);
      break;
    }

    case 0x9E: {
      // SBC A, (HL)
      sbc_8(cpu, &cpu->registers.a, cpu->memory[cpu->registers.hl]);
      break;
    }

    case 0x9F: {
      // SBC A, A
      sbc_8(cpu, &cpu->registers.a, cpu->registers.a);
      break;
    }

    case 0xA0: {
      // AND A, B
      and_8(cpu, &cpu->registers.a, cpu->registers.b);
      break;
    }

    case 0xA1: {
      // AND A, C
      and_8(cpu, &cpu->registers.a, cpu->registers.c);
      break;
    }

    case 0xA2: {
      // AND A, D
      and_8(cpu, &cpu->registers.a, cpu->registers.d);
      break;
    }

    case 0xA3: {
      // AND A, E
      and_8(cpu, &cpu->registers.a, cpu->registers.e);
      break;
    }

    case 0xA4: {
      // AND A, H
      and_8(cpu, &cpu->registers.a, cpu->registers.h);
      break;
    }

    case 0xA5: {
      // AND A, L
      and_8(cpu, &cpu->registers.a, cpu->registers.l);
      break;
    }

    case 0xA6: {
      // AND A, (HL)
      and_8(cpu, &cpu->registers.a, cpu->memory[cpu->registers.hl]);
      break;
    }

    case 0xA7: {
      // AND A, A
      and_8(cpu, &cpu->registers.a, cpu->registers.a);
      break;
    }

    case 0xA8: {
      // XOR A, B
      xor_8(cpu, &cpu->registers.a, cpu->registers.b);
      break;
    }

    case 0xA9: {
      // XOR A, C
      xor_8(cpu, &cpu->registers.a, cpu->registers.c);
      break;
    }

    case 0xAA: {
      // XOR A, D
      xor_8(cpu, &cpu->registers.a, cpu->registers.d);
      break;
    }

    case 0xAB: {
      // XOR A, E
      xor_8(cpu, &cpu->registers.a, cpu->registers.e);
      break;
    }

    case 0xAC: {
      // XOR A, H
      xor_8(cpu, &cpu->registers.a, cpu->registers.h);
      break;
    }

    case 0xAD: {
      // XOR A, L
      xor_8(cpu, &cpu->registers.a, cpu->registers.l);
      break;
    }

    case 0xAE: {
      // XOR A, (HL)
      xor_8(cpu, &cpu->registers.a, cpu->memory[cpu->registers.hl]);
      break;
    }

    case 0xAF: {
      // XOR A, A
      xor_8(cpu, &cpu->registers.a, cpu->registers.a);
      break;
    }

    case 0xB0: {
      // OR A, B
      or_8(cpu, &cpu->registers.a, cpu->registers.b);
      break;
    }

    case 0xB1: {
      // OR A, C
      or_8(cpu, &cpu->registers.a, cpu->registers.c);
      break;
    }

    case 0xB2: {
      // OR A, D
      or_8(cpu, &cpu->registers.a, cpu->registers.d);
      break;
    }

    case 0xB3: {
      // OR A, E
      or_8(cpu, &cpu->registers.a, cpu->registers.e);
      break;
    }

    case 0xB4: {
      // OR A, H
      or_8(cpu, &cpu->registers.a, cpu->registers.h);
      break;
    }

    case 0xB5: {
      // OR A, L
      or_8(cpu, &cpu->registers.a, cpu->registers.l);
      break;
    }

    case 0xB6: {
      // OR A, (HL)
      or_8(cpu, &cpu->registers.a, cpu->memory[cpu->registers.hl]);
      break;
    }

    case 0xB7: {
      // OR A, A
      or_8(cpu, &cpu->registers.a, cpu->registers.a);
      break;
    }

    case 0xB8: {
      // CP A, B
      cp_8(cpu, &cpu->registers.a, cpu->registers.b);
      break;
    }

    case 0xB9: {
      // CP A, C
      cp_8(cpu, &cpu->registers.a, cpu->registers.c);
      break;
    }

    case 0xBA: {
      // CP A, D
      cp_8(cpu, &cpu->registers.a, cpu->registers.d);
      break;
    }

    case 0xBB: {
      // CP A, E
      cp_8(cpu, &cpu->registers.a, cpu->registers.e);
      break;
    }

    case 0xBC: {
      // CP A, H
      cp_8(cpu, &cpu->registers.a, cpu->registers.h);
      break;
    }

    case 0xBD: {
      // CP A, L
      cp_8(cpu, &cpu->registers.a, cpu->registers.l);
      break;
    }

    case 0xBE: {
      // CP A, (HL)
      cp_8(cpu, &cpu->registers.a, cpu->memory[cpu->registers.hl]);
      break;
    }

    case 0xBF: {
      // CP A, A
      cp_8(cpu, &cpu->registers.a, cpu->registers.a);
      break;
    }

    case 0xC0: {
      // RET NZ
      if (!get_zero_flag(&cpu->registers)) {
        cpu->has_jumped = 1;
        pop_16(cpu, &cpu->pc);
      }
      break;
    }

    case 0xC1: {
      // POP BC
      pop_16(cpu, &cpu->registers.bc);
      break;
    }

    case 0xC2: {
      // JP NZ, u16
      uint16_t address = fetch_16(cpu);
      if (!get_zero_flag(&cpu->registers)) {
        cpu->has_jumped = 1;
        jp(cpu, address);
      }
      break;
    }

    case 0xC3: {
      // JP u16
      jp(cpu, fetch_16(cpu));
      break;
    }

    case 0xC4: {
      // CALL NZ, u16
      uint16_t address = fetch_16(cpu);
      if (!get_zero_flag(&cpu->registers)) {
        cpu->has_jumped = 1;
        call(cpu, address);
      }
      break;
    }

    case 0xC5: {
      // PUSH BC
      push_16(cpu, cpu->registers.bc);
      break;
    }

    case 0xC6: {
      // ADD A, u8
      add_8(cpu, &cpu->registers.a, fetch_8(cpu));
      break;
    }

    case 0xC7: {
      // RST 00h
      rst(cpu, 0x00);
      break;
    }

    case 0xC8: {
      // RET Z
      if (get_zero_flag(&cpu->registers)) {
        cpu->has_jumped = 1;
        pop_16(cpu, &cpu->pc);
      }
      break;
    }

    case 0xC9: {
      // RET
      pop_16(cpu, &cpu->pc);
      break;
    }

    case 0xCA: {
      // JP Z, u16
      uint16_t address = fetch_16(cpu);
      if (get_zero_flag(&cpu->registers)) {
        cpu->has_jumped = 1;
        jp(cpu, address);
      }
      break;
    }

    case 0xCB: {
      // PREFIX CB
      uint8_t immediate = fetch_8(cpu);
      cpu->elapsed_cycles += op_cycles_cb[immediate];

      timer_tick(&cpu->counter, op_cycles_cb[immediate]);
      timer_tick(&cpu->divider, op_cycles_cb[immediate]);
      switch (immediate) {
        case 0x00: {
          rlc_8(cpu, &cpu->registers.b);
          break;
        }
        case 0x01: {
          rlc_8(cpu, &cpu->registers.c);
          break;
        }

        case 0x02: {
          rlc_8(cpu, &cpu->registers.d);
          break;
        }

        case 0x03: {
          rlc_8(cpu, &cpu->registers.e);
          break;
        }

        case 0x04: {
          rlc_8(cpu, &cpu->registers.h);
          break;
        }

        case 0x05: {
          rlc_8(cpu, &cpu->registers.l);
          break;
        }

        case 0x06: {
          rlc_8(cpu, &cpu->memory[cpu->registers.hl]);
          break;
        }

        case 0x07: {
          rlc_8(cpu, &cpu->registers.a);
          break;
        }

        case 0x08: {
          rrc_8(cpu, &cpu->registers.b);
          break;
        }

        case 0x09: {
          rrc_8(cpu, &cpu->registers.c);
          break;
        }

        case 0x0A: {
          rrc_8(cpu, &cpu->registers.d);
          break;
        }

        case 0x0B: {
          rrc_8(cpu, &cpu->registers.e);
          break;
        }

        case 0x0C: {
          rrc_8(cpu, &cpu->registers.h);
          break;
        }

        case 0x0D: {
          rrc_8(cpu, &cpu->registers.l);
          break;
        }

        case 0x0E: {
          rrc_8(cpu, &cpu->memory[cpu->registers.hl]);
          break;
        }

        case 0x0F: {
          rrc_8(cpu, &cpu->registers.a);
          break;
        }

        case 0x10: {
          rl_8(cpu, &cpu->registers.b);
          break;
        }

        case 0x11: {
          rl_8(cpu, &cpu->registers.c);
          break;
        }

        case 0x12: {
          rl_8(cpu, &cpu->registers.d);
          break;
        }

        case 0x13: {
          rl_8(cpu, &cpu->registers.e);
          break;
        }

        case 0x14: {
          rl_8(cpu, &cpu->registers.h);
          break;
        }

        case 0x15: {
          rl_8(cpu, &cpu->registers.l);
          break;
        }

        case 0x16: {
          rl_8(cpu, &cpu->memory[cpu->registers.hl]);
          break;
        }

        case 0x17: {
          rl_8(cpu, &cpu->registers.a);
          break;
        }

        case 0x18: {
          rr_8(cpu, &cpu->registers.b);
          break;
        }

        case 0x19: {
          rr_8(cpu, &cpu->registers.c);
          break;
        }

        case 0x1A: {
          rr_8(cpu, &cpu->registers.d);
          break;
        }

        case 0x1B: {
          rr_8(cpu, &cpu->registers.e);
          break;
        }

        case 0x1C: {
          rr_8(cpu, &cpu->registers.h);
          break;
        }

        case 0x1D: {
          rr_8(cpu, &cpu->registers.l);
          break;
        }

        case 0x1E: {
          rr_8(cpu, &cpu->memory[cpu->registers.hl]);
          break;
        }

        case 0x1F: {
          rr_8(cpu, &cpu->registers.a);
          break;
        }

        case 0x20: {
          sla_8(cpu, &cpu->registers.b);
          break;
        }

        case 0x21: {
          sla_8(cpu, &cpu->registers.c);
          break;
        }

        case 0x22: {
          sla_8(cpu, &cpu->registers.d);
          break;
        }

        case 0x23: {
          sla_8(cpu, &cpu->registers.e);
          break;
        }

        case 0x24: {
          sla_8(cpu, &cpu->registers.h);
          break;
        }

        case 0x25: {
          sla_8(cpu, &cpu->registers.l);
          break;
        }

        case 0x26: {
          sla_8(cpu, &cpu->memory[cpu->registers.hl]);
          break;
        }

        case 0x27: {
          sla_8(cpu, &cpu->registers.a);
          break;
        }

        case 0x28: {
          sra_8(cpu, &cpu->registers.b);
          break;
        }

        case 0x29: {
          sra_8(cpu, &cpu->registers.c);
          break;
        }

        case 0x2A: {
          sra_8(cpu, &cpu->registers.d);
          break;
        }

        case 0x2B: {
          sra_8(cpu, &cpu->registers.e);
          break;
        }

        case 0x2C: {
          sra_8(cpu, &cpu->registers.h);
          break;
        }

        case 0x2D: {
          sra_8(cpu, &cpu->registers.l);
          break;
        }

        case 0x2E: {
          sra_8(cpu, &cpu->memory[cpu->registers.hl]);
          break;
        }

        case 0x2F: {
          sra_8(cpu, &cpu->registers.a);
          break;
        }

        case 0x30: {
          swap_8(cpu, &cpu->registers.b);
          break;
        }

        case 0x31: {
          swap_8(cpu, &cpu->registers.c);
          break;
        }

        case 0x32: {
          swap_8(cpu, &cpu->registers.d);
          break;
        }

        case 0x33: {
          swap_8(cpu, &cpu->registers.e);
          break;
        }

        case 0x34: {
          swap_8(cpu, &cpu->registers.h);
          break;
        }

        case 0x35: {
          swap_8(cpu, &cpu->registers.l);
          break;
        }

        case 0x36: {
          swap_8(cpu, &cpu->memory[cpu->registers.hl]);
          break;
        }

        case 0x37: {
          swap_8(cpu, &cpu->registers.a);
          break;
        }

        case 0x38: {
          srl_8(cpu, &cpu->registers.b);
          break;
        }

        case 0x39: {
          srl_8(cpu, &cpu->registers.c);
          break;
        }

        case 0x3A: {
          srl_8(cpu, &cpu->registers.d);
          break;
        }

        case 0x3B: {
          srl_8(cpu, &cpu->registers.e);
          break;
        }

        case 0x3C: {
          srl_8(cpu, &cpu->registers.h);
          break;
        }

        case 0x3D: {
          srl_8(cpu, &cpu->registers.l);
          break;
        }

        case 0x3E: {
          srl_8(cpu, &cpu->memory[cpu->registers.hl]);
          break;
        }

        case 0x3F: {
          srl_8(cpu, &cpu->registers.a);
          break;
        }

        case 0x40: {
          bit_test(cpu, cpu->registers.b, 0);
          break;
        }

        case 0x41: {
          bit_test(cpu, cpu->registers.c, 0);
          break;
        }

        case 0x42: {
          bit_test(cpu, cpu->registers.d, 0);
          break;
        }

        case 0x43: {
          bit_test(cpu, cpu->registers.e, 0);
          break;
        }

        case 0x44: {
          bit_test(cpu, cpu->registers.h, 0);
          break;
        }

        case 0x45: {
          bit_test(cpu, cpu->registers.l, 0);
          break;
        }

        case 0x46: {
          bit_test(cpu, cpu->memory[cpu->registers.hl], 0);
          break;
        }

        case 0x47: {
          bit_test(cpu, cpu->registers.a, 0);
          break;
        }

        case 0x48: {
          bit_test(cpu, cpu->registers.b, 1);
          break;
        }

        case 0x49: {
          bit_test(cpu, cpu->registers.c, 1);
          break;
        }

        case 0x4A: {
          bit_test(cpu, cpu->registers.d, 1);
          break;
        }

        case 0x4B: {
          bit_test(cpu, cpu->registers.e, 1);
          break;
        }

        case 0x4C: {
          bit_test(cpu, cpu->registers.h, 1);
          break;
        }

        case 0x4D: {
          bit_test(cpu, cpu->registers.l, 1);
          break;
        }

        case 0x4E: {
          bit_test(cpu, cpu->memory[cpu->registers.hl], 1);
          break;
        }

        case 0x4F: {
          bit_test(cpu, cpu->registers.a, 1);
          break;
        }

        case 0x50: {
          bit_test(cpu, cpu->registers.b, 2);
          break;
        }

        case 0x51: {
          bit_test(cpu, cpu->registers.c, 2);
          break;
        }

        case 0x52: {
          bit_test(cpu, cpu->registers.d, 2);
          break;
        }

        case 0x53: {
          bit_test(cpu, cpu->registers.e, 2);
          break;
        }

        case 0x54: {
          bit_test(cpu, cpu->registers.h, 2);
          break;
        }

        case 0x55: {
          bit_test(cpu, cpu->registers.l, 2);
          break;
        }

        case 0x56: {
          bit_test(cpu, cpu->memory[cpu->registers.hl], 2);
          break;
        }

        case 0x57: {
          bit_test(cpu, cpu->registers.a, 2);
          break;
        }

        case 0x58: {
          bit_test(cpu, cpu->registers.b, 3);
          break;
        }

        case 0x59: {
          bit_test(cpu, cpu->registers.c, 3);
          break;
        }

        case 0x5A: {
          bit_test(cpu, cpu->registers.d, 3);
          break;
        }

        case 0x5B: {
          bit_test(cpu, cpu->registers.e, 3);
          break;
        }

        case 0x5C: {
          bit_test(cpu, cpu->registers.h, 3);
          break;
        }

        case 0x5D: {
          bit_test(cpu, cpu->registers.l, 3);
          break;
        }

        case 0x5E: {
          bit_test(cpu, cpu->memory[cpu->registers.hl], 3);
          break;
        }

        case 0x5F: {
          bit_test(cpu, cpu->registers.a, 3);
          break;
        }

        case 0x60: {
          bit_test(cpu, cpu->registers.b, 4);
          break;
        }

        case 0x61: {
          bit_test(cpu, cpu->registers.c, 4);
          break;
        }

        case 0x62: {
          bit_test(cpu, cpu->registers.d, 4);
          break;
        }

        case 0x63: {
          bit_test(cpu, cpu->registers.e, 4);
          break;
        }

        case 0x64: {
          bit_test(cpu, cpu->registers.h, 4);
          break;
        }

        case 0x65: {
          bit_test(cpu, cpu->registers.l, 4);
          break;
        }

        case 0x66: {
          bit_test(cpu, cpu->memory[cpu->registers.hl], 4);
          break;
        }

        case 0x67: {
          bit_test(cpu, cpu->registers.a, 4);
          break;
        }

        case 0x68: {
          bit_test(cpu, cpu->registers.b, 5);
          break;
        }

        case 0x69: {
          bit_test(cpu, cpu->registers.c, 5);
          break;
        }

        case 0x6A: {
          bit_test(cpu, cpu->registers.d, 5);
          break;
        }

        case 0x6B: {
          bit_test(cpu, cpu->registers.e, 5);
          break;
        }

        case 0x6C: {
          bit_test(cpu, cpu->registers.h, 5);
          break;
        }

        case 0x6D: {
          bit_test(cpu, cpu->registers.l, 5);
          break;
        }

        case 0x6E: {
          bit_test(cpu, cpu->memory[cpu->registers.hl], 5);
          break;
        }

        case 0x6F: {
          bit_test(cpu, cpu->registers.a, 5);
          break;
        }

        case 0x70: {
          bit_test(cpu, cpu->registers.b, 6);
          break;
        }

        case 0x71: {
          bit_test(cpu, cpu->registers.c, 6);
          break;
        }

        case 0x72: {
          bit_test(cpu, cpu->registers.d, 6);
          break;
        }

        case 0x73: {
          bit_test(cpu, cpu->registers.e, 6);
          break;
        }

        case 0x74: {
          bit_test(cpu, cpu->registers.h, 6);
          break;
        }

        case 0x75: {
          bit_test(cpu, cpu->registers.l, 6);
          break;
        }

        case 0x76: {
          bit_test(cpu, cpu->memory[cpu->registers.hl], 6);
          break;
        }

        case 0x77: {
          bit_test(cpu, cpu->registers.a, 6);
          break;
        }

        case 0x78: {
          bit_test(cpu, cpu->registers.b, 7);
          break;
        }

        case 0x79: {
          bit_test(cpu, cpu->registers.c, 7);
          break;
        }

        case 0x7A: {
          bit_test(cpu, cpu->registers.d, 7);
          break;
        }

        case 0x7B: {
          bit_test(cpu, cpu->registers.e, 7);
          break;
        }

        case 0x7C: {
          bit_test(cpu, cpu->registers.h, 7);
          break;
        }

        case 0x7D: {
          bit_test(cpu, cpu->registers.l, 7);
          break;
        }

        case 0x7E: {
          bit_test(cpu, cpu->memory[cpu->registers.hl], 7);
          break;
        }

        case 0x7F: {
          bit_test(cpu, cpu->registers.a, 7);
          break;
        }

        case 0x80: {
          res_8(cpu, &cpu->registers.b, 0);
          break;
        }

        case 0x81: {
          res_8(cpu, &cpu->registers.c, 0);
          break;
        }

        case 0x82: {
          res_8(cpu, &cpu->registers.d, 0);
          break;
        }

        case 0x83: {
          res_8(cpu, &cpu->registers.e, 0);
          break;
        }

        case 0x84: {
          res_8(cpu, &cpu->registers.h, 0);
          break;
        }

        case 0x85: {
          res_8(cpu, &cpu->registers.l, 0);
          break;
        }

        case 0x86: {
          res_8(cpu, &cpu->memory[cpu->registers.hl], 0);
          break;
        }

        case 0x87: {
          res_8(cpu, &cpu->registers.a, 0);
          break;
        }

        case 0x88: {
          res_8(cpu, &cpu->registers.b, 1);
          break;
        }

        case 0x89: {
          res_8(cpu, &cpu->registers.c, 1);
          break;
        }

        case 0x8A: {
          res_8(cpu, &cpu->registers.d, 1);
          break;
        }

        case 0x8B: {
          res_8(cpu, &cpu->registers.e, 1);
          break;
        }

        case 0x8C: {
          res_8(cpu, &cpu->registers.h, 1);
          break;
        }

        case 0x8D: {
          res_8(cpu, &cpu->registers.l, 1);
          break;
        }

        case 0x8E: {
          res_8(cpu, &cpu->memory[cpu->registers.hl], 1);
          break;
        }

        case 0x8F: {
          res_8(cpu, &cpu->registers.a, 1);
          break;
        }

        case 0x90: {
          res_8(cpu, &cpu->registers.b, 2);
          break;
        }

        case 0x91: {
          res_8(cpu, &cpu->registers.c, 2);
          break;
        }

        case 0x92: {
          res_8(cpu, &cpu->registers.d, 2);
          break;
        }

        case 0x93: {
          res_8(cpu, &cpu->registers.e, 2);
          break;
        }

        case 0x94: {
          res_8(cpu, &cpu->registers.h, 2);
          break;
        }

        case 0x95: {
          res_8(cpu, &cpu->registers.l, 2);
          break;
        }

        case 0x96: {
          res_8(cpu, &cpu->memory[cpu->registers.hl], 2);
          break;
        }

        case 0x97: {
          res_8(cpu, &cpu->registers.a, 2);
          break;
        }

        case 0x98: {
          res_8(cpu, &cpu->registers.b, 3);
          break;
        }

        case 0x99: {
          res_8(cpu, &cpu->registers.c, 3);
          break;
        }

        case 0x9A: {
          res_8(cpu, &cpu->registers.d, 3);
          break;
        }

        case 0x9B: {
          res_8(cpu, &cpu->registers.e, 3);
          break;
        }

        case 0x9C: {
          res_8(cpu, &cpu->registers.h, 3);
          break;
        }

        case 0x9D: {
          res_8(cpu, &cpu->registers.l, 3);
          break;
        }

        case 0x9E: {
          res_8(cpu, &cpu->memory[cpu->registers.hl], 3);
          break;
        }

        case 0x9F: {
          res_8(cpu, &cpu->registers.a, 3);
          break;
        }

        case 0xA0: {
          res_8(cpu, &cpu->registers.b, 4);
          break;
        }

        case 0xA1: {
          res_8(cpu, &cpu->registers.c, 4);
          break;
        }

        case 0xA2: {
          res_8(cpu, &cpu->registers.d, 4);
          break;
        }

        case 0xA3: {
          res_8(cpu, &cpu->registers.e, 4);
          break;
        }

        case 0xA4: {
          res_8(cpu, &cpu->registers.h, 4);
          break;
        }

        case 0xA5: {
          res_8(cpu, &cpu->registers.l, 4);
          break;
        }

        case 0xA6: {
          res_8(cpu, &cpu->memory[cpu->registers.hl], 4);
          break;
        }

        case 0xA7: {
          res_8(cpu, &cpu->registers.a, 4);
          break;
        }

        case 0xA8: {
          res_8(cpu, &cpu->registers.b, 5);
          break;
        }

        case 0xA9: {
          res_8(cpu, &cpu->registers.c, 5);
          break;
        }

        case 0xAA: {
          res_8(cpu, &cpu->registers.d, 5);
          break;
        }

        case 0xAB: {
          res_8(cpu, &cpu->registers.e, 5);
          break;
        }

        case 0xAC: {
          res_8(cpu, &cpu->registers.h, 5);
          break;
        }

        case 0xAD: {
          res_8(cpu, &cpu->registers.l, 5);
          break;
        }

        case 0xAE: {
          res_8(cpu, &cpu->memory[cpu->registers.hl], 5);
          break;
        }

        case 0xAF: {
          res_8(cpu, &cpu->registers.a, 5);
          break;
        }

        case 0xB0: {
          res_8(cpu, &cpu->registers.b, 6);
          break;
        }

        case 0xB1: {
          res_8(cpu, &cpu->registers.c, 6);
          break;
        }

        case 0xB2: {
          res_8(cpu, &cpu->registers.d, 6);
          break;
        }

        case 0xB3: {
          res_8(cpu, &cpu->registers.e, 6);
          break;
        }

        case 0xB4: {
          res_8(cpu, &cpu->registers.h, 6);
          break;
        }

        case 0xB5: {
          res_8(cpu, &cpu->registers.l, 6);
          break;
        }

        case 0xB6: {
          res_8(cpu, &cpu->memory[cpu->registers.hl], 6);
          break;
        }

        case 0xB7: {
          res_8(cpu, &cpu->registers.a, 6);
          break;
        }

        case 0xB8: {
          res_8(cpu, &cpu->registers.b, 7);
          break;
        }

        case 0xB9: {
          res_8(cpu, &cpu->registers.c, 7);
          break;
        }

        case 0xBA: {
          res_8(cpu, &cpu->registers.d, 7);
          break;
        }

        case 0xBB: {
          res_8(cpu, &cpu->registers.e, 7);
          break;
        }

        case 0xBC: {
          res_8(cpu, &cpu->registers.h, 7);
          break;
        }

        case 0xBD: {
          res_8(cpu, &cpu->registers.l, 7);
          break;
        }

        case 0xBE: {
          res_8(cpu, &cpu->memory[cpu->registers.hl], 7);
          break;
        }

        case 0xBF: {
          res_8(cpu, &cpu->registers.a, 7);
          break;
        }

        case 0xC0: {
          set_8(cpu, &cpu->registers.b, 0);
          break;
        }

        case 0xC1: {
          set_8(cpu, &cpu->registers.c, 0);
          break;
        }

        case 0xC2: {
          set_8(cpu, &cpu->registers.d, 0);
          break;
        }

        case 0xC3: {
          set_8(cpu, &cpu->registers.e, 0);
          break;
        }

        case 0xC4: {
          set_8(cpu, &cpu->registers.h, 0);
          break;
        }

        case 0xC5: {
          set_8(cpu, &cpu->registers.l, 0);
          break;
        }

        case 0xC6: {
          set_8(cpu, &cpu->memory[cpu->registers.hl], 0);
          break;
        }

        case 0xC7: {
          set_8(cpu, &cpu->registers.a, 0);
          break;
        }

        case 0xC8: {
          set_8(cpu, &cpu->registers.b, 1);
          break;
        }

        case 0xC9: {
          set_8(cpu, &cpu->registers.c, 1);
          break;
        }

        case 0xCA: {
          set_8(cpu, &cpu->registers.d, 1);
          break;
        }

        case 0xCB: {
          set_8(cpu, &cpu->registers.e, 1);
          break;
        }

        case 0xCC: {
          set_8(cpu, &cpu->registers.h, 1);
          break;
        }

        case 0xCD: {
          set_8(cpu, &cpu->registers.l, 1);
          break;
        }

        case 0xCE: {
          set_8(cpu, &cpu->memory[cpu->registers.hl], 1);
          break;
        }

        case 0xCF: {
          set_8(cpu, &cpu->registers.a, 1);
          break;
        }

        case 0xD0: {
          set_8(cpu, &cpu->registers.b, 2);
          break;
        }

        case 0xD1: {
          set_8(cpu, &cpu->registers.c, 2);
          break;
        }

        case 0xD2: {
          set_8(cpu, &cpu->registers.d, 2);
          break;
        }

        case 0xD3: {
          set_8(cpu, &cpu->registers.e, 2);
          break;
        }

        case 0xD4: {
          set_8(cpu, &cpu->registers.h, 2);
          break;
        }

        case 0xD5: {
          set_8(cpu, &cpu->registers.l, 2);
          break;
        }

        case 0xD6: {
          set_8(cpu, &cpu->memory[cpu->registers.hl], 2);
          break;
        }

        case 0xD7: {
          set_8(cpu, &cpu->registers.a, 2);
          break;
        }

        case 0xD8: {
          set_8(cpu, &cpu->registers.b, 3);
          break;
        }

        case 0xD9: {
          set_8(cpu, &cpu->registers.c, 3);
          break;
        }

        case 0xDA: {
          set_8(cpu, &cpu->registers.d, 3);
          break;
        }

        case 0xDB: {
          set_8(cpu, &cpu->registers.e, 3);
          break;
        }

        case 0xDC: {
          set_8(cpu, &cpu->registers.h, 3);
          break;
        }

        case 0xDD: {
          set_8(cpu, &cpu->registers.l, 3);
          break;
        }

        case 0xDE: {
          set_8(cpu, &cpu->memory[cpu->registers.hl], 3);
          break;
        }

        case 0xDF: {
          set_8(cpu, &cpu->registers.a, 3);
          break;
        }

        case 0xE0: {
          set_8(cpu, &cpu->registers.b, 4);
          break;
        }

        case 0xE1: {
          set_8(cpu, &cpu->registers.c, 4);
          break;
        }

        case 0xE2: {
          set_8(cpu, &cpu->registers.d, 4);
          break;
        }

        case 0xE3: {
          set_8(cpu, &cpu->registers.e, 4);
          break;
        }

        case 0xE4: {
          set_8(cpu, &cpu->registers.h, 4);
          break;
        }

        case 0xE5: {
          set_8(cpu, &cpu->registers.l, 4);
          break;
        }

        case 0xE6: {
          set_8(cpu, &cpu->memory[cpu->registers.hl], 4);
          break;
        }

        case 0xE7: {
          set_8(cpu, &cpu->registers.a, 4);
          break;
        }

        case 0xE8: {
          set_8(cpu, &cpu->registers.b, 5);
          break;
        }

        case 0xE9: {
          set_8(cpu, &cpu->registers.c, 5);
          break;
        }

        case 0xEA: {
          set_8(cpu, &cpu->registers.d, 5);
          break;
        }

        case 0xEB: {
          set_8(cpu, &cpu->registers.e, 5);
          break;
        }

        case 0xEC: {
          set_8(cpu, &cpu->registers.h, 5);
          break;
        }

        case 0xED: {
          set_8(cpu, &cpu->registers.l, 5);
          break;
        }

        case 0xEE: {
          set_8(cpu, &cpu->memory[cpu->registers.hl], 5);
          break;
        }

        case 0xEF: {
          set_8(cpu, &cpu->registers.a, 5);
          break;
        }

        case 0xF0: {
          set_8(cpu, &cpu->registers.b, 6);
          break;
        }

        case 0xF1: {
          set_8(cpu, &cpu->registers.c, 6);
          break;
        }

        case 0xF2: {
          set_8(cpu, &cpu->registers.d, 6);
          break;
        }

        case 0xF3: {
          set_8(cpu, &cpu->registers.e, 6);
          break;
        }

        case 0xF4: {
          set_8(cpu, &cpu->registers.h, 6);
          break;
        }

        case 0xF5: {
          set_8(cpu, &cpu->registers.l, 6);
          break;
        }

        case 0xF6: {
          set_8(cpu, &cpu->memory[cpu->registers.hl], 6);
          break;
        }

        case 0xF7: {
          set_8(cpu, &cpu->registers.a, 6);
          break;
        }

        case 0xF8: {
          set_8(cpu, &cpu->registers.b, 7);
          break;
        }

        case 0xF9: {
          set_8(cpu, &cpu->registers.c, 7);
          break;
        }

        case 0xFA: {
          set_8(cpu, &cpu->registers.d, 7);
          break;
        }

        case 0xFB: {
          set_8(cpu, &cpu->registers.e, 7);
          break;
        }

        case 0xFC: {
          set_8(cpu, &cpu->registers.h, 7);
          break;
        }

        case 0xFD: {
          set_8(cpu, &cpu->registers.l, 7);
          break;
        }

        case 0xFE: {
          set_8(cpu, &cpu->memory[cpu->registers.hl], 7);
          break;
        }

        case 0xFF: {
          set_8(cpu, &cpu->registers.a, 7);
          break;
        }
      }
      break;
    }

    case 0xCC: {
      // CALL Z, u16
      uint16_t address = fetch_16(cpu);
      if (get_zero_flag(&cpu->registers)) {
        cpu->has_jumped = 1;
        call(cpu, address);
      }
      break;
    }

    case 0xCD: {
      // CALL u16
      call(cpu, fetch_16(cpu));
      break;
    }

    case 0xCE: {
      // ADC A, u8
      adc_8(cpu, &cpu->registers.a, fetch_8(cpu));
      break;
    }

    case 0xCF: {
      // RST 0x08h
      rst(cpu, 0x08);
      break;
    }

    case 0xD0: {
      // RET NC
      if (!get_carry_flag(&cpu->registers)) {
        cpu->has_jumped = 1;
        pop_16(cpu, &cpu->pc);
      }
      break;
    }

    case 0xD1: {
      // POP DE
      pop_16(cpu, &cpu->registers.de);
      break;
    }

    case 0xD2: {
      // JP NC, u16
      uint16_t address = fetch_16(cpu);
      if (!get_carry_flag(&cpu->registers)) {
        cpu->has_jumped = 1;
        jp(cpu, address);
      }
      break;
    }

    case 0xD4: {
      // CALL NC, u16
      uint16_t address = fetch_16(cpu);
      if (!get_carry_flag(&cpu->registers)) {
        cpu->has_jumped = 1;
        call(cpu, address);
      }
      break;
    }

    case 0xD5: {
      // PUSH DE
      push_16(cpu, cpu->registers.de);
      break;
    }

    case 0xD6: {
      // SUB A, u8
      sub_8(cpu, &cpu->registers.a, fetch_8(cpu));
      break;
    }

    case 0xD7: {
      // RST 10h
      rst(cpu, 0x10);
      break;
    }

    case 0xD8: {
      // RET C
      if (get_carry_flag(&cpu->registers)) {
        cpu->has_jumped = 1;
        pop_16(cpu, &cpu->pc);
      }
      break;
    }

    case 0xD9: {
      // RETI
      pop_16(cpu, &cpu->pc);
      cpu->interrupts_enabled = 1;
      break;
    }

    case 0xDA: {
      // JP C, u16
      uint16_t address = fetch_16(cpu);
      if (get_carry_flag(&cpu->registers)) {
        cpu->has_jumped = 1;
        jp(cpu, address);
      }
      break;
    }

    case 0xDC: {
      // CALL C, u16
      uint16_t address = fetch_16(cpu);
      if (get_carry_flag(&cpu->registers)) {
        cpu->has_jumped = 1;
        call(cpu, address);
      }
      break;
    }

    case 0xDE: {
      // SBC A, u8
      sbc_8(cpu, &cpu->registers.a, fetch_8(cpu));
      break;
    }

    case 0xDF: {
      // RST 18h
      rst(cpu, 0x18);
      break;
    }

    case 0xE0: {
      // LD (FF00 + u8), A
      write_8(cpu, 0xFF00 + fetch_8(cpu), cpu->registers.a);
      break;
    }

    case 0xE1: {
      // POP HL
      pop_16(cpu, &cpu->registers.hl);
      break;
    }

    case 0xE2: {
      // LD (FF00 + C), A
      write_8(cpu, 0xFF00 + cpu->registers.c, cpu->registers.a);
      break;
    }

    case 0xE5: {
      // PUSH HL
      push_16(cpu, cpu->registers.hl);
      break;
    }

    case 0xE6: {
      // AND A, u8
      and_8(cpu, &cpu->registers.a, fetch_8(cpu));
      break;
    }

    case 0xE7: {
      // RST 20h
      rst(cpu, 0x20);
      break;
    }

    case 0xE8: {
      // ADD SP, i8
      //      add_16(cpu, &cpu->sp, fetch_8(cpu));
      uint16_t old_sp = cpu->sp;
      int8_t imm = fetch_8(cpu);

      int result = old_sp + imm;

      set_zero_flag(&cpu->registers, 0);
      set_substraction_flag(&cpu->registers, 0);
      set_half_carry_flag(&cpu->registers,
                          ((old_sp ^ imm ^ (result & 0xFFFF)) & 0x10) == 0x10);
      set_carry_flag(&cpu->registers,
                     ((old_sp ^ imm ^ (result & 0xFFFF)) & 0x100) == 0x100);

      cpu->sp = result & 0xFFFF;
      break;
    }

    case 0xE9: {
      // JP HL
      jp(cpu, cpu->registers.hl);
      break;
    }

    case 0xEA: {
      // LD (u16), A
      write_8(cpu, fetch_16(cpu), cpu->registers.a);
      break;
    }

    case 0xEE: {
      // XOR A, u8
      xor_8(cpu, &cpu->registers.a, fetch_8(cpu));
      break;
    }

    case 0xEF: {
      // RST 28h
      rst(cpu, 0x28);
      break;
    }

    case 0xF0: {
      // LD A, (FF00 + u8)

      cpu->registers.a = cpu->memory[0xFF00 + fetch_8(cpu)];
      break;
    }

    case 0xF1: {
      // POP AF
      pop_16(cpu, &cpu->registers.af);
      // Make sure to zero-out the last 4 bits?
      cpu->registers.f &= 0xF0;
      break;
    }

    case 0xF2: {
      // LD A, (FF00+C)
      cpu->registers.a = cpu->memory[0xFF00 + cpu->registers.c];
      break;
    }

    case 0xF3: {
      // DI
      cpu->interrupts_enabled = 0;
      break;
    }

    case 0xF5: {
      // PUSH AF
      push_16(cpu, cpu->registers.af);
      break;
    }

    case 0xF6: {
      // OR A, u8
      or_8(cpu, &cpu->registers.a, fetch_8(cpu));
      break;
    }

    case 0xF7: {
      // RST 30h
      rst(cpu, 0x30);
      break;
    }

    case 0xF8: {
      //       LD HL, SP + i8
      uint16_t temp = cpu->sp;
      int8_t imm = (int8_t)fetch_8(cpu);

      int result = temp + imm;

      set_zero_flag(&cpu->registers, 0);
      set_substraction_flag(&cpu->registers, 0);
      set_half_carry_flag(&cpu->registers,
                          ((temp ^ imm ^ (result & 0xFFFF)) & 0x10) == 0x10);
      set_carry_flag(&cpu->registers,
                     ((temp ^ imm ^ (result & 0xFFFF)) & 0x100) == 0x100);
      cpu->registers.hl = result & 0xFFFF;

      break;
    }

    case 0xF9: {
      // LD SP, HL
      cpu->sp = cpu->registers.hl;
      break;
    }

    case 0xFA: {
      // LD A, (u16)
      cpu->registers.a = cpu->memory[fetch_16(cpu)];
      break;
    }

    case 0xFB: {
      // EI
      cpu->interrupts_enabled = 1;
      break;
    }

    case 0xFE: {
      // CP A, u8
      cp_8(cpu, &cpu->registers.a, fetch_8(cpu));
      break;
    }

    case 0xFF: {
      // RST 38h
      rst(cpu, 0x38);
      break;
    }

    default: {
      fprintf(stderr, "Undiscovered instruction %x %x\n", opcode, cpu->pc);
      fflush(stderr);
      *(char*) NULL = 0xba;
      break;
    }
  }
}

uint8_t fetch_8(CPU *cpu) { return cpu->memory[cpu->pc++]; }

uint16_t fetch_16(CPU *cpu) {
  uint16_t result = get_16(&cpu->memory[cpu->pc]);
  cpu->pc += 2;
  return result;
}

void write_8(CPU *cpu, uint16_t address, uint8_t value) {
  cpu->memory[address] = value;
}

void write_16(CPU *cpu, uint16_t address, uint16_t value) {
  load_16(&cpu->memory[address], value);
}

void add_8(CPU *cpu, uint8_t *dest, uint8_t src) {
  uint8_t result = *dest + src;

  set_substraction_flag(&cpu->registers, 0);
  set_zero_flag(&cpu->registers, result == 0);
  set_half_carry_flag(&cpu->registers, ((*dest) & 0xF) > (0xF - (src & 0xF)));
  set_carry_flag(&cpu->registers, ((*dest) > (0xFF - src)));

  *dest = result;
}

void inc_8(CPU *cpu, uint8_t *dest) {
  uint8_t result = *dest + 1;

  set_substraction_flag(&cpu->registers, 0);
  set_zero_flag(&cpu->registers, result == 0);
  set_half_carry_flag(&cpu->registers, ((*dest) & 0xF) > (0xF - 1));

  *dest = result;
}

void sub_8(CPU *cpu, uint8_t *dest, uint8_t src) {
  uint8_t result = *dest - src;

  set_substraction_flag(&cpu->registers, 1);
  set_zero_flag(&cpu->registers, result == 0);
  set_half_carry_flag(&cpu->registers, ((*dest) & 0xF) < (src & 0xF));
  set_carry_flag(&cpu->registers, (*dest) < src);

  *dest = result;
}

void dec_8(CPU *cpu, uint8_t *dest) {
  uint8_t result = *dest - 1;

  set_zero_flag(&cpu->registers, result == 0);
  set_substraction_flag(&cpu->registers, 1);
  set_half_carry_flag(&cpu->registers, ((*dest & 0xF) == 0));

  *dest = result;
}

void adc_8(CPU *cpu, uint8_t *dest, uint8_t src) {
  uint8_t old_carry = get_carry_flag(&cpu->registers);
  uint16_t result = *dest + src + old_carry;

  set_zero_flag(&cpu->registers, (result & 0xFF) == 0);
  set_substraction_flag(&cpu->registers, 0);
  set_carry_flag(&cpu->registers, result > 0xFF);
  set_half_carry_flag(&cpu->registers,
                      ((*dest) & 0xF) + (src & 0xF) + old_carry > 0xF);

  *dest = result & 0xFF;
}

void sbc_8(CPU *cpu, uint8_t *dest, uint8_t src) {
  int32_t old_carry = get_carry_flag(&cpu->registers);

  int32_t result = *dest - src - old_carry;
  set_zero_flag(&cpu->registers, (result & 0xFF) == 0);
  set_substraction_flag(&cpu->registers, 1);

  set_carry_flag(&cpu->registers, result < 0x00);
  set_half_carry_flag(&cpu->registers,
                      (((*dest) & 0xF) - (src & 0xF) - (old_carry & 0xF)) < 0);
  *dest = result & 0xFF;
}

void add_16(CPU *cpu, uint16_t *dest, uint16_t src) {
  uint16_t result = *dest + src;
  set_substraction_flag(&cpu->registers, 0);
  // In case of half carry stuff not working break glass.
  set_half_carry_flag(&cpu->registers,
                      ((*dest) & 0x7FF) > (0x7FF - (src & 0x7FF)));
  set_carry_flag(&cpu->registers, (*dest) > ((0xFFFF) - src));

  *dest = result;
}

void rrc_8(CPU *cpu, uint8_t *dest) {
  uint8_t bit_0 = (*dest) & 1;

  set_zero_flag(&cpu->registers, *dest == 0);
  set_substraction_flag(&cpu->registers, 0);
  set_half_carry_flag(&cpu->registers, 0);

  (*dest) >>= 1;
  (*dest) |= bit_0 << 7;

  set_carry_flag(&cpu->registers, bit_0);
}

void rl_8(CPU *cpu, uint8_t *dest) {
  uint8_t bit_7 = (*dest) >> 7;

  set_substraction_flag(&cpu->registers, 0);
  set_half_carry_flag(&cpu->registers, 0);

  (*dest) <<= 1;
  (*dest) |= get_carry_flag(&cpu->registers);
  set_zero_flag(&cpu->registers, *dest == 0);
  set_carry_flag(&cpu->registers, bit_7);
}

void rlc_8(CPU *cpu, uint8_t *dest) {
  uint8_t bit_7 = (*dest) >> 7;

  set_zero_flag(&cpu->registers, *dest == 0);
  set_substraction_flag(&cpu->registers, 0);
  set_half_carry_flag(&cpu->registers, 0);
  set_carry_flag(&cpu->registers, bit_7);

  (*dest) <<= 1;
  (*dest) |= bit_7;
}

void rr_8(CPU *cpu, uint8_t *dest) {
  uint8_t bit_0 = (*dest) & 1;

  set_substraction_flag(&cpu->registers, 0);
  set_half_carry_flag(&cpu->registers, 0);

  (*dest) >>= 1;
  (*dest) |= get_carry_flag(&cpu->registers) * (1 << 7);

  set_zero_flag(&cpu->registers, *dest == 0);
  set_carry_flag(&cpu->registers, bit_0);
}

void and_8(CPU *cpu, uint8_t *dest, uint8_t src) {
  (*dest) &= src;

  set_zero_flag(&cpu->registers, *dest == 0);
  set_substraction_flag(&cpu->registers, 0);
  set_half_carry_flag(&cpu->registers, 1);
  set_carry_flag(&cpu->registers, 0);
}

void or_8(CPU *cpu, uint8_t *dest, uint8_t src) {
  (*dest) |= src;

  set_zero_flag(&cpu->registers, *dest == 0);
  set_substraction_flag(&cpu->registers, 0);
  set_half_carry_flag(&cpu->registers, 0);
  set_carry_flag(&cpu->registers, 0);
}

void xor_8(CPU *cpu, uint8_t *dest, uint8_t src) {
  (*dest) ^= src;

  set_zero_flag(&cpu->registers, *dest == 0);
  set_substraction_flag(&cpu->registers, 0);
  set_half_carry_flag(&cpu->registers, 0);
  set_carry_flag(&cpu->registers, 0);
}

void cp_8(CPU *cpu, uint8_t *dest, uint8_t src) {
  uint8_t result = *dest - src;

  set_substraction_flag(&cpu->registers, 1);
  set_zero_flag(&cpu->registers, result == 0x00);
  set_half_carry_flag(&cpu->registers, ((*dest) & 0xF) < (src & 0xF));
  set_carry_flag(&cpu->registers, (*dest) < src);
}

void pop_16(CPU *cpu, uint16_t *dest) {
  *dest = get_16(&cpu->memory[cpu->sp]);
  cpu->sp += 2;
}

void push_16(CPU *cpu, uint16_t dest) {
  cpu->sp -= 2;
  load_16(&cpu->memory[cpu->sp], dest);
}

void jr(CPU *cpu, int8_t offset) { cpu->pc = (int16_t)cpu->pc + offset; }

void jp(CPU *cpu, uint16_t address) { cpu->pc = address; }

void call(CPU *cpu, uint16_t address) {
  push_16(cpu, cpu->pc);
  jp(cpu, address);
}

void rst(CPU *cpu, uint8_t dest) { call(cpu, dest); }

void bit_test(CPU *cpu, uint8_t dest, uint8_t bit) {
  set_zero_flag(&cpu->registers, !(dest & (1u << bit)));
  set_half_carry_flag(&cpu->registers, 1);
  set_substraction_flag(&cpu->registers, 0);
}

void sla_8(CPU *cpu, uint8_t *dest) {
  uint8_t bit_7 = (*dest) >> 7;
  (*dest) <<= 1;
  (*dest) &= ~1;
  set_zero_flag(&cpu->registers, (*dest) == 0x00);
  set_carry_flag(&cpu->registers, bit_7);
  set_substraction_flag(&cpu->registers, 0);
  set_half_carry_flag(&cpu->registers, 0);
}

void sra_8(CPU *cpu, uint8_t *dest) {
  uint8_t bit_0 = (*dest) & 1;
  uint8_t bit_7 = (*dest) >> 7;
  (*dest) >>= 1;
  (*dest) |= (bit_7) * (1 << 7);
  set_zero_flag(&cpu->registers, (*dest) == 0x00);
  set_carry_flag(&cpu->registers, bit_0);
  set_substraction_flag(&cpu->registers, 0);
  set_half_carry_flag(&cpu->registers, 0);
}

void swap_8(CPU *cpu, uint8_t *dest) {
  uint8_t lower_nibble = (*dest) & 0xF;
  uint8_t upper_nibble = ((*dest) >> 4);
  (*dest) = (lower_nibble << 4) | upper_nibble;

  set_zero_flag(&cpu->registers, (*dest) == 0x00);
  set_substraction_flag(&cpu->registers, 0);
  set_half_carry_flag(&cpu->registers, 0);
  set_carry_flag(&cpu->registers, 0);
}

void srl_8(CPU *cpu, uint8_t *dest) {
  uint8_t bit_0 = (*dest) & 1;
  (*dest) >>= 1;
  (*dest) &= ~(1 << 7);

  set_zero_flag(&cpu->registers, (*dest) == 0x00);
  set_substraction_flag(&cpu->registers, 0);
  set_half_carry_flag(&cpu->registers, 0);
  set_carry_flag(&cpu->registers, bit_0);
}

void res_8(CPU *cpu, uint8_t *dest, uint8_t bit) {
  (void)cpu;
  (*dest) &= ~(1 << bit);
}

void set_8(CPU *cpu, uint8_t *dest, uint8_t bit) {
  (void)cpu;
  (*dest) |= (1 << bit);
}

void daa_8(CPU *cpu, uint8_t *dest) {
  if (!get_substraction_flag(&cpu->registers)) {
    if (get_carry_flag(&cpu->registers) || (*dest) > 0x99) {
      (*dest) += 0x60;
      set_carry_flag(&cpu->registers, 1);
    }
    if (get_half_carry_flag(&cpu->registers) || ((*dest) & 0x0f) > 0x09) {
      (*dest) += 0x6;
    }
  } else {
    if (get_carry_flag(&cpu->registers)) {
      (*dest) -= 0x60;
    }
    if (get_half_carry_flag(&cpu->registers)) {
      (*dest) -= 0x6;
    }
  }
  set_zero_flag(&cpu->registers, (*dest) == 0);
  set_half_carry_flag(&cpu->registers, 0);
}
