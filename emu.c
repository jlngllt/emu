#define _POSIX_C_SOURCE 199309L

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <ncurses.h>

#define FREQ_SYNC 60.

#define GET_0100(var) ((var & 0x0F00) >> 8)
#define GET_0010(var) ((var & 0x00F0) >> 4)
#define GET_1000(var) ((var & 0xF000) >> 12)
#define GET_0001(var) (var & 0x000F)
#define GET_0011(var) (var & 0x00FF)
#define GET_0111(var) (var & 0x0FFF)

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

uint8_t Chip8_fontset[CHAR_ENCODED * NUMBER_OF_CHAR] = {
   0xF0, 0x90, 0x90, 0x90, 0xF0, /* 0 */
   0x20, 0x60, 0x20, 0x20, 0x70, /* 1 */
   0xF0, 0x10, 0xF0, 0x80, 0xF0, /* 2 */
   0xF0, 0x10, 0xF0, 0x10, 0xF0, /* 3 */
   0x90, 0x90, 0xF0, 0x10, 0x10, /* 4 */
   0xF0, 0x80, 0xF0, 0x10, 0xF0, /* 5 */
   0xF0, 0x80, 0xF0, 0x90, 0xF0, /* 6 */
   0xF0, 0x10, 0x20, 0x40, 0x40, /* 7 */
   0xF0, 0x90, 0xF0, 0x90, 0xF0, /* 8 */
   0xF0, 0x90, 0xF0, 0x10, 0xF0, /* 9 */
   0xF0, 0x90, 0xF0, 0x90, 0x90, /* A */
   0xE0, 0x90, 0xE0, 0x90, 0xE0, /* B */
   0xF0, 0x80, 0x80, 0x80, 0xF0, /* C */
   0xE0, 0x90, 0x90, 0x90, 0xE0, /* D */
   0xF0, 0x80, 0xF0, 0x80, 0xF0, /* E */
   0xF0, 0x80, 0xF0, 0x80, 0x80  /* F */
};

int emu_load_rom(char *path)
{
   FILE *f = fopen(path, "r");

   if (!f) {
      perror("fopen");
      exit(EXIT_FAILURE);
   }

   fread(&(Memory[0x200]), MEMORY_SIZE - 0x200, 1, f);
   fseek(f, 0L, SEEK_END);
   Rom_size = ftell(f);

   return 0;
}

struct timespec emu_gettime()
{
   struct timespec t;

   if (clock_gettime(CLOCK_REALTIME, &t) == -1) {
      perror("clock gettime");
      exit(EXIT_FAILURE);
   }
   return t;
}

uint32_t emu_timespec2nsec(struct timespec t)
{
   return (uint32_t)(t.tv_sec * 1000000000L + t.tv_nsec);
}

struct timespec emu_difftimespec(struct timespec t1, struct timespec t2)
{
   struct timespec t;
   t.tv_sec  = t2.tv_sec - t1.tv_sec;
   t.tv_nsec = t2.tv_nsec - t1.tv_nsec;
   return t;
}

void emu_nanosleep(uint32_t freq, struct timespec t)
{
   struct timespec t_period, t_sleep;
   double period = 1. / (double)freq;

   t_period.tv_sec  = 0;
   t_period.tv_nsec = (__time_t)(period * (double)1000000000L);
   t_sleep = emu_difftimespec(t, t_period);

   nanosleep(&t_sleep, NULL);
}

void emu_d_compute_freq(struct timespec t)
{
   double freq;
   uint32_t wait_nsec;

   wait_nsec = emu_timespec2nsec(t);
   freq = 1. / (double)wait_nsec * (double)1000000000L;
   mvprintw(HEIGHT + 3, 0, "freq = %g", freq);
}

