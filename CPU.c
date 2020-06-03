#include "CPU.h"
#include "Instructions.h"
#include <string.h>
#include <assert.h>
#include <stdio.h>
#define DEBUG_SBC 0
#define DEBUG_SP 0

void start_cpu(CPU *cpu) {
	cpu->pc = 0x0100;
	cpu->is_halted = 0;
	cpu->is_stopped = 0;
	cpu->interrupts_enabled = 0;

	// Simulate bios
	cpu->registers.d_registers.af = 0x11B0;
	cpu->registers.d_registers.bc = 0x0013;
	cpu->registers.d_registers.de = 0x00D8;
	cpu->registers.d_registers.hl = 0x014D;

	/*
	cpu->registers.d_registers.af = 0x1180;
	cpu->registers.d_registers.de = 0xFF56;
	cpu->registers.d_registers.hl = 0x000D;
	cpu->registers.d_registers.af = 0x0000;
	cpu->registers.d_registers.bc = 0x0000;
	cpu->registers.d_registers.de = 0x0000;
	cpu->registers.d_registers.hl = 0x0000;
	*/

	cpu->sp = 0xFFFE;

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

   	cpu->broke = 0;
}

void step(CPU *cpu) {
	// Size of the instruction
	uint16_t size = 1;
	// Whether we jumped or not
	int has_jumped = 0;

	uint8_t opcode = cpu->memory[cpu->pc];

	// print_registers(&cpu->registers);
	// printf("SP = %.4hx\n", cpu->sp);
	// printf("PC = %.4hx\n", cpu->pc);
	printf("Executing %.4hx at PC = %.4hx\n", opcode, cpu->pc);
	printf("Stack layout:\n");
	for (int i = 0; i < 2; i += 2) {
		printf("%.4hx ", get_16(&cpu->memory[cpu->sp + i]));
	}
	printf("Register A: %hhx\n", cpu->registers.registers.a);
	printf("Register B: %hhx\n", cpu->registers.registers.b);
	printf("SP: %hx\n", cpu->sp);
	printf("immediate: %hhx\n", cpu->memory[cpu->pc + 1]);
	printf("\n");
	fflush(stdout);
	/*
	uint16_t breakpoint = 0xc7fe;
	if (cpu->pc == breakpoint || cpu->broke) {
		cpu->broke = 1;
		printf("Executing %.4hx at PC = %.4hx\n", opcode, cpu->pc);
		printf("SP is at %.4hx and contains %.4hx\n", cpu->sp, get_16(&cpu->memory[cpu->sp]));
		print_registers(&cpu->registers);
		printf("[dfff] = %.4hx\n", get_16(&cpu->memory[0xdfff]));
		printf("[c000] = %.4hx\n", get_16(&cpu->memory[0xc000]));
		printf("\n");
		printf("press return to step");
		fgetc(stdin);
		fflush(stdout);
	}
	*/

	// if (1) {
	// 	cpu->broke = 1;
	// 	printf("Executing %.4hx at PC = %.4hx\n", opcode, cpu->pc);
	// 	printf("SP is at %.4hx and contains %.4hx\n", cpu->sp, get_16(&cpu->memory[cpu->sp]));
	// 	print_registers(&cpu->registers);
	// 	printf("[dfff] = %.4hx\n", get_16(&cpu->memory[0xdfff]));
	// 	printf("[c000] = %.4hx\n", get_16(&cpu->memory[0xc000]));
	// 	printf("\n");
	// 	printf("press return to step");
	// 	fgetc(stdin);
	// 	fflush(stdout);
	// }

	// printf("Executing %.4hx at PC = %.4hx\n", opcode, cpu->pc);

	switch (opcode) {
		case 0x00: {
			// NOP
			size = 1;
			break;
		}
		case 0x01: {
			// LD BC, u16
			uint16_t immediate = get_16(&cpu->memory[cpu->pc + 1]);
			cpu->registers.d_registers.bc = immediate;
			size = 3;
			break;
		}
		case 0x02: {
			// LD (BC), A
			uint16_t address = cpu->registers.d_registers.bc;
			cpu->memory[address] = cpu->registers.registers.a;
			size = 1;
			break;
		}
		case 0x03: {
			// INC BC
			cpu->registers.d_registers.bc++;
			size = 1;
			break;
		}
		case 0x04: {
			// INC B
			inc_8(cpu, &cpu->registers.registers.b);
			size = 1;
			break;
		}

		case 0x05: {
			// DEC B
			dec_8(cpu, &cpu->registers.registers.b);
			size = 1;
			break;
		}

		case 0x06: {
			// LD B, u8
			uint8_t immediate = cpu->memory[cpu->pc + 1];
			cpu->registers.registers.b = immediate;
			size = 2;
			break;
		}

		case 0x07: {
			// RLCA
			rlc_8(cpu, &cpu->registers.registers.a);
			set_zero_flag(&cpu->registers, 0);
			size = 1;
			break;
		}

		case 0x08: {
			// LD (u16), SP
			uint16_t address = get_16(&cpu->memory[cpu->pc]);
			cpu->memory[address] = cpu->sp;
			size = 3;
			break;
		}

		case 0x09: {
			// ADD HL, BC
			uint16_t bc_value = cpu->registers.d_registers.bc;
			add_16(cpu, &cpu->registers.d_registers.hl, bc_value);
			size = 1;
			break;
		}

		case 0x0A: {
			// LD A, (BC)
			uint16_t address = cpu->registers.d_registers.bc;
			cpu->registers.registers.a = cpu->memory[address];
			size = 1;
			break;
		}

		case 0x0B: {
			// DEC BC
			cpu->registers.d_registers.bc--;
			size = 1;
			break;
		}

		case 0x0C: {
			// INC C
			inc_8(cpu, &cpu->registers.registers.c);
			size = 1;
			break;
		}

		case 0x0D: {
			// DEC C
			dec_8(cpu, &cpu->registers.registers.c);
			size = 1;
			break;
		}

		case 0x0E: {
			// LD C, u8
			uint8_t immediate = cpu->memory[cpu->pc + 1];
			cpu->registers.registers.c = immediate;
			size = 2;
			break;
		}

		case 0x0F: {
			// RRCA
			rrc_8(cpu, &cpu->registers.registers.a);
			set_zero_flag(&cpu->registers, 0);
			size = 1;
			break;
		}

		case 0x10: {
			// STOP
			cpu->is_stopped = 1;
			size = 2;
			break;
		}

		case 0x11: {
			// LD DE, u16
			uint16_t immediate = get_16(&cpu->memory[cpu->pc + 1]);
			cpu->registers.d_registers.de = immediate;
			size = 3;
			break;
		}

		case 0x12: {
			// LD (DE), A
			uint16_t address = cpu->registers.d_registers.de;
			cpu->memory[address] = cpu->registers.registers.a;
			size = 1;
			break;
		}

		case 0x13: {
			// INC DE
			cpu->registers.d_registers.de++;
			size = 1;
			break;
		}

		case 0x14: {
			// INC D
			inc_8(cpu, &cpu->registers.registers.d);
			size = 1;
			break;
		}

		case 0x15: {
			// DEC D
			dec_8(cpu, &cpu->registers.registers.d);
			size = 1;
			break;
		}

		case 0x16: {
			// LD D, u8
			uint8_t immediate = cpu->memory[cpu->pc + 1];
			cpu->registers.registers.d = immediate;
			size = 2;
			break;
		}

		case 0x17: {
			// RLA
			rl_8(cpu, &cpu->registers.registers.a);
			size = 1;
			break;
		}

		case 0x18: {
			// JR i8
			has_jumped = 1;
			int8_t immediate = (int8_t) cpu->memory[cpu->pc + 1];
			cpu->pc = (int16_t) cpu->pc + immediate + 2;
			size = 2;
			break;
		}

		case 0x19: {
			// ADD HL, DE
			uint16_t de_value = cpu->registers.d_registers.de;
			add_16(cpu, &cpu->registers.d_registers.hl, de_value);
			size = 1;
			break;
		}

		case 0x1A: {
			// LD A, (DE)
			uint16_t address = cpu->registers.d_registers.de;
			cpu->registers.registers.a = cpu->memory[address];
			size = 1;
			break;
		}

		case 0x1B: {
			// DEC DE
			cpu->registers.d_registers.de--;
			size = 1;
			break;
		}

		case 0x1C: {
			// INC E
			inc_8(cpu, &cpu->registers.registers.e);
			size = 1;
			break;
		}

		case 0x1D: {
			// DEC E
			dec_8(cpu, &cpu->registers.registers.e);
			size = 1;
			break;
		}

		case 0x1E: {
			// LD E, u8
			uint8_t immediate = cpu->memory[cpu->pc + 1];
			cpu->registers.registers.e = immediate;
			size = 2;
			break;
		}

		case 0x1F: {
			// RRA
			rr_8(cpu, &cpu->registers.registers.a);
			size = 1;
			break;
		}

		case 0x20: {
			// JR NZ, i8
			if (!get_zero_flag(&cpu->registers)) {
				has_jumped = 1;
				int8_t immediate = (int8_t) cpu->memory[cpu->pc + 1];
				cpu->pc = (int16_t) cpu->pc + immediate + 2;
			}
			size = 2;
			break;
		}

		case 0x21: {
			// LD HL, u16
			uint16_t immediate = get_16(&cpu->memory[cpu->pc + 1]);
			cpu->registers.d_registers.hl = immediate;
			size = 3;
			break;
		}

		case 0x22: {
			// LD (HL+), A
			uint16_t address = cpu->registers.d_registers.hl++;
			cpu->memory[address] = cpu->registers.registers.a;
			size = 1;
			break;
		}

		case 0x23: {
			// INC HL
			cpu->registers.d_registers.hl++;
			size = 1;
			break;
		}

		case 0x24: {
			// INC H
			inc_8(cpu, &cpu->registers.registers.h);
			size = 1;
			break;
		}

		case 0x25: {
			// DEC H
			dec_8(cpu, &cpu->registers.registers.h);
			size = 1;
			break;
		}

		case 0x26: {
			// LD H, u8
			uint8_t immediate = cpu->memory[cpu->pc + 1];
			cpu->registers.registers.h = immediate;
			size = 2;
			break;
		}

		case 0x27: {
			// DAA
			daa_8(cpu, &cpu->registers.registers.a);
			size = 1;
			break;
		}

		case 0x28: {
			// JR Z, i8
			if (get_zero_flag(&cpu->registers)) {
				has_jumped = 1;
				int8_t immediate = (int8_t) cpu->memory[cpu->pc + 1];
				cpu->pc = (int16_t) cpu->pc + immediate + 2;
			}
			size = 2;
			break;
		}

		case 0x29: {
			// ADD HL, HL
			uint16_t hl_value = cpu->registers.d_registers.hl;
			add_16(cpu, &cpu->registers.d_registers.hl, hl_value);
			size = 1;
			break;
		}

		case 0x2A: {
			// LD A, (HL+)
			uint16_t address = cpu->registers.d_registers.hl++;
			cpu->registers.registers.a = cpu->memory[address];
			size = 1;
			break;
		}

		case 0x2B: {
			// DEC HL
			cpu->registers.d_registers.hl--;
			size = 1;
			break;
		}

		case 0x2C: {
			// INC L
			inc_8(cpu, &cpu->registers.registers.l);
			size = 1;
			break;
		}

		case 0x2D: {
			// DEC L
			dec_8(cpu, &cpu->registers.registers.l);
			size = 1;
			break;
		}

		case 0x2E: {
			// LD L, u8 
			uint8_t immediate = cpu->memory[cpu->pc + 1];
			cpu->registers.registers.l = immediate;
			size = 2; 
			break;
		}

		case 0x2F: {
			// CPL
			set_substraction_flag(&cpu->registers, 1);
			set_half_carry_flag(&cpu->registers, 1);
			cpu->registers.registers.a ^= 0xFF;
			size = 1;
			break;
		}

		case 0x30: {
			// JR NC, i8
			if (!get_carry_flag(&cpu->registers)) {
				has_jumped = 1;
				int8_t immediate = (int8_t) cpu->memory[cpu->pc + 1];
				cpu->pc = (int16_t) cpu->pc + immediate + 2;
			}
			size = 2;
			break;
		}

		case 0x31: {
			// LD SP, u16
			uint16_t immediate = get_16(&cpu->memory[cpu->pc + 1]);
			cpu->sp = immediate;
			size = 3;
			break;
		}

		case 0x32: {
			// LD (HL-), A
			uint16_t address = cpu->registers.d_registers.hl--;
			cpu->memory[address] = cpu->registers.registers.a;
			size = 1;
			break;
		}

		case 0x33: {
			// INC SP
			#if DEBUG_SP
			printf("SP is at %.4hx and contains %.4hx\n", cpu->sp, get_16(&cpu->memory[cpu->sp]));
			print_registers(&cpu->registers);
			#endif
			cpu->sp++;
			#if DEBUG_SP
			printf("SP is at %.4hx and contains %.4hx\n", cpu->sp, get_16(&cpu->memory[cpu->sp]));
			print_registers(&cpu->registers);
			printf("\n\n");
			fgetc(stdin);
			#endif
			size = 1;
			break;
		}

		case 0x34: {
			// INC (HL)
			uint16_t address = cpu->registers.d_registers.hl;
			inc_8(cpu, &cpu->memory[address]);
			size = 1;
			break;
		}

		case 0x35: {
			// DEC (HL)
			uint16_t address = cpu->registers.d_registers.hl;
			dec_8(cpu, &cpu->memory[address]);
			size = 1;
			break;
		}

		case 0x36: {
			// LD (HL), u8
			uint16_t address = cpu->registers.d_registers.hl;
			uint8_t immediate = cpu->memory[cpu->pc + 1];
			cpu->memory[address] = immediate;
			size = 2;
			break;
		}

		case 0x37: {
			// SCF
			set_substraction_flag(&cpu->registers, 0);
			set_half_carry_flag(&cpu->registers, 0);
			set_carry_flag(&cpu->registers, 1);
			size = 1;
			break;
		}

		case 0x38: {
			// JR C, i8
			if (get_carry_flag(&cpu->registers)) {
				has_jumped = 1;
				int8_t immediate = cpu->memory[cpu->pc + 1];
				cpu->pc = (int16_t) cpu->pc + immediate + 2;
			}
			size = 2;
			break;
		}

		case 0x39: {
			// ADD HL, SP
			add_16(cpu, &cpu->registers.d_registers.hl, cpu->sp);
			size = 1;
			break;
		}

		case 0x3A: {
			// LD A, (HL-)
			uint16_t address = cpu->registers.d_registers.hl--;
			cpu->registers.registers.a = cpu->memory[address];
			size = 1;
			break;
		}

		case 0x3B: {
			// DEC SP
			cpu->sp--;
			size = 1;
			break;
		}

		case 0x3C: {
			// INC A
			inc_8(cpu, &cpu->registers.registers.a);
			size = 1;
			break;
		}

		case 0x3D: {
			// DEC A
			dec_8(cpu, &cpu->registers.registers.a);
			size = 1;
			break;
		}

		case 0x3E: {
			// LD A, u8
			uint8_t immediate = cpu->memory[cpu->pc + 1];
			cpu->registers.registers.a = immediate;
			size = 2;
			break;
		}

		case 0x3F: {
			// CCF
			uint8_t carry_flag = get_carry_flag(&cpu->registers);
			set_substraction_flag(&cpu->registers, 0);
			set_half_carry_flag(&cpu->registers, 0);
			set_carry_flag(&cpu->registers, carry_flag ^ 1);
			size = 1;
			break;
		}

		case 0x40: {
			// LD B, B == NOP
			size = 1;
			break;
		}

		case 0x41: {
			// LD B, C
			cpu->registers.registers.b = cpu->registers.registers.c;
			size = 1;
			break;
		}

		case 0x42: {
			// LD B, D
			cpu->registers.registers.b = cpu->registers.registers.d;
			size = 1;
			break;
		}

		case 0x43: {
			// LD B, E
			cpu->registers.registers.b = cpu->registers.registers.e;
			size = 1;
			break;
		}

		case 0x44: {
			// LD B, H
			cpu->registers.registers.b = cpu->registers.registers.h;
			size = 1;
			break;
		}

		case 0x45: {
			// LD B, L
			cpu->registers.registers.b = cpu->registers.registers.l;
			size = 1;
			break;
		}

		case 0x46: {
			// LD B, (HL)
			uint16_t address = cpu->registers.d_registers.hl;
			cpu->registers.registers.b = cpu->memory[address];
			size = 1;
			break;
		}

		case 0x47: {
			// LD B, A
			cpu->registers.registers.b = cpu->registers.registers.a;
			size = 1;
			break;
		}

		case 0x48: {
			// LD C, B
			cpu->registers.registers.c = cpu->registers.registers.b;
			size = 1;
			break;
		}

		case 0x49: {
			// LD C, C == NOP
			size = 1;
			break;
		}

		case 0x4A: {
			// LD C, D
			cpu->registers.registers.c = cpu->registers.registers.d;
			size = 1;
			break;
		}

		case 0x4B: {
			// LD C, E
			cpu->registers.registers.c = cpu->registers.registers.e;
			size = 1;
			break;
		}

		case 0x4C: {
			// LD C, H
			cpu->registers.registers.c = cpu->registers.registers.h;
			size = 1;
			break;
		}

		case 0x4D: {
			// LD C, L
			cpu->registers.registers.c = cpu->registers.registers.l;
			size = 1;
			break;
		}

		case 0x4E: {
			// LD C, (HL)
			uint16_t address = cpu->registers.d_registers.hl;
			cpu->registers.registers.c = cpu->memory[address];
			size = 1;
			break;
		}

		case 0x4F: {
			// LD C, A
			cpu->registers.registers.c = cpu->registers.registers.a;
			size = 1;
			break;
		}

		case 0x50: {
			// LD D, B
			cpu->registers.registers.d = cpu->registers.registers.b;
			size = 1;
			break;
		}

		case 0x51: {
			// LD D, C
			cpu->registers.registers.d = cpu->registers.registers.c;
			size = 1;
			break;
		}

		case 0x52: {
			// LD D, D == NOP
			size = 1;
			break;
		}

		case 0x53: {
			// LD D, E
			cpu->registers.registers.d = cpu->registers.registers.e;
			size = 1;
			break;
		}

		case 0x54: {
			// LD D, H
			cpu->registers.registers.d = cpu->registers.registers.h;
			size = 1;
			break;
		}

		case 0x55: {
			// LD D, L
			cpu->registers.registers.d = cpu->registers.registers.l;
			size = 1;
			break;
		}

		case 0x56: {
			// LD D, (HL)
			uint16_t address = cpu->registers.d_registers.hl;
			cpu->registers.registers.d = cpu->memory[address];
			size = 1;
			break;
		}

		case 0x57: {
			// LD D, A
			cpu->registers.registers.d = cpu->registers.registers.a;
			size = 1;
			break;
		}

		case 0x58: {
			// LD E, B
			cpu->registers.registers.e = cpu->registers.registers.b;
			size = 1;
			break;
		}

		case 0x59: {
			// LD E, C
			cpu->registers.registers.e = cpu->registers.registers.c;
			size = 1;
			break;
		}

		case 0x5A: {
			// LD E, D
			cpu->registers.registers.e = cpu->registers.registers.d;
			size = 1;
			break;
		}

		case 0x5B: {
			// LD E, E == NOP
			size = 1;
			break;
		}

		case 0x5C: {
			// LD E, H
			cpu->registers.registers.e = cpu->registers.registers.h;
			size = 1;
			break;
		}

		case 0x5D: {
			// LD E, L
			cpu->registers.registers.e = cpu->registers.registers.l;
			size = 1;
			break;
		}

		case 0x5E: {
			// LD E, (HL)
			uint16_t address = cpu->registers.d_registers.hl;
			cpu->registers.registers.e = cpu->memory[address];
			size = 1;
			break;
		}

		case 0x5F: {
			// LD E, A
			cpu->registers.registers.e = cpu->registers.registers.a;
			size = 1;
			break;
		}

		case 0x60: {
			// LD H, B
			cpu->registers.registers.h = cpu->registers.registers.b;
			size = 1;
			break;
		}

		case 0x61: {
			// LD H, C
			cpu->registers.registers.h = cpu->registers.registers.c;
			size = 1;
			break;
		}

		case 0x62: {
			// LD H, D
			cpu->registers.registers.h = cpu->registers.registers.d;
			size = 1;
			break;
		}

		case 0x63: {
			// LD H, E
			cpu->registers.registers.h = cpu->registers.registers.e;
			size = 1;
			break;
		}

		case 0x64: {
			// LD H, H == NOP
			size = 1;
			break;
		}

		case 0x65: {
			// LD H, L
			cpu->registers.registers.h = cpu->registers.registers.l;
			size = 1;
			break;
		}

		case 0x66: {
			// LD H, (HL)
			uint16_t address = cpu->registers.d_registers.hl;
			cpu->registers.registers.h = cpu->memory[address];
			size = 1;
			break;
		}

		case 0x67: {
			// LD H, A
			cpu->registers.registers.h = cpu->registers.registers.a;
			size = 1;
			break;
		}

		case 0x68: {
			// LD L, B
			cpu->registers.registers.l = cpu->registers.registers.b;
			size = 1;
			break;
		}

		case 0x69: {
			// LD L, C
			cpu->registers.registers.l = cpu->registers.registers.c;
			size = 1;
			break;
		}

		case 0x6A: {
			// LD L, D
			cpu->registers.registers.l = cpu->registers.registers.d;
			size = 1;
			break; 
		}

		case 0x6B: {
			// LD L, E == NOP
			cpu->registers.registers.l = cpu->registers.registers.e;
			size = 1;
			break;
		}

		case 0x6C: {
			// LD L, H
			cpu->registers.registers.l = cpu->registers.registers.h;
			size = 1;
			break;
		}

		case 0x6D: {
			// LD L, L == NOP
			size = 1;
			break;
		}

		case 0x6E: {
			// LD L, (HL)
			uint16_t address = cpu->registers.d_registers.hl;
			cpu->registers.registers.l = cpu->memory[address];
			size = 1;
			break;
		}

		case 0x6F: {
			// LD L, A
			cpu->registers.registers.l = cpu->registers.registers.a;
			size = 1;
			break;
		}

		case 0x70: {
			// LD (HL), B
			uint16_t address = cpu->registers.d_registers.hl;
			cpu->memory[address] = cpu->registers.registers.b;
			size = 1;
			break;
		}

		case 0x71: {
			// LD (HL), C
			uint16_t address = cpu->registers.d_registers.hl;
			cpu->memory[address] = cpu->registers.registers.c;
			size = 1;
			break;
		}

		case 0x72: {
			// LD (HL), D
			uint16_t address = cpu->registers.d_registers.hl;
			cpu->memory[address] = cpu->registers.registers.d;
			size = 1;
			break;
		}

		case 0x73: {
			// LD (HL), E
			uint16_t address = cpu->registers.d_registers.hl;
			cpu->memory[address] = cpu->registers.registers.e;
			size = 1;
			break;
		}

		case 0x74: {
			// LD (HL), H
			uint16_t address = cpu->registers.d_registers.hl;
			cpu->memory[address] = cpu->registers.registers.h;
			size = 1;
			break;
		}

		case 0x75: {
			// LD (HL), L
			uint16_t address = cpu->registers.d_registers.hl;
			cpu->memory[address] = cpu->registers.registers.l;
			size = 1;
			break;
		}

		case 0x76: {
			// HALT
			cpu->is_halted = 1;
			size = 1;
			break;
		}

		case 0x77: {
			// LD (HL), A
			uint16_t address = cpu->registers.d_registers.hl;
			cpu->memory[address] = cpu->registers.registers.a;
			size = 1;
			break;
		}

		case 0x78: {
			// LD A, B
			cpu->registers.registers.a = cpu->registers.registers.b;
			size = 1;
			break;
		}

		case 0x79: {
			// LD A, C
			cpu->registers.registers.a = cpu->registers.registers.c;
			size = 1;
			break;
		}

		case 0x7A: {
			// LD A, D
			cpu->registers.registers.a = cpu->registers.registers.d;
			size = 1;
			break;
		}

		case 0x7B: {
			// LD A, E
			cpu->registers.registers.a = cpu->registers.registers.e;
			size = 1;
			break;
		}

		case 0x7C: {
			// LD A, H
			cpu->registers.registers.a = cpu->registers.registers.h;
			size = 1;
			break;
		}

		case 0x7D: {
			// LD A, L
			cpu->registers.registers.a = cpu->registers.registers.l;
			size = 1;
			break;
		}

		case 0x7E: {
			// LD A, (HL)
			uint16_t address = cpu->registers.d_registers.hl;
			cpu->registers.registers.a = cpu->memory[address];
			size = 1;
			break;
		}

		case 0x7F: {
			// LD A, A == NOP
			size = 1;
			break;
		}

		case 0x80: {
			// ADD A, B
			add_8(cpu, &cpu->registers.registers.a, cpu->registers.registers.b);
			size = 1;
			break;
		}

		case 0x81: {
			// ADD A, C
			add_8(cpu, &cpu->registers.registers.a, cpu->registers.registers.c);
			size = 1;
			break;
		}

		case 0x82: {
			// ADD A, D
			add_8(cpu, &cpu->registers.registers.a, cpu->registers.registers.b);
			size = 1;
			break;
		}

		case 0x83: {
			// ADD A, E
			add_8(cpu, &cpu->registers.registers.a, cpu->registers.registers.e);
			size = 1;
			break;
		}

		case 0x84: {
			// ADD A, H
			add_8(cpu, &cpu->registers.registers.a, cpu->registers.registers.h);
			size = 1;
			break;
		}

		case 0x85: {
			// ADD A, L
			add_8(cpu, &cpu->registers.registers.a, cpu->registers.registers.l);
			size = 1;
			break;
		}

		case 0x86: {
			// ADD A, (HL)
			uint16_t address = cpu->registers.d_registers.hl;
			add_8(cpu, &cpu->registers.registers.a, cpu->memory[address]);
			size = 1;
			break;
		}

		case 0x87: {
			// ADD A, A
			add_8(cpu, &cpu->registers.registers.a, cpu->registers.registers.a);
			size = 1;
			break;
		}

		case 0x88: {
			// ADC A, B
			adc_8(cpu, &cpu->registers.registers.a, cpu->registers.registers.b);
			size = 1;
			break;
		}

		case 0x89: {
			// ADC A, C
			adc_8(cpu, &cpu->registers.registers.a, cpu->registers.registers.c);
			size = 1;
			break;
		}

		case 0x8A: {
			// ADC A, D
			adc_8(cpu, &cpu->registers.registers.a, cpu->registers.registers.d);
			size = 1;
			break;
		}

		case 0x8B: {
			// ADC A, E
			adc_8(cpu, &cpu->registers.registers.a, cpu->registers.registers.e);
			size = 1;
			break;
		}

		case 0x8C: {
			// ADC A, H
			adc_8(cpu, &cpu->registers.registers.a, cpu->registers.registers.h);
			size = 1;
			break;
		}

		case 0x8D: {
			// ADC A, L
			adc_8(cpu, &cpu->registers.registers.a, cpu->registers.registers.l);
			size = 1;
			break;
		}

		case 0x8E: {
			// ADC A, (HL)
			uint16_t address = cpu->registers.d_registers.hl;
			adc_8(cpu, &cpu->registers.registers.a, cpu->memory[address]);
			size = 1;
			break;
		}

		case 0x8F: {
			// ADC A, A			
			adc_8(cpu, &cpu->registers.registers.a, cpu->registers.registers.a);
			size = 1;
			break;
		}

		case 0x90: {
			// SUB A, B
			sub_8(cpu, &cpu->registers.registers.a, cpu->registers.registers.b);
			size = 1;
			break;
		}

		case 0x91: {
			// SUB A, C
			sub_8(cpu, &cpu->registers.registers.a, cpu->registers.registers.c);
			size = 1;
			break;
		}

		case 0x92: {
			// SUB A, D
			sub_8(cpu, &cpu->registers.registers.a, cpu->registers.registers.d);
			size = 1;
			break;
		}

		case 0x93: {
			// SUB A, E
			sub_8(cpu, &cpu->registers.registers.a, cpu->registers.registers.e);
			size = 1;
			break;
		}

		case 0x94: {
			// SUB A, H
			sub_8(cpu, &cpu->registers.registers.a, cpu->registers.registers.e);
			size = 1;
			break;
		}

		case 0x95: {
			// SUB A, L
			sub_8(cpu, &cpu->registers.registers.a, cpu->registers.registers.l);
			size = 1;
			break;
		}

		case 0x96: {
			// SUB A, (HL)
			uint16_t address = cpu->registers.d_registers.hl;
			sub_8(cpu, &cpu->registers.registers.a, cpu->memory[address]);
			size = 1;
			break;
		}

		case 0x97: {
			// SUB A, A
			sub_8(cpu, &cpu->registers.registers.a, cpu->registers.registers.a);
			size = 1;
			break;
		}

		case 0x98: {
			// SBC A, B
			sbc_8(cpu, &cpu->registers.registers.a, cpu->registers.registers.b);
			size = 1;
			break;
		}

		case 0x99: {
			// SBC A, C
			sbc_8(cpu, &cpu->registers.registers.a, cpu->registers.registers.c);
			size = 1;
			break;
		}

		case 0x9A: {
			// SBC A, D
			sbc_8(cpu, &cpu->registers.registers.a, cpu->registers.registers.d);
			size = 1;
			break;
		}

		case 0x9B: {
			// SBC A, E
			sbc_8(cpu, &cpu->registers.registers.a, cpu->registers.registers.e);
			size = 1;
			break;
		}

		case 0x9C: {
			// SBC A, H
			sbc_8(cpu, &cpu->registers.registers.a, cpu->registers.registers.h);
			size = 1;
			break;
		}

		case 0x9D: {
			// SBC A, L
			sbc_8(cpu, &cpu->registers.registers.a, cpu->registers.registers.l);
			size = 1;
			break;
		}

		case 0x9E: {
			// SBC A, (HL)
			uint16_t address = cpu->registers.d_registers.hl;
			sbc_8(cpu, &cpu->registers.registers.a, cpu->memory[address]);
			size = 1;
			break;
		}

		case 0x9F: {
			// SBC A, A
			sbc_8(cpu, &cpu->registers.registers.a, cpu->registers.registers.a);
			size = 1;
			break;
		}

		case 0xA0: {
			// AND A, B
			and_8(cpu, &cpu->registers.registers.a, cpu->registers.registers.b);
			size = 1;
			break;
		}

		case 0xA1: {
			// AND A, C
			and_8(cpu, &cpu->registers.registers.a, cpu->registers.registers.c);
			size = 1;
			break;
		}

		case 0xA2: {
			// AND A, D
			and_8(cpu, &cpu->registers.registers.a, cpu->registers.registers.d);
			size = 1;
			break;
		}

		case 0xA3: {
			// AND A, E
			and_8(cpu, &cpu->registers.registers.a, cpu->registers.registers.e);
			size = 1;
			break;
		}

		case 0xA4: {
			// AND A, H
			and_8(cpu, &cpu->registers.registers.a, cpu->registers.registers.h);
			size = 1;
			break;
		}

		case 0xA5: {
			// AND A, L
			and_8(cpu, &cpu->registers.registers.a, cpu->registers.registers.l);
			size = 1;
			break;
		}

		case 0xA6: {
			// AND A, (HL)
			uint16_t address = cpu->registers.d_registers.hl;
			and_8(cpu, &cpu->registers.registers.a, cpu->memory[address]);
			size = 1;
			break;
		}

		case 0xA7: {
			// AND A, A
			and_8(cpu, &cpu->registers.registers.a, cpu->registers.registers.a);
			size = 1;
			break;
		}

		case 0xA8: {
			// XOR A, B
			xor_8(cpu, &cpu->registers.registers.a, cpu->registers.registers.b);
			size = 1;
			break;
		}

		case 0xA9: {
			// XOR A, C
			xor_8(cpu, &cpu->registers.registers.a, cpu->registers.registers.c);
			size = 1;
			break;
		}

		case 0xAA: {
			// XOR A, D
			xor_8(cpu, &cpu->registers.registers.a, cpu->registers.registers.d);
			size = 1;
			break;
		}

		case 0xAB: {
			// XOR A, E
			xor_8(cpu, &cpu->registers.registers.a, cpu->registers.registers.e);
			size = 1;
			break;
		}
		
		case 0xAC: {
			// XOR A, H
			xor_8(cpu, &cpu->registers.registers.a, cpu->registers.registers.h);
			size = 1;
			break;
		}

		case 0xAD: {
			// XOR A, L
			xor_8(cpu, &cpu->registers.registers.a, cpu->registers.registers.l);
			size = 1;
			break;
		}

		case 0xAE: {
			// XOR A, (HL)
			uint16_t address = cpu->registers.d_registers.hl;
			xor_8(cpu, &cpu->registers.registers.a, cpu->memory[address]);
			size = 1;
			break;
		}

		case 0xAF: {
			// XOR A, A
			xor_8(cpu, &cpu->registers.registers.a, cpu->registers.registers.a);
			size = 1;
			break;
		}

		case 0xB0: {
			// OR A, B
			or_8(cpu, &cpu->registers.registers.a, cpu->registers.registers.b);
			size = 1;
			break;
		}

		case 0xB1: {
			// OR A, C
			or_8(cpu, &cpu->registers.registers.a, cpu->registers.registers.c);
			size = 1;
			break;
		}

		case 0xB2: {
			// OR A, D
			or_8(cpu, &cpu->registers.registers.a, cpu->registers.registers.d);
			size = 1;
			break;
		}

		case 0xB3: {
			// OR A, E
			or_8(cpu, &cpu->registers.registers.a, cpu->registers.registers.e);
			size = 1;
			break;
		}

		case 0xB4: {
			// OR A, H
			or_8(cpu, &cpu->registers.registers.a, cpu->registers.registers.h);
			size = 1;
			break;
		}

		case 0xB5: {
			// OR A, L
			or_8(cpu, &cpu->registers.registers.a, cpu->registers.registers.l);
			size = 1;
			break;
		}

		case 0xB6: {
			// OR A, (HL)
			uint16_t address = cpu->registers.d_registers.hl;
			or_8(cpu, &cpu->registers.registers.a, cpu->memory[address]);
			size = 1;
			break;
		}

		case 0xB7: {
			// OR A, A
			or_8(cpu, &cpu->registers.registers.a, cpu->registers.registers.a);
			size = 1;
			break;
		}

		case 0xB8: {
			// CP A, B
			cp_8(cpu, &cpu->registers.registers.a, cpu->registers.registers.b);
			size = 1;
			break;
		}

		case 0xB9: {
			// CP A, C
			cp_8(cpu, &cpu->registers.registers.a, cpu->registers.registers.c);
			size = 1;
			break;
		}

		case 0xBA: {
			// CP A, D
			cp_8(cpu, &cpu->registers.registers.a, cpu->registers.registers.d);
			size = 1;
			break;
		}

		case 0xBB: {
			// CP A, E
			cp_8(cpu, &cpu->registers.registers.a, cpu->registers.registers.e);
			size = 1;
			break;
		}

		case 0xBC: {
			// CP A, H
			cp_8(cpu, &cpu->registers.registers.a, cpu->registers.registers.h);
			size = 1;
			break;
		}

		case 0xBD: {
			// CP A, L
			cp_8(cpu, &cpu->registers.registers.a, cpu->registers.registers.l);
			size = 1;
			break;
		}

		case 0xBE: {
			// CP A, (HL)
			uint16_t address = cpu->registers.d_registers.hl;
			cp_8(cpu, &cpu->registers.registers.a, cpu->memory[address]);
			size = 1;
			break;
		}

		case 0xBF: {
			// CP A, A
			cp_8(cpu, &cpu->registers.registers.a, cpu->registers.registers.a);
			size = 1;
			break;
		}

		case 0xC0: {
			// RET NZ
			if (!get_zero_flag(&cpu->registers)) {
				has_jumped = 1;
				pop_16(cpu, &cpu->pc);
			}
			size = 1;
			break;
		}

		case 0xC1: {
			// POP BC
			pop_16(cpu, &cpu->registers.d_registers.bc);
			size = 1;
			break;
		}

		case 0xC2: {
			// JP NZ, u16
			if (!get_zero_flag(&cpu->registers)) {
				has_jumped = 1;
				uint16_t immediate = get_16(&cpu->memory[cpu->pc + 1]);
				cpu->pc = immediate;
			}
			size = 3;
			break;
		}

		case 0xC3: {
			// JP u16
			has_jumped = 1;
			uint16_t immediate = get_16(&cpu->memory[cpu->pc + 1]);
			cpu->pc = immediate;
			size = 3;
			break;
		}

		case 0xC4: {
			// CALL NZ, u16
			if (!get_zero_flag(&cpu->registers)) {
				has_jumped = 1;
				uint16_t immediate = get_16(&cpu->memory[cpu->pc + 1]);
				push_16(cpu, cpu->pc + 3);
				cpu->pc = immediate;
			}
			size = 3;
			break;
		}

		case 0xC5: {
			// PUSH BC
			push_16(cpu, cpu->registers.d_registers.bc);
			size = 1;
			break;
		}

		case 0xC6: {
			// ADD A, u8
			uint8_t immediate = cpu->memory[cpu->pc + 1];
			add_8(cpu, &cpu->registers.registers.a, immediate);
			size = 2;
			break;
		}

		case 0xC7: {
			// RST 00h
			has_jumped = 1;
			rst(cpu, 0x00);
			size = 1;
			break;
		}

		case 0xC8: {
			// RET Z
			if (get_zero_flag(&cpu->registers)) {
				has_jumped = 1;
				pop_16(cpu, &cpu->pc);
			}
			size = 1;
			break;
		}


		case 0xC9: {
			// RET
			has_jumped = 1;
			pop_16(cpu, &cpu->pc);
			size = 1;
			break;
		}

		case 0xCA: {
			// JP Z, u16
			if (get_zero_flag(&cpu->registers)) {
				has_jumped = 1;
				uint16_t immediate = get_16(&cpu->memory[cpu->pc + 1]);
				cpu->pc = immediate;
			}
			size = 3;
			break;
		}

		case 0xCB: {
			// PREFIX CB
			size = 2;
			uint8_t immediate = cpu->memory[cpu->pc + 1];
			// fprintf(stderr, "We're in no man's land at opcode %x %x\n", opcode, immediate);
			// fflush(stderr);
			switch (immediate) {
				case 0x00: {
					rlc_8(cpu, &cpu->registers.registers.b);
					break;
				}
				case 0x01: {
					rlc_8(cpu, &cpu->registers.registers.c);
					break;
				}

				case 0x02: {
					rlc_8(cpu, &cpu->registers.registers.d);
					break;
				}

				case 0x03: {
					rlc_8(cpu, &cpu->registers.registers.e);
					break;
				}

				case 0x04: {
					rlc_8(cpu, &cpu->registers.registers.h);
					break;
				}

				case 0x05: {
					rlc_8(cpu, &cpu->registers.registers.l);
					break;
				}


				case 0x06: {
					uint16_t address = cpu->registers.d_registers.hl;
					rlc_8(cpu, &cpu->memory[address]);
					break;
				}

				case 0x07: {
					rlc_8(cpu, &cpu->registers.registers.a);
					break;
				}

				case 0x08: {
					rrc_8(cpu, &cpu->registers.registers.b);
					break;
				}

				case 0x09: {
					rrc_8(cpu, &cpu->registers.registers.c);
					break;
				}

				case 0x0A: {
					rrc_8(cpu, &cpu->registers.registers.d);
					break;
				}

				case 0x0B: {
					rrc_8(cpu, &cpu->registers.registers.e);
					break;
				}

				case 0x0C: {
					rrc_8(cpu, &cpu->registers.registers.h);
					break;
				}

				case 0x0D: {
					rrc_8(cpu, &cpu->registers.registers.l);
					break;
				}

				case 0x0E: {
					uint16_t address = cpu->registers.d_registers.hl;
					rrc_8(cpu, &cpu->memory[address]);
					break;
				}

				case 0x0F: {
					rrc_8(cpu, &cpu->registers.registers.a);
					break;
				}

				case 0x10: {
					rl_8(cpu, &cpu->registers.registers.b);
					break;
				}

				case 0x11: {
					rl_8(cpu, &cpu->registers.registers.c);
					break;
				}

				case 0x12: {
					rl_8(cpu, &cpu->registers.registers.d);
					break;
				}

				case 0x13: {
					rl_8(cpu, &cpu->registers.registers.e);
					break;
				}

				case 0x14: {
					rl_8(cpu, &cpu->registers.registers.h);
					break;
				}

				case 0x15: {
					rl_8(cpu, &cpu->registers.registers.l);
					break;
				}

				case 0x16: {
					uint16_t address = cpu->registers.d_registers.hl;
					rl_8(cpu, &cpu->memory[address]);
					break;
				}

				case 0x17: {
					rl_8(cpu, &cpu->registers.registers.a);
					break;
				}

				case 0x18: {
					rr_8(cpu, &cpu->registers.registers.b);
					break;
				}

				case 0x19: {
					rr_8(cpu, &cpu->registers.registers.c);
					break;
				}

				case 0x1A: {
					rr_8(cpu, &cpu->registers.registers.d);
					break;
				}

				case 0x1B: {
					rr_8(cpu, &cpu->registers.registers.e);
					break;
				}

				case 0x1C: {
					rr_8(cpu, &cpu->registers.registers.h);
					break;
				}

				case 0x1D: {
					rr_8(cpu, &cpu->registers.registers.l);
					break;
				}

				case 0x1E: {
					uint16_t address = cpu->registers.d_registers.hl;
					rr_8(cpu, &cpu->memory[address]);
					break;
				}

				case 0x1F: {
					rr_8(cpu, &cpu->registers.registers.a);
					break;
				}

				case 0x20: {
					sla_8(cpu, &cpu->registers.registers.b);
					break;
				}


				case 0x21: {
					sla_8(cpu, &cpu->registers.registers.c);
					break;
				}


				case 0x22: {
					sla_8(cpu, &cpu->registers.registers.d);
					break;
				}


				case 0x23: {
					sla_8(cpu, &cpu->registers.registers.e);
					break;
				}


				case 0x24: {
					sla_8(cpu, &cpu->registers.registers.h);
					break;
				}


				case 0x25: {
					sla_8(cpu, &cpu->registers.registers.l);
					break;
				}


				case 0x26: {
					uint16_t address = cpu->registers.d_registers.hl;
					sla_8(cpu, &cpu->memory[address]);
					break;
				}


				case 0x27: {
					sla_8(cpu, &cpu->registers.registers.a);
					break;
				}


				case 0x28: {
					sra_8(cpu, &cpu->registers.registers.b);
					break;
				}


				case 0x29: {
					sra_8(cpu, &cpu->registers.registers.c);
					break;
				}


				case 0x2A: {
					sra_8(cpu, &cpu->registers.registers.d);
					break;
				}


				case 0x2B: {
					sra_8(cpu, &cpu->registers.registers.e);
					break;
				}


				case 0x2C: {
					sra_8(cpu, &cpu->registers.registers.h);
					break;
				}


				case 0x2D: {
					sra_8(cpu, &cpu->registers.registers.l);
					break;
				}

				case 0x2E: {
					uint16_t address = cpu->registers.d_registers.hl;
					sra_8(cpu, &cpu->memory[address]);
					break;
				}


				case 0x2F: {
					sra_8(cpu, &cpu->registers.registers.a);
					break;
				}

				case 0x30: {
					swap_8(cpu, &cpu->registers.registers.b);
					break;
				}

				case 0x31: {
					swap_8(cpu, &cpu->registers.registers.c);
					break;
				}

				case 0x32: {
					swap_8(cpu, &cpu->registers.registers.d);
					break;
				}

				case 0x33: {
					swap_8(cpu, &cpu->registers.registers.e);
					break;
				}

				case 0x34: {
					swap_8(cpu, &cpu->registers.registers.h);
					break;
				}

				case 0x35: {
					swap_8(cpu, &cpu->registers.registers.l);
					break;
				}

				case 0x36: {
					uint16_t address = cpu->registers.d_registers.hl;
					swap_8(cpu, &cpu->memory[address]);
					break;
				}

				case 0x37: {
					swap_8(cpu, &cpu->registers.registers.a);
					break;
				}

				case 0x38: {
					srl_8(cpu, &cpu->registers.registers.b);
					break;
				}

				case 0x39: {
					srl_8(cpu, &cpu->registers.registers.c);
					break;
				}

				case 0x3A: {
					srl_8(cpu, &cpu->registers.registers.d);
					break;
				}

				case 0x3B: {
					srl_8(cpu, &cpu->registers.registers.e);
					break;
				}

				case 0x3C: {
					srl_8(cpu, &cpu->registers.registers.h);
					break;
				}

				case 0x3D: {
					srl_8(cpu, &cpu->registers.registers.l);
					break;
				}

				case 0x3E: {
					uint16_t address = cpu->registers.d_registers.hl;
					srl_8(cpu, &cpu->memory[address]);
					break;
				}

				case 0x3F: {
					srl_8(cpu, &cpu->registers.registers.a);
					break;
				}

				case 0x40: {
					bit_test(cpu, cpu->registers.registers.b, 0);
					break;
				}

				case 0x41: {
					bit_test(cpu, cpu->registers.registers.c, 0);
					break;
				}

				case 0x42: {
					bit_test(cpu, cpu->registers.registers.d, 0);
					break;
				}

				case 0x43: {
					bit_test(cpu, cpu->registers.registers.e, 0);
					break;
				}

				case 0x44: {
					bit_test(cpu, cpu->registers.registers.h, 0);
					break;
				}

				case 0x45: {
					bit_test(cpu, cpu->registers.registers.l, 0);
					break;
				}

				case 0x46: {
					uint16_t address = cpu->registers.d_registers.hl;
					bit_test(cpu, cpu->memory[address], 0);
					break;
				}

				case 0x47: {
					bit_test(cpu, cpu->registers.registers.a, 0);
					break;
				}

				case 0x48: {
					bit_test(cpu, cpu->registers.registers.b, 1);
					break;
				}

				case 0x49: {
					bit_test(cpu, cpu->registers.registers.c, 1);
					break;
				}

				case 0x4A: {
					bit_test(cpu, cpu->registers.registers.d, 1);
					break;
				}

				case 0x4B: {
					bit_test(cpu, cpu->registers.registers.e, 1);
					break;
				}

				case 0x4C: {
					bit_test(cpu, cpu->registers.registers.h, 1);
					break;
				}

				case 0x4D: {
					bit_test(cpu, cpu->registers.registers.l, 1);
					break;
				}

				case 0x4E: {
					uint16_t address = cpu->registers.d_registers.hl;
					bit_test(cpu, cpu->memory[address], 1);
					break;
				}

				case 0x4F: {
					bit_test(cpu, cpu->registers.registers.a, 1);
					break;
				}

				case 0x50: {
					bit_test(cpu, cpu->registers.registers.b, 2);
					break;
				}

				case 0x51: {
					bit_test(cpu, cpu->registers.registers.c, 2);
					break;
				}

				case 0x52: {
					bit_test(cpu, cpu->registers.registers.d, 2);
					break;
				}

				case 0x53: {
					bit_test(cpu, cpu->registers.registers.e, 2);
					break;
				}

				case 0x54: {
					bit_test(cpu, cpu->registers.registers.h, 2);
					break;
				}

				case 0x55: {
					bit_test(cpu, cpu->registers.registers.l, 2);
					break;
				}

				case 0x56: {
					uint16_t address = cpu->registers.d_registers.hl;
					bit_test(cpu, cpu->memory[address], 2);
					break;
				}

				case 0x57: {
					bit_test(cpu, cpu->registers.registers.a, 2);
					break;
				}

				case 0x58: {
					bit_test(cpu, cpu->registers.registers.b, 3);
					break;
				}

				case 0x59: {
					bit_test(cpu, cpu->registers.registers.c, 3);
					break;
				}

				case 0x5A: {
					bit_test(cpu, cpu->registers.registers.d, 3);
					break;
				}


				case 0x5B: {
					bit_test(cpu, cpu->registers.registers.e, 3);
					break;
				}


				case 0x5C: {
					bit_test(cpu, cpu->registers.registers.h, 3);
					break;
				}

				case 0x5D: {
					bit_test(cpu, cpu->registers.registers.l, 3);
					break;
				}

				case 0x5E: {
					uint16_t address = cpu->registers.d_registers.hl;
					bit_test(cpu, cpu->memory[address], 3);
					break;
				}

				case 0x5F: {
					bit_test(cpu, cpu->registers.registers.a, 3);
					break;
				}


				case 0x60: {
					bit_test(cpu, cpu->registers.registers.b, 4);
					break;
				}


				case 0x61: {
					bit_test(cpu, cpu->registers.registers.c, 4);
					break;
				}

				case 0x62: {
					bit_test(cpu, cpu->registers.registers.d, 4);
					break;
				}


				case 0x63: {
					bit_test(cpu, cpu->registers.registers.e, 4);
					break;
				}


				case 0x64: {
					bit_test(cpu, cpu->registers.registers.h, 4);
					break;
				}

				case 0x65: {
					bit_test(cpu, cpu->registers.registers.l, 4);
					break;
				}

				case 0x66: {
					uint16_t address = cpu->registers.d_registers.hl;
					bit_test(cpu, cpu->memory[address], 4);
					break;
				}

				case 0x67: {
					bit_test(cpu, cpu->registers.registers.a, 4);
					break;
				}

				case 0x68: {
					bit_test(cpu, cpu->registers.registers.b, 5);
					break;
				}

				case 0x69: {
					bit_test(cpu, cpu->registers.registers.c, 5);
					break;
				}

				case 0x6A: {
					bit_test(cpu, cpu->registers.registers.d, 5);
					break;
				}

				case 0x6B: {
					bit_test(cpu, cpu->registers.registers.e, 5);
					break;
				}

				case 0x6C: {
					bit_test(cpu, cpu->registers.registers.h, 5);
					break;
				}

				case 0x6D: {
					bit_test(cpu, cpu->registers.registers.l, 5);
					break;
				}

				case 0x6E: {
					uint16_t address = cpu->registers.d_registers.hl;
					bit_test(cpu, cpu->memory[address], 5);
					break;
				}

				case 0x6F: {
					bit_test(cpu, cpu->registers.registers.a, 5);
					break;
				}

				case 0x70: {
					bit_test(cpu, cpu->registers.registers.b, 6);
					break;
				}

				case 0x71: {
					bit_test(cpu, cpu->registers.registers.c, 6);
					break;
				}

				case 0x72: {
					bit_test(cpu, cpu->registers.registers.d, 6);
					break;
				}

				case 0x73: {
					bit_test(cpu, cpu->registers.registers.e, 6);
					break;
				}

				case 0x74: {
					bit_test(cpu, cpu->registers.registers.h, 6);
					break;
				}

				case 0x75: {
					bit_test(cpu, cpu->registers.registers.l, 6);
					break;
				}

				case 0x76: {
					uint16_t address = cpu->registers.d_registers.hl;
					bit_test(cpu, cpu->memory[address], 6);
					break;
				}

				case 0x77: {
					bit_test(cpu, cpu->registers.registers.a, 6);
					break;
				}

				case 0x78: {
					bit_test(cpu, cpu->registers.registers.b, 7);
					break;
				}

				case 0x79: {
					bit_test(cpu, cpu->registers.registers.c, 7);
					break;
				}

				case 0x7A: {
					bit_test(cpu, cpu->registers.registers.d, 7);
					break;
				}

				case 0x7B: {
					bit_test(cpu, cpu->registers.registers.e, 7);
					break;
				}

				case 0x7C: {
					bit_test(cpu, cpu->registers.registers.h, 7);
					break;
				}

				case 0x7D: {
					bit_test(cpu, cpu->registers.registers.l, 7);
					break;
				}

				case 0x7E: {
					uint16_t address = cpu->registers.d_registers.hl;
					bit_test(cpu, cpu->memory[address], 7);
					break;
				}

				case 0x7F: {
					bit_test(cpu, cpu->registers.registers.a, 7);
					break;
				}

				case 0x80: {
					res_8(cpu, &cpu->registers.registers.b, 0);
					break;
				}

				case 0x81: {
					res_8(cpu, &cpu->registers.registers.c, 0);
					break;
				}

				case 0x82: {
					res_8(cpu, &cpu->registers.registers.d, 0);
					break;
				}

				case 0x83: {
					res_8(cpu, &cpu->registers.registers.e, 0);
					break;
				}

				case 0x84: {
					res_8(cpu, &cpu->registers.registers.h, 0);
					break;
				}

				case 0x85: {
					res_8(cpu, &cpu->registers.registers.l, 0);
					break;
				}

				case 0x86: {
					uint16_t address = cpu->registers.d_registers.hl;
					res_8(cpu, &cpu->memory[address], 0);
					break;
				}

				case 0x87: {
					res_8(cpu, &cpu->registers.registers.a, 0);
					break;
				}

				case 0x88: {
					res_8(cpu, &cpu->registers.registers.b, 1);
					break;
				}

				case 0x89: {
					res_8(cpu, &cpu->registers.registers.c, 1);
					break;
				}

				case 0x8A: {
					res_8(cpu, &cpu->registers.registers.d, 1);
					break;
				}

				case 0x8B: {
					res_8(cpu, &cpu->registers.registers.e, 1);
					break;
				}

				case 0x8C: {
					res_8(cpu, &cpu->registers.registers.h, 1);
					break;
				}

				case 0x8D: {
					res_8(cpu, &cpu->registers.registers.l, 1);
					break;
				}

				case 0x8E: {
					uint16_t address = cpu->registers.d_registers.hl;
					res_8(cpu, &cpu->memory[address], 1);
					break;
				}

				case 0x8F: {
					res_8(cpu, &cpu->registers.registers.a, 1);
					break;
				}

				case 0x90: {
					res_8(cpu, &cpu->registers.registers.b, 2);
					break;
				}

				case 0x91: {
					res_8(cpu, &cpu->registers.registers.c, 2);
					break;
				}

				case 0x92: {
					res_8(cpu, &cpu->registers.registers.d, 2);
					break;
				}

				case 0x93: {
					res_8(cpu, &cpu->registers.registers.e, 2);
					break;
				}

				case 0x94: {
					res_8(cpu, &cpu->registers.registers.h, 2);
					break;
				}

				case 0x95: {
					res_8(cpu, &cpu->registers.registers.l, 2);
					break;
				}

				case 0x96: {
					uint16_t address = cpu->registers.d_registers.hl;
					res_8(cpu, &cpu->memory[address], 2);
					break;
				}

				case 0x97: {
					res_8(cpu, &cpu->registers.registers.a, 2);
					break;
				}

				case 0x98: {
					res_8(cpu, &cpu->registers.registers.b, 3);
					break;
				}

				case 0x99: {
					res_8(cpu, &cpu->registers.registers.c, 3);
					break;
				}

				case 0x9A: {
					res_8(cpu, &cpu->registers.registers.d, 3);
					break;
				}

				case 0x9B: {
					res_8(cpu, &cpu->registers.registers.e, 3);
					break;
				}

				case 0x9C: {
					res_8(cpu, &cpu->registers.registers.h, 3);
					break;
				}

				case 0x9D: {
					res_8(cpu, &cpu->registers.registers.l, 3);
					break;
				}

				case 0x9E: {
					uint16_t address = cpu->registers.d_registers.hl;
					res_8(cpu, &cpu->memory[address], 3);
					break;
				}

				case 0x9F: {
					res_8(cpu, &cpu->registers.registers.a, 3);
					break;
				}

				case 0xA0: {
					res_8(cpu, &cpu->registers.registers.b, 4);
					break;
				}

				case 0xA1: {
					res_8(cpu, &cpu->registers.registers.c, 4);
					break;
				}

				case 0xA2: {
					res_8(cpu, &cpu->registers.registers.d, 4);
					break;
				}

				case 0xA3: {
					res_8(cpu, &cpu->registers.registers.e, 4);
					break;
				}

				case 0xA4: {
					res_8(cpu, &cpu->registers.registers.h, 4);
					break;
				}

				case 0xA5: {
					res_8(cpu, &cpu->registers.registers.l, 4);
					break;
				}

				case 0xA6: {
					uint16_t address = cpu->registers.d_registers.hl;
					res_8(cpu, &cpu->memory[address], 4);
					break;
				}

				case 0xA7: {
					res_8(cpu, &cpu->registers.registers.a, 4);
					break;
				}

				case 0xA8: {
					res_8(cpu, &cpu->registers.registers.b, 5);
					break;
				}

				case 0xA9: {
					res_8(cpu, &cpu->registers.registers.c, 5);
					break;
				}

				case 0xAA: {
					res_8(cpu, &cpu->registers.registers.d, 5);
					break;
				}

				case 0xAB: {
					res_8(cpu, &cpu->registers.registers.e, 5);
					break;
				}

				case 0xAC: {
					res_8(cpu, &cpu->registers.registers.h, 5);
					break;
				}

				case 0xAD: {
					res_8(cpu, &cpu->registers.registers.l, 5);
					break;
				}

				case 0xAE: {
					uint16_t address = cpu->registers.d_registers.hl;
					res_8(cpu, &cpu->memory[address], 5);
					break;
				}

				case 0xAF: {
					res_8(cpu, &cpu->registers.registers.a, 5);
					break;
				}

				case 0xB0: {
					res_8(cpu, &cpu->registers.registers.b, 6);
					break;
				}

				case 0xB1: {
					res_8(cpu, &cpu->registers.registers.c, 6);
					break;
				}

				case 0xB2: {
					res_8(cpu, &cpu->registers.registers.d, 6);
					break;
				}

				case 0xB3: {
					res_8(cpu, &cpu->registers.registers.e, 6);
					break;
				}

				case 0xB4: {
					res_8(cpu, &cpu->registers.registers.h, 6);
					break;
				}

				case 0xB5: {
					res_8(cpu, &cpu->registers.registers.l, 6);
					break;
				}

				case 0xB6: {
					uint16_t address = cpu->registers.d_registers.hl;
					res_8(cpu, &cpu->memory[address], 6);
					break;
				}

				case 0xB7: {
					res_8(cpu, &cpu->registers.registers.a, 6);
					break;
				}

				case 0xB8: {
					res_8(cpu, &cpu->registers.registers.b, 7);
					break;
				}

				case 0xB9: {
					res_8(cpu, &cpu->registers.registers.c, 7);
					break;
				}

				case 0xBA: {
					res_8(cpu, &cpu->registers.registers.d, 7);
					break;
				}

				case 0xBB: {
					res_8(cpu, &cpu->registers.registers.e, 7);
					break;
				}

				case 0xBC: {
					res_8(cpu, &cpu->registers.registers.h, 7);
					break;
				}

				case 0xBD: {
					res_8(cpu, &cpu->registers.registers.l, 7);
					break;
				}

				case 0xBE: {
					uint16_t address = cpu->registers.d_registers.hl;
					res_8(cpu, &cpu->memory[address], 7);
					break;
				}

				case 0xBF: {
					res_8(cpu, &cpu->registers.registers.a, 7);
					break;
				}

				case 0xC0: {
					set_8(cpu, &cpu->registers.registers.b, 0);
					break;
				}

				case 0xC1: {
					set_8(cpu, &cpu->registers.registers.c, 0);
					break;
				}

				case 0xC2: {
					set_8(cpu, &cpu->registers.registers.d, 0);
					break;
				}

				case 0xC3: {
					set_8(cpu, &cpu->registers.registers.e, 0);
					break;
				}

				case 0xC4: {
					set_8(cpu, &cpu->registers.registers.h, 0);
					break;
				}

				case 0xC5: {
					set_8(cpu, &cpu->registers.registers.l, 0);
					break;
				}

				case 0xC6: {
					uint16_t address = cpu->registers.d_registers.hl;
					set_8(cpu, &cpu->memory[address], 0);
					break;
				}

				case 0xC7: {
					set_8(cpu, &cpu->registers.registers.a, 0);
					break;
				}

				case 0xC8: {
					set_8(cpu, &cpu->registers.registers.b, 1);
					break;
				}

				case 0xC9: {
					set_8(cpu, &cpu->registers.registers.c, 1);
					break;
				}

				case 0xCA: {
					set_8(cpu, &cpu->registers.registers.d, 1);
					break;
				}

				case 0xCB: {
					set_8(cpu, &cpu->registers.registers.e, 1);
					break;
				}

				case 0xCC: {
					set_8(cpu, &cpu->registers.registers.h, 1);
					break;
				}

				case 0xCD: {
					set_8(cpu, &cpu->registers.registers.l, 1);
					break;
				}

				case 0xCE: {
					uint16_t address = cpu->registers.d_registers.hl;
					set_8(cpu, &cpu->memory[address], 1);
					break;
				}

				case 0xCF: {
					set_8(cpu, &cpu->registers.registers.a, 1);
					break;
				}

				case 0xD0: {
					set_8(cpu, &cpu->registers.registers.b, 2);
					break;
				}

				case 0xD1: {
					set_8(cpu, &cpu->registers.registers.c, 2);
					break;
				}

				case 0xD2: {
					set_8(cpu, &cpu->registers.registers.d, 2);
					break;
				}

				case 0xD3: {
					set_8(cpu, &cpu->registers.registers.e, 2);
					break;
				}

				case 0xD4: {
					set_8(cpu, &cpu->registers.registers.h, 2);
					break;
				}

				case 0xD5: {
					set_8(cpu, &cpu->registers.registers.l, 2);
					break;
				}

				case 0xD6: {
					uint16_t address = cpu->registers.d_registers.hl;
					set_8(cpu, &cpu->memory[address], 2);
					break;
				}

				case 0xD7: {
					set_8(cpu, &cpu->registers.registers.a, 2);
					break;
				}

				case 0xD8: {
					set_8(cpu, &cpu->registers.registers.b, 3);
					break;
				}

				case 0xD9: {
					set_8(cpu, &cpu->registers.registers.c, 3);
					break;
				}

				case 0xDA: {
					set_8(cpu, &cpu->registers.registers.d, 3);
					break;
				}

				case 0xDB: {
					set_8(cpu, &cpu->registers.registers.e, 3);
					break;
				}

				case 0xDC: {
					set_8(cpu, &cpu->registers.registers.h, 3);
					break;
				}

				case 0xDD: {
					set_8(cpu, &cpu->registers.registers.l, 3);
					break;
				}

				case 0xDE: {
					uint16_t address = cpu->registers.d_registers.hl;
					set_8(cpu, &cpu->memory[address], 3);
					break;
				}

				case 0xDF: {
					set_8(cpu, &cpu->registers.registers.a, 3);
					break;
				}

				case 0xE0: {
					set_8(cpu, &cpu->registers.registers.b, 4);
					break;
				}

				case 0xE1: {
					set_8(cpu, &cpu->registers.registers.c, 4);
					break;
				}

				case 0xE2: {
					set_8(cpu, &cpu->registers.registers.d, 4);
					break;
				}

				case 0xE3: {
					set_8(cpu, &cpu->registers.registers.e, 4);
					break;
				}

				case 0xE4: {
					set_8(cpu, &cpu->registers.registers.h, 4);
					break;
				}

				case 0xE5: {
					set_8(cpu, &cpu->registers.registers.l, 4);
					break;
				}

				case 0xE6: {
					uint16_t address = cpu->registers.d_registers.hl;
					set_8(cpu, &cpu->memory[address], 4);
					break;
				}

				case 0xE7: {
					set_8(cpu, &cpu->registers.registers.a, 4);
					break;
				}

				case 0xE8: {
					set_8(cpu, &cpu->registers.registers.b, 5);
					break;
				}

				case 0xE9: {
					set_8(cpu, &cpu->registers.registers.c, 5);
					break;
				}

				case 0xEA: {
					set_8(cpu, &cpu->registers.registers.d, 5);
					break;
				}

				case 0xEB: {
					set_8(cpu, &cpu->registers.registers.e, 5);
					break;
				}

				case 0xEC: {
					set_8(cpu, &cpu->registers.registers.h, 5);
					break;
				}

				case 0xED: {
					set_8(cpu, &cpu->registers.registers.l, 5);
					break;
				}

				case 0xEE: {
					uint16_t address = cpu->registers.d_registers.hl;
					set_8(cpu, &cpu->memory[address], 5);
					break;
				}

				case 0xEF: {
					set_8(cpu, &cpu->registers.registers.a, 5);
					break;
				}

				case 0xF0: {
					set_8(cpu, &cpu->registers.registers.b, 6);
					break;
				}

				case 0xF1: {
					set_8(cpu, &cpu->registers.registers.c, 6);
					break;
				}

				case 0xF2: {
					set_8(cpu, &cpu->registers.registers.d, 6);
					break;
				}

				case 0xF3: {
					set_8(cpu, &cpu->registers.registers.e, 6);
					break;
				}

				case 0xF4: {
					set_8(cpu, &cpu->registers.registers.h, 6);
					break;
				}

				case 0xF5: {
					set_8(cpu, &cpu->registers.registers.l, 6);
					break;
				}

				case 0xF6: {
					uint16_t address = cpu->registers.d_registers.hl;
					set_8(cpu, &cpu->memory[address], 6);
					break;
				}

				case 0xF7: {
					set_8(cpu, &cpu->registers.registers.a, 6);
					break;
				}

				case 0xF8: {
					set_8(cpu, &cpu->registers.registers.b, 7);
					break;
				}

				case 0xF9: {
					set_8(cpu, &cpu->registers.registers.c, 7);
					break;
				}

				case 0xFA: {
					set_8(cpu, &cpu->registers.registers.d, 7);
					break;
				}

				case 0xFB: {
					set_8(cpu, &cpu->registers.registers.e, 7);
					break;
				}

				case 0xFC: {
					set_8(cpu, &cpu->registers.registers.h, 7);
					break;
				}

				case 0xFD: {
					set_8(cpu, &cpu->registers.registers.l, 7);
					break;
				}

				case 0xFE: {
					uint16_t address = cpu->registers.d_registers.hl;
					set_8(cpu, &cpu->memory[address], 7);
					break;
				}

				case 0xFF: {
					set_8(cpu, &cpu->registers.registers.a, 7);
					break;
				}

			}
			break;
		}

		case 0xCC: {
			// CALL Z, u16
			if (get_zero_flag(&cpu->registers)) {
				has_jumped = 1;
				uint16_t immediate = get_16(&cpu->memory[cpu->pc + 1]);
				push_16(cpu, cpu->pc + 3);
				cpu->pc = immediate;
			}
			size = 3;
			break;
		}

		case 0xCD: {
			// CALL u16
			has_jumped = 1;
			uint16_t immediate = get_16(&cpu->memory[cpu->pc + 1]);
			push_16(cpu, cpu->pc + 3);
			cpu->pc = immediate;
			size = 3;
			break;
		}

		case 0xCE: {
			// ADC A, u8
			uint8_t immediate = cpu->memory[cpu->pc + 1];
			// printf("immediate: %hhx\n", immediate);
			adc_8(cpu, &cpu->registers.registers.a, immediate);
			// print_registers(&cpu->registers);
			// fgetc(stdin);
			size = 2;
			break;

		}

		case 0xCF: {
			// RST 0x08h
			has_jumped = 1;
			rst(cpu, 0x08);
			size = 1;
			break;
		}

		case 0xD0: {
			// RET NC
			if (!get_carry_flag(&cpu->registers)) {
				has_jumped = 1;
				pop_16(cpu, &cpu->pc);
			}
			size = 1;
			break;
		}

		case 0xD1: {
			// POP DE
			pop_16(cpu, &cpu->registers.d_registers.de);
			size = 1;
			break;
		}

		case 0xD2: {
			// JP NC, u16
			if (!get_carry_flag(&cpu->registers)) {
				has_jumped = 1;
				uint16_t immediate = get_16(&cpu->memory[cpu->pc + 1]);
				cpu->pc = immediate;
			}
			size = 3;
			break;
		}

		case 0xD4: {
			// CALL NC, u16
			if (!get_carry_flag(&cpu->registers)) {
				has_jumped = 1;
				uint16_t immediate = get_16(&cpu->memory[cpu->pc + 1]);
				push_16(cpu, cpu->pc + 3);
				cpu->pc = immediate;
			}
			size = 3;
			break;
		}

		case 0xD5: {
			// PUSH DE
			push_16(cpu, cpu->registers.d_registers.de);
			size = 1;
			break;
		}

		case 0xD6: {
			// SUB A, u8
			uint8_t immediate = cpu->memory[cpu->pc + 1];
			sub_8(cpu, &cpu->registers.registers.a, immediate);
			size = 2;
			break;
		}

		case 0xD7: {
			// RST 10h
			has_jumped = 1;
			rst(cpu, 0x10);
			size = 1;
			break;
		}

		case 0xD8: {
			// RET C
			if (get_carry_flag(&cpu->registers)) {
				has_jumped = 1;
				pop_16(cpu, &cpu->pc);
			}
			size = 1;
			break;
		}

		case 0xD9: {
			// RETI
			has_jumped = 1;
			pop_16(cpu, &cpu->pc);
			cpu->interrupts_enabled = 1;
			size = 1;
			break;
		}

		case 0xDA: {
			// JP C, u16
			if (get_carry_flag(&cpu->registers)) {
				has_jumped = 1;
				uint16_t immediate = get_16(&cpu->memory[cpu->pc + 1]);
				cpu->pc = immediate;
			}
			size = 3;
			break;
		}

		case 0xDC: {
			// CALL C, u16
			if (get_carry_flag(&cpu->registers)) {
				has_jumped = 1;
				uint16_t immediate = get_16(&cpu->memory[cpu->pc + 1]);
				push_16(cpu, cpu->pc + 3);
				cpu->pc = immediate;
			}
			size = 3;
			break;
		}

		case 0xDE: {
			// SBC A, u8
			uint8_t immediate = cpu->memory[cpu->pc + 1];
			#if DEBUG_SBC == 1
			printf("immediate: %hhx\n", immediate);
			print_registers(&cpu->registers);
			#endif
			sbc_8(cpu, &cpu->registers.registers.a, immediate);
			#if DEBUG_SBC == 1
			print_registers(&cpu->registers);
			fgetc(stdin);
			#endif
			size = 2;
			break;
		}

		case 0xDF: {
			// RST 18h
			has_jumped = 1;
			rst(cpu, 0x18);
			size = 1;
			break;
		}

		case 0xE0: {
			// LD (FF00 + u8), A
			uint16_t address = 0xFF00 + cpu->memory[cpu->pc + 1];
			cpu->memory[address] = cpu->registers.registers.a;
			size = 2;
			break;
		}

		case 0xE1: {
			// POP HL
			pop_16(cpu, &cpu->registers.d_registers.hl);
			size = 1;
			break;
		}

		case 0xE2: {
			// LD (FF00 + C), A
			uint16_t address = 0xFF00 + cpu->registers.registers.c;
			cpu->memory[address] = cpu->registers.registers.a;
			size = 1;
			break;
		}

		case 0xE5: {
			// PUSH HL
			push_16(cpu, cpu->registers.d_registers.hl);
			size = 1;
			break;
		}

		case 0xE6: {
			// AND A, u8
			uint8_t immediate = cpu->memory[cpu->pc + 1];
			and_8(cpu, &cpu->registers.registers.a, immediate);
			size = 2;
			break;
		}

		case 0xE7: {
			// RST 20h
			has_jumped = 1;
			rst(cpu, 0x20);
			size = 1;
			break;
		}

		case 0xE8: {
			// ADD SP, i8
			uint8_t immediate = cpu->memory[cpu->pc + 1];
			#if DEBUG_SP
			printf("immediate: %x\n", immediate);
			print_registers(&cpu->registers);
			#endif
			add_16(cpu, &cpu->sp, immediate);
			set_zero_flag(&cpu->registers, 0);
			#if DEBUG_SP
			print_registers(&cpu->registers);
			fgetc(stdin);
			#endif
			size = 2;
			break;
		}

		case 0xE9: {
			// JP HL
			has_jumped = 1;
			cpu->pc = cpu->registers.d_registers.hl;
			size = 1;
			break;
		}

		case 0xEA: {
			// LD (u16), A
			uint16_t address = get_16(&cpu->memory[cpu->pc + 1]);
			cpu->memory[address] = cpu->registers.registers.a;
			size = 3;
			break;
		}

		case 0xEE: {
			// XOR A, u8
			uint8_t immediate = cpu->memory[cpu->pc + 1];
			xor_8(cpu, &cpu->registers.registers.a, immediate);
			size = 2;
			break;
		}

		case 0xEF: {
			// RST 28h
			has_jumped = 1;
			rst(cpu, 0x28);
			size = 1;
			break;
		}

		case 0xF0: {
			// LD A, (FF00 + u8)
			uint16_t address = 0xFF00 + cpu->memory[cpu->pc + 1];
			cpu->registers.registers.a = cpu->memory[address];
			size = 2;
			break;
		}

		case 0xF1: {
			// POP AF
			pop_16(cpu, &cpu->registers.d_registers.af);
			size = 1;
			break;
		}

		case 0xF2: {
			// LD A, (FF00+C)
			uint8_t address = 0xFF00 + cpu->registers.registers.c;
			cpu->registers.registers.a = cpu->memory[address];
			size = 1;
			break;
		}

		case 0xF3: {
			// DI
			cpu->interrupts_enabled = 0;
			size = 1;
			break;
		}

		case 0xF5: {
			// PUSH AF
			push_16(cpu, cpu->registers.d_registers.af);
			size = 1;
			break;
		}

		case 0xF6: {
			// OR A, u8
			uint8_t immediate = cpu->memory[cpu->pc + 1];
			or_8(cpu, &cpu->registers.registers.a, immediate);
			size = 2;
			break;
		}

		case 0xF7: {
			// RST 30h
			has_jumped = 1;
			rst(cpu, 0x30);
			break;
		}

		case 0xF8: {
			// LD HL, SP + i8
			fprintf(stderr, "This instruction might be unstable %x\n", opcode);
			int8_t immediate = (int8_t) cpu->memory[cpu->pc];
			cpu->registers.d_registers.hl = (int16_t) cpu->sp + immediate;
			size = 2;
			break;
		}

		case 0xF9: {
			// LD SP, HL
			cpu->sp = cpu->registers.d_registers.hl;
			size = 1;
			break;
		}

		case 0xFA: {
			// LD A, (u16)
			uint16_t address = get_16(&cpu->memory[cpu->pc + 1]);
			cpu->registers.registers.a = cpu->memory[address];
			size = 3;
			break;
		}

		case 0xFB: {
			// EI
			cpu->interrupts_enabled = 1;
			size = 1;
			break;
		}

		case 0xFE: {
			uint8_t immediate = cpu->memory[cpu->pc + 1];
			cp_8(cpu, &cpu->registers.registers.a, immediate);
			size = 2;
			break;
		}

		case 0xFF: {
			// RST 38h
			has_jumped = 1;
			rst(cpu, 0x38);
			size = 1;
			break;
		}

		default: {
			fprintf(stderr, "Undiscovered instruction %x %x\n", opcode, cpu->pc);
			fflush(stderr);
			size = 1;
			break;
		}

	}

	if (!has_jumped) {
		cpu->pc += size;
	}
}

