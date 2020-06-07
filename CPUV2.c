#include <CPUV2.h>

uint16_t fetch16(Cpu *cpu) {
  uint16_t result = cpu->memory[cpu->registers.pc] |
                    (cpu->memory[cpu->registers.pc + 1] << 8);
  cpu->registers.pc += 2;
  return result;
}

uint8_t fetch8(Cpu *cpu) { return cpu->memory[cpu->registers.pc++]; }

void cycle(Cpu *cpu) {
  cpu->opcode = fetch8(cpu);
  Instruction current_instruction = cpu->instruction_lut[cpu->opcode];

  cpu->wait_cycles = current_instruction.cycles;

  // Updating (HL) address helper
  cpu->fetch_reg[6] = &cpu->memory[cpu->registers.hl];

  current_instruction.fetch(cpu);
  if (current_instruction.mnemonic[0] == 0) {
    printf("%s -> %02X", "Not found", cpu->opcode);
  }
  printf("%s \n", current_instruction.mnemonic);

  current_instruction.execute(cpu);
  while (cpu->wait_cycles--)
    ;
}

// God forgive me for what I am about to do.
void init_instruction_lut(Cpu *cpu) {
  INSTR(0x16, "LD D, u8", 19, 20, purple_fetch, ld8);
  INSTR(0x36, "LD (HL), u8", 20, 10, purple_fetch, ld8);
  INSTR(0x0B, "DEC BC", 20, 20, fetch_dark_green, dec16);
  INSTR(0xCD, "CALL u16,", 20, 20, fetch_orange, call);
  INSTR(0xC9, "RET", 20, 20, fetch_orange, ret);
  INSTR(0xC5, "PUSH BC", 20, 20, fetch_dark_purple, push16);
  INSTR(0xD1, "POP DE", 20, 20, fetch_dark_purple, pop16);
  INSTR(0xF7, "RST 30h", 20, 20, fetch_orange, rst);
}

uint8_t *get_ind_1(Cpu *cpu, uint8_t offset) {
  uint8_t *result;
  switch (offset) {
    case 0x0: {
      result = &cpu->memory[cpu->registers.bc];
    } break;
    case 0x1: {
      result = &cpu->memory[cpu->registers.de];
    } break;
    case 0x2: {
      result = &cpu->memory[cpu->registers.hl];
      cpu->registers.hl++;
    } break;
    case 0x3: {
      result = &cpu->memory[cpu->registers.hl];
      cpu->registers.hl--;
    } break;
    default:
      result = NULL;
      break;
  }

  return result;
}

void purple_fetch(Cpu *cpu) {
  switch (cpu->opcode) {
    case 0x00 ... 0x3F: {
      uint8_t hi_nib = cpu->opcode >> 4;
      uint8_t lo_nib = cpu->opcode & 0xF;

      switch (lo_nib) {
        case 0x2: {
          cpu->src8 = cpu->registers.a;
          cpu->dest8 = get_ind_1(cpu, hi_nib);
        } break;
        case 0x6: {
          cpu->src8 = fetch8(cpu);
          cpu->dest8 = cpu->fetch_reg[hi_nib * 2];
        } break;
        case 0xA: {
          cpu->dest8 = &cpu->registers.a;
          cpu->src8 = *get_ind_1(cpu, hi_nib);
        } break;
        case 0xE: {
          cpu->src8 = fetch8(cpu);
          cpu->dest8 = cpu->fetch_reg[hi_nib * 2 + 1];
        } break;
        default:
          break;
      }
    } break;
    case 0x40 ... 0x7F: {
      uint8_t offset = cpu->opcode - 0x40;
      cpu->src8 = *(cpu->fetch_reg[offset & 0x7]);
      cpu->dest8 = (cpu->fetch_reg[(offset >> 3)]);
    } break;
    case 0xE0: {
      cpu->src8 = cpu->registers.a;
      cpu->dest8 = &cpu->memory[0xFF00 + fetch8(cpu)];
    } break;
    case 0xF0: {
      cpu->src8 = cpu->memory[0xFF00 + fetch8(cpu)];
      cpu->dest8 = &cpu->registers.a;
    } break;
    case 0xE2: {
      cpu->src8 = cpu->registers.a;
      cpu->dest8 = &cpu->memory[0xFF00 + cpu->registers.c];
    } break;
    case 0xF2: {
      cpu->src8 = cpu->memory[0xFF00 + cpu->registers.c];
      cpu->dest8 = &cpu->registers.a;
    } break;
    case 0xEA: {
      cpu->src8 = cpu->registers.a;
      cpu->dest8 = &cpu->memory[fetch16(cpu)];
    } break;
    case 0xFA: {
      cpu->src8 = cpu->memory[fetch16(cpu)];
      cpu->dest8 = &cpu->registers.a;
    } break;
  }
}

