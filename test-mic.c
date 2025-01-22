#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>
#include <stdint.h>

#define PWM_PIN GPIO8 // PA8 для ШИМ
#define ADC_PIN GPIO0 // PA0 для ADC (микрофон)

void wait(int time) {
  for (time = 0; time < 800000; time++) __asm__("nop");
}

void gpio_setup(void) {
  rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_AFIO);

  gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, PWM_PIN);
  gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_ANALOG, ADC_PIN);
}

void adc_setup(void) {
  rcc_periph_clock_enable(RCC_ADC1);

  adc_power_on(ADC1);
  adc_set_regular_sequence(ADC1, 1, (uint8_t[]){0});
  adc_set_sample_time_on_all_channels(ADC1, ADC_SMPR_SMP_55DOT5CYC);
  adc_power_on(ADC1);
}

uint16_t read_adc(void) {
  adc_start_conversion_regular(ADC1);
  while (!(ADC1_SR & ADC_SR_EOC)) {}
  return adc_read_regular(ADC1);
}

void pwm_setup(void) {
  rcc_periph_clock_enable(RCC_TIM1);

  timer_set_prescaler(TIM1, 72);
  timer_set_period(TIM1, 1000);
  timer_set_oc_mode(TIM1, TIM_OC1, TIM_OCM_PWM1);
  timer_enable_oc_output(TIM1, TIM_OC1);
  timer_enable_preload(TIM1);
  timer_enable_break_main_output(TIM1);
  timer_enable_counter(TIM1);
}

int main(void) {
  gpio_setup();
  adc_setup();
  pwm_setup();

  uint16_t adc_value;
  uint32_t pwm_duty_cycle;

  while (1) {
    adc_value = read_adc();
    pwm_duty_cycle = (adc_value * 1000) / 4095;
    timer_set_oc_value(TIM1, TIM_OC1, pwm_duty_cycle);
    wait(1000);
  }

  return 0;
}
