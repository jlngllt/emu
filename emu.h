#ifndef EMU_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <ncurses.h>

/* CHIP 8 description */
/* ---
 * - MEMORY
 * Memory size 4096 bytes
 * 0x0000 - 0x0200 (512 byte) : interpreter - font data ?
 * 0x0F00 - 0x0FFF            : display refresh
 * 0x0EA9 - 0x0EFF            : call stack, internal use, other variables
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

/* prototype */
int load_rom(char *path);
uint16_t mainloop(uint16_t pc);

uint16_t fetch_opcode(uint16_t pc);
uint16_t decode_opcode(uint16_t opcode, uint16_t pc);
int print_gfx();
int print_debug(uint16_t opcode, uint16_t mem_pc);

uint16_t opcode_0NNN(uint16_t pc);
uint16_t opcode_00E0(uint16_t pc);
uint16_t opcode_00EE(uint16_t pc);
uint16_t opcode_1NNN(uint16_t opcode, uint16_t pc);
uint16_t opcode_2NNN(uint16_t opcode, uint16_t pc);
uint16_t opcode_3XNN(uint16_t opcode, uint16_t pc);
uint16_t opcode_4XNN(uint16_t opcode, uint16_t pc);
uint16_t opcode_5XY0(uint16_t opcode, uint16_t pc);
uint16_t opcode_6XNN(uint16_t opcode, uint16_t pc);
uint16_t opcode_7XNN(uint16_t opcode, uint16_t pc);
uint16_t opcode_8XY0(uint16_t opcode, uint16_t pc);
uint16_t opcode_8XY1(uint16_t opcode, uint16_t pc);
uint16_t opcode_8XY2(uint16_t opcode, uint16_t pc);
uint16_t opcode_8XY3(uint16_t opcode, uint16_t pc);
uint16_t opcode_8XY4(uint16_t opcode, uint16_t pc);
uint16_t opcode_8XY5(uint16_t opcode, uint16_t pc);
uint16_t opcode_8XY6(uint16_t opcode, uint16_t pc);
uint16_t opcode_8XY7(uint16_t opcode, uint16_t pc);
uint16_t opcode_8XYE(uint16_t opcode, uint16_t pc);
uint16_t opcode_9XY0(uint16_t opcode, uint16_t pc);
uint16_t opcode_ANNN(uint16_t opcode, uint16_t pc);
uint16_t opcode_BNNN(uint16_t opcode, uint16_t pc);
uint16_t opcode_CXNN(uint16_t opcode, uint16_t pc);
uint16_t opcode_DXYN(uint16_t opcode, uint16_t pc);
uint16_t opcode_EX9E(uint16_t opcode, uint16_t pc);
uint16_t opcode_EXA1(uint16_t opcode, uint16_t pc);
uint16_t opcode_FX07(uint16_t opcode, uint16_t pc);
uint16_t opcode_FX0A(uint16_t opcode, uint16_t pc);
uint16_t opcode_FX15(uint16_t opcode, uint16_t pc);
uint16_t opcode_FX18(uint16_t opcode, uint16_t pc);
uint16_t opcode_FX1E(uint16_t opcode, uint16_t pc);
uint16_t opcode_FX29(uint16_t pc);
uint16_t opcode_FX33(uint16_t opcode, uint16_t pc);
uint16_t opcode_FX55(uint16_t opcode, uint16_t pc);
uint16_t opcode_FX65(uint16_t opcode, uint16_t pc);

#endif