int32_t emu_print_gfx()
{
   int i = 0;
#ifdef DEBUG
   int j = 0;

   for (j = 0, i = 0; i < WIDTH; i++) {
      if ((i % 10) == 0)
         mvprintw(0, i + 2, "%d", j++);
      mvprintw(1, i + 2, "%d", i % 10);
   }

   for (j = 0, i = 0; i < HEIGHT; i++) {
      if ((i % 10) == 0)
         mvprintw(i + 2, 0, "%d", j++);
      mvprintw(i + 2, 1, "%d", i % 10);
   }
   mvprintw(0, 0, " ");
#endif

   for (i = 0; i < WIDTH * HEIGHT; i++) {
      mvprintw((i / WIDTH) + 2, i % WIDTH + 2, " ");
      if (Gfx[i] == 1)
         mvprintw((i / WIDTH) + 2, i % WIDTH + 2, "*");
   }
   return 0;
}

void emu_process_input()
{
   int32_t c;

   c = wgetch(stdscr);

   switch (c) {
      case '7': Key = 0x1; break;
      case '8': Key = 0x2; break;
      case '9': Key = 0x3; break;
      case '0': Key = 0xc; break;
      case 'u': Key = 0x4; break;
      case 'i': Key = 0x5; break;
      case 'o': Key = 0x6; break;
      case 'p': Key = 0xd; break;
      case 'j': Key = 0x7; break;
      case 'k': Key = 0x8; break;
      case 'l': Key = 0x9; break;
      case 'm': Key = 0xe; break;
      case ',': Key = 0xa; break;
      case ';': Key = 0x0; break;
      case ':': Key = 0xb; break;
      case '!': Key = 0xf; break;
      default: Key = Key; break;
   }
}

/*
 * Calls RCA 1802 program at address NNN. Not necessary for most ROMs.
 */
uint16_t emu_opcode_0NNN(uint16_t pc)
{
   printf("usesless no ? :'(\n");
   pc = (uint16_t)(pc + 2);
   return pc;
}

/*
 * Clears the screen.
 */
uint16_t emu_opcode_00E0(uint16_t pc)
{
   memset(Gfx, 0, sizeof(uint8_t) * WIDTH * HEIGHT);
   pc = (uint16_t)(pc + 2);
   return pc;
}

/*
 * Returns from a subroutine.
 */
uint16_t emu_opcode_00EE(uint16_t pc)
{
   if (Sp > 0) {
      pc = Stack[Sp - 1];
      Sp = (uint16_t)(Sp - 1);
   }
   return pc;
}

/*
 * Jumps to address NNN.
 */
uint16_t emu_opcode_1NNN(uint16_t opcode, uint16_t pc)
{
   emu_print_gfx();
   pc = GET_0111(opcode);
   return pc;
}

/*
 * Calls subroutine at NNN.
 */
uint16_t emu_opcode_2NNN(uint16_t opcode, uint16_t pc)
{
   Stack[Sp] = (uint16_t)(pc + 2);
   Sp = (uint16_t)(Sp + 1);
   pc = GET_0111(opcode);
   return pc;
}

/*
 * Skips the next instruction if VX equals NN.
 */
uint16_t emu_opcode_3XNN(uint16_t opcode, uint16_t pc)
{
   if (V[GET_0100(opcode)] == GET_0011(opcode))
      pc = (uint16_t)(pc + 2);
   pc = (uint16_t)(pc + 2);
   return pc;
}

/*
 * Skips the next instruction if VX doesn't equal NN.
 */
uint16_t emu_opcode_4XNN(uint16_t opcode, uint16_t pc)
{
   if (V[GET_0100(opcode)] != GET_0011(opcode))
      pc = (uint16_t)(pc + 2);
   pc = (uint16_t)(pc + 2);
   return pc;
}

/*
 * Skips the next instruction if VX equals VY.
 */
uint16_t emu_opcode_5XY0(uint16_t opcode, uint16_t pc)
{
   if (V[GET_0100(opcode)] == V[GET_0010(opcode)])
      pc = (uint16_t)(pc + 2);
   pc = (uint16_t)(pc + 2);
   return pc;
}

/*
 * Sets VX to NN.
 */
