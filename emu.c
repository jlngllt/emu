#include "emu.h"

#define MEMORY_SIZE 0x1000 /* 4096 */
uint8_t Memory[MEMORY_SIZE];

#define DATA_REGISTER_NUMBER 16
uint8_t V[DATA_REGISTER_NUMBER]; /* data register */

uint16_t Opcode;

#define WIDTH 0x40 /* 64 */
#define HEIGHT 0x20 /* 32 */
uint8_t Gfx[WIDTH*HEIGHT]; /* graphic */

uint8_t Dt; /* delay timer */
uint8_t St; /* sound timer */

uint16_t I;
uint16_t Pc;

uint16_t Stack[16];
uint16_t Sp;

uint16_t Key;

#define FONT_ADDR 0x0010 /* font adress in memory */
#define CHAR_ENCODED 5 /* bytes */
#define NUMBER_OF_CHAR 16

uint8_t Chip8_fontset[CHAR_ENCODED * NUMBER_OF_CHAR] =
{ 
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

int load_rom(char *path)
{
   FILE *f = fopen(path, "r");
   fread(&(Memory[0x200]), MEMORY_SIZE - 0x200, 1, f);
   return 0;
}

int print_gfx()
{
   int i = 0;

   for (i = 0; i < WIDTH * HEIGHT; i++)
   {
      if (Gfx[i] == 1)
         mvprintw(i / WIDTH, i % WIDTH, "*"); 
   }
   refresh();

   return 0;
}

int print_debug()
{
   mvprintw(34, 0, "Opcode: 0x%04x", Opcode);
   mvprintw(35, 0, "I:      0x%04x", I);
   mvprintw(35, 0, "V[0]:   0x%02x", V[0]);
   mvprintw(36, 0, "V[1]:   0x%02x", V[1]);
   mvprintw(37, 0, "V[2]:   0x%02x", V[2]);
   mvprintw(38, 0, "V[3]:   0x%02x", V[3]);
   mvprintw(39, 0, "V[4]:   0x%02x", V[4]);
   mvprintw(40, 0, "V[5]:   0x%02x", V[5]);
   mvprintw(41, 0, "V[6]:   0x%02x", V[6]);
   mvprintw(42, 0, "V[7]:   0x%02x", V[7]);
   mvprintw(43, 0, "V[8]:   0x%02x", V[8]);
   mvprintw(44, 0, "V[9]:   0x%02x", V[9]);
   mvprintw(45, 0, "V[10]:  0x%02x", V[10]);
   mvprintw(46, 0, "V[11]:  0x%02x", V[11]);
   mvprintw(47, 0, "V[12]:  0x%02x", V[12]);
   mvprintw(48, 0, "V[13]:  0x%02x", V[13]);
   mvprintw(49, 0, "V[14]:  0x%02x", V[14]);
   mvprintw(50, 0, "V[15]:  0x%02x", V[15]);

   refresh();

   return 0;
}

#if 0
int main(int argc, char *argv[])
#else
int main()
#endif
{
   srand(0);
   Pc = 0x200;
   Opcode = 0;
   I = 0;
   Sp = 0;

   /* init ncurses */
   initscr();
   noecho();
   curs_set(FALSE);

   /* charge la font */
   memcpy(&Memory[FONT_ADDR], Chip8_fontset, CHAR_ENCODED * NUMBER_OF_CHAR * sizeof(uint8_t));
   /* charge la rom */
   load_rom("./rom/PONG");

   for (;;)
      mainloop();

   endwin();
   return 0;
}

int mainloop()
{
   fetch_opcode();
   decode_opcode();
   print_gfx();

#ifdef DEBUG
   print_debug();
#endif

#ifdef SLEEP 
    sleep(1);
#endif
   return 0;
}

int fetch_opcode()
{
   /* fetch opcode */
   /* --- Example -- */
   /* Memory = ... 0xA2 0xF0 ...*/
   /* Memory     = 0xA2 */
   /* Memory + 1 = 0xF0 */
   /* opcode doit être = 0xA2F0 */
   Opcode = (uint16_t)(Memory[Pc] << 8 | Memory[Pc + 1]);

   return 0;
}

int decode_opcode()
{
   uint16_t first_3_bytes = Opcode & 0xFFF;
   switch ((Opcode & 0xF000) >> 12)
   {
      case 0x0:
      {
         if (first_3_bytes == 0x0E0)
         {
            opcode_00E0();
         }
         else if (first_3_bytes == 0x0EE)
         {
            opcode_00EE();
         }
         else
         {
            opcode_0NNN();
         }
      }
      break;
      case 0x1:
      {
         opcode_1NNN();
      }
      break;
      case 0x2:
      {
         opcode_2NNN();
      }
      break;
      case 0x3:
      {
         opcode_3XNN();
      }
      break;
      case 0x4:
      {
         opcode_4XNN();
      }
      break;
      case 0x5:
      {
         opcode_5XY0();
      }
      break;
      case 0x6:
      {
         opcode_6XNN();
      }
      break;
      case 0x7:
      {
         opcode_7XNN();
      }
      break;
      case 0x8:
      {
         if ((first_3_bytes & 0x00F) == 0x0)
            opcode_8XY0();
         else if ((first_3_bytes & 0x00F) == 0x1)
            opcode_8XY1();
         else if ((first_3_bytes & 0x00F) == 0x2)
            opcode_8XY2();
         else if ((first_3_bytes & 0x00F) == 0x3)
            opcode_8XY3();
         else if ((first_3_bytes & 0x00F) == 0x5)
            opcode_8XY4();
         else if ((first_3_bytes & 0x00F) == 0x5)
            opcode_8XY5();
         else if ((first_3_bytes & 0x00F) == 0x6)
            opcode_8XY6();
         else if ((first_3_bytes & 0x00F) == 0x7)
            opcode_8XY7();
         else if ((first_3_bytes & 0x00F) == 0xE)
            opcode_8XYE();
         else
            printf("erreur) opcode inconnu, %x\n", Opcode);
      }
      break;
      case 0x9:
      {
         opcode_9XY0();
      }
      break;
      case 0xA:
      {
         opcode_ANNN();
      }
      break;
      case 0xB:
      {
         opcode_BNNN();
      }
      break;
      case 0xC:
      {
         opcode_CXNN();
      }
      break;
      case 0xD:
      {
         opcode_DXYN();
      }
      break;
      case 0xE:
      {
         if ((first_3_bytes & 0x0FF) == 0x9E)
            opcode_EX9E();
         else if ((first_3_bytes & 0x0FF) == 0xA1)
            opcode_EXA1();
         else
            printf("erreur) opcode inconnu, %x\n", Opcode);
      }
      break;
      case 0xF:
      {
         if ((first_3_bytes & 0x0FF) == 0x07)
            opcode_FX07();
         else if ((first_3_bytes & 0x0FF) == 0x0A)
            opcode_FX0A();
         else if ((first_3_bytes & 0x0FF) == 0x15)
            opcode_FX15();
         else if ((first_3_bytes & 0x0FF) == 0x18)
            opcode_FX18();
         else if ((first_3_bytes & 0x0FF) == 0x1E)
            opcode_FX1E();
         else if ((first_3_bytes & 0x0FF) == 0x29)
            opcode_FX29();
         else if ((first_3_bytes & 0x0FF) == 0x33)
            opcode_FX33();
         else if ((first_3_bytes & 0x0FF) == 0x55)
            opcode_FX55();
         else if ((first_3_bytes & 0x0FF) == 0x65)
            opcode_FX65();
         else
            printf("erreur) opcode inconnu, %x\n", Opcode);
      }
      break;
      default:
      {
         printf("impossible d'arriver ici normalement\n");
      }
      break;
   }

   return 0;
}

/*
 * Calls RCA 1802 program at address NNN. Not necessary for most ROMs.
 */
int opcode_0NNN()
{
   printf("usesless no ? :'(\n");
   Pc = (uint16_t)(Pc + 2);
   return 0;
}
/*
 * Clears the screen.
 */
int opcode_00E0()
{
   memset(Gfx, 0, sizeof(uint8_t) * WIDTH * HEIGHT);
   Pc = (uint16_t)(Pc + 2);
   return 0;
}
/*
 * Returns from a subroutine.
 */
int opcode_00EE()
{
   if (Sp > 0)
   {
      Pc = Stack[Sp - 1];
      Sp = (uint16_t)(Sp - 1);
   }
   return 0;
}
/*
 * Jumps to address NNN.
 */
int opcode_1NNN()
{
   Pc = Opcode & 0x0FFF;
   return 0;
}
/*
 * Calls subroutine at NNN.
 */
int opcode_2NNN()
{
   Stack[Sp] = Pc;
   Sp = (uint16_t)(Sp + 1);
   Pc = Opcode & 0x0FFF;
   return 0;
}
/*
 * Skips the next instruction if VX equals NN.
 */
int opcode_3XNN()
{
   if (V[(Opcode & 0x0F00) >> 8] == (Opcode & 0x00FF))
      Pc = (uint16_t)(Pc + 4);
   Pc = (uint16_t)(Pc + 2);
   return 0;
}
/*
 * Skips the next instruction if VX doesn't equal NN.
 */
int opcode_4XNN()
{
   if (V[(Opcode & 0x0F00) >> 8] == (Opcode & 0x00FF))
      Pc = (uint16_t)(Pc + 4);
   Pc = (uint16_t)(Pc + 2);
   return 0;
}
/*
 * Skips the next instruction if VX equals VY.
 */
int opcode_5XY0()
{
   if (V[(Opcode & 0x0F00) >> 8] == V[(Opcode & 0x00F0) >> 4])
      Pc = (uint16_t)(Pc + 4);
   Pc = (uint16_t)(Pc + 2);
   return 0;
}
/*
 * Sets VX to NN.
 */
int opcode_6XNN()
{
   V[(Opcode & 0x0F00) >> 8] = (uint8_t)(Opcode & 0x00FF);
   Pc = (uint16_t)(Pc + 2);
   return 0;
}
/*
 * Adds NN to VX.
 */
int opcode_7XNN()
{
   V[(Opcode & 0x0F00) >> 8] = (uint8_t)(V[(Opcode & 0x0F00) >> 8] + (Opcode & 0x00FF));
   Pc = (uint16_t)(Pc + 2);
   return 0;
}
/*
 * Sets VX to the value of VY.
 */
int opcode_8XY0()
{
   V[(Opcode & 0x0F00) >> 8] = V[(Opcode & 0x00F0) >> 4];
   Pc = (uint16_t)(Pc + 2);
   return 0;
}
/*
 * Sets VX to VX or VY.
 */
int opcode_8XY1()
{
   V[(Opcode & 0x0F00) >> 8] |= V[(Opcode & 0x00F0) >> 4];
   Pc = (uint16_t)(Pc + 2);
   return 0;
}
/*
 * Sets VX to VX and VY.
 */
int opcode_8XY2()
{
   V[(Opcode & 0x0F00) >> 8] &= V[(Opcode & 0x00F0) >> 4];
   Pc = (uint16_t)(Pc + 2);
   return 0;
}
/*
 * Sets VX to VX xor VY.
 */
int opcode_8XY3()
{
   V[(Opcode & 0x0F00) >> 8] ^= V[(Opcode & 0x00F0) >> 4];
   Pc = (uint16_t)(Pc + 2);
   return 0;
}
/*
 * Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there
 * isn't.
 */
int opcode_8XY4()
{
   V[(Opcode & 0x0F00) >> 8] = (uint8_t)(V[(Opcode & 0x0F00) >> 8] + V[(Opcode & 0x00F0) >> 4]);
   if (V[(Opcode & 0x0F00) >> 8] + V[(Opcode & 0x00F0) >> 4] > 0xFF)
      V[0xF] = 1;
   else
      V[0xF] = 0;

   Pc = (uint16_t)(Pc + 2);
   return 0;
}
/*
 * VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when
 * there isn't.
 */
int opcode_8XY5()
{
   V[(Opcode & 0x0F00) >> 8] = (uint8_t)(V[(Opcode & 0x0F00) >> 8] - V[(Opcode & 0x00F0) >> 4]);
   if (V[(Opcode & 0x0F00) >> 8] - V[(Opcode & 0x00F0) >> 4] < 0)
      V[0xF] = 1;
   else
      V[0xF] = 0;
   Pc = (uint16_t)(Pc + 2);
   return 0;
}
/*
 * Shifts VX right by one. VF is set to the value of the least significant bit
 * of VX before the shift.[2]
 */
int opcode_8XY6()
{
   uint8_t tmp;
   tmp = V[(Opcode & 0x0F00) >> 8];
   V[(Opcode & 0x0F00) >> 8] = (uint16_t)(tmp >> 1);
   Pc = (uint16_t)(Pc + 2);

   return 0;
}
/*
 * Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when
 * there isn't.
 */
int opcode_8XY7()
{
   V[(Opcode & 0x0F00) >> 8] = (uint8_t)(V[(Opcode & 0x00F0) >> 4] - V[(Opcode & 0x0F00) >> 8]);
   Pc = (uint16_t)(Pc + 2);
   return 0;
}
/*
 * Shifts VX left by one. VF is set to the value of the most significant bit of
 * VX before the shift
 */
int opcode_8XYE()
{
   uint8_t tmp;
   tmp = V[(Opcode & 0x0F00) >> 8];
   V[(Opcode & 0x0F00) >> 8] = (uint8_t)(tmp << 1);
   return 0;
}
/*
 * Skips the next instruction if VX doesn't equal VY.
 */
int opcode_9XY0()
{
   if (V[(Opcode & 0x0F00) >> 8] == V[(Opcode & 0x00F0) >> 4])
      Pc = (uint16_t)(Pc + 4);
   Pc = (uint16_t)(Pc + 2);
   return 0;
}
/*
 * Sets I to the address NNN.
 */
int opcode_ANNN()
{
   I = (uint16_t)(Opcode & 0x0FFF);
   Pc = (uint16_t)(Pc + 2);
   return 0;
}
/*
 * Jumps to the address NNN plus V0.
 */
int opcode_BNNN()
{
   Pc = (uint16_t)((Opcode & 0x0FFF) + V[0]);
   Pc = (uint16_t)(Pc + 2);
   return 0;
}
/*
 * Sets VX to the result of a bitwise and operation on a random number and NN.
 */
int opcode_CXNN()
{
   srand((unsigned int)(rand()));
   V[(Opcode & 0x0F00) >> 8] = (uint8_t)(rand() % 0xFF) & V[(Opcode & 0x00FF)];
   Pc = (uint16_t)(Pc + 2);
   return 0;
}
/*
 * Sprites stored in memory at location in index register (I), 8bits wide. Wraps
 * around the screen. If when drawn, clears a pixel, register VF is set to 1
 * otherwise it is zero. All drawing is XOR drawing (i.e. it toggles the screen
 * pixels). Sprites are drawn starting at position VX, VY. N is the number of
 * 8bit rows that need to be drawn. If N is greater than 1, second line
 * continues at position VX, VY+1, and so on.
 */
int opcode_DXYN()
{
   uint8_t i_row, j_col, x, y, nb_of_row = 0;
   uint8_t pix = 0;

   x = V[(Opcode & 0x0F00) >> 8];
   y = V[(Opcode & 0x00F0) >> 4];
   nb_of_row = (Opcode & 0x000F);

   V[0xF] = 0;

   for (i_row = 0; i_row < nb_of_row; i_row++)
   {
      for (j_col = 0; j_col < 8; j_col++)
      {
         pix = (Memory[I + i_row] >> (7 - j_col)) & 0x01;
         Gfx[(WIDTH * (y + i_row)) + (x + j_col)] ^= pix;
         if (Gfx[(WIDTH * (y + i_row)) + (x + j_col)] != pix)
            V[0xF] = 1;
      }
   }
   Pc = (uint16_t)(Pc + 2);

   return 0;
}
/*
 * Skips the next instruction if the key stored in VX is pressed.
 */
int opcode_EX9E()
{
   if (V[(Opcode & 0x0F00) >> 8] == Key)
      Pc = (uint16_t)(Pc + 4);
   Pc = (uint16_t)(Pc + 2);

   return 0;
}
/*
 * Skips the next instruction if the key stored in VX isn't pressed.
 */
int opcode_EXA1()
{
   if (V[(Opcode & 0x0F00) >> 8] != Key)
      Pc = (uint16_t)(Pc + 4);
   Pc = (uint16_t)(Pc + 2);
   return 0;
}
/*
 * Sets VX to the value of the delay timer.
 */
int opcode_FX07()
{
   V[(Opcode & 0x0F00) >> 8] = Dt;
   Pc = (uint16_t)(Pc + 2);
   return 0;
}
/*
 * A key press is awaited, and then stored in VX.
 */
int opcode_FX0A()
{
   Key = V[(Opcode & 0x0F00) >> 8];
   Pc = (uint16_t)(Pc + 2);
   return 0;
}
/*
 * Sets the delay timer to VX.
 */
int opcode_FX15()
{
   Dt = V[(Opcode & 0x0F00) >> 8];
   Pc = (uint16_t)(Pc + 2);
   return 0;
}
/*
 * Sets the sound timer to VX.
 */
int opcode_FX18()
{
   St = V[(Opcode & 0x0F00) >> 8];
   Pc = (uint16_t)(Pc + 2);
   return 0;
}
/*
 * Adds VX to I
 */
int opcode_FX1E()
{
   I = (uint16_t)(I + V[(Opcode & 0x0F00) >> 8]);
   Pc = (uint16_t)(Pc + 2);
   return 0;
}
/*
 * Sets I to the location of the sprite for the character in VX. Characters 0-F
 * (in hexadecimal) are represented by a 4x5 font.
 */
int opcode_FX29()
{
   I = FONT_ADDR;
   Pc = (uint16_t)(Pc + 2);
   return 0;
}
/*
 * Stores the binary-coded decimal representation of VX, with the most
 * significant of three digits at the address in I, the middle digit at I plus
 * 1, and the least significant digit at I plus 2. (In other words, take the
 * decimal representation of VX, place the hundreds digit in memory at location
 * in I, the tens digit at location I+1, and the ones digit at location I+2.)
 */
int opcode_FX33()
{
   Memory[I] = V[(Opcode & 0x0F00) >> 8] / 100;
   Memory[I + 1] = (V[(Opcode & 0x0F00) >> 8] / 10) % 10;
   Memory[I + 2] = (V[(Opcode & 0x0F00) >> 8] / 100) % 10;
   Pc = (uint16_t)(Pc + 2);
   return 0;
}
/*
 * Stores V0 to VX (including VX) in memory starting at address I
 */
int opcode_FX55()
{
   uint8_t i = 0;
   for (i = 0; i <= ((Opcode & 0x0F00) >> 8); i++)
      Memory[I + i] = V[i];
   Pc = (uint16_t)(Pc + 2);
   return 0;
}
/*
 * Fills V0 to VX (including VX) with values from memory starting at address I.
 */
int opcode_FX65()
{
   uint8_t i = 0;

   for (i = 0; i <= ((Opcode & 0x0F00) >> 8); i++)
      Memory[I + i] = V[i];
   Pc = (uint16_t)(Pc + 2);
   return 0;
}
