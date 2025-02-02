#include "pwm.h"
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>
#include "pin-mapping.h"

void init_pwm(void) {
  rcc_periph_clock_enable(RCC_TIM4);

  timer_set_prescaler(RED_TIM, 72);         // Делитель для получения частоты
  timer_set_period(RED_TIM, PWM_MAX_VALUE);          // Период ШИМ

  timer_set_oc_mode(RED_TIM, RED_OC, TIM_OCM_PWM1);
  timer_enable_oc_output(RED_TIM, RED_OC);

  timer_set_oc_mode(GREEN_TIM, GREEN_OC, TIM_OCM_PWM1);
  timer_enable_oc_output(GREEN_TIM, GREEN_OC);

  timer_set_oc_mode(BLUE_TIM, BLUE_OC, TIM_OCM_PWM1);
  timer_enable_oc_output(BLUE_TIM, BLUE_OC);

  timer_enable_preload(RED_TIM);
  timer_enable_counter(RED_TIM);
}