uint16_t emu_opcode_6XNN(uint16_t opcode, uint16_t pc)
{
   V[GET_0100(opcode)] = (uint8_t)GET_0011(opcode);
   pc = (uint16_t)(pc + 2);
   return pc;
}

/*
 * Adds NN to VX.
 */
uint16_t emu_opcode_7XNN(uint16_t opcode, uint16_t pc)
{
   V[GET_0100(opcode)] = (uint8_t)(V[GET_0100(opcode)] + GET_0011(opcode));
   pc = (uint16_t)(pc + 2);
   return pc;
}

/*
 * Sets VX to the value of VY.
 */
uint16_t emu_opcode_8XY0(uint16_t opcode, uint16_t pc)
{
   V[GET_0100(opcode)] = V[GET_0010(opcode)];
   pc = (uint16_t)(pc + 2);
   return pc;
}

/*
 * Sets VX to VX or VY.
 */
uint16_t emu_opcode_8XY1(uint16_t opcode, uint16_t pc)
{
   V[GET_0100(opcode)] |= V[GET_0010(opcode)];
   pc = (uint16_t)(pc + 2);
   return pc;
}

/*
 * Sets VX to VX and VY.
 */
uint16_t emu_opcode_8XY2(uint16_t opcode, uint16_t pc)
{
   V[GET_0100(opcode)] &= V[GET_0010(opcode)];
   pc = (uint16_t)(pc + 2);
   return pc;
}

/*
 * Sets VX to VX xor VY.
 */
uint16_t emu_opcode_8XY3(uint16_t opcode, uint16_t pc)
{
   V[GET_0100(opcode)] ^= V[GET_0010(opcode)];
   pc = (uint16_t)(pc + 2);
   return pc;
}

/*
 * Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there
 * isn't.
 */
uint16_t emu_opcode_8XY4(uint16_t opcode, uint16_t pc)
{
   V[GET_0100(opcode)] = (uint8_t)(V[GET_0100(opcode)] + V[GET_0010(opcode)]);

   if (V[GET_0100(opcode)] + V[GET_0010(opcode)] > 0xFF)
      V[0xF] = 1;
   else
      V[0xF] = 0;
   pc = (uint16_t)(pc + 2);
   return pc;
}

/*
 * VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when
 * there isn't.
 */
uint16_t emu_opcode_8XY5(uint16_t opcode, uint16_t pc)
{
   V[GET_0100(opcode)] = (uint8_t)(V[GET_0100(opcode)] - V[GET_0010(opcode)]);

   if (V[GET_0100(opcode)] - V[GET_0010(opcode)] < 0)
      V[0xF] = 1;
   else
      V[0xF] = 0;

   pc = (uint16_t)(pc + 2);
   return pc;
}

/*
 * Shifts VX right by one. VF is set to the value of the least significant bit
 * of VX before the shift.[2]
 */
uint16_t emu_opcode_8XY6(uint16_t opcode, uint16_t pc)
{
   uint8_t tmp;

   tmp = V[GET_0100(opcode)];
   V[GET_0100(opcode)] = (uint16_t)(tmp >> 1);
   pc = (uint16_t)(pc + 2);
   return pc;
}

/*
 * Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when
 * there isn't.
 */
uint16_t emu_opcode_8XY7(uint16_t opcode, uint16_t pc)
{
   V[GET_0100(opcode)] = (uint8_t)(V[GET_0010(opcode)] - V[GET_0100(opcode)]);
   pc = (uint16_t)(pc + 2);
   return pc;
}

/*
 * Shifts VX left by one. VF is set to the value of the most significant bit of
 * VX before the shift
 */
uint16_t emu_opcode_8XYE(uint16_t opcode, uint16_t pc)
{
   uint8_t tmp;

   tmp = V[GET_0100(opcode)];
   V[GET_0100(opcode)] = (uint8_t)(tmp << 1);
   return pc;
}

/*
 * Skips the next instruction if VX doesn't equal VY.
 */
