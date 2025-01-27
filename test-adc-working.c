#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <stdint.h>

#define LED_PIN GPIO13 // PA5 (светодиод на Blue Pill)
#define ADC_PIN GPIO0  // PA0 (вход АЦП)

void wait(int time) {
  for (int i = 0; i < 3600 * time; i++)
    __asm__("nop");
}

void gpio_setup(void) {
  rcc_periph_clock_enable(RCC_GPIOA); // Включаем тактирование GPIOA
  rcc_periph_clock_enable(RCC_GPIOC); // Включаем тактирование GPIOA
  gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL,
                LED_PIN); // Настройка светодиода
  gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_ANALOG,
                0); // Настройка входа АЦП
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

void togglewithdelay() {
  gpio_toggle(GPIOC, LED_PIN); // Мигаем светодиодом
  wait(500);                   // Задержка для стабилизации ADC
  gpio_toggle(GPIOC, LED_PIN); // Мигаем светодиодом
  wait(1000);                  // Задержка для стабилизации ADC
}

uint16_t read_adc(void) {
  adc_start_conversion_regular(ADC1); // Запуск преобразования
  while (!adc_eoc(ADC1)) {
  } // Ожидание завершения преобразования
  return adc_read_regular(ADC1); // Чтение результата
}

int main(void) {
  gpio_setup();
  /*adc_setup();*/
  /*adc_start_conversion_regular(ADC1); // Запуск преобразования*/
  while (true) {
    gpio_toggle(GPIOC, LED_PIN); // Мигаем светодиодом
    while (!adc_eoc(ADC1)) {
    } // Ожидание завершения преобразования
    /*uint16_t adc_value = adc_read_regular(ADC1);          // Чтение результата*/
    uint16_t delay = 1000 * 1000 / 4095;
    if (delay < 1000) {
      delay = 100;
    }

    for (int i = 0; i < 10; i++) {
      gpio_toggle(GPIOC, LED_PIN); // Мигаем светодиодом
      wait(delay);                 // Задержка для стабилизации ADC
    }
  }
}
