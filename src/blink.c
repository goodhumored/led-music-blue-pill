#include "blink.h"
#include "pin-mapping.h"
#include "util.h"
#include <stdint.h>
#ifndef UNIT_TEST
#include <libopencm3/stm32/gpio.h>
#endif

void blink(int times, uint32_t duration, uint32_t delay) {
#ifndef UNIT_TEST
  for (int i = 0; i < times; i++) {
    gpio_toggle(GPIOC, DEBUG_LED);
    wait(duration);
    gpio_toggle(GPIOC, DEBUG_LED);
    wait(delay);
  }
  #endif
}