void fetch_none(Cpu *cpu) {
  (void)cpu;
  return;
}

void fetch_green(Cpu *cpu) {
  switch (cpu->opcode) {
    case 0x00 ... 0x3F: {
      uint8_t hi_nib = cpu->opcode >> 4;
      uint8_t lo_nib = cpu->opcode & 0xF;

      cpu->dest8 = cpu->fetch_reg[((lo_nib >> 3) != 0) + 2 * hi_nib];
      cpu->src8 = 0;

    } break;
    case 0x80 ... 0xBF: {
      cpu->dest8 = &cpu->registers.a;

      uint8_t offset = cpu->opcode - 0x80;
      cpu->src8 = *(cpu->fetch_reg[offset & 0x7]);
      cpu->dest8 = (cpu->fetch_reg[(offset >> 3)]);

    } break;
    case 0xC0 ... 0xFF: {
      cpu->dest8 = &cpu->registers.a;
      cpu->src8 = fetch8(cpu);
    }
  }
}

void fetch_dark_purple(Cpu *cpu) {
  uint8_t hi_nib = cpu->opcode >> 4;
  uint8_t lo_nib = cpu->opcode & 0xF;

  switch (cpu->opcode) {
    case 0x00 ... 0x3F: {
      switch (lo_nib) {
        case 0x1: {
          cpu->dest16 = cpu->fetch_dreg[hi_nib];
          cpu->src16 = fetch16(cpu);
        } break;
        case 0x8: {
          cpu->dest16 = (uint16_t *)(&cpu->memory[fetch16(cpu)]);
          cpu->src16 = cpu->registers.sp;
        } break;
      }
    } break;
    case 0xC0 ... 0xEF: {
      switch (lo_nib) {
        case 0x1: {
          cpu->dest16 = cpu->fetch_dreg[hi_nib - 0xC];
        } break;
        case 0x5: {
          cpu->src16 = *cpu->fetch_dreg[hi_nib - 0xC];
        } break;
      }
    } break;
    case 0xF1: {
      cpu->dest16 = &cpu->registers.af;
    } break;
    case 0xF5: {
      cpu->src16 = cpu->registers.af;
    } break;
    case 0xF9: {
      cpu->dest16 = &cpu->registers.sp;
      cpu->src16 = cpu->registers.hl;
    } break;

    default:
      break;
  }
}

void fetch_dark_green(Cpu *cpu) {
  switch (cpu->opcode) {
    case 0x00 ... 0x3F: {
      uint8_t hi_nib = cpu->opcode >> 4;
      uint8_t lo_nib = cpu->opcode & 0xF;

      switch (lo_nib) {
        case 0x9: {
          cpu->dest16 = &cpu->registers.hl;
          cpu->src16 = *cpu->fetch_dreg[hi_nib];
        } break;
        default:
          cpu->dest16 = cpu->fetch_dreg[hi_nib];
          break;
      }

    } break;
    case 0xE8: {
      cpu->dest16 = &cpu->registers.sp;
      cpu->src8 = fetch8(cpu);
    } break;
    case 0xF8: {
      cpu->dest16 = &cpu->registers.hl;
      int16_t temp = cpu->registers.sp;
      temp += (int16_t)fetch8(cpu);
      cpu->src16 = (uint16_t)temp;
    } break;

    default:
      break;
  }
}

