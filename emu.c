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

uint8_t I;
uint16_t Pc;

#if 0
int main(int argc, char *argv[])
#endif
int main()
{
   Pc = 0x200;
   Opcode = 0;
   I = 0;

   for (;;)
      mainloop();

   return 0;
}

int mainloop()
{
   fetch_opcode();
   decode_opcode();
   execute_opcode();
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
   Opcode = Memory[Pc] << 8 | Memory[Pc + 1];

   return 0;
}

int decode_opcode()
{
   switch ((Opcode & 0xF000) >> 12)
   {
      case 0x0:
         break;
      case 0x1:
         break;
      case 0x2:
         break;
      case 0x3:
         break;
      case 0x4:
         break;
      case 0x5:
         break;
      case 0x6:
         break;
      case 0x7:
         break;
      case 0x8:
         break;
      case 0x9:
         break;
      case 0xA:
         break;
      case 0xB:
         break;
      case 0xC:
         break;
      case 0xD:
         break;
      case 0xE:
         break;
      case 0xF:
         break;
      default:
         break;
   }
   I = Opcode & 0x0FFF;
   Pc += 2;

   return 0;
}

int execute_opcode()
{

   return 0;
}