void add_8(CPU *cpu, uint8_t *dest, uint8_t src) {
	uint8_t result = *dest + src;
	set_substraction_flag(&cpu->registers, 0);
	if (result == 0x00) {
		set_zero_flag(&cpu->registers, 1);
	} else {
		set_zero_flag(&cpu->registers, 0);
	}

	if (((*dest) & 0xF) > (0xF - (src & 0xF))) {
		set_half_carry_flag(&cpu->registers, 1);
	} else {
		set_half_carry_flag(&cpu->registers, 0);
	}

	if ((*dest) > (0xFF - src)) {
		set_carry_flag(&cpu->registers, 1);
	} else {
		set_carry_flag(&cpu->registers, 0);
	}
	*dest = result;
}

void inc_8(CPU *cpu, uint8_t *dest) {
	uint8_t result = *dest + 1;
	set_substraction_flag(&cpu->registers, 0);
	if (result == 0x00) {
		set_zero_flag(&cpu->registers, 1);
	} else {
		set_zero_flag(&cpu->registers, 0);
	}

	if (((*dest) & 0xF) > (0xF - 1)) {
		set_half_carry_flag(&cpu->registers, 1);
	} else {
		set_half_carry_flag(&cpu->registers, 0);
	}
	*dest = result;
}

void sub_8(CPU *cpu, uint8_t *dest, uint8_t src) {
	uint8_t result = *dest - src;
	set_substraction_flag(&cpu->registers, 1);
	if (result == 0x00) {
		set_zero_flag(&cpu->registers, 1);
	} else {
		set_zero_flag(&cpu->registers, 0);
	}
	if (((*dest) & 0xF) < (src & 0xF)) {
		set_half_carry_flag(&cpu->registers, 1);
	} else {
		set_half_carry_flag(&cpu->registers, 0);
	}

	if ((*dest) < src) {
		set_carry_flag(&cpu->registers, 1);
	} else {
		set_carry_flag(&cpu->registers, 0);
	}

	*dest = result;
}