uint8_t get_mask(uint8_t offset) {
  uint8_t hi_nib = offset >> 4;
  uint8_t lo_nib = offset & 0xF;

  uint8_t mask = 0;

  if (lo_nib <= 0xF) {
    mask = 0;
  } else {
    mask = 1;
  }

  if ((hi_nib & 1) == 0) {
    mask |= FLAG_Z;
  } else {
    mask |= FLAG_C;
  }
  return mask;
}

void fetch_orange(Cpu *cpu) {
  uint8_t lo_nib = cpu->opcode & 0xF;

  switch (cpu->opcode) {
    case 0x00 ... 0x3F: {
      if (cpu->opcode >= 0x20) {
        cpu->src8 = get_mask(cpu->opcode - 0x20);
      }
      // TODO Fix condintional jumps reeee
      *(cpu->dest8) = fetch8(cpu);
    } break;
    case 0xC0 ... 0xFF: {
      switch (lo_nib) {
        case 0x02 ... 0x04: {
          cpu->src16 = fetch16(cpu);
        } break;
        case 0xA ... 0xD: {
          cpu->src16 = fetch16(cpu);
        } break;
      }
      switch (lo_nib) {
        case 0x3: {
        } break;
        case 0xD: {
        } break;
        default: {
          cpu->src8 = get_mask(cpu->opcode - 0xC0);
          break;
        }
      }
    } break;
  }
}

void fetch_blue(Cpu *cpu) {
  // INFO: Where we get the opcode for the instruction depends on the 0xCB
  // handler
  uint8_t lo_nib = cpu->opcode & 0xF;
  cpu->dest8 = cpu->fetch_reg[lo_nib & 0x7];
}

void init_cpu(Cpu *cpu) {
  cpu->fetch_reg[0] = &cpu->registers.b;
  cpu->fetch_reg[1] = &cpu->registers.c;
  cpu->fetch_reg[2] = &cpu->registers.d;
  cpu->fetch_reg[3] = &cpu->registers.e;
  cpu->fetch_reg[4] = &cpu->registers.h;
  cpu->fetch_reg[5] = &cpu->registers.l;
  cpu->fetch_reg[6] = 0;  // Needs to be updated every cycle
  cpu->fetch_reg[7] = &cpu->registers.a;

  cpu->fetch_dreg[0] = &cpu->registers.bc;
  cpu->fetch_dreg[1] = &cpu->registers.de;
  cpu->fetch_dreg[2] = &cpu->registers.hl;
  cpu->fetch_dreg[3] = &cpu->registers.sp;
}

uint8_t read8(Cpu *cpu) { return cpu->memory[cpu->registers.pc]; }

void init_debug_cpu(Cpu *cpu) { memset(cpu, 0, sizeof(Cpu)); }

void ld16(Cpu *cpu) { *cpu->dest16 = cpu->src16; }

void ld8(Cpu *cpu) { *(cpu->dest8) = cpu->src8; }

void dec16(Cpu *cpu) { (*(cpu->dest16))--; }

void inc16(Cpu *cpu) { (*(cpu->dest16))++; }

void push16(Cpu *cpu) {
  cpu->registers.sp -= 2;
  load16(&cpu->memory[cpu->registers.sp], cpu->src16);
}

void pop16(Cpu *cpu) {
  *cpu->dest16 = cpu->memory[cpu->registers.sp];
  cpu->registers.sp += 2;
}

