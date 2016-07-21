#ifndef _EMU_DEBUG_H
#define _EMU_DEBUG_H

#include <stdint.h>

struct debug_state
{
   double frequence;
   uint16_t opcode;
   uint16_t pc;
};

void emu_debug_draw(struct debug_state *d);
int32_t emu_print_debug(uint16_t opcode, uint16_t mem_pc);

#endif
