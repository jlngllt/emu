#include "emu.h"
#include <ncurses.h>

void emu_process_input(int *k)
{
   int c = 0;
   c = wgetch(stdscr);
   switch (c)
   {
      case 'i' : *k = 0x1; break;
      case '2' : *k = 0x2; break;
      case '3' : *k = 0x3; break;
      case KEY_UP : *k = 0xc; break;
      case 'k' : *k = 0x4; break;
      case 'z' : *k = 0x5; break;
      case 'e' : *k = 0x6; break;
      case KEY_DOWN : *k = 0xd; break;
      case 'j' : *k = 0x7; break;
      case 's' : *k = 0x8; break;
      case 'd' : *k = 0x9; break;
      case KEY_LEFT:
      case KEY_RIGHT : *k = 0xe; break;
      case 'l' : *k = 0xa; break;
      case 'x' : *k = 0x0; break;
      case 'c' : *k = 0xb; break;
      case 'v' : *k = 0xf; break;
      case ERR :
      default :
      break;
   }

}

void emu_print_gfx(uint8_t *gfx, int32_t x0, int32_t y0)
{
   int i = 0;
   for (i = 0; i < WIDTH * HEIGHT; i++) {
      mvprintw((i / WIDTH) + y0, (i % WIDTH) + x0, " ");
      if (gfx[i] == 1)
         mvprintw((i / WIDTH) + y0, (i % WIDTH) + x0, "*");
   }
}

int32_t emu_init_gfx(void)
{
   initscr();
   noecho();
   cbreak();
   curs_set(0);
   keypad(stdscr, TRUE);
   nodelay(stdscr, true);

   return 0;
}

void emu_free_gfx(void)
{
   endwin();
}