void call(Cpu *cpu) {
  uint8_t condition = flag_condition(cpu);

  if (condition) {
    cpu->registers.sp -= 2;
    load16(&cpu->memory[cpu->registers.sp], cpu->registers.pc);
    cpu->registers.pc = cpu->src16;
  }
}
// TODO conditional calls and returns do not work :^)
void load16(uint8_t *dest, uint16_t val) {
  *dest = val & 0xFF;
  *(dest + 1) = val >> 8;
}

uint16_t get16(uint8_t *src) { return (*(src + 1) << 8) + (*src); }

void ret(Cpu *cpu) {
  uint8_t condition = flag_condition(cpu);
  if (condition) {
    cpu->registers.pc = get16(&cpu->memory[cpu->registers.sp]);
    cpu->registers.sp += 2;
  }
}

uint8_t get_flag(Cpu *cpu, uint8_t flag) {
  return (cpu->registers.f & flag) != 0;
}

void set_flag(Cpu *cpu, uint8_t flag, uint8_t value) {
  if (value) {
    cpu->registers.f |= (flag);
  } else {
    cpu->registers.f &= (~flag);
  }
}

void add8(Cpu *cpu) {
  uint8_t dest = *cpu->dest8;
  uint8_t src = cpu->src8;

  uint8_t result = *cpu->dest8 + cpu->src8;

  set_flag(cpu, FLAG_S, 0);
  set_flag(cpu, FLAG_Z, result == 0);
  set_flag(cpu, FLAG_H, (dest & 0xF) > (0xF - (src & 0xF)));
  set_flag(cpu, FLAG_C, (dest > (0xFF - src)));

  *cpu->dest8 = result & 0xFF;
}

void adc8(Cpu *cpu) {
  uint8_t dest = *cpu->dest8;
  uint8_t src = cpu->src8;
  uint8_t old_carry = get_flag(cpu, FLAG_C);

  uint16_t result = *cpu->dest8 + cpu->src8 + old_carry;

  set_flag(cpu, FLAG_S, 0);
  set_flag(cpu, FLAG_Z, (result & 0xFF) == 0);
  set_flag(cpu, FLAG_H, (dest & 0xF) + (src & 0xF) + old_carry > 0xF);
  set_flag(cpu, FLAG_C, result > 0xFF);

  *cpu->dest8 = result & 0xFF;
}

void sub8(Cpu *cpu) {
  uint8_t dest = *cpu->dest8;
  uint8_t src = cpu->src8;
  uint8_t result = dest - src;

  set_flag(cpu, FLAG_S, 1);
  set_flag(cpu, FLAG_Z, result == 0);
  set_flag(cpu, FLAG_H, (dest & 0xF) < (src & 0xF));
  set_flag(cpu, FLAG_C, dest < src);

  *cpu->dest8 = result & 0xFF;
}

void sbc8(Cpu *cpu) {
  uint8_t dest = *cpu->dest8;
  uint8_t src = cpu->src8;
  uint8_t old_carry = get_flag(cpu, FLAG_C);

  int32_t result = dest - src - old_carry;

  set_flag(cpu, FLAG_Z, (result & 0xFF) == 0);
  set_flag(cpu, FLAG_S, 1);
  set_flag(cpu, FLAG_C, result < 0x00);
  set_flag(cpu, FLAG_H, (dest & 0xF) - (src & 0xF) - old_carry < 0);

  *cpu->dest8 = result & 0xFF;
}

void and8(Cpu *cpu) {
  uint8_t dest = *cpu->dest8;
  uint8_t src = cpu->src8;

  uint8_t result = dest & src;

  set_flag(cpu, FLAG_Z, result == 0);
  set_flag(cpu, FLAG_S, 0);
  set_flag(cpu, FLAG_H, 1);
  set_flag(cpu, FLAG_C, 0);

  *cpu->dest8 = result & 0xFF;
}