void dec_8(CPU *cpu, uint8_t *dest) {
	uint8_t result = *dest - 1;
	set_substraction_flag(&cpu->registers, 1);
	if (result == 0x00) {
		set_zero_flag(&cpu->registers, 1);
	} else {
		set_zero_flag(&cpu->registers, 0);
	}
	if (((*dest) & 0xF) < 1) {
		set_half_carry_flag(&cpu->registers, 1);
	} else {
		set_half_carry_flag(&cpu->registers, 0);
	}

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

	if (((*dest) & 0x7FF) > (0x7FF - (src & 0x7FF))) {
		set_half_carry_flag(&cpu->registers, 1);
	} else {
		set_half_carry_flag(&cpu->registers, 0);
	}

	if ((*dest) > (0xFFFF) - src) {
		set_carry_flag(&cpu->registers, 1);
	} else {
		set_carry_flag(&cpu->registers, 0);
	}
	*dest = result;
}

void rlc_8(CPU *cpu, uint8_t *dest) {
	// printf("rlc_8\n");
	uint8_t bit_7 = (*dest) >> 7;

	set_zero_flag(&cpu->registers, 0);
	set_substraction_flag(&cpu->registers, 0);
	set_half_carry_flag(&cpu->registers, 0);
	set_carry_flag(&cpu->registers, bit_7);

	(*dest) <<= 1;
	(*dest) |= bit_7;
}

