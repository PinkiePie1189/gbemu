#include "Registers.h"

void set_carry_flag(Registers *regs, uint8_t value) {
	if (value) {
		regs->registers.f |= (1u << 4);
	} else {
		regs->registers.f &= ~(1u << 4);
	}
}

void set_half_carry_flag(Registers *regs, uint8_t value) {
	if (value) {
		regs->registers.f |= (1u << 5);
	} else {
		regs->registers.f &= ~(1u << 5);
	}
}

void set_substraction_flag(Registers *regs, uint8_t value) {
	if (value) {
		regs->registers.f |= (1u << 6);
	} else {
		regs->registers.f &= ~(1u << 6);
	}
}

void set_zero_flag(Registers *regs, uint8_t value) {
	if (value) {
		regs->registers.f |= (1u << 7);
	} else {
		regs->registers.f &= ~(1u << 7);
	}
}

uint8_t get_carry_flag(Registers *regs) {
	return (regs->registers.f >> 4) & 1;
}

uint8_t get_zero_flag(Registers *regs) {
	return (regs->registers.f >> 7) & 1;
}

uint8_t get_half_carry_flag(Registers *regs) {
	return (regs->registers.f >> 5) & 1;	
}

uint8_t get_substraction_flag(Registers *regs) {
	return (regs->registers.f >> 6) & 1;
}

void print_registers(Registers *regs) {
	// printf("A = %.2hhx\n", regs->registers.a);
	// printf("B = %.2hhx\n", regs->registers.b);
	// printf("C = %.2hhx\n", regs->registers.c);
	// printf("D = %.2hhx\n", regs->registers.d);
	// printf("E = %.2hhx\n", regs->registers.e);
	// printf("H = %.2hhx\n", regs->registers.h);
	// printf("L = %.2hhx\n", regs->registers.l);
	printf("Registers:\n");
	printf("AF = %.4hx\n", regs->d_registers.af);
	printf("BC = %.4hx\n", regs->d_registers.bc);
	printf("DE = %.4hx\n", regs->d_registers.de);
	printf("HL = %.4hx\n", regs->d_registers.hl);
}
