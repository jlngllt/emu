#include "emu.h"

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

#ifdef DEBUG
   #include "emu_debug.h"
   struct debug_emu DebugEmu;
#endif


void emu_load_rom(st_emu *emu, char *path)
{
   FILE *f = fopen(path, "r");

   if (!f) {
      perror("fopen");
      exit(EXIT_FAILURE);
   }

   fread(&(emu->memory[0x200]), MEMORY_SIZE - 0x200, 1, f);
   fseek(f, 0L, SEEK_END);
   emu->rom_size = (int32_t)ftell(f);
}

struct timespec emu_gettime()
{
   struct timespec t;

#ifdef __MACH__ /* OS X does not have clock_gettime, use clock_get_time */
   clock_serv_t cclock;
   mach_timespec_t mts;
   host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
   clock_get_time(cclock, &mts);
   mach_port_deallocate(mach_task_self(), cclock);
   t.tv_sec = mts.tv_sec;
   t.tv_nsec = mts.tv_nsec;

#else
   clock_gettime(CLOCK_REALTIME, &t);
#endif

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
   t_period.tv_nsec = (long)(period * (double)1000000000L);
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
   return (uint16_t)(pc + 2);
}

/*
 * Clears the screen.
 */
uint16_t emu_opcode_00E0(uint8_t *gfx, uint16_t pc)
{
   memset(gfx, 0, sizeof(uint8_t) * WIDTH * HEIGHT);
   return (uint16_t)(pc + 2);
}

/*
 * Returns from a subroutine.
 */
uint16_t emu_opcode_00EE(struct st_stack *s, uint16_t pc)
{
   uint16_t tmp_pc = pc;
   if (s->p > 0) {
      tmp_pc = s->addr[s->p - 1];
      s->p = (uint16_t)(s->p- 1);
   }
   return tmp_pc;
}

/*
 * Jumps to address NNN.
 */
uint16_t emu_opcode_1NNN(uint16_t opcode)
{
   return GET_0111(opcode);
}

/*
 * Calls subroutine at NNN.
 */
uint16_t emu_opcode_2NNN(struct st_stack *s, uint16_t opcode, uint16_t pc)
{
   s->addr[s->p] = (uint16_t)(pc + 2);
   s->p = (uint16_t)(s->p + 1);
   return GET_0111(opcode);
}

/*
 * Skips the next instruction if VX equals NN.
 */
uint16_t emu_opcode_3XNN(const uint8_t *v, uint16_t opcode, uint16_t pc)
{
   if (v[GET_0100(opcode)] == GET_0011(opcode))
      return (uint16_t)(pc + 4);
   return (uint16_t)(pc + 2);
}

/*
 * Skips the next instruction if VX doesn't equal NN.
 */
uint16_t emu_opcode_4XNN(const uint8_t *v, uint16_t opcode, uint16_t pc)
{
   if (v[GET_0100(opcode)] != GET_0011(opcode))
      return (uint16_t)(pc + 4);
   return (uint16_t)(pc + 2);
}

/*
 * Skips the next instruction if VX equals VY.
 */
uint16_t emu_opcode_5XY0(const uint8_t *v, uint16_t opcode, uint16_t pc)
{
   if (v[GET_0100(opcode)] == v[GET_0010(opcode)])
      return (uint16_t)(pc + 4);
   return (uint16_t)(pc + 2);
}

/*
 * Sets VX to NN.
 */
uint16_t emu_opcode_6XNN(uint8_t *v, uint16_t opcode, uint16_t pc)
{
   v[GET_0100(opcode)] = (uint8_t)GET_0011(opcode);
   return (uint16_t)(pc + 2);
}

/*
 * Adds NN to VX.
 */
uint16_t emu_opcode_7XNN(uint8_t *v, uint16_t opcode, uint16_t pc)
{
   v[GET_0100(opcode)] = (uint8_t)(v[GET_0100(opcode)] + GET_0011(opcode));
   return (uint16_t)(pc + 2);
}

/*
 * Sets VX to the value of VY.
 */
uint16_t emu_opcode_8XY0(uint8_t *v, uint16_t opcode, uint16_t pc)
{
   v[GET_0100(opcode)] = v[GET_0010(opcode)];
   return (uint16_t)(pc + 2);
}

/*
 * Sets VX to VX or VY.
 */
