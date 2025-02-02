#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>
#include <stdint.h>
#include "pwm.h"
#include "adc.h"
#include "pin-mapping.h"
#include "led-strip-controller.h"
#include "util.h"

double get_amplitude(void) {
  double adc_value;
  double max_value = 0;
  double min_value = 1;
  for (int i = 0; i < 1000; i++) { // 100 выборок для точности
    adc_value = read_adc();
    if (adc_value > max_value) {
      max_value = adc_value;
    }
    if (adc_value < min_value) {
      min_value = adc_value;
    }
  }
  return max_value - min_value;
}

int main(void) {
  init_gpio();
  init_adc();
  init_pwm();

  double amplitude;
  float x = 0;

  start_conversion();
  while (1) {
    amplitude = get_amplitude();
    set_led_color(amplitude, amplitude, amplitude);
    wait(10);
  }

  return 0;
}