void xor8(Cpu *cpu) {
  uint8_t dest = *cpu->dest8;
  uint8_t src = cpu->src8;

  uint8_t result = dest ^ src;

  set_flag(cpu, FLAG_Z, result == 0);
  set_flag(cpu, FLAG_S, 0);
  set_flag(cpu, FLAG_H, 0);
  set_flag(cpu, FLAG_C, 0);

  *cpu->dest8 = result & 0xFF;
}

void or8(Cpu *cpu) {
  uint8_t dest = *cpu->dest8;
  uint8_t src = cpu->src8;

  uint8_t result = dest | src;

  set_flag(cpu, FLAG_Z, result == 0);
  set_flag(cpu, FLAG_S, 0);
  set_flag(cpu, FLAG_H, 0);
  set_flag(cpu, FLAG_C, 0);

  *cpu->dest8 = result & 0xFF;
}

void cp8(Cpu *cpu) {
  uint8_t dest = *cpu->dest8;
  uint8_t src = cpu->src8;

  uint8_t result = dest - src;

  set_flag(cpu, FLAG_S, 1);
  set_flag(cpu, FLAG_Z, result == 0);
  set_flag(cpu, FLAG_H, (dest & 0xF) < (src & 0xF));
  set_flag(cpu, FLAG_C, dest < src);
}

void add16(Cpu *cpu) {
  uint16_t dest = *cpu->dest8;
  uint16_t src = cpu->src8;

  uint16_t result = dest + src;

  set_flag(cpu, FLAG_S, 0);
  set_flag(cpu, FLAG_H, (dest & 0xFFF) + (src & 0xFFF) > 0xFFF);
  set_flag(cpu, FLAG_C, (dest) > ((0xFFFF) - src));

  *cpu->dest16 = result;
}
// ping google.com
// nu o sa stii niciodata
// nu o sa stii niciodata
void rst(Cpu *cpu) {
  cpu->src16 = cpu->opcode - 0xC7;
  call(cpu);
}

void jp(Cpu *cpu) {
  uint8_t condition = flag_condition(cpu);

  if (condition) {
    cpu->registers.pc = cpu->src16;
  }
}

uint8_t flag_condition(Cpu *cpu) {
  uint8_t hi_nib = cpu->opcode >> 4;
  uint8_t lo_nib = cpu->opcode & 0xF;

  uint8_t condition = 1;

  switch (cpu->opcode) {
    case 0x20 ... 0x3F: {
      if (!(hi_nib & 0x1)) {
        condition =
            lo_nib <= 0x7 ? !get_flag(cpu, FLAG_Z) : get_flag(cpu, FLAG_Z);
      } else {
        condition =
            lo_nib <= 0x7 ? !get_flag(cpu, FLAG_C) : get_flag(cpu, FLAG_C);
      }
    } break;
    case 0xC0 ... 0xDF: {
      if (hi_nib & 0x1) {
        condition =
            lo_nib <= 0x7 ? !get_flag(cpu, FLAG_Z) : get_flag(cpu, FLAG_Z);
      } else {
        condition =
            lo_nib <= 0x7 ? !get_flag(cpu, FLAG_C) : get_flag(cpu, FLAG_C);
      }
    }
  }
  if (cpu->opcode == 0xC3 || cpu->opcode == 0xCD || cpu->opcode == 0xC9) {
    condition = 1;
  }
  return condition;
}

void nop(Cpu *cpu) {}

void jr(Cpu *cpu) {
  uint8_t condition = flag_condition(cpu);

  if (condition) {
    int16_t new_pc = cpu->registers.pc;
    new_pc += (int8_t)cpu->src8;
    cpu->registers.pc = (uint16_t)new_pc;
  }
}

void cb(Cpu *cpu) {
  uint8_t op = fetch8(cpu);
  uint8_t index;

  switch (op) {
    case 0x00 ... 0x3F: {
      index = op >> 3;
    }
    case 0x40 ... 0xFF: {
      op = op - 0x40;
      index = 8;

      index += op / 64;
    }
  }

  Instruction *cb_instr = &cpu->instruction_cb_lut[index];

  cb_instr->fetch(cpu);
  cb_instr->execute(cpu);

  cpu->wait_cycles += cb_instr->cycles;
}

