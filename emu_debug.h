#ifndef _EMU_DEBUG_H
#define _EMU_DEBUG_H

#include <stdint.h>

struct debug_emu
{
   double freq;
   st_emu emu;
   st_emu prev_emu;
};

void emu_debug_draw(struct debug_emu *d);
void emu_debug_draw_mem(const uint8_t *mem, const uint8_t *prev_mem, int32_t rom_size, uint16_t emu_pc, uint16_t prev_emu_pc, uint8_t n_line, uint8_t x0, uint8_t y0);
int32_t emu_print_debug(uint16_t opcode, uint16_t mem_pc);

#endif
