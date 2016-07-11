#ifndef EMU_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
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
int mainloop();
int fetch_opcode();
int decode_opcode();
int print_gfx();
int print_debug();
int opcode_0NNN();
int opcode_00E0();
int opcode_00EE();
int opcode_1NNN();
int opcode_2NNN();
int opcode_3XNN();
int opcode_4XNN();
int opcode_5XY0();
int opcode_6XNN();
int opcode_7XNN();
int opcode_8XY0();
int opcode_8XY1();
int opcode_8XY2();
int opcode_8XY3();
int opcode_8XY4();
int opcode_8XY5();
int opcode_8XY6();
int opcode_8XY7();
int opcode_8XYE();
int opcode_9XY0();
int opcode_ANNN();
int opcode_BNNN();
int opcode_CXNN();
int opcode_DXYN();
int opcode_EX9E();
int opcode_EXA1();
int opcode_FX07();
int opcode_FX0A();
int opcode_FX15();
int opcode_FX18();
int opcode_FX1E();
int opcode_FX29();
int opcode_FX33();
int opcode_FX55();
int opcode_FX65();

#endif
