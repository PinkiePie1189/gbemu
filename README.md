# gbemu

An emulator for the [Nintendo Game Boy](https://en.wikipedia.org/wiki/Game_Boy) written in C. Currently a work in progress.
# Building and running
Build the emulator on any major Linux distribution using `make`.
Then run the emulator using `./gbemu bios_path rom_path`.
# What works so far
The project is at an early stage, having only most of the CPU functionality implemented, which still has bugs. [These](https://github.com/retrio/gb-test-roms/tree/master/cpu_instrs) tests are used in order to check the system's functionality. So far the CPU's state looks as follows, with a tick indicating a passing test:
- [ ] 01-special
- [ ] 02-interrupts
- [ ] 03-op sp,hl
- [ ] 04-op r,imm
- [x] 05-op rp
- [x] 06-ld r,r
- [ ] 07-jr,jp,call,ret,rst
- [ ] 08-misc instrs
- [ ] 09-op r,r
- [x] 10-bit ops
- [ ] 11-op a,(hl)
