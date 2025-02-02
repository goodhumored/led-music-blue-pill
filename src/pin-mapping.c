#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include "pin-mapping.h"

void init_gpio(void) {
  rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_GPIOB);

  gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, RED_PIN | GREEN_PIN | BLUE_PIN);
  gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_ANALOG, MIC_PIN);
}