void rrc_8(CPU *cpu, uint8_t *dest) {
	// printf("rrc_8\n");
	uint8_t bit_0 = (*dest) & 1;
	set_zero_flag(&cpu->registers, 0);
	set_substraction_flag(&cpu->registers, 0);
	set_half_carry_flag(&cpu->registers, 0);
	set_carry_flag(&cpu->registers, bit_0);

	(*dest) >>= 1;
	(*dest) |= bit_0 * (1u << 7);
}

void rl_8(CPU *cpu, uint8_t *dest) {
	// printf("rl_8\n");
	uint8_t bit_7 = (*dest) >> 7;

	set_zero_flag(&cpu->registers, 0);
	set_substraction_flag(&cpu->registers, 0);
	set_half_carry_flag(&cpu->registers, 0);

	(*dest) <<= 1;
	(*dest) |= get_carry_flag(&cpu->registers);
	set_carry_flag(&cpu->registers, bit_7);
}

void rr_8(CPU *cpu, uint8_t *dest) {
	// printf("rr_8\n");
	uint8_t bit_0 = (*dest) & 1;

	set_zero_flag(&cpu->registers, 0);
	set_substraction_flag(&cpu->registers, 0);
	set_half_carry_flag(&cpu->registers, 0);

	(*dest) >>= 1;
	(*dest) |= get_carry_flag(&cpu->registers) * (1 << 7);

	set_carry_flag(&cpu->registers, bit_0);

}

