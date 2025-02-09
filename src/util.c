#include "util.h"
#include <stdint.h>
#define MS_TICKS 3600

void wait(uint32_t time) {
  uint32_t stop = MS_TICKS * time;
  for (uint32_t i = 0; i < stop; i++) __asm__("nop");
}