uint16_t emu_opcode_9XY0(uint16_t opcode, uint16_t pc)
{
   if (V[GET_0100(opcode)] != V[GET_0010(opcode)])
      pc = (uint16_t)(pc + 2);
   pc = (uint16_t)(pc + 2);
   return pc;
}

/*
 * Sets I to the address NNN.
 */
uint16_t emu_opcode_ANNN(uint16_t opcode, uint16_t pc)
{
   I  = (uint16_t)GET_0111(opcode);
   pc = (uint16_t)(pc + 2);
   return pc;
}

/*
 * Jumps to the address NNN plus V0.
 */
uint16_t emu_opcode_BNNN(uint16_t opcode, uint16_t pc)
{
   pc = (uint16_t)(GET_0111(opcode) + V[0]);
   pc = (uint16_t)(pc + 2);
   return pc;
}

/*
 * Sets VX to the result of a bitwise and operation on a random number and NN.
 */
uint16_t emu_opcode_CXNN(uint16_t opcode, uint16_t pc)
{
   int random = rand();

   srand((unsigned int)(random));

#if DEBUG
   mvprintw(HEIGHT + 27, 0, "%02x & %02x = %02x", (random % 0xFF),
         GET_0011(opcode), (uint8_t)((random % 0xFF) & GET_0011(opcode)));
#endif
   V[GET_0100(opcode)] = (uint8_t)((random % 0xFF) & GET_0011(opcode));

   pc = (uint16_t)(pc + 2);
   return pc;
}

/*
 * Sprites stored in memory at location in index register (I), 8bits wide. Wraps
 * around the screen. If when drawn, clears a pixel, register VF is set to 1
 * otherwise it is zero. All drawing is XOR drawing (i.e. it toggles the screen
 * pixels). Sprites are drawn starting at position VX, VY. N is the number of
 * 8bit rows that need to be drawn. If N is greater than 1, second line
 * continues at position VX, VY+1, and so on.
 */
uint16_t emu_opcode_DXYN(uint16_t opcode, uint16_t pc)
{
   uint8_t i_row, j_col, x, y, nb_of_row = 0;
   uint8_t pix = 0;
   int k = 0;

#if DEBUG
   static int nb_of_row_last = 0;
#endif

   x = V[GET_0100(opcode)];
   y = V[GET_0010(opcode)];
   nb_of_row = GET_0001(opcode);
   V[0xF] = 0;

#if DEBUG
   mvprintw(HEIGHT + 4 + nb_of_row_last, WIDTH + 3 + 0, "  ");
   mvprintw(HEIGHT + 4 + nb_of_row_last, WIDTH + 5 + 8, "  ");
   mvprintw(HEIGHT + 2, WIDTH + 2, "x = %d, y = %d, row = %d, emplacement tableau %d", x, y, nb_of_row, (WIDTH * y) + x);
   refresh();
#endif

   for (i_row = 0; i_row < nb_of_row; i_row++) {
      k++;
      for (j_col = 0; j_col < 8; j_col++) {
         pix = (Memory[I + i_row] >> (7 - j_col)) & 0x01;
#if DEBUG
         k++;
         mvprintw(0, WIDTH + 70, "JULIEN");
         mvprintw(k, WIDTH + 70, "|x = %3d, y = %3d|", ((y + i_row) % HEIGHT), ((x + j_col) % WIDTH));
         refresh();
#endif

         if (Gfx[(WIDTH * (y + i_row)) + (x + j_col)] != pix) {
            V[0xF] = 1;
         }
         Gfx[(WIDTH * (y + i_row)) + (x + j_col)] ^= pix;
      }
   }
#if DEBUG
   mvprintw(HEIGHT + 4, WIDTH + 5, "_");
   mvprintw(HEIGHT + 5, WIDTH + 4, "|");

   for (i_row = 0; i_row < 0xF; i_row++) {
      for (j_col = 0; j_col < 8; j_col++) {
         if (((Memory[I + i_row] >> (7 - j_col)) & 0x01) != 0)
            mvprintw(HEIGHT + 5 + i_row, WIDTH + 5 + j_col, "x");
         else
            mvprintw(HEIGHT + 5 + i_row, WIDTH + 5 + j_col, ".");
      }
   }
   nb_of_row_last = nb_of_row;
   mvprintw(HEIGHT + 4 + nb_of_row, WIDTH + 3 + 0,    "->");
   mvprintw(HEIGHT + 4 + nb_of_row, WIDTH + 5 + 8,    "<-");
   mvprintw(HEIGHT + 4 + i_row, WIDTH + 5 + j_col, "|");
   mvprintw(HEIGHT + 5 + i_row, WIDTH + 4 + j_col, "'");

   refresh();
#endif
   pc = (uint16_t)(pc + 2);
   return pc;
}