void and_8(CPU *cpu, uint8_t *dest, uint8_t src) {
	(*dest) &= src;

	if ((*dest) == 0x00) {
		set_zero_flag(&cpu->registers, 1);
	} else {
		set_zero_flag(&cpu->registers, 0);
	}

	set_substraction_flag(&cpu->registers, 0);
	set_half_carry_flag(&cpu->registers, 1);
	set_carry_flag(&cpu->registers, 0);
}

void or_8(CPU *cpu, uint8_t *dest, uint8_t src) {
	(*dest) |= src;

	if ((*dest) == 0x00) {
		set_zero_flag(&cpu->registers, 1);
	} else {
		set_zero_flag(&cpu->registers, 0);
	}
	set_substraction_flag(&cpu->registers, 0);
	set_half_carry_flag(&cpu->registers, 0);
	set_carry_flag(&cpu->registers, 0);
}

void xor_8(CPU *cpu, uint8_t *dest, uint8_t src) {
	(*dest) ^= src;

	if ((*dest) == 0x00) {
		set_zero_flag(&cpu->registers, 1);
	} else {
		set_zero_flag(&cpu->registers, 0);
	}
	set_substraction_flag(&cpu->registers, 0);
	set_half_carry_flag(&cpu->registers, 0);
	set_carry_flag(&cpu->registers, 0);
}

