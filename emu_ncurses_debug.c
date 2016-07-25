#include "emu.h"
#include "emu_debug.h"

#include <ncurses.h>

#define SIZE_ESPACE 1
#define SIZE_OPCODE 4

#define PRINT_STYLE(y, x, text, var) \
   do { \
      if (d->emu.var != d->prev_emu.var) \
      { \
         attron(A_STANDOUT); \
         mvprintw(y, x, text, d->emu.var); \
      } else { \
         mvprintw(y, x, text, d->emu.var); \
      } \
      attroff(A_STANDOUT); \
   } while (0) \


void emu_debug_draw_mem(const uint8_t *mem, const uint8_t *prev_mem, int32_t rom_size, uint16_t emu_pc, uint16_t prev_emu_pc, uint8_t n_line, uint8_t x0, uint8_t y0)
{
   int32_t pc, j, x, y, i_elem;
   int32_t size_total = (rom_size + INIT_ADDR);
   int32_t size_print_elem = SIZE_OPCODE + SIZE_ESPACE;
   /* affichage de la mémoire */;
   for (i_elem = 0, y = 0, x = 0, pc = 0, j = 0; pc < size_total; pc += 2, i_elem++) {
      attroff(A_STANDOUT);
      if (i_elem == (emu_pc / 2))
         attron(A_STANDOUT);
      attroff(A_UNDERLINE);
      if (i_elem == (prev_emu_pc / 2))
         attron(A_UNDERLINE);
      if (prev_mem[pc] != mem[pc] || prev_mem[pc + 1] != mem[pc + 1])
      {
         attron(A_STANDOUT);
         mvprintw(y0 + y, x0 + x, "%02x%02x", mem[pc], mem[pc + 1]);
      } else {
         mvprintw(y0 + y, x0 + x, "%02x%02x", mem[pc], mem[pc + 1]);
      }
      attroff(A_STANDOUT);
      /* nombre de ligne sur lequel on affiche la mémoire */
      if (x > (size_print_elem * (size_total / 2)) / n_line) {
         y++;
         x = 0;
      }
      else
         x += SIZE_OPCODE + SIZE_ESPACE;
   }

   refresh();
}

void emu_debug_draw(struct debug_emu *d)
{
   int i = 0;
   emu_debug_draw_mem(d->emu.memory, d->prev_emu.memory, d->emu.rom_size, d->emu.pc, d->prev_emu.pc, 30, WIDTH + 1 , 0);
   mvprintw(HEIGHT  + 1,  0, "frequence : %g", d->freq);
   mvprintw(HEIGHT  + 2,  0, "Pc        : ");
   PRINT_STYLE(HEIGHT + 2,  12, "0x%02x", pc);
   mvprintw(HEIGHT  + 3,  0, "opcode    : ");
   PRINT_STYLE(HEIGHT + 3,  12, "0x%02x", opcode);
   mvprintw(HEIGHT  + 4,  0, "I         : ");
   PRINT_STYLE(HEIGHT + 4,  12, "0x%04x", i);
   mvprintw(HEIGHT  + 5,  0, "Dt        : ");
   PRINT_STYLE(HEIGHT + 5,  12, "0x%02x", dt);
   mvprintw(HEIGHT  + 7,  0, "V /");
   for (i = 0; i < 16; i++)
   {
      mvprintw(HEIGHT  + 8, i * 9 + 1, " V[%02d] | ", i);
      mvprintw(HEIGHT  + 9, i * 9 + 1, "       | ");
      PRINT_STYLE(HEIGHT + 9, i * 9 + 1, "0x%04x",v[i]);
   }
   mvprintw(HEIGHT + 11, 0, "Stack /");
   for (i = 0; i < 16; i++)
   {
      mvprintw(HEIGHT  + 12, i * 9 + 1, " s[%02d] | ", i);
      mvprintw(HEIGHT  + 13, i * 9 + 1, "       | ");
      PRINT_STYLE(HEIGHT + 13, i * 9 + 1, "0x%04x", stack.addr[i]);
   }
   mvprintw(HEIGHT  + 14,  0, "Sp       : ");
   PRINT_STYLE(HEIGHT + 14,  12, "0x%02x", stack.p);
}
