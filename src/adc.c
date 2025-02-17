#include "adc.h"
#include "pin-mapping.h"
#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/rcc.h>
#include <stdint.h>

#define MAX_ADC_VALUE 4095 // 12 бит АЦП макс значение 4095 (2**12)
#define ADC_PRESCALE 

void init_adc(void) {
  rcc_periph_clock_enable(MIC_RCC); // Включаем тактирование ADC1
  adc_power_off(MIC_ADC);  // Выключаем ADC перед настройкой
  adc_set_sample_time(MIC_ADC, MIC_CHANNEL, ADC_SMPR_SMP_239DOT5CYC); // Время выборки
  adc_set_continuous_conversion_mode(MIC_ADC);
  adc_set_regular_sequence(MIC_ADC, 1, (uint8_t[]){0}); // Канал 0
  adc_power_on(MIC_ADC); // Включаем ADC
}

void start_conversion(void) {
  adc_start_conversion_regular(MIC_ADC); // Запуск преобразования
}

uint32_t read_adc(void) {
  while (!adc_eoc(MIC_ADC)) {}
  return adc_read_regular(MIC_ADC);
}