uint16_t emu_opcode_8XY1(uint8_t *v, uint16_t opcode, uint16_t pc)
{
   v[GET_0100(opcode)] |= v[GET_0010(opcode)];
   return (uint16_t)(pc + 2);
}

/*
 * Sets VX to VX and VY.
 */
uint16_t emu_opcode_8XY2(uint8_t *v, uint16_t opcode, uint16_t pc)
{
   uint16_t tmp;
   v[GET_0100(opcode)] &= v[GET_0010(opcode)];
   tmp = (uint16_t)(pc + 2);
   return tmp;
}

/*
 * Sets VX to VX xor VY.
 */
uint16_t emu_opcode_8XY3(uint8_t *v, uint16_t opcode, uint16_t pc)
{
   v[GET_0100(opcode)] ^= v[GET_0010(opcode)];
   return (uint16_t)(pc + 2);
}

/*
 * Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there
 * isn't.
 */
uint16_t emu_opcode_8XY4(uint8_t *v, uint16_t opcode, uint16_t pc)
{
   if (v[GET_0100(opcode)] + v[GET_0010(opcode)] > 0xFF)
      v[0xF] = 1;
   else
      v[0xF] = 0;
   v[GET_0100(opcode)] = (uint8_t)(v[GET_0100(opcode)] + v[GET_0010(opcode)]);
   return (uint16_t)(pc + 2);
}

/*
 * VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when
 * there isn't.
 */
uint16_t emu_opcode_8XY5(uint8_t *v, uint16_t opcode, uint16_t pc)
{
   if (v[GET_0100(opcode)] > v[GET_0010(opcode)])
      v[0xF] = 1;
   else
      v[0xF] = 0;
   v[GET_0100(opcode)] = (uint8_t)(v[GET_0100(opcode)] - v[GET_0010(opcode)]);
   return (uint16_t)(pc + 2);
}

/*
 * Shifts VX right by one. VF is set to the value of the least significant bit
 * of VX before the shift.[2]
 */
uint16_t emu_opcode_8XY6(uint8_t *v, uint16_t opcode, uint16_t pc)
{
   uint8_t tmp;

   tmp = v[GET_0100(opcode)];
   v[0xf] = (v[GET_0100(opcode)] & 0x01);
   v[GET_0100(opcode)] = (uint8_t)(tmp >> 1);
   return (uint16_t)(pc + 2);
}

/*
 * Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when
 * there isn't.
 */
uint16_t emu_opcode_8XY7(uint8_t *v, uint16_t opcode, uint16_t pc)
{
   if (v[GET_0010(opcode)] >= v[GET_0100(opcode)])
      v[0xF] = 1;
   else
      v[0xF] = 0;
   v[GET_0100(opcode)] = (uint8_t)(v[GET_0010(opcode)] - v[GET_0100(opcode)]);
   return (uint16_t)(pc + 2);
}

/*
 * Shifts VX left by one. VF is set to the value of the most significant bit of
 * VX before the shift
 */
uint16_t emu_opcode_8XYE(uint8_t *v, uint16_t opcode, uint16_t pc)
{
   uint8_t tmp;

   tmp = v[GET_0100(opcode)];
   v[0xf] = v[GET_0100(opcode)] & 0x80 >> 7;
   v[GET_0100(opcode)] = (uint8_t)(tmp << 1);
   return (uint16_t)(pc + 2);
}

/*
 * Skips the next instruction if VX doesn't equal VY.
 */
uint16_t emu_opcode_9XY0(const uint8_t *v, uint16_t opcode, uint16_t pc)
{
   if (v[GET_0100(opcode)] != v[GET_0010(opcode)])
      return (uint16_t)(pc + 4);
   return (uint16_t)(pc + 2);
}

/*
 * Sets I to the address NNN.
 */
uint16_t emu_opcode_ANNN(uint16_t *i, uint16_t opcode, uint16_t pc)
{
   *i = (uint16_t)GET_0111(opcode);
   return (uint16_t)(pc + 2);
}

/*
 * Jumps to the address NNN plus V0.
 */
uint16_t emu_opcode_BNNN(const uint8_t *v, uint16_t opcode)
{
   return (uint16_t)(GET_0111(opcode) + v[0]);
}

/*
 * Sets VX to the result of a bitwise and operation on a random number and NN.
 */
uint16_t emu_opcode_CXNN(uint8_t *v, uint16_t opcode, uint16_t pc)
{
   int random = rand();

   srand((unsigned int)(random));
   v[GET_0100(opcode)] = (uint8_t)((random % 0xFF) & GET_0011(opcode));
   return (uint16_t)(pc + 2);
}