/*
 * Skips the next instruction if the key stored in VX is pressed.
 */
uint16_t emu_opcode_EX9E(uint16_t opcode, uint16_t pc)
{
   if (V[GET_0100(opcode)] == Key)
      pc = (uint16_t)(pc + 2);
   pc = (uint16_t)(pc + 2);
   return pc;
}

/*
 * Skips the next instruction if the key stored in VX isn't pressed.
 */
uint16_t emu_opcode_EXA1(uint16_t opcode,
      uint16_t pc)
{
   if (V[GET_0100(opcode)] != Key)
      pc = (uint16_t)(pc + 2);
   pc = (uint16_t)(pc + 2);
   return pc;
}

/*
 * Sets VX to the value of the delay timer.
 */
uint16_t emu_opcode_FX07(uint16_t opcode,
      uint16_t pc)
{
   V[GET_0100(opcode)] = Dt;
   pc = (uint16_t)(pc + 2);
   return pc;
}

/*
 * A key press is awaited, and then stored in VX.
 */
uint16_t emu_opcode_FX0A(uint16_t opcode,
      uint16_t pc)
{
   Key = V[GET_0100(opcode)];
   pc  = (uint16_t)(pc + 2);
   return pc;
}

/*
 * Sets the delay timer to VX.
 */
uint16_t emu_opcode_FX15(uint16_t opcode, uint16_t pc)
{
   Dt = V[GET_0100(opcode)];
   pc = (uint16_t)(pc + 2);
   return pc;
}

/*
 * Sets the sound timer to VX.
 */
uint16_t emu_opcode_FX18(uint16_t opcode, uint16_t pc)
{
   St = V[GET_0100(opcode)];
   pc = (uint16_t)(pc + 2);
   return pc;
}

/*
 * Adds VX to I
 */
uint16_t emu_opcode_FX1E(uint16_t opcode, uint16_t pc)
{
   I  = (uint16_t)(I + V[GET_0100(opcode)]);
   pc = (uint16_t)(pc + 2);
   return pc;
}

/*
 * Sets I to the location of the sprite for the character in VX. Characters 0-F
 * (in hexadecimal) are represented by a 4x5 font.
 */
uint16_t emu_opcode_FX29(uint16_t pc)
{
   I = FONT_ADDR;
   pc = (uint16_t)(pc + 2);
   return pc;
}

/*
 * Stores the binary-coded decimal representation of VX, with the most
 * significant of three digits at the address in I, the middle digit at I plus
 * 1, and the least significant digit at I plus 2. (In other words, take the
 * decimal representation of VX, place the hundreds digit in memory at location
 * in I, the tens digit at location I+1, and the ones digit at location I+2.)
 */
uint16_t emu_opcode_FX33(uint16_t opcode, uint16_t pc)
{
   Memory[I] = V[GET_0100(opcode)] / 100;
   Memory[I + 1] = (V[GET_0100(opcode)] / 10) % 10;
   Memory[I + 2] = (V[GET_0100(opcode)] / 100) % 10;
   pc = (uint16_t)(pc + 2);
   return pc;
}

/*
 * Stores V0 to VX (including VX) in memory starting at address I
 */
