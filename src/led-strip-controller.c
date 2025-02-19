#include "led-strip-controller.h"
#include "pin-mapping.h"
#include "pwm.h"
#ifndef UNIT_TEST
#include <libopencm3/stm32/timer.h>
#endif
#include <stdint.h>

// Множетили для выходного значения
#define RED_MULTIPLIER (uint16_t)(1 * PWM_MAX_VALUE)
#define GREEN_MULTIPLIER (uint16_t)(0.4 * PWM_MAX_VALUE)
#define BLUE_MULTIPLIER (uint16_t)(0.2 * PWM_MAX_VALUE)

// минимальные значения цветов 0-1
#define RED_THRESHOLD (uint16_t)(0.05 * PWM_MAX_VALUE)
#define GREEN_THRESHOLD (uint16_t)(0.05 * PWM_MAX_VALUE)
#define BLUE_THRESHOLD (uint16_t)(0.05 * PWM_MAX_VALUE)

#ifndef UNIT_TEST
void set_pwm_value(uint16_t value, uint16_t threshold, uint16_t multiplier,
                   enum tim_oc_id channel, uint32_t timer_id) {
  uint32_t value_to_set = 0;
  if (value > PWM_MAX_VALUE)
    value_to_set = multiplier;
  else if (value > threshold)
    value_to_set = value * multiplier / 1000;
  timer_set_oc_value(timer_id, channel, value_to_set);
}
#endif

void set_chanel_color(enum ColorChannel c, uint32_t value) {
#ifndef UNIT_TEST
  enum tim_oc_id channel = RED_OC;
  unsigned int timer_id = RED_TIM;
  double multiplier = RED_MULTIPLIER;
  double threshold = RED_THRESHOLD;
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
  set_pwm_value(value, threshold, multiplier, channel, timer_id);
#endif
}

void set_led_color(uint32_t r, uint32_t g, uint32_t b) {
  set_chanel_color(RED, r);
  set_chanel_color(GREEN, g);
  set_chanel_color(BLUE, b);
}
