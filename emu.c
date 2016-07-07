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

#if 0
int main(int argc, char *argv[])
#endif
int main()
{
   for (;;)
      mainloop();
   return 0;
}

int mainloop()
{
   return 0;
}