uint16_t emu_opcode_FX55(uint16_t opcode, uint16_t pc)
{
   uint8_t i = 0;

   for (i = 0; i <= GET_0100(opcode); i++)
      Memory[I + i] = V[i];
   pc = (uint16_t)(pc + 2);
   return pc;
}

/*
 * Fills V0 to VX (including VX) with values from memory starting at address I.
 */
uint16_t emu_opcode_FX65(uint16_t opcode, uint16_t pc)
{
   uint8_t i = 0;

   for (i = 0; i <= GET_0100(opcode); i++)
      Memory[I + i] = V[i];
   pc = (uint16_t)(pc + 2);
   return pc;
}

int32_t emu_print_debug(uint16_t opcode,
      uint16_t mem_pc)
{
   int pc, j, x, y;

#define SIZE_ESPACE 1
#define SIZE_OPCODE 4
#define N_LINE      15
#define OFFSET_W    5
#define OFFSET_Y    5
#define X_0_RAM     WIDTH + OFFSET_W
#define Y_0_RAM     0 + OFFSET_H

   mvprintw(HEIGHT + 2,  0, "Pc       : 0x%04x", mem_pc);
   mvprintw(HEIGHT + 3,  0, "opcode   : 0x%04x", opcode);
   mvprintw(HEIGHT + 4,  0, "I        : 0x%04x", I);
   mvprintw(HEIGHT + 5,  0, "Dt       : 0x%02x", Dt);
   mvprintw(HEIGHT + 6,  0, "V[0]     : 0x%02x", V[0]);
   mvprintw(HEIGHT + 7,  0, "V[1]     : 0x%02x", V[1]);
   mvprintw(HEIGHT + 8,  0, "V[2]     : 0x%02x", V[2]);
   mvprintw(HEIGHT + 9,  0, "V[3]     : 0x%02x", V[3]);
   mvprintw(HEIGHT + 10, 0, "V[4]     : 0x%02x", V[4]);
   mvprintw(HEIGHT + 11, 0, "V[5]     : 0x%02x", V[5]);
   mvprintw(HEIGHT + 12, 0, "V[6]     : 0x%02x", V[6]);
   mvprintw(HEIGHT + 13, 0, "V[7]     : 0x%02x", V[7]);
   mvprintw(HEIGHT + 14, 0, "V[8]     : 0x%02x", V[8]);
   mvprintw(HEIGHT + 15, 0, "V[9]     : 0x%02x", V[9]);
   mvprintw(HEIGHT + 16, 0, "V[10]    : 0x%02x", V[10]);
   mvprintw(HEIGHT + 17, 0, "V[11]    : 0x%02x", V[11]);
   mvprintw(HEIGHT + 18, 0, "V[12]    : 0x%02x", V[12]);
   mvprintw(HEIGHT + 19, 0, "V[13]    : 0x%02x", V[13]);
   mvprintw(HEIGHT + 20, 0, "V[14]    : 0x%02x", V[14]);
   mvprintw(HEIGHT + 21, 0, "V[15]    : 0x%02x", V[15]);
   mvprintw(HEIGHT + 22, 0,   "Sp       : %02d", Sp);
   mvprintw(HEIGHT + 23, 0, "Stack[0] : 0x%02x", Stack[0]);
   mvprintw(HEIGHT + 24, 0, "Stack[1] : 0x%02x", Stack[1]);
   mvprintw(HEIGHT + 25, 0, "Stack[2] : 0x%02x", Stack[2]);

   /* affichage de la mémoire */
   for (y = 0, x = 0, pc = 0, j = 0; pc < (Rom_size + 0x200); pc += 2, j++) {
      attroff(A_UNDERLINE);
      if (j == (mem_pc / 2))
         attron(A_UNDERLINE);
      mvprintw(y, X_0_RAM + x, "%02x%02x", Memory[pc], Memory[pc + 1]);

      /* nombre de ligne sur lequel on affiche la mémoire */
      if (x > (Rom_size + 0x200) / N_LINE) {
         y++;
         x = 0;
      } else {
         x += SIZE_OPCODE + SIZE_ESPACE;
      }
   }

   refresh();

#undef SIZE_ESPACE
#undef SIZE_OPCODE
#undef N_LINE
#undef OFFSET_W
#undef OFFSET_Y
#undef X_0_RAM
#undef Y_0_RAM

   return 0;
}

