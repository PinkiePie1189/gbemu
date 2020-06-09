#include "CPUV2.h"

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

#if pop == 0
  printf("\nPC:%04X [%02X]\t -> %s ", cpu->registers.pc - 0x1, cpu->opcode,
         current_instruction.mnemonic);
#endif
  current_instruction.fetch(cpu);
  fflush(stdout);
  current_instruction.execute(cpu);
  while (cpu->wait_cycles--)
    ;
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
          DEBUG("%04X %02X", (uint16_t)((long)cpu->dest8 - (long)cpu->memory),
                cpu->src8);
        } break;
        case 0x6: {
          cpu->src8 = fetch8(cpu);
          cpu->dest8 = cpu->fetch_reg[hi_nib * 2];
          DEBUG("%s %02X", get_register_name(hi_nib * 2), cpu->src8);
        } break;
        case 0xA: {
          cpu->dest8 = &cpu->registers.a;
          cpu->src8 = *get_ind_1(cpu, hi_nib);
          DEBUG("%s %02X", get_register_name(7), cpu->src8);
        } break;
        case 0xE: {
          cpu->src8 = fetch8(cpu);
          cpu->dest8 = cpu->fetch_reg[hi_nib * 2 + 1];
          DEBUG("%s %02X", get_register_name(hi_nib * 2 + 1), cpu->src8);
        } break;
        default:
          break;
      }
    } break;
    case 0x40 ... 0x7F: {
      uint8_t offset = cpu->opcode - 0x40;
      cpu->src8 = *(cpu->fetch_reg[offset & 0x7]);
      cpu->dest8 = (cpu->fetch_reg[(offset >> 3)]);
      DEBUG("%s %s", get_register_name(offset >> 3),
            get_register_name(offset & 0x7));
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

      cpu->dest8 = cpu->fetch_reg[(lo_nib >> 3) + 2 * hi_nib];
      cpu->src8 = 0;
      DEBUG("%s", get_register_name((lo_nib >> 3) + 2 * hi_nib));
    } break;
    case 0x80 ... 0xBF: {
      cpu->dest8 = &cpu->registers.a;

      uint8_t offset = cpu->opcode - 0x80;
      cpu->src8 = *(cpu->fetch_reg[offset & 0x7]);
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
          DEBUG("%s", get_dreg_name(hi_nib - 0xC));
        } break;
        case 0x5: {
          cpu->src16 = *cpu->fetch_dreg[hi_nib - 0xC];
          DEBUG("%s", get_dreg_name(hi_nib - 0xC));
        } break;
      }
    } break;
    case 0xF1: {
      cpu->dest16 = &cpu->registers.af;
      DEBUG("%s", "AF");
    } break;
    case 0xF5: {
      cpu->src16 = cpu->registers.af;
      DEBUG("%s", "AF");
    } break;
    case 0xF9: {
      cpu->dest16 = &cpu->registers.sp;
      cpu->src16 = cpu->registers.hl;
      DEBUG("%s %s", "SP", "HL");
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

void fetch_orange(Cpu *cpu) {
  uint8_t lo_nib = cpu->opcode & 0xF;

  switch (cpu->opcode) {
    case 0x00 ... 0x3F: {
      cpu->src8 = fetch8(cpu);
      DEBUG("%02X", cpu->src8);
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
      DEBUG("%04X", cpu->src16);
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

  cpu->registers.pc = 0x0100;
  cpu->is_halted = 0;
  //  cpu->st = 0;
  cpu->int_enabled = 0;

  // Simulate bios
  cpu->registers.af = 0x11B0;
  cpu->registers.bc = 0x0013;
  cpu->registers.de = 0x00D8;
  cpu->registers.hl = 0x014D;

  cpu->registers.sp = 0xFFFE;

  // Setup memory
  memset(cpu->memory, 0, sizeof(cpu->memory));
  cpu->memory[0xFF05] = 0x00;
  cpu->memory[0xFF06] = 0x00;
  cpu->memory[0xFF07] = 0x00;
  cpu->memory[0xFF10] = 0x80;
  cpu->memory[0xFF11] = 0xBF;
  cpu->memory[0xFF12] = 0xF3;
  cpu->memory[0xFF14] = 0xBF;
  cpu->memory[0xFF16] = 0x3F;
  cpu->memory[0xFF17] = 0x00;
  cpu->memory[0xFF19] = 0xBF;
  cpu->memory[0xFF1A] = 0x7F;
  cpu->memory[0xFF1B] = 0xFF;
  cpu->memory[0xFF1C] = 0x9F;
  cpu->memory[0xFF1E] = 0xBF;
  cpu->memory[0xFF21] = 0x00;
  cpu->memory[0xFF22] = 0x00;
  cpu->memory[0xFF23] = 0xBF;
  cpu->memory[0xFF24] = 0x77;
  cpu->memory[0xFF25] = 0xF3;
  cpu->memory[0xFF26] = 0xF1;
  cpu->memory[0xFF40] = 0x91;
  cpu->memory[0xFF42] = 0x00;
  cpu->memory[0xFF43] = 0x00;
  cpu->memory[0xFF45] = 0x00;
  cpu->memory[0xFF47] = 0xFC;
  cpu->memory[0xFF48] = 0xFF;
  cpu->memory[0xFF49] = 0xFF;
  cpu->memory[0xFF4A] = 0x00;
  cpu->memory[0xFF4B] = 0x00;
  cpu->memory[0xFFFF] = 0x00;
}

uint8_t read8(Cpu *cpu) { return cpu->memory[cpu->registers.pc]; }

void ld16(Cpu *cpu) { *cpu->dest16 = cpu->src16; }

void ld8(Cpu *cpu) { *(cpu->dest8) = cpu->src8; }

void dec16(Cpu *cpu) { (*(cpu->dest16))--; }

void inc16(Cpu *cpu) { (*(cpu->dest16))++; }

void push16(Cpu *cpu) {
  cpu->registers.sp -= 2;
  load16(&cpu->memory[cpu->registers.sp], cpu->src16);
}

void pop16(Cpu *cpu) {
  *cpu->dest16 = get16(&cpu->memory[cpu->registers.sp]);
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
  uint16_t dest = *cpu->dest16;
  uint16_t src = cpu->src16;

  uint16_t result = dest + src;

  set_flag(cpu, FLAG_S, 0);
  set_flag(cpu, FLAG_H, (dest & 0xFFF) + (src & 0xFFF) > 0xFFF);
  set_flag(cpu, FLAG_C, (dest) > ((0xFFFF) - src));

  *cpu->dest16 = result;
}

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

void nop8(Cpu *cpu) { (void)cpu; }

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
  cpu->opcode = op;
  switch (op) {
    case 0x00 ... 0x3F: {
      index = op >> 3;
    } break;
    case 0x40 ... 0xFF: {
      op = op - 0x40;
      index = op / 64 + 8;
    } break;
    default: {
      index = 0;
    } break;
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
  *cpu->dest8 |= get_flag(cpu, FLAG_C);

  set_flag(cpu, FLAG_Z, *cpu->dest8 == 0);
  set_flag(cpu, FLAG_S, 0);
  set_flag(cpu, FLAG_H, 0);
  set_flag(cpu, FLAG_C, carry);
}
void rr(Cpu *cpu) {
  uint8_t dest = *cpu->dest8;
  uint8_t carry = dest & 0x1;

  *cpu->dest8 >>= 1;
  *cpu->dest8 |= (get_flag(cpu, FLAG_C) << 7);

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

void bit(Cpu *cpu) {
  uint8_t opcode = cpu->opcode;

  uint8_t hi_nib = opcode >> 4;
  uint8_t lo_nib = opcode & 0xF;

  uint8_t bit = (hi_nib - 0x4) * 2 + (lo_nib >> 3);
  bit = 1 << bit;

  set_flag(cpu, FLAG_Z, (bit & *cpu->dest8) == 0);
  set_flag(cpu, FLAG_S, 0);
  set_flag(cpu, FLAG_H, 1);
}

void res(Cpu *cpu) {
  uint8_t opcode = cpu->opcode;

  uint8_t hi_nib = opcode >> 4;
  uint8_t lo_nib = opcode & 0xF;

  uint8_t bitm = (hi_nib - 0x4) * 2 + (lo_nib >> 3);
  bitm = ~(1 << bitm);

  *cpu->dest8 &= bitm;
}

void set(Cpu *cpu) {
  uint8_t opcode = cpu->opcode;

  uint8_t hi_nib = opcode >> 4;
  uint8_t lo_nib = opcode & 0xF;

  uint8_t bitm = (hi_nib - 0x4) * 2 + (lo_nib >> 3);
  bitm = 1 << bitm;

  *cpu->dest8 |= bitm;
}

void rlca(Cpu *cpu) {
  cpu->dest8 = &cpu->registers.a;
  rlc(cpu);
  set_flag(cpu, FLAG_Z, 0);
}

void rla(Cpu *cpu) {
  cpu->dest8 = &cpu->registers.a;
  rl(cpu);
  set_flag(cpu, FLAG_Z, 0);
}

void rrca(Cpu *cpu) {
  cpu->dest8 = &cpu->registers.a;
  rrc(cpu);
  set_flag(cpu, FLAG_Z, 0);
}

void rra(Cpu *cpu) {
  cpu->dest8 = &cpu->registers.a;
  rr(cpu);
  set_flag(cpu, FLAG_Z, 0);
}

void ei(Cpu *cpu) { cpu->int_enabled = 1; }
void di(Cpu *cpu) { cpu->int_enabled = 0; }

void stop(Cpu *cpu) {
  cpu->src8 = 0xCE;
  exit(0);
}

void hlt(Cpu *cpu) {
  cpu->src8 = 0xCE;
  exit(0);
}

void inc8(Cpu *cpu) {
  uint8_t result = *cpu->dest8 + 1;

  set_flag(cpu, FLAG_S, 0);
  set_flag(cpu, FLAG_Z, result == 0);
  set_flag(cpu, FLAG_H, (*cpu->dest8 & 0xF) == 0xF);

  *cpu->dest8 = result;
}

void reti(Cpu *cpu) {
  cpu->int_enabled = 1;
  ret(cpu);
}
void dec8(Cpu *cpu) {
  uint8_t result = *cpu->dest8 - 1;

  set_flag(cpu, FLAG_Z, result == 0);
  set_flag(cpu, FLAG_S, 1);
  set_flag(cpu, FLAG_H, ((*cpu->dest8 & 0xF) == 0));

  *cpu->dest8 = result;
}

void daa(Cpu *cpu) {
  (void)cpu;
  puts("sper ca nu ii folosita\n");
}

void scf(Cpu *cpu) {
  set_flag(cpu, FLAG_S, 0);
  set_flag(cpu, FLAG_H, 0);
  set_flag(cpu, FLAG_C, 1);
}

void cpl(Cpu *cpu) {
  set_flag(cpu, FLAG_S, 1);
  set_flag(cpu, FLAG_H, 1);

  cpu->registers.a ^= 0xFF;
}

void ccf(Cpu *cpu) {
  set_flag(cpu, FLAG_S, 0);
  set_flag(cpu, FLAG_H, 0);

  set_flag(cpu, FLAG_C, get_flag(cpu, FLAG_C) ^ 1);
}

// God forgive me for what I am about to do.
void init_instruction_lut(Cpu *cpu) {
  //  #define INSTR(opcode, mnemonic, size, cycles, fetch, exec)
  INSTR(0x00, "NOP", 1, 4, fetch_none, nop8);
  INSTR(0x10, "STOP", 2, 4, fetch_none, stop);
  INSTR(0x20, "JR NZ, i8", 2, 8, fetch_orange, jr);
  INSTR(0x30, "JR NC, i8", 2, 8, fetch_orange, jr);

  for (uint16_t opcode = 0x1; opcode <= 0x31; opcode += 0x10) {
    INSTR(opcode, "LD", 3, 12, fetch_dark_purple, ld16);
  }

  for (uint16_t opcode = 0x2; opcode <= 0x32; opcode += 0x10) {
    INSTR(opcode, "LD", 1, 8, purple_fetch, ld8);
  }
  for (uint16_t opcode = 0x3; opcode <= 0x33; opcode += 0x10) {
    INSTR(opcode, "INC", 1, 8, fetch_dark_green, inc16);
  }
  for (uint16_t opcode = 0x4; opcode <= 0x34; opcode += 0x10) {
    INSTR(opcode, "INC", 1, 4, fetch_green, inc8);
  }
  for (uint16_t opcode = 0x5; opcode <= 0x35; opcode += 0x10) {
    INSTR(opcode, "DEC", 1, 4, fetch_green, dec8);
  }
  for (uint16_t opcode = 0x6; opcode <= 0x36; opcode += 0x10) {
    INSTR(opcode, "LD", 2, 8, purple_fetch, ld8);
  }
  INSTR(0x07, "RLCA", 1, 4, fetch_none, rlca);
  INSTR(0x17, "RLCA", 1, 4, fetch_none, rla);
  INSTR(0x27, "DAA", 1, 4, fetch_none, daa);
  INSTR(0x37, "SCF", 1, 4, fetch_none, scf);

  INSTR(0x08, "LD", 3, 20, fetch_dark_purple, ld16);
  INSTR(0x18, "JR", 2, 12, fetch_orange, jr);
  INSTR(0x28, "JR Z", 2, 8, fetch_orange, jr);
  INSTR(0x38, "JR C", 2, 8, fetch_orange, jr);

  for (uint16_t opcode = 0x9; opcode <= 0x39; opcode += 0x10) {
    INSTR(opcode, "ADD", 1, 8, fetch_dark_green, add16);
  }
  for (uint16_t opcode = 0xA; opcode <= 0x3A; opcode += 0x10) {
    INSTR(opcode, "LD", 1, 8, purple_fetch, ld8);
  }
  for (uint16_t opcode = 0xB; opcode <= 0x3B; opcode += 0x10) {
    INSTR(opcode, "DEC", 1, 8, fetch_dark_green, dec16);
  }
  for (uint16_t opcode = 0xC; opcode <= 0x3C; opcode += 0x10) {
    INSTR(opcode, "INC", 1, 4, fetch_green, inc8);
  }
  for (uint16_t opcode = 0xD; opcode <= 0x3D; opcode += 0x10) {
    INSTR(opcode, "DEC", 1, 4, fetch_green, dec8);
  }
  for (uint16_t opcode = 0xE; opcode <= 0x3E; opcode += 0x10) {
    INSTR(opcode, "LD", 2, 8, purple_fetch, ld8);
  }

  INSTR(0x0F, "RRCA", 1, 4, fetch_none, rrca);
  INSTR(0x1F, "RRA", 1, 4, fetch_none, rra);
  INSTR(0x2F, "CPL", 1, 4, fetch_none, cpl);
  INSTR(0x3F, "CCF", 1, 4, fetch_none, ccf);

  for (uint16_t opcode = 0x40; opcode <= 0x7F; opcode += 0x1) {
    INSTR(opcode, "LD", 1, 4, purple_fetch, ld8);
  }
  // set halt
  INSTR(0x76, "HALT", 1, 4, fetch_none, hlt);

  for (uint16_t opcode = 0x80; opcode <= 0x87; opcode += 0x1) {
    INSTR(opcode, "ADD", 1, 4, fetch_green, add8);
  }
  for (uint16_t opcode = 0x88; opcode <= 0x8F; opcode += 0x1) {
    INSTR(opcode, "ADC", 1, 4, fetch_green, adc8);
  }
  for (uint16_t opcode = 0x90; opcode <= 0x97; opcode += 0x1) {
    INSTR(opcode, "SUB", 1, 4, fetch_green, sub8);
  }
  for (uint16_t opcode = 0x98; opcode <= 0x9F; opcode += 0x1) {
    INSTR(opcode, "SBC", 1, 4, fetch_green, sbc8);
  }
  for (uint16_t opcode = 0xA0; opcode <= 0xA7; opcode += 0x1) {
    INSTR(opcode, "AND", 1, 4, fetch_green, and8);
  }
  for (uint16_t opcode = 0xA8; opcode <= 0xAF; opcode += 0x1) {
    INSTR(opcode, "XOR", 1, 4, fetch_green, xor8);
  }
  for (uint16_t opcode = 0xB0; opcode <= 0xB7; opcode += 0x1) {
    INSTR(opcode, "OR", 1, 4, fetch_green, or8);
  }
  for (uint16_t opcode = 0xB8; opcode <= 0xBF; opcode += 0x1) {
    INSTR(opcode, "CP", 1, 4, fetch_green, cp8);
  }

  INSTR(0xC0, "RET NZ", 1, 8, fetch_orange, ret);
  INSTR(0xD0, "RET NC", 1, 8, fetch_orange, ret);
  INSTR(0xE0, "LD", 2, 12, purple_fetch, ld8);
  INSTR(0xF0, "LD", 2, 12, purple_fetch, ld8);

  for (uint16_t opcode = 0xC1; opcode <= 0xF1; opcode += 0x10) {
    INSTR(opcode, "POP", 1, 12, fetch_dark_purple, pop16);
  }

  INSTR(0xC2, "JP NZ", 3, 12, fetch_orange, jp);
  INSTR(0xD2, "JP NC", 3, 12, fetch_orange, jp);
  INSTR(0xE2, "LD", 2, 8, purple_fetch, ld8);
  INSTR(0xF2, "LD", 2, 8, purple_fetch, ld8);

  INSTR(0xC3, "JP", 3, 16, fetch_orange, jp);
  INSTR(0xD3, "???", 0, 0, fetch_none, NULL);
  INSTR(0xE3, "???", 0, 0, fetch_none, NULL);
  INSTR(0xF3, "DI", 1, 4, fetch_none, di);

  INSTR(0xC4, "CALL", 3, 12, fetch_orange, call);
  INSTR(0xD4, "CALL", 3, 12, fetch_orange, call);
  INSTR(0xE4, "???", 0, 0, fetch_none, NULL);
  INSTR(0xF4, "???", 0, 0, fetch_none, NULL);

  for (uint16_t opcode = 0xC5; opcode <= 0xF5; opcode += 0x10) {
    INSTR(opcode, "PUSH", 1, 16, fetch_dark_purple, push16);
  }

  INSTR(0xC6, "ADD", 2, 8, fetch_green, add8);
  INSTR(0xD6, "SUB", 2, 8, fetch_green, sub8);
  INSTR(0xE6, "AND", 2, 8, fetch_green, and8);
  INSTR(0xF6, "OR", 2, 8, fetch_green, or8);

  for (uint16_t opcode = 0xC7; opcode <= 0xF7; opcode += 0x10) {
    INSTR(opcode, "RST", 1, 16, fetch_orange, rst);
  }

  INSTR(0xC8, "RET Z", 1, 8, fetch_orange, ret);
  INSTR(0xD8, "RET C", 1, 8, fetch_orange, ret);
  INSTR(0xE8, "ADD", 2, 16, fetch_dark_green, add16);
  INSTR(0xF8, "LD", 2, 16, fetch_dark_green, ld16);

  INSTR(0xC9, "RET", 1, 16, fetch_orange, ret);
  INSTR(0xD9, "RETI", 1, 16, fetch_orange, reti);
  INSTR(0xE9, "JP", 1, 4, fetch_orange, jp);
  INSTR(0xF9, "LD", 2, 16, fetch_dark_purple, ld16);

  INSTR(0xC9, "RET", 1, 16, fetch_orange, ret);
  INSTR(0xD9, "RETI", 1, 16, fetch_orange, reti);
  INSTR(0xE9, "JP", 1, 4, fetch_orange, jp);
  INSTR(0xF9, "LD", 2, 16, fetch_dark_purple, ld16);

  INSTR(0xCA, "JP Z", 3, 12, fetch_orange, jp);
  INSTR(0xDA, "JP C", 3, 12, fetch_orange, jp);
  INSTR(0xEA, "LD", 3, 16, purple_fetch, ld16);
  INSTR(0xFA, "LD", 3, 16, purple_fetch, ld16);

  INSTR(0xCB, "CB PREFIX", 1, 4, fetch_none, cb);
  INSTR(0xDB, "???", 0, 0, NULL, NULL);
  INSTR(0xEB, "???", 0, 0, NULL, NULL);
  INSTR(0xFB, "EI", 1, 4, fetch_none, ei);

  INSTR(0xCC, "CALL", 3, 12, fetch_orange, call);
  INSTR(0xDC, "CALL", 3, 12, fetch_orange, call);
  INSTR(0xEC, "???", 0, 0, NULL, NULL);
  INSTR(0xFC, "???", 0, 0, NULL, NULL);

  INSTR(0xCD, "CALL", 3, 24, fetch_orange, call);
  INSTR(0xDD, "???", 0, 0, NULL, NULL);
  INSTR(0xED, "???", 0, 0, NULL, NULL);
  INSTR(0xFD, "???", 0, 0, NULL, NULL);

  INSTR(0xCE, "ADC", 2, 8, fetch_green, adc8);
  INSTR(0xDE, "SBC", 2, 8, fetch_green, sbc8);
  INSTR(0xEE, "XOR", 2, 8, fetch_green, xor8);
  INSTR(0xFE, "CP", 2, 8, fetch_green, cp8);

  for (uint16_t opcode = 0xCF; opcode <= 0xFF; opcode += 0x10) {
    INSTR(opcode, "RST", 1, 16, fetch_orange, rst);
  }

  // CB PREFIX
  CBINSTR(0x00, "RLC", 2, 8, fetch_blue, rlc);
  CBINSTR(0x01, "RRC", 2, 8, fetch_blue, rrc);
  CBINSTR(0x02, "RL", 2, 8, fetch_blue, rl);
  CBINSTR(0x03, "RR", 2, 8, fetch_blue, rr);
  CBINSTR(0x04, "SLA", 2, 8, fetch_blue, sla);
  CBINSTR(0x05, "SRA", 2, 8, fetch_blue, sra);
  CBINSTR(0x06, "SWAP", 2, 8, fetch_blue, swap);
  CBINSTR(0x07, "SRL", 2, 8, fetch_blue, srl);
  CBINSTR(0x08, "BIT", 2, 8, fetch_blue, bit);
  CBINSTR(0x09, "RES", 2, 8, fetch_blue, res);
  CBINSTR(0x0A, "SET", 2, 8, fetch_blue, set);
}

char *get_dreg_name(uint8_t offset) {
  switch (offset) {
    case 0: {
      return "BC";
    } break;
    case 1: {
      return "DE";
    } break;
    case 2: {
      return "HL";
    } break;
    case 3: {
      return "AF";
    } break;
    default:
      return "ALLOOOOO";
  }
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

char *get_register_name(uint8_t index) {
  switch (index) {
    case 0: {
      return "B";
    } break;
    case 1: {
      return "C";
    } break;
    case 2: {
      return "D";
    } break;
    case 3: {
      return "E";
    } break;
    case 4: {
      return "H";
    } break;
    case 5: {
      return "L";
    } break;
    case 6: {
      return "(HL)";
    } break;
    case 7: {
      return "A";
    } break;
    default:
      return "REEEE";
  }
}

void cpu_test() {
  Cpu *cpu = (Cpu *)calloc(1, sizeof(Cpu));

  init_cpu(cpu);
  init_instruction_lut(cpu);

  // Test inc8
  // Z0H-
  uint16_t counter = 0x100;

  cpu->memory[0x100] = 0x3C;

  for (uint8_t val = 0; val < 0xFF; val++) {
    cpu->registers.a = val;
    cycle(cpu);
    if (cpu->registers.a != ((val + 1) & 0xFF)) {
      printf("nu");
      exit(0);
    }
    if (cpu->registers.zero != (val == 0xFF)) {
      printf("nuu");
      exit(0);
    }
    if (cpu->registers.half_carry != ((val & 0x7) == 0x7)) {
      printf("REEE");
      exit(0);
    }
    cpu->registers.pc = 0x100;
  }

  init_cpu(cpu);

  // Test add
  cpu->memory[0x100] = 0x80;
  for (int a = 0; a <= 0xFF; a++) {
    for (int b = 0; b <= 0xFF; b++) {
      cpu->registers.a = a;
      cpu->registers.b = b;
      int sum = (a + b) & 0xFF;

      cycle(cpu);
      if (cpu->registers.a != sum) {
        printf("Aduna prost");
        exit(0);
      }

      if (cpu->registers.zero != (sum == 0)) {
        printf("zero flag prost");
        exit(0);
      }
      if (cpu->registers.half_carry != (((a & 0xF) + (b & 0xF)) > 0xF)) {
        printf("half flag prost");
        exit(0);
      }

      cpu->registers.pc = 0x100;
    }
  }
  init_cpu(cpu);

  // Test sub
  cpu->memory[0x100] = 0x90;
  for (int a = 0; a <= 0xFF; a++) {
    for (int b = 0; b <= 0xFF; b++) {
      cpu->registers.a = a;
      cpu->registers.b = b;
      int sum = (a - b) & 0xFF;

      cycle(cpu);
      if (cpu->registers.a != sum) {
        printf("Scade prost");
        exit(0);
      }

      if (cpu->registers.zero != (sum == 0)) {
        printf("zero flag prost");
        exit(0);
      }
      if (cpu->registers.half_carry != ((a & 0xF) < (b & 0xF))) {
        printf("half flag prost");
        exit(0);
      }

      cpu->registers.pc = 0x100;
    }
  }
}