void cp_8(CPU *cpu, uint8_t *dest, uint8_t src) {
	uint8_t result = *dest - src;

	set_substraction_flag(&cpu->registers, 1);
	if (result == 0x00) {
		set_zero_flag(&cpu->registers, 1);
	} else {
		set_zero_flag(&cpu->registers, 0);
	}
	if (((*dest) & 0xF) < (src & 0xF)) {
		set_half_carry_flag(&cpu->registers, 1);
	} else {
		set_half_carry_flag(&cpu->registers, 0);
	}
	if ((*dest) < src) {
		set_carry_flag(&cpu->registers, 1);
	} else {
		set_carry_flag(&cpu->registers, 0);
	}
}

void pop_16(CPU *cpu, uint16_t *dest) {
	*dest = get_16(&cpu->memory[cpu->sp]);
	cpu->sp += 2;
}

void push_16(CPU *cpu, uint16_t dest) {
	cpu->sp -= 2;
	load_16(&cpu->memory[cpu->sp], dest); 
}

void rst(CPU *cpu, uint8_t dest) {
	push_16(cpu, cpu->pc + 1);
	cpu->pc = dest;
}

void bit_test(CPU *cpu, uint8_t dest, uint8_t bit) {
	if (dest & (1u << bit)) {
		set_zero_flag(&cpu->registers, 0);
	} else {
		set_zero_flag(&cpu->registers, 1);
	}
	set_half_carry_flag(&cpu->registers, 1);
	set_substraction_flag(&cpu->registers, 0);

}

