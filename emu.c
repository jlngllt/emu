#include "emu.h"

#ifdef DEBUG
   #include "emu_debug.h"
   struct debug_state DebugStat;
#endif

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

void emu_load_rom(char *path)
{
   FILE *f = fopen(path, "r");

   if (!f) {
      perror("fopen");
      exit(EXIT_FAILURE);
   }

   fread(&(Memory[0x200]), MEMORY_SIZE - 0x200, 1, f);
   fseek(f, 0L, SEEK_END);
   Rom_size = ftell(f);
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

double emu_d_compute_freq(struct timespec t)
{
   double freq;
   uint32_t wait_nsec;

   wait_nsec = emu_timespec2nsec(t);
   freq = 1. / (double)wait_nsec * (double)1000000000L;
   return freq;
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
   emu_print_gfx(0, 0);
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

   x = V[GET_0100(opcode)];
   y = V[GET_0010(opcode)];
   nb_of_row = GET_0001(opcode);
   V[0xF] = 0;

   for (i_row = 0; i_row < nb_of_row; i_row++) {
      for (j_col = 0; j_col < 8; j_col++) {
         pix = (Memory[I + i_row] >> (7 - j_col)) & 0x01;
         if (Gfx[(WIDTH * (y + i_row)) + (x + j_col)] != pix) {
            V[0xF] = 1;
         }
         Gfx[(WIDTH * (y + i_row)) + (x + j_col)] ^= pix;
      }
   }
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
uint16_t emu_opcode_EXA1(uint16_t opcode, uint16_t pc)
{
   if (V[GET_0100(opcode)] != Key)
      pc = (uint16_t)(pc + 2);
   pc = (uint16_t)(pc + 2);
   return pc;
}

/*
 * Sets VX to the value of the delay timer.
 */
uint16_t emu_opcode_FX07(uint16_t opcode, uint16_t pc)
{
   V[GET_0100(opcode)] = Dt;
   pc = (uint16_t)(pc + 2);
   return pc;
}

/*
 * A key press is awaited, and then stored in VX.
 */
uint16_t emu_opcode_FX0A(uint16_t opcode, uint16_t pc)
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

uint16_t emu_decode_opcode(uint16_t opcode, uint16_t pc)
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
            printf("opcode 0x04%x unkonw\n", opcode);
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
            printf("opcode 0x04%x unkonw\n", opcode);
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
            printf("opcode 0x04%x unkonw\n", opcode);
      break;

      default:
         printf("opcode 0x04%x unkonw\n", opcode);
      break;
   }
   return pc;
}

uint16_t emu_mainloop(uint16_t pc)
{
   struct timespec t1, t2;
   uint16_t opcode;

   t1 = emu_gettime();

   emu_process_input();
   opcode = emu_fetch_opcode(pc);
   pc = emu_decode_opcode(opcode, pc);
   if (Dt > 0) Dt--;

   t2 = emu_gettime();
   emu_nanosleep(FREQ_SYNC, emu_difftimespec(t1, t2));

#ifdef DEBUG
   DebugStat.frequence = emu_d_compute_freq(emu_difftimespec(t1, emu_gettime()));
   DebugStat.opcode = opcode;
   DebugStat.pc = pc;
   emu_debug_draw(&DebugStat);
#endif

   return pc;
}

int32_t main(int32_t argc, char *argv[])
{
   uint16_t pc = 0x200;
   /* rom par default */
   char *rom = DEFAULT_ROM;

   I  = 0;
   Sp = 0;

   if (argc > 1)
   {
      rom = argv[1];
   }

   /* init random */
   srand((unsigned int)(time(NULL)));
   /* charge la font */
   memcpy(&Memory[FONT_ADDR],
         Chip8_fontset,
         CHAR_ENCODED * NUMBER_OF_CHAR * sizeof(uint8_t));

   /* charge la rom */
   emu_load_rom(rom);
   emu_init_gfx();

   while (1) {
      pc = emu_mainloop(pc);
   }

   emu_free_gfx();
   return 0;
}
