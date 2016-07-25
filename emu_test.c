#include "emu.h"
#include <assert.h>

static uint8_t font[CHAR_ENCODED * NUMBER_OF_CHAR] = {
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

void test(void)
{
   st_emu test_emu;
   uint16_t opcode = 0x1123;
   uint16_t pc = 0;
   uint8_t dt = 0;

   test_emu.pc = INIT_ADDR;
   test_emu.i = 0;
   memset(&test_emu.v, 0, sizeof(uint8_t) * DATA_REGISTER_NUMBER);
   memset(&test_emu.stack, 0, sizeof(struct st_stack));
   memset(&test_emu.gfx, 0, sizeof(uint8_t) * WIDTH * HEIGHT);
   memset(&test_emu.memory, 0, MEMORY_SIZE);
   memcpy(&test_emu.memory[FONT_ADDR], font, CHAR_ENCODED * NUMBER_OF_CHAR * sizeof(uint8_t));

   /* opcode = 0x0123 */
   test_emu.memory[test_emu.pc + 1] = 0x01;
   test_emu.memory[test_emu.pc] = 0x23;


   /* 0NNN */
   {
      pc = 34;
      pc = emu_opcode_0NNN(pc);
      assert(pc == 36);
   }

   /* 00E0 */
   {
      uint32_t i = 0;
      pc = 0;
      pc = emu_opcode_00E0(test_emu.gfx, pc);
      for (i = 0; i < WIDTH * HEIGHT; i++)
      {
         assert(test_emu.gfx[i] == 0x00);
      }
      assert(pc == 2);
   }

   /* 00EE */
   {
      pc = 0;
      opcode = 0x00EE;
      test_emu.stack.addr[0] = 0x1234;
      test_emu.stack.p = 1;
      pc = emu_opcode_00EE(&test_emu.stack, pc);
      assert(pc == 0x1234);
      assert(test_emu.stack.p == 0);
   }

   /* 1NNN */
   {
      opcode = 0x1134;
      pc = emu_opcode_1NNN(opcode);
      assert(pc == 0x0134);
   }

   /* 2NNN */
   {
      pc = 0x4567;
      opcode = 0x2234;
      assert(test_emu.stack.p == 0);
      pc = emu_opcode_2NNN(&test_emu.stack, opcode, pc);
      assert(pc == 0x0234);
      assert(test_emu.stack.addr[0] == 0x4567 + 2);
      assert(test_emu.stack.p == 1);
   }

   /* 3XNN */
   {
      pc = 0;
      opcode = 0x3101;
      test_emu.v[1] = 0x01;
      pc = emu_opcode_3XNN(test_emu.v, opcode, pc);
      assert(pc == 4);

      pc = 0;
      opcode = 0x3fe5;
      test_emu.v[0xf] = 0xe5;
      pc = emu_opcode_3XNN(test_emu.v, opcode, pc);
      assert(pc == 4);

      pc = 0;
      opcode = 0x3101;
      test_emu.v[1] = 0x02;
      pc = emu_opcode_3XNN(test_emu.v, opcode, pc);
      assert(pc == 2);

   }

   /* 4XNN */
   {
      pc = 0;
      opcode = 0x4101;
      test_emu.v[1] = 0x01;
      pc = emu_opcode_4XNN(test_emu.v, opcode, pc);
      assert(pc == 2);

      pc = 0;
      opcode = 0x4fe5;
      test_emu.v[0xf] = 0xe5;
      pc = emu_opcode_4XNN(test_emu.v, opcode, pc);
      assert(pc == 2);

      pc = 0;
      opcode = 0x4101;
      test_emu.v[1] = 0x02;
      pc = emu_opcode_4XNN(test_emu.v, opcode, pc);
      assert(pc == 4);
   }

   /* 5XY0 */
   {
      pc = 0;
      opcode = 0x5120;
      test_emu.v[1] = 0x01;
      test_emu.v[2] = 0x01;
      pc = emu_opcode_5XY0(test_emu.v, opcode, pc);
      assert(pc == 4);

      pc = 0;
      opcode = 0x5fe0;
      test_emu.v[0xf] = 0xe5;
      test_emu.v[0xe] = 0xe4;
      pc = emu_opcode_5XY0(test_emu.v, opcode, pc);
      assert(pc == 2);

      pc = 0;
      opcode = 0x5060;
      test_emu.v[0] = 0xe8;
      test_emu.v[6] = 0xe8;
      pc = emu_opcode_5XY0(test_emu.v, opcode, pc);
      assert(pc == 4);
   }

   /* 6XNN */
   {
      pc = 0;
      opcode = 0x6324;
      pc = emu_opcode_6XNN(test_emu.v, opcode, pc);
      assert(test_emu.v[3] == 0x24);
      assert(pc == 2);
   }

   /* 7XNN */
   {

      pc = 0;
      opcode = 0x7e12;
      test_emu.v[0xe] = 0x01;
      pc = emu_opcode_7XNN(test_emu.v, opcode, pc);
      assert(test_emu.v[0xe] == 0x13);
      assert(pc == 2);

      pc = 0;
      opcode = 0x73ff;
      test_emu.v[3] = 0x0e;
      pc = emu_opcode_7XNN(test_emu.v, opcode, pc);
      assert(test_emu.v[3] == 0x0d);
      assert(pc == 2);
   }

   /* 8XY0 */
   {
      pc = 0;
      opcode = 0x81e0;
      test_emu.v[0xe] = 0x12;
      pc = emu_opcode_8XY0(test_emu.v, opcode, pc);
      assert(test_emu.v[0x1] == 0x12);
      assert(pc == 2);

      pc = 0;
      opcode = 0x8f80;
      test_emu.v[0xf] = 0x20;
      test_emu.v[8] = 0x45;
      pc = emu_opcode_8XY0(test_emu.v, opcode, pc);
      assert(test_emu.v[0xf] == 0x45);
      assert(pc == 2);
   }

   /* 8XY1 */
   {
      pc = 0;
      opcode = 0x81e1;
      test_emu.v[0x1] = 0xff;
      test_emu.v[0xe] = 0x12;
      pc = emu_opcode_8XY1(test_emu.v, opcode, pc);
      assert(test_emu.v[0x1] == 0xff);
      assert(pc == 2);

      pc = 0;
      opcode = 0x8f81;
      test_emu.v[0xf] = 0xe5;
      test_emu.v[0x8] = 0x08;
      pc = emu_opcode_8XY1(test_emu.v, opcode, pc);
      assert(test_emu.v[0xf] == 0xed);
      assert(pc == 2);
   }

   /* 8XY2 */
   {
      pc = 0;
      opcode = 0x8e82;
      test_emu.v[0xe] = 0xe5;
      test_emu.v[0x8] = 0x08;
      pc = emu_opcode_8XY2(test_emu.v, opcode, pc);
      assert(test_emu.v[0xe] == 0x00);
      assert(pc == 2);

      pc = 0;
      opcode = 0x8f82;
      test_emu.v[0xf] = 0xff;
      test_emu.v[0x8] = 0x08;
      pc = emu_opcode_8XY2(test_emu.v, opcode, pc);
      assert(test_emu.v[0xf] == 0x08);
      assert(pc == 2);
   }

   /* 8XY3 */
   {
      pc = 0;
      opcode = 0x8e83;
      test_emu.v[0xe] = 0xe5;
      test_emu.v[0x8] = 0x08;
      pc = emu_opcode_8XY3(test_emu.v, opcode, pc);
      assert(pc == 2);
      assert(test_emu.v[0xe] == 0xed);
   }

   /* 8XY4 */
   {
      pc = 0;
      opcode = 0x8284;
      test_emu.v[0x2] = 0xff;
      test_emu.v[0x8] = 0x1;
      test_emu.v[0xf] = 0x0;
      pc = emu_opcode_8XY4(test_emu.v, opcode, pc);
      assert(pc == 2);
      assert(test_emu.v[0x2] == 0x00);
      assert(test_emu.v[0xf] == 0x01);

      pc = 0;
      opcode = 0x8284;
      test_emu.v[0x2] = 0x10;
      test_emu.v[0x8] = 0x1;
      test_emu.v[0xf] = 0x0;
      pc = emu_opcode_8XY4(test_emu.v, opcode, pc);
      assert(pc == 2);
      assert(test_emu.v[0x2] == 0x11);
      assert(test_emu.v[0xf] == 0x00);
   }

   /* 8XY5 */
   {
      pc = 0;
      opcode = 0x8785;
      test_emu.v[0x7] = 0xff;
      test_emu.v[0x8] = 0xfe;
      test_emu.v[0xf] = 0x0;
      pc = emu_opcode_8XY5(test_emu.v, opcode, pc);
      assert(pc == 2);
      assert(test_emu.v[0x7] == 0x1);
      assert(test_emu.v[0xf] == 0x1);

      pc = 0;
      opcode = 0x8125;
      test_emu.v[0x1] = 0x04;
      test_emu.v[0x2] = 0x05;
      test_emu.v[0xf] = 0x0;
      pc = emu_opcode_8XY5(test_emu.v, opcode, pc);
      assert(pc == 2);
      assert(test_emu.v[0x1] == 0xff);
      assert(test_emu.v[0xf] == 0x0);
   }

   /* 8XY6 */
   {
      pc = 0;
      opcode = 0x8086;
      test_emu.v[0x0] = 0x02;
      pc = emu_opcode_8XY6(test_emu.v, opcode, pc);
      assert(test_emu.v[0x0] == 0x01);
      assert(test_emu.v[0xf] == 0x00);
      assert(pc == 2);

      pc = 0;
      opcode = 0x8086;
      test_emu.v[0x0] = 0x03;
      pc = emu_opcode_8XY6(test_emu.v, opcode, pc);
      assert(test_emu.v[0x0] == 0x01);
      assert(test_emu.v[0xf] == 0x01);
      assert(pc == 2);
   }

   /* 8XY7 */
   {
      pc = 0;
      opcode = 0x8127;
      test_emu.v[0x1] = 0xe5;
      test_emu.v[0x2] = 0xe5;
      pc = emu_opcode_8XY7(test_emu.v, opcode, pc);
      assert(test_emu.v[0x1] == 0x00);
      assert(test_emu.v[0xf] == 0x01);

      pc = 0;
      opcode = 0x8127;
      test_emu.v[0x1] = 0xe5;
      test_emu.v[0x2] = 0xe6;
      pc = emu_opcode_8XY7(test_emu.v, opcode, pc);
      assert(test_emu.v[0x1] == 0x01);
      assert(test_emu.v[0xf] == 0x01);
      assert(pc == 2);

      pc = 0;
      opcode = 0x8127;
      test_emu.v[0x1] = 0xe6;
      test_emu.v[0x2] = 0xe5;
      pc = emu_opcode_8XY7(test_emu.v, opcode, pc);
      assert(test_emu.v[0x1] == 0xff);
      assert(test_emu.v[0xf] == 0x00);
      assert(pc == 2);
   }

   /* 8XYE */
   {
      pc = 0;
      opcode = 0x812E;
      test_emu.v[0x1] = 0x81;
      pc = emu_opcode_8XYE(test_emu.v, opcode, pc);
      assert(test_emu.v[0x1] == 0x2);
      assert(test_emu.v[0xf] == 0x1);
      assert(pc == 2);

      pc = 0;
      opcode = 0x812E;
      test_emu.v[0x1] = 0x72;
      pc = emu_opcode_8XYE(test_emu.v, opcode, pc);
      assert(test_emu.v[0x1] == 0xe4);
      assert(test_emu.v[0xf] == 0x0);
      assert(pc == 2);
   }

   /* 9XY0 */
   {
      pc = 0;
      opcode = 0x9480;
      test_emu.v[0x4] = 0x81;
      test_emu.v[0x8] = 0x81;
      pc = emu_opcode_9XY0(test_emu.v, opcode, pc);
      assert(pc == 2);

      pc = 0;
      opcode = 0x9480;
      test_emu.v[0x4] = 0x81;
      test_emu.v[0x8] = 0x82;
      pc = emu_opcode_9XY0(test_emu.v, opcode, pc);
      assert(pc == 4);
   }

   /* ANNN */
   {
      pc = 0;
      test_emu.i = 0;
      opcode = 0xA123;
      pc = emu_opcode_ANNN(&test_emu.i, opcode, pc);
      assert(test_emu.i == 0x123);
      assert(pc == 2);
   }

   /* BNNN */
   {
      pc = 0;
      opcode = 0xB123;
      test_emu.v[0] = 0x1;
      pc = emu_opcode_BNNN(test_emu.v, opcode);
      assert(pc == 0x0124);
   }

   /* CXNN */
   {
      pc = 0;
      opcode = 0xC1ff;
      test_emu.v[0] = 0x0f;
      pc = emu_opcode_CXNN(test_emu.v,  opcode, pc);
      assert(pc == 2);
   }

#if 0
   emu_opcode_DXYN(const uint8_t *mem, uint8_t *gfx, uint8_t *v, uint16_t i, uint16_t opcode, uint16_t pc);
#endif

   /* EX9E */
   {
      pc = 0;
      opcode = 0xE89E;
      test_emu.v[8] = 0x06;
      test_emu.key = 0x6;
      pc = emu_opcode_EX9E(test_emu.v, test_emu.key, opcode, pc);
      assert(pc == 4);

      pc = 0;
      opcode = 0xEe9E;
      test_emu.v[0xe] = 0x0f;
      test_emu.key = 0x6;
      pc = emu_opcode_EX9E(test_emu.v, test_emu.key, opcode, pc);
      assert(pc == 2);
   }

   /* EXA1 */
   {
      pc = 0;
      opcode = 0xE9A1;
      test_emu.v[0x9] = 0x02;
      test_emu.key = 0x6;
      pc = emu_opcode_EXA1(test_emu.v, test_emu.key, opcode, pc);
      assert(pc == 4);

      pc = 0;
      opcode = 0xE9A1;
      test_emu.v[0x9] = 0x06;
      test_emu.key = 0x6;
      pc = emu_opcode_EXA1(test_emu.v, test_emu.key, opcode, pc);
      assert(pc == 2);
   }

   /* FX07 */
   {
      pc = 0;
      dt = 10;
      opcode = 0xF107;
      test_emu.v[0x1] = 0x00;
      pc = emu_opcode_FX07(test_emu.v, dt, opcode, pc);
      assert(test_emu.v[0x1] = 10);
      assert(pc == 2);
   }
#if 0
   /* not well implemented yet */
   emu_opcode_FX0A(uint16_t *k, const uint8_t *v, uint16_t opcode, uint16_t pc);
#endif

   /* FX15 */
   {
      pc = 0;
      opcode = 0xF115;
      test_emu.v[0x1] = 0x18;
      pc = emu_opcode_FX15(&test_emu.dt, test_emu.v, opcode, pc);
      assert(test_emu.dt = 0x18);
      assert(pc == 2);
   }

   /* FX18 */
   {
      pc = 0;
      opcode = 0xF118;
      test_emu.v[0x1] = 0x18;
      pc = emu_opcode_FX18(&test_emu.st, test_emu.v, opcode, pc);
      assert(test_emu.st = 0x18);
      assert(pc == 2);
   }

   /* FX1E */
   {
      pc = 0;
      opcode = 0xFb1E;
      test_emu.i = 0x50;
      test_emu.v[0xb] = 0x01;
      pc = emu_opcode_FX1E(&test_emu.i, test_emu.v, opcode, pc);
      assert(test_emu.i = 0x51);
      assert(pc == 2);
   }

   /* FX29 */
   {
      pc = 0;
      opcode = 0xFb29;
      test_emu.i = 0x50;
      test_emu.v[0xb] = 0x4;
      pc = emu_opcode_FX29(&test_emu.i, test_emu.v, opcode, pc);
      assert(test_emu.i == FONT_ADDR + CHAR_ENCODED * test_emu.v[0xb]);
      assert(test_emu.memory[test_emu.i] == 0x90);
      assert(pc == 2);
   }

   /* FX33 */
   {
      pc = 0;
      opcode = 0xfe33;
      test_emu.i = 0x50;
      test_emu.v[0xe] = 0x00; /* 123 */
      pc = emu_opcode_FX33(test_emu.memory, test_emu.v, test_emu.i, opcode, pc);
      assert(test_emu.memory[test_emu.i    ] == 0);
      assert(test_emu.memory[test_emu.i + 1] == 0);
      assert(test_emu.memory[test_emu.i + 2] == 0);
      assert(pc == 2);
   }

   /* FX55 */
   {
      pc = 0;
      opcode = 0xF255;
      test_emu.i = 0x50;
      test_emu.v[0x0] = 0x12;
      test_emu.v[0x1] = 0x34;
      test_emu.v[0x2] = 0x56;
      pc = emu_opcode_FX55(test_emu.memory, test_emu.v, test_emu.i, opcode, pc);
      assert(test_emu.memory[test_emu.i] == 0x12);
      assert(test_emu.memory[test_emu.i + 1] == 0x34);
      assert(test_emu.memory[test_emu.i + 2] == 0x56);
      assert(pc == 2);
   }

   /* FX65 */
   {
      pc = 0;
      opcode = 0xF265;
      test_emu.i = 0x80;
      test_emu.memory[test_emu.i] = 0x12;
      test_emu.memory[test_emu.i + 1] = 0x12;
      test_emu.memory[test_emu.i + 2] = 0x45;
      pc = emu_opcode_FX65(test_emu.memory, test_emu.v, test_emu.i, opcode, pc);
      assert(test_emu.v[0] == 0x12);
      assert(test_emu.v[1] == 0x12);
      assert(test_emu.v[2] == 0x45);
      assert(pc == 2);
   }
}