int32_t emu_init_ncurses(void)
{
   initscr();
   noecho();
   cbreak();
   curs_set(0);
   keypad(stdscr, TRUE);

#ifndef DEBUG
   nodelay(stdscr, TRUE);
#endif

   return 0;
}

uint16_t emu_fetch_opcode(uint16_t pc)
{
   /* fetch opcode */
   /* --- Example -- */
   /* Memory = ... 0xA2 0xF0 ...*/
   /* Memory     = 0xA2 */
   /* Memory + 1 = 0xF0 */
   /* opcode doit être = 0xA2F0 */
   return (uint16_t)(Memory[pc] << 8 | Memory[pc + 1]);
}

uint16_t emu_decode_opcode(uint16_t opcode,
      uint16_t pc)
{
   switch (GET_1000(opcode)) {
      case 0x0:
         if (GET_0011(opcode) == 0xE0)
            pc = emu_opcode_00E0(pc);
         else if (GET_0011(opcode) == 0xEE)
            pc = emu_opcode_00EE(pc);
         else
            pc = emu_opcode_0NNN(pc);
      break;

      case 0x1:
         pc = emu_opcode_1NNN(opcode, pc);
      break;

      case 0x2:
         pc = emu_opcode_2NNN(opcode, pc);
      break;

      case 0x3:
         pc = emu_opcode_3XNN(opcode, pc);
      break;

      case 0x4:
         pc = emu_opcode_4XNN(opcode, pc);
      break;

      case 0x5:
         pc = emu_opcode_5XY0(opcode, pc);
      break;

      case 0x6:
         pc = emu_opcode_6XNN(opcode, pc);
      break;

      case 0x7:
         pc = emu_opcode_7XNN(opcode, pc);
      break;

      case 0x8:
         if ((GET_0001(opcode)) == 0x0)
            pc = emu_opcode_8XY0(opcode, pc);
         else if (GET_0001(opcode) == 0x1)
            pc = emu_opcode_8XY1(opcode, pc);
         else if (GET_0001(opcode) == 0x2)
            pc = emu_opcode_8XY2(opcode, pc);
         else if (GET_0001(opcode) == 0x3)
            pc = emu_opcode_8XY3(opcode, pc);
         else if (GET_0001(opcode) == 0x4)
            pc = emu_opcode_8XY4(opcode, pc);
         else if (GET_0001(opcode) == 0x5)
            pc = emu_opcode_8XY5(opcode, pc);
         else if (GET_0001(opcode) == 0x6)
            pc = emu_opcode_8XY6(opcode, pc);
         else if (GET_0001(opcode) == 0x7)
            pc = emu_opcode_8XY7(opcode, pc);
         else if (GET_0001(opcode) == 0xE)
            pc = emu_opcode_8XYE(opcode, pc);
         else
            printf("erreur) opcode inconnu, %x\n", opcode);
      break;

      case 0x9:
         pc = emu_opcode_9XY0(opcode, pc);
      break;

      case 0xA:
         pc = emu_opcode_ANNN(opcode, pc);
      break;

      case 0xB:
         pc = emu_opcode_BNNN(opcode, pc);
      break;

      case 0xC:
         pc = emu_opcode_CXNN(opcode, pc);
      break;

      case 0xD:
         pc = emu_opcode_DXYN(opcode, pc);
      break;

      case 0xE:
         if (GET_0011(opcode) == 0x9E)
            pc = emu_opcode_EX9E(opcode, pc);
         else if (GET_0011(opcode) == 0xA1)
            pc = emu_opcode_EXA1(opcode, pc);
         else
            printf("erreur) opcode inconnu, %x\n", opcode);
      break;

      case 0xF:
         if (GET_0011(opcode) == 0x07)
            pc = emu_opcode_FX07(opcode, pc);
         else if (GET_0011(opcode) == 0x0A)
            pc = emu_opcode_FX0A(opcode, pc);
         else if (GET_0011(opcode) == 0x15)
            pc = emu_opcode_FX15(opcode, pc);
         else if (GET_0011(opcode) == 0x18)
            pc = emu_opcode_FX18(opcode, pc);
         else if (GET_0011(opcode) == 0x1E)
            pc = emu_opcode_FX1E(opcode, pc);
         else if (GET_0011(opcode) == 0x29)
            pc = emu_opcode_FX29(pc);
         else if (GET_0011(opcode) == 0x33)
            pc = emu_opcode_FX33(opcode, pc);
         else if (GET_0011(opcode) == 0x55)
            pc = emu_opcode_FX55(opcode, pc);
         else if (GET_0011(opcode) == 0x65)
            pc = emu_opcode_FX65(opcode, pc);
         else
            printf("erreur) opcode inconnu, %x\n", opcode);
      break;

      default:
         printf("impossible d'arriver ici normalement\n");
      break;
   }
   return pc;
}

