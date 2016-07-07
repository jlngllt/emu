#ifndef EMU_H_

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

#include <stdint.h>
#include <stdio.h>

/* prototype */
int mainloop();

#endif
