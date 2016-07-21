#include "emu.h"
#include <ncurses.h>

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

void emu_print_gfx(int32_t x0, int32_t y0)
{
   int i = 0;
   for (i = 0; i < WIDTH * HEIGHT; i++) {
      mvprintw((i / WIDTH) + y0, (i % WIDTH) + x0, " ");
      if (Gfx[i] == 1)
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
   nodelay(stdscr, TRUE);

   return 0;
}

void emu_free_gfx(void)
{
   endwin();
}
