#include "blink.h"
#include "pin-mapping.h"
#include "util.h"
#include <stdint.h>
#include <libopencm3/stm32/gpio.h>

void blink(int times, uint32_t duration, uint32_t delay) {
  for (int i = 0; i < times; i++) {
    gpio_toggle(GPIOC, DEBUG_LED);
    wait(duration);
    gpio_toggle(GPIOC, DEBUG_LED);
    wait(delay);
  }
}