/*
 * Sprites stored in memory at location in index register (I), 8bits wide. Wraps
 * around the screen. If when drawn, clears a pixel, register VF is set to 1
 * otherwise it is zero. All drawing is XOR drawing (i.e. it toggles the screen
 * pixels). Sprites are drawn starting at position VX, VY. N is the number of
 * 8bit rows that need to be drawn. If N is greater than 1, second line
 * continues at position VX, VY+1, and so on.
 */
uint16_t emu_opcode_DXYN(const uint8_t *mem, uint8_t *gfx, uint8_t *v, uint16_t i, uint16_t opcode, uint16_t pc)
{
   uint8_t i_row, j_col, x, y, nb_of_row = 0;
   uint8_t pix = 0;

   x = v[GET_0100(opcode)];
   y = v[GET_0010(opcode)];
   nb_of_row = GET_0001(opcode);
   v[0xF] = 0;

   for (i_row = 0; i_row < nb_of_row; i_row++) {
      for (j_col = 0; j_col < 8; j_col++) {
         pix = (mem[i + i_row] >> (7 - j_col)) & 0x01;
         if (gfx[(WIDTH * (y + i_row)) + (x + j_col)] != pix) {
            v[0xF] = 1;
         }
         gfx[(WIDTH * (y + i_row)) + (x + j_col)] ^= pix;
      }
   }
   return (uint16_t)(pc + 2);
}

/*
 * Skips the next instruction if the key stored in VX is pressed.
 */
uint16_t emu_opcode_EX9E(const uint8_t *v, uint16_t k, uint16_t opcode, uint16_t pc)
{
   if (v[GET_0100(opcode)] == k)
      return (uint16_t)(pc + 4);
   return (uint16_t)(pc + 2);
}

/*
 * Skips the next instruction if the key stored in VX isn't pressed.
 */
uint16_t emu_opcode_EXA1(const uint8_t *v, uint16_t k, uint16_t opcode, uint16_t pc)
{
   if (v[GET_0100(opcode)] != k)
      return (uint16_t)(pc + 4);
   return (uint16_t)(pc + 2);
}

/*
 * Sets VX to the value of the delay timer.
 */
uint16_t emu_opcode_FX07(uint8_t *v, uint8_t dt, uint16_t opcode, uint16_t pc)
{
   v[GET_0100(opcode)] = dt;
   return (uint16_t)(pc + 2);
}

/*
 * A key press is awaited, and then stored in VX.
 */
uint16_t emu_opcode_FX0A(uint16_t *k, const uint8_t *v, uint16_t opcode, uint16_t pc)
{
   *k = v[GET_0100(opcode)];
   return (uint16_t)(pc + 2);
}

/*
 * Sets the delay timer to VX.
 */
uint16_t emu_opcode_FX15(uint8_t *dt, const uint8_t *v, uint16_t opcode, uint16_t pc)
{
   *dt = v[GET_0100(opcode)];
   return (uint16_t)(pc + 2);
}

/*
 * Sets the sound timer to VX.
 */
uint16_t emu_opcode_FX18(uint8_t *st, const uint8_t *v, uint16_t opcode, uint16_t pc)
{
   *st = v[GET_0100(opcode)];
   return (uint16_t)(pc + 2);
}

/*
 * Adds VX to I
 */
uint16_t emu_opcode_FX1E(uint16_t *i, const uint8_t *v, uint16_t opcode, uint16_t pc)
{
   *i  = (uint16_t)(*i + v[GET_0100(opcode)]);
   return (uint16_t)(pc + 2);
}

/*
 * Sets I to the location of the sprite for the character in VX. Characters 0-F
 * (in hexadecimal) are represented by a 4x5 font.
 */
uint16_t emu_opcode_FX29(uint16_t *i, uint16_t pc)
{
   *i = FONT_ADDR;
   return (uint16_t)(pc + 2);
}

/*
 * Stores the binary-coded decimal representation of VX, with the most
 * significant of three digits at the address in I, the middle digit at I plus
 * 1, and the least significant digit at I plus 2. (In other words, take the
 * decimal representation of VX, place the hundreds digit in memory at location
 * in I, the tens digit at location I+1, and the ones digit at location I+2.)
 */