uint16_t emu_mainloop(uint16_t pc)
{
   struct timespec t1, t2;
   uint16_t opcode;

#ifdef DEBUG
   static uint16_t mem_pc;
#endif

#ifdef DEBUG
   mem_pc = pc;
#endif

   t1 = emu_gettime();

   emu_process_input();
   opcode = emu_fetch_opcode(pc);
   pc = emu_decode_opcode(opcode, pc);
#ifdef DEBUG
   emu_print_debug(opcode, mem_pc);
#endif

   if (Dt > 0)
      Dt--;

   t2 = emu_gettime();
   emu_nanosleep(FREQ_SYNC, emu_difftimespec(t1, t2));

   emu_d_compute_freq(emu_difftimespec(t1, emu_gettime()));
   return pc;
}

int32_t main(int32_t argc,
      char *argv[])
{
#if DEBUG
   int32_t n_it = 0;
   int32_t c;
#endif
   uint16_t pc = 0x200;

   I  = 0;
   Sp = 0;

   /* init random */
   srand((unsigned int)(time(NULL)));

   if (argc < 2)
   {
      fprintf(stderr, "rom not known\n");
      exit(EXIT_FAILURE);
   }
   /* charge la font */
   memcpy(&Memory[FONT_ADDR],
         Chip8_fontset,
         CHAR_ENCODED * NUMBER_OF_CHAR * sizeof(uint8_t));

   /* charge la rom */
   emu_load_rom(argv[1]);
   emu_init_ncurses();

   for (;;) {
#if DEBUG
      c = wgetch(stdscr);

      switch (c) {
         case KEY_RIGHT:
            pc = emu_mainloop(pc);
            n_it++;
            mvprintw(HEIGHT + 28, 0, "it: %04d", n_it);
            refresh();
         break;

         case ' ':
            c = getch();
            switch (c) {
               case 'è': Key = 0x1; break;
               case '_': Key = 0x2; break;
               case 'ç': Key = 0x3; break;
               case 'à': Key = 0xc; break;
               case 'u': Key = 0x4; break;
               case 'i': Key = 0x5; break;
               case 'o': Key = 0x6; break;
               case 'p': Key = 0xd; break;
               case 'j': Key = 0x7; break;
               case 'k': Key = 0x8; break;
               case 'l': Key = 0x9; break;
               case 'm': Key = 0xe; break;
               case ',': Key = 0xa; break;
               case ';': Key = 0x0; break;
               case ':': Key = 0xb; break;
               case '!': Key = 0xf; break;
               default: Key = Key; break;
            }
            mvprintw(HEIGHT + 28, 20, "Key : 0x%x", Key);
            refresh();
         break;

         default:
            refresh();
         break;
      }

#else
      pc = emu_mainloop(pc);
#endif
   }
   endwin();
   return 0;
}
