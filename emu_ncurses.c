#include "emu.h"
#include <ncurses.h>

void emu_process_input(uint16_t *k)
{
   int32_t c = wgetch(stdscr);
   switch (c) {
      case '7': *k = 0x1; break;
      case '8': *k = 0x2; break;
      case '9': *k = 0x3; break;
      case '0': *k = 0xc; break;
      case 'u': *k = 0x4; break;
      case 'i': *k = 0x5; break;
      case 'o': *k = 0x6; break;
      case 'p': *k = 0xd; break;
      case 'j': *k = 0x7; break;
      case 'k': *k = 0x8; break;
      case 'l': *k = 0x9; break;
      case 'm': *k = 0xe; break;
      case ',': *k = 0xa; break;
      case ';': *k = 0x0; break;
      case ':': *k = 0xb; break;
      case '!': *k = 0xf; break;
      default: *k = *k; break;
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
#ifdef DEBUG
   nodelay(stdscr, FALSE);
#else
   nodelay(stdscr, TRUE);
#endif

   return 0;
}

void emu_free_gfx(void)
{
   endwin();
}
