#include "led-strip-controller.h"
#include "pin-mapping.h"
#include "pwm.h"
#include <libopencm3/stm32/timer.h>
#include <stdint.h>

// Множетили для выходного значения
#define RED_MULTIPLIER 0.7
#define GREEN_MULTIPLIER 0.7
#define BLUE_MULTIPLIER 0.9

// минимальные значения цветов 0-1
#define RED_THRESHOLD 0.05
#define GREEN_THRESHOLD 0.05
#define BLUE_THRESHOLD 0.05

void set_chanel_color(enum ColorChannel c, double value) {
  enum tim_oc_id channel = RED_OC;
  unsigned int timer_id = RED_TIM;
  double multiplier = RED_MULTIPLIER;
  double threshold = RED_THRESHOLD;
  uint32_t value_to_set = 0;
  if (c == GREEN) {
    timer_id = GREEN_TIM;
    channel = GREEN_OC;
    multiplier = GREEN_MULTIPLIER;
    threshold = GREEN_THRESHOLD;
  } else if (c == BLUE) {
    timer_id = BLUE_TIM;
    channel = BLUE_OC;
    multiplier = BLUE_MULTIPLIER;
    threshold = BLUE_THRESHOLD;
  }
  if (value > threshold) 
    value_to_set = (uint32_t)(value * multiplier * PWM_MAX_VALUE);
  timer_set_oc_value(timer_id, channel, value_to_set);
}

void set_led_color(double r, double g, double b) {
  set_chanel_color(RED, r);
  set_chanel_color(GREEN, g);
  set_chanel_color(BLUE, b);
}