uint16_t emu_opcode_FX33(uint8_t *mem, uint8_t *v, uint16_t i, uint16_t opcode, uint16_t pc)
{
   mem[i] = v[GET_0100(opcode)] / 100;
   mem[i + 1] = (v[GET_0100(opcode)] / 10) % 10;
   mem[i + 2] = (v[GET_0100(opcode)] / 100) % 10;
   return (uint16_t)(pc + 2);
}

/*
 * Stores V0 to VX (including VX) in memory starting at address I
 */

uint16_t emu_opcode_FX55(uint8_t *mem, uint8_t *v, uint16_t i, uint16_t opcode, uint16_t pc)
{
   uint8_t j = 0;
   for (j = 0; j <= GET_0100(opcode); j++)
      mem[i + j] = v[j];
   return (uint16_t)(pc + 2);
}

/*
 * Fills V0 to VX (including VX) with values from memory starting at address I.
 */
uint16_t emu_opcode_FX65(uint8_t *mem, uint8_t *v, uint16_t i, uint16_t opcode, uint16_t pc)
{
   uint8_t j = 0;

   for (j = 0; j <= GET_0100(opcode); j++)
      mem[i + j] = v[j];
   return (uint16_t)(pc + 2);
}

uint16_t emu_fetch_opcode(uint8_t *mem, uint16_t pc)
{
   /* fetch opcode */
   /* --- Example -- */
   /* Memory = ... 0xA2 0xF0 ...*/
   /* Memory     = 0xA2 */
   /* Memory + 1 = 0xF0 */
   /* opcode doit être = 0xA2F0 */
   return (uint16_t)(mem[pc] << 8 | mem[pc + 1]);
}

void emu_decode_opcode(st_emu *emu)
{
   uint16_t *p_pc = &(emu->pc);
   uint16_t opcode = emu->opcode;
   uint16_t pc = emu->pc;

   switch (GET_1000(opcode)) {
      case 0x0:
         if (GET_0011(opcode) == 0xE0)
            *p_pc = emu_opcode_00E0(emu->gfx, pc);
         else if (GET_0011(opcode) == 0xEE)
            *p_pc = emu_opcode_00EE(&emu->stack, pc);
         else
            *p_pc = emu_opcode_0NNN(pc);
      break;

      case 0x1:
         *p_pc = emu_opcode_1NNN(opcode);
      break;

      case 0x2:
         *p_pc = emu_opcode_2NNN(&emu->stack, opcode, pc);
      break;

      case 0x3:
         *p_pc = emu_opcode_3XNN(emu->v, opcode, pc);
      break;

      case 0x4:
         *p_pc = emu_opcode_4XNN(emu->v, opcode, pc);
      break;

      case 0x5:
         *p_pc = emu_opcode_5XY0(emu->v, opcode, pc);
      break;

      case 0x6:
         *p_pc = emu_opcode_6XNN(emu->v, opcode, pc);
      break;

      case 0x7:
         *p_pc = emu_opcode_7XNN(emu->v, opcode, pc);
      break;

      case 0x8:
         if ((GET_0001(opcode)) == 0x0)
            *p_pc = emu_opcode_8XY0(emu->v, opcode, pc);
         else if (GET_0001(opcode) == 0x1)
            *p_pc = emu_opcode_8XY1(emu->v, opcode, pc);
         else if (GET_0001(opcode) == 0x2)
            *p_pc = emu_opcode_8XY2(emu->v, opcode, pc);
         else if (GET_0001(opcode) == 0x3)
            *p_pc = emu_opcode_8XY3(emu->v, opcode, pc);
         else if (GET_0001(opcode) == 0x4)
            *p_pc = emu_opcode_8XY4(emu->v, opcode, pc);
         else if (GET_0001(opcode) == 0x5)
            *p_pc = emu_opcode_8XY5(emu->v, opcode, pc);
         else if (GET_0001(opcode) == 0x6)
            *p_pc = emu_opcode_8XY6(emu->v, opcode, pc);
         else if (GET_0001(opcode) == 0x7)
            *p_pc = emu_opcode_8XY7(emu->v, opcode, pc);
         else if (GET_0001(opcode) == 0xE)
            *p_pc = emu_opcode_8XYE(emu->v, opcode, pc);
         else
            printf("opcode 0x04%x unkonw\n", opcode);
      break;

      case 0x9:
         *p_pc = emu_opcode_9XY0(emu->v, opcode, pc);
      break;

      case 0xA:
         *p_pc = emu_opcode_ANNN(&emu->i, opcode, pc);
      break;

      case 0xB:
         *p_pc = emu_opcode_BNNN(emu->v, opcode);
      break;

      case 0xC:
         *p_pc = emu_opcode_CXNN(emu->v, opcode, pc);
      break;

      case 0xD:
         *p_pc = emu_opcode_DXYN(emu->memory, emu->gfx, emu->v, emu->i, opcode, pc);
      break;

      case 0xE:
         if (GET_0011(opcode) == 0x9E)
            *p_pc = emu_opcode_EX9E(emu->v, emu->key, opcode, pc);
         else if (GET_0011(opcode) == 0xA1)
            *p_pc = emu_opcode_EXA1(emu->v, emu->key, opcode, pc);
         else
            printf("opcode 0x04%x unkonw\n", opcode);
      break;

      case 0xF:
         if (GET_0011(opcode) == 0x07)
            *p_pc = emu_opcode_FX07(emu->v, emu->dt, opcode, pc);
         else if (GET_0011(opcode) == 0x0A)
            *p_pc = emu_opcode_FX0A(&emu->key, emu->v, opcode, pc);
         else if (GET_0011(opcode) == 0x15)
            *p_pc = emu_opcode_FX15(&emu->dt, emu->v, opcode, pc);
         else if (GET_0011(opcode) == 0x18)
            *p_pc = emu_opcode_FX18(&emu->st, emu->v, opcode, pc);
         else if (GET_0011(opcode) == 0x1E)
            *p_pc = emu_opcode_FX1E(&emu->i, emu->v, opcode, pc);
         else if (GET_0011(opcode) == 0x29)
            *p_pc = emu_opcode_FX29(&emu->i, pc);
         else if (GET_0011(opcode) == 0x33)
            *p_pc = emu_opcode_FX33(emu->memory, emu->v, emu->i, opcode, pc);
         else if (GET_0011(opcode) == 0x55)
            *p_pc = emu_opcode_FX55(emu->memory, emu->v, emu->i, opcode, pc);
         else if (GET_0011(opcode) == 0x65)
            *p_pc = emu_opcode_FX65(emu->memory, emu->v, emu->i, opcode, pc);
         else
            printf("opcode 0x04%x unkonw\n", opcode);
      break;

      default:
         printf("opcode 0x04%x unkonw\n", opcode);
      break;
   }
}

