#ifndef _EMU_H_
#define _EMU_H_

#define _POSIX_C_SOURCE 199309L

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define FREQ_SYNC 60.
#define DEFAULT_ROM "./rom/PONG"

#define GET_0100(var) ((var & 0x0F00) >> 8)
#define GET_0010(var) ((var & 0x00F0) >> 4)
#define GET_1000(var) ((var & 0xF000) >> 12)
#define GET_0001(var) (var & 0x000F)
#define GET_0011(var) (var & 0x00FF)
#define GET_0111(var) (var & 0x0FFF)

#define MEMORY_SIZE 0x1000 /* 4096 */
uint8_t Memory[MEMORY_SIZE];

#define DATA_REGISTER_NUMBER 16
uint8_t V[DATA_REGISTER_NUMBER]; /* data register */

#define TAMPON 4
#define WIDTH  0x40 /* 64 */
#define HEIGHT 0x20 /* 32 */
uint8_t Gfx[WIDTH * HEIGHT]; /* graphic */

uint8_t Dt; /* delay timer */
uint8_t St; /* sound timer */

uint16_t I;

uint16_t Stack[16];
uint16_t Sp;

uint16_t Key;
int32_t Rom_size;

#define FONT_ADDR 0x0010 /* font adress in memory */
#define CHAR_ENCODED 5 /* bytes */
#define NUMBER_OF_CHAR 16

/* CHIP 8 description */
/* ---
 * - MEMORY
 * Memory size 4096 bytes
 * 0x0000 - 0x0200 (512 byte) : interpreter - font data ?
 * 0x0F00 - 0x0FFF : display refresh
 * 0x0EA9 - 0x0EFF : call stack, internal use, other variables
 * ---
 * - REGISTER
 * 16 * 8 bit data register : V0 - VF
 * ---
 * - STACK
 * stack return addresses when subroutines are called
 * ---
 * - TIMER
 * 2 timers, both count down at 60hz until they reach 0
 * Delay timer : timing the events of game (R/W)
 * Sound timer : sound effect (!0 => SOUND)
 * ---
 * - INPUT
 * 16 keys : 0 - F
 *  8 4 2 6 : directionnal input
 *   3 opcode :
 *    1 : key pressed
 *    2 : key not pressed
 *    3 : wait key to be press and then store it in one data register
 * ---
 * - GRAPHICS and SOUND
 * 64 * 32 pixels
 * sprite : 8 pixels wide : 1 - 15 pixels in height
 * ---
 * - OPCODE
 * 35 opcodes: 2 bytes long - big endian
 * ---
 */

int32_t emu_init_gfx(void);
uint16_t emu_decode_opcode(uint16_t opcode, uint16_t pc);
uint16_t emu_fetch_opcode(uint16_t pc);
uint16_t emu_mainloop(uint16_t pc);
uint16_t emu_opcode_00E0(uint16_t pc);
uint16_t emu_opcode_00EE(uint16_t pc);
uint16_t emu_opcode_0NNN(uint16_t pc);
uint16_t emu_opcode_1NNN(uint16_t opcode, uint16_t pc);
uint16_t emu_opcode_2NNN(uint16_t opcode, uint16_t pc);
uint16_t emu_opcode_3XNN(uint16_t opcode, uint16_t pc);
uint16_t emu_opcode_4XNN(uint16_t opcode, uint16_t pc);
uint16_t emu_opcode_5XY0(uint16_t opcode, uint16_t pc);
uint16_t emu_opcode_6XNN(uint16_t opcode, uint16_t pc);
uint16_t emu_opcode_7XNN(uint16_t opcode, uint16_t pc);
uint16_t emu_opcode_8XY0(uint16_t opcode, uint16_t pc);
uint16_t emu_opcode_8XY1(uint16_t opcode, uint16_t pc);
uint16_t emu_opcode_8XY2(uint16_t opcode, uint16_t pc);
uint16_t emu_opcode_8XY3(uint16_t opcode, uint16_t pc);
uint16_t emu_opcode_8XY4(uint16_t opcode, uint16_t pc);
uint16_t emu_opcode_8XY5(uint16_t opcode, uint16_t pc);
uint16_t emu_opcode_8XY6(uint16_t opcode, uint16_t pc);
uint16_t emu_opcode_8XY7(uint16_t opcode, uint16_t pc);
uint16_t emu_opcode_8XYE(uint16_t opcode, uint16_t pc);
uint16_t emu_opcode_9XY0(uint16_t opcode, uint16_t pc);
uint16_t emu_opcode_ANNN(uint16_t opcode, uint16_t pc);
uint16_t emu_opcode_BNNN(uint16_t opcode, uint16_t pc);
uint16_t emu_opcode_CXNN(uint16_t opcode, uint16_t pc);
uint16_t emu_opcode_DXYN(uint16_t opcode, uint16_t pc);
uint16_t emu_opcode_EX9E(uint16_t opcode, uint16_t pc);
uint16_t emu_opcode_EXA1(uint16_t opcode, uint16_t pc);
uint16_t emu_opcode_FX07(uint16_t opcode, uint16_t pc);
uint16_t emu_opcode_FX0A(uint16_t opcode, uint16_t pc);
uint16_t emu_opcode_FX15(uint16_t opcode, uint16_t pc);
uint16_t emu_opcode_FX18(uint16_t opcode, uint16_t pc);
uint16_t emu_opcode_FX1E(uint16_t opcode, uint16_t pc);
uint16_t emu_opcode_FX29(uint16_t pc);
uint16_t emu_opcode_FX33(uint16_t opcode, uint16_t pc);
uint16_t emu_opcode_FX55(uint16_t opcode, uint16_t pc);
uint16_t emu_opcode_FX65(uint16_t opcode, uint16_t pc);
uint32_t emu_timespec2nsec(struct timespec t);
double emu_d_compute_freq(struct timespec t);
void emu_nanosleep(uint32_t freq, struct timespec t);
void emu_process_input(void);
void emu_load_rom(char *path);
void emu_free_gfx(void);
void emu_print_gfx(int32_t x0, int32_t y0);
struct timespec emu_difftimespec(struct timespec t1, struct timespec t2);
struct timespec emu_gettime(void);

#endif
