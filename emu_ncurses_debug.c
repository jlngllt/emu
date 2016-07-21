#include "emu.h"
#include "emu_debug.h"

#include <ncurses.h>

int32_t emu_print_debug(uint16_t opcode, uint16_t mem_pc)
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

void emu_debug_draw(struct debug_state *d)
{
   mvprintw(50, 0, "fuck");
   mvprintw(HEIGHT + 2, 2, "opcode    = 0x%04x", d->opcode);
   mvprintw(HEIGHT + 3, 2, "pc        = 0x%04x", d->pc);
   mvprintw(HEIGHT + 4, 2, "frequence = %g", d->frequence);
}