void sla_8(CPU *cpu, uint8_t *dest) {
	uint8_t bit_7 = (*dest) >> 7;
	(*dest) <<= 1;
	(*dest) &= ~1;
	if ((*dest) == 0x00) {
		set_zero_flag(&cpu->registers, 1);
	} else {
		set_zero_flag(&cpu->registers, 0);
	}
	set_carry_flag(&cpu->registers, bit_7);
}

void sra_8(CPU *cpu, uint8_t *dest) {
	uint8_t bit_0 = (*dest) & 1;
	uint8_t bit_7 = (*dest) >> 7;
	(*dest) >>= 1;
	(*dest) |= (bit_7) * (1 << 7);
	if ((*dest) == 0x00) {
		set_zero_flag(&cpu->registers, 1);
	} else {
		set_zero_flag(&cpu->registers, 0);
	}
	set_carry_flag(&cpu->registers, bit_0);
}

void swap_8(CPU *cpu, uint8_t *dest) {
	uint8_t lower_nibble = (*dest) & 0xF;
	uint8_t upper_nibble = ((*dest) & (0xFF - 0xF)) >> 4;
	(*dest) = (lower_nibble << 4) | upper_nibble;
	if ((*dest) == 0x00) {
		set_zero_flag(&cpu->registers, 1);
	} else {
		set_zero_flag(&cpu->registers, 0);
	}
	set_substraction_flag(&cpu->registers, 0);
	set_half_carry_flag(&cpu->registers, 0);
	set_carry_flag(&cpu->registers, 0);
}

void srl_8(CPU *cpu, uint8_t *dest) {
	uint8_t bit_0 = (*dest) & 1;
	(*dest) >>= 1;
	(*dest) &= ~(1 << 7);
	if ((*dest) == 0x00) {
		set_zero_flag(&cpu->registers, 1);
	} else {
		set_zero_flag(&cpu->registers, 0);
	}
	set_substraction_flag(&cpu->registers, 0);
	set_half_carry_flag(&cpu->registers, 0);
	set_carry_flag(&cpu->registers, bit_0);
}

void res_8(CPU *cpu, uint8_t *dest, uint8_t bit) {
	(void) cpu;
	(*dest) &= ~(1 << bit);
}

void set_8(CPU *cpu, uint8_t *dest, uint8_t bit) {
	(void) cpu;
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