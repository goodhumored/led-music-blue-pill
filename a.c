#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>
#include <stdint.h>

#define PWM_PIN GPIO8 // PA8 для ШИМ
#define ADC_PIN GPIO0 // PA0 для ADC (микрофон)
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>

volatile uint32_t max_value = 0;
volatile uint32_t hold_time_counter = 0;

void tim2_setup(void) {
    // Включаем тактирование TIM2
    rcc_periph_clock_enable(RCC_TIM2);

    // Настраиваем таймер на частоту 1 кГц (прерывание каждые 1 мс)
    timer_set_prescaler(TIM2, rcc_apb1_frequency / 1000000 - 1); // Предделитель для 1 МГц
    timer_set_period(TIM2, 1000 - 1); // Период = 1000 тиков (1 мс)

    // Разрешаем прерывание по обновлению таймера
    timer_enable_irq(TIM2, TIM_DIER_UIE);

    // Включаем таймер
    timer_enable_counter(TIM2);

    // Настраиваем прерывание в NVIC
    nvic_enable_irq(NVIC_TIM2_IRQ);
}

void tim2_isr(void) {
  if (timer_get_flag(TIM2, TIM_SR_UIF)) {
    // Сбрасываем флаг прерывания
    timer_clear_flag(TIM2, TIM_SR_UIF);
    if (hold_time_counter > 1000) max_value = 0;
  }
}

void wait(int time) {
  for (int i = 0; i < 3600 * time; i++) __asm__("nop");
}

void gpio_setup(void) {
  rcc_periph_clock_enable(RCC_GPIOA);

  gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, PWM_PIN);
  gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_ANALOG, ADC_PIN);
}

void adc_setup(void) {
  rcc_periph_clock_enable(RCC_ADC1);
  adc_power_off(ADC1); // Выключить ADC перед настройкой
  rcc_set_adcpre(RCC_CFGR_ADCPRE_PCLK2_DIV8); // Делитель 8 (72 МГц / 8 = 9 МГц)
  adc_power_on(ADC1);
  wait(10); // 100 = 1 second (на всё (0.1сек итерация)) in debug, 3x times slower (3.3 (0.33сек?)) in prod
  /*adc_enable_scan_mode(ADC1);*/
  adc_set_regular_sequence(ADC1, 1, (uint8_t[]){ 0 });
  adc_set_sample_time(ADC1, 0, ADC_SMPR_SMP_1DOT5CYC);
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

void adc_setup1(void) {
  rcc_periph_clock_enable(RCC_ADC1); // Включаем тактирование ADC1
  adc_power_off(ADC1);               // Выключаем ADC перед настройкой
  adc_set_sample_time(ADC1, 0, ADC_SMPR_SMP_239DOT5CYC); // Время выборки
  adc_set_continuous_conversion_mode(ADC1);
  adc_set_regular_sequence(ADC1, 1, (uint8_t[]){0}); // Канал 0
  adc_power_on(ADC1);                                // Включаем ADC
  wait(1000); // Задержка для стабилизации ADC
}

int main(void) {
  gpio_setup();
  adc_setup1();
  pwm_setup();

  uint16_t adc_value;
  uint32_t pwm_duty_cycle;
  uint16_t amplitude;
  uint32_t i = 0;
  adc_start_conversion_regular(ADC1); // Запуск преобразования

  while (1) {
    /*amplitude = get_amplitude();  // Получаем амплитуду*/
    while (!adc_eoc(ADC1)) {}
    /*amplitude = adc_read_regular(ADC1);*/
    // Преобразуем амплитуду в диапазон от 0 до 1000 для ШИМ
    /*pwm_duty_cycle = (amplitude * 1000) / 4095;*/
    i = (i + 25) % 1001;
    /*pwm_duty_cycle = i;*/
    timer_set_oc_value(TIM1, TIM_OC1, pwm_duty_cycle);
    wait(1000); // 100 = 1 second (на всё (0.1сек итерация)) in debug, 3x times slower (3.3 (0.33сек?)) in prod
  }

  return 0;
}