void emu_mainloop(st_emu *emu)
{
   struct timespec t1, t2;

   t1 = emu_gettime();

   emu_process_input(&emu->key);
   emu->opcode = emu_fetch_opcode(emu->memory, emu->pc);
   emu_decode_opcode(emu);
   if (emu->dt > 0) emu->dt--;
   emu_print_gfx(emu->gfx, 0, 0);

   t2 = emu_gettime();
#ifndef DEBUG
   emu_nanosleep(FREQ_SYNC, emu_difftimespec(t1, t2));
#endif

#ifdef DEBUG
   DebugEmu.freq = emu_d_compute_freq(emu_difftimespec(t1, emu_gettime()));
   memcpy(&DebugEmu.prev_emu, &DebugEmu.emu, sizeof(st_emu));
   memcpy(&DebugEmu.emu, emu, sizeof(st_emu));
   emu_debug_draw(&DebugEmu);
#endif
}

int32_t main(int32_t argc, char *argv[])
{
   /* rom par default */
   char *rom = DEFAULT_ROM;
   st_emu emu;

   emu.pc = INIT_ADDR;
   emu.i = 0;
   memset(&emu.memory, 0, sizeof(uint8_t) * MEMORY_SIZE);
   memset(&emu.v, 0, sizeof(uint8_t) * DATA_REGISTER_NUMBER);
   memset(&emu.stack, 0, sizeof(struct st_stack));
   memset(&emu.gfx, 0, sizeof(uint8_t) * WIDTH * HEIGHT);

#ifdef DEBUG
   test();
#endif

   if (argc > 1)
      rom = argv[1];

   /* init random */
   srand((unsigned int)(time(NULL)));
   /* charge la font */
   memcpy(&emu.memory[FONT_ADDR],
         Chip8_fontset,
         CHAR_ENCODED * NUMBER_OF_CHAR * sizeof(uint8_t));

   /* charge la rom */
   emu_load_rom(&emu, rom);
   emu_init_gfx();


   while (1)
      emu_mainloop(&emu);

   emu_free_gfx();
   return 0;
}