void rlc(Cpu *cpu) {
  uint8_t dest = *cpu->dest8;
  uint8_t carry = dest >> 7;

  set_flag(cpu, FLAG_Z, dest == 0);
  set_flag(cpu, FLAG_S, 0);
  set_flag(cpu, FLAG_H, 0);
  set_flag(cpu, FLAG_C, carry);

  *cpu->dest8 <<= 1;
  *cpu->dest8 |= carry;
}

void rrc(Cpu *cpu) {
  uint8_t dest = *cpu->dest8;
  uint8_t carry = dest & 0x1;

  set_flag(cpu, FLAG_Z, dest == 0);
  set_flag(cpu, FLAG_S, 0);
  set_flag(cpu, FLAG_H, 0);
  set_flag(cpu, FLAG_C, carry);

  *cpu->dest8 >>= 1;
  *cpu->dest8 |= (carry << 7);
}

void rl(Cpu *cpu) {
  uint8_t dest = *cpu->dest8;
  uint8_t carry = dest >> 7;

  *cpu->dest8 <<= 1;
  *cpu->dest8 |= get_flag(cpu, FLAG_Z);

  set_flag(cpu, FLAG_Z, *cpu->dest8 == 0);
  set_flag(cpu, FLAG_S, 0);
  set_flag(cpu, FLAG_H, 0);
  set_flag(cpu, FLAG_C, carry);
}
void rr(Cpu *cpu) {
  uint8_t dest = *cpu->dest8;
  uint8_t carry = dest & 0x1;

  *cpu->dest8 >>= 1;
  *cpu->dest8 |= (get_flag(cpu, FLAG_Z) << 7);

  set_flag(cpu, FLAG_Z, *cpu->dest8 == 0);
  set_flag(cpu, FLAG_S, 0);
  set_flag(cpu, FLAG_H, 0);
  set_flag(cpu, FLAG_C, carry);
}

void sla(Cpu *cpu) {
  uint8_t dest = *cpu->dest8;
  uint8_t carry = dest >> 7;
  dest <<= 1;

  *cpu->dest8 = dest;
  set_flag(cpu, FLAG_Z, dest == 0);
  set_flag(cpu, FLAG_C, carry);
  set_flag(cpu, FLAG_H, 0);
  set_flag(cpu, FLAG_S, 0);
}

void sra(Cpu *cpu) {
  uint8_t dest = *cpu->dest8;
  uint8_t carry = dest & 0x1;
  dest >>= 1;
  dest |= (carry << 7);

  *cpu->dest8 = dest;
  set_flag(cpu, FLAG_Z, dest == 0);
  set_flag(cpu, FLAG_C, carry);
  set_flag(cpu, FLAG_H, 0);
  set_flag(cpu, FLAG_S, 0);
}

void swap(Cpu *cpu) {
  uint8_t dest = *cpu->dest8;

  dest >>= 4;
  dest |= ((*cpu->dest8 & 0xF) << 4);

  *cpu->dest8 = dest;

  set_flag(cpu, FLAG_Z, dest == 0);
  set_flag(cpu, FLAG_S, 0);
  set_flag(cpu, FLAG_H, 0);
  set_flag(cpu, FLAG_C, 0);
}

void srl(Cpu *cpu) {
  uint8_t dest = *cpu->dest8;
  uint8_t carry = dest & 0x1;
  dest >>= 1;

  *cpu->dest8 = dest;

  set_flag(cpu, FLAG_C, carry);
  set_flag(cpu, FLAG_Z, dest == 0);
  set_flag(cpu, FLAG_H, 0);
  set_flag(cpu, FLAG_S, 0);
}

void bit(Cpu *cpu) { uint8_t bit; }

/*
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */
