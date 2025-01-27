#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>
#include <stdint.h>

#define PWM_PIN GPIO8 // PA8 для ШИМ
#define ADC_PIN GPIO0 // PA0 для ADC (микрофон)

void wait(int time) {
  for (int i = 0; i < 3600 * time; i++) __asm__("nop");
}

void gpio_setup(void) {
  rcc_periph_clock_enable(RCC_GPIOA);

  gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, PWM_PIN);
  gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_ANALOG, ADC_PIN);
}

void adc_setup(void) {
  rcc_periph_clock_enable(RCC_ADC1); // Включаем тактирование ADC1
  adc_power_off(ADC1);               // Выключаем ADC перед настройкой
  adc_set_sample_time(ADC1, 0, ADC_SMPR_SMP_239DOT5CYC); // Время выборки
  adc_set_continuous_conversion_mode(ADC1);
  adc_set_regular_sequence(ADC1, 1, (uint8_t[]){0}); // Канал 0
  adc_power_on(ADC1);                                // Включаем ADC
  wait(1000); // Задержка для стабилизации ADC
}

uint16_t read_adc(void) {
    while (!adc_eoc(ADC1)) {
    } // Ожидание завершения преобразования
    return adc_read_regular(ADC1);          // Чтение результата
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

uint16_t get_amplitude(void) {
  uint16_t adc_value;
  uint16_t max_value = 0;
  uint16_t min_value = 4095;

  // Считываем несколько значений ADC для получения амплитуды
  for (int i = 0; i < 100; i++) { // 100 выборок для точности
    adc_value = read_adc();
    if (adc_value > max_value) {
      max_value = adc_value;
    }
    if (adc_value < min_value) {
      min_value = adc_value;
    }
    wait(5); // 100 = 1 second (на всё (0.1сек итерация)) in debug, 3x times slower (3.3 (0.33сек?)) in prod
  }

  // Вычисляем амплитуду как разницу между максимальным и минимальным значением
  return max_value - min_value;
}

int main(void) {
  gpio_setup();
  adc_setup();
  pwm_setup();

  uint16_t adc_value;
  uint32_t pwm_duty_cycle;
  uint16_t amplitude;
  uint32_t i = 0;

  adc_start_conversion_regular(ADC1); // Запуск преобразования
  while (1) {
    amplitude = get_amplitude();  // Получаем амплитуду
    /*while (!adc_eoc(ADC1)) {*/
    /*} // Ожидание завершения преобразования*/
    /*amplitude = adc_read_regular(ADC1);          // Чтение результата*/
    // Преобразуем амплитуду в диапазон от 0 до 1000 для ШИМ
    pwm_duty_cycle = (amplitude * 1000) / 4095;
    i = (i + 25) % 1001;
    /*pwm_duty_cycle = i;*/
    timer_set_oc_value(TIM1, TIM_OC1, pwm_duty_cycle);
    wait(50); // 100 = 1 second (на всё (0.1сек итерация)) in debug, 3x times slower (3.3 (0.33сек?)) in prod
  }

  return 0;
}
