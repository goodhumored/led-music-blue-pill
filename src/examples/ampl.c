#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <stdint.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>

#define LED_PIN GPIO13 // PA5 (светодиод на Blue Pill)
#define ADC_PIN GPIO0  // PA0 (вход АЦП)
#define ELEMENT_TIME 500
#define DIT (1*ELEMENT_TIME)
#define DAH (3*ELEMENT_TIME)
#define INTRA (1*ELEMENT_TIME)
#define INTER (3*ELEMENT_TIME)
#define WORD (7*ELEMENT_TIME)


static void clock_setup(void)
{
	rcc_clock_setup_pll(&rcc_hse_configs[RCC_CLOCK_HSE8_72MHZ]);
}
uint16_t frequency_sequence[] = {
	DIT,
	INTRA,
	DIT,
	INTRA,
	DIT,
	INTER,
	DAH,
	INTRA,
	DAH,
	INTRA,
	DAH,
	INTER,
	DIT,
	INTRA,
	DIT,
	INTRA,
	DIT,
	WORD,
};

int frequency_sel = 0;

void tim2_setup(void) {
  // Включаем тактирование TIM2
  rcc_periph_clock_enable(RCC_TIM2);
  // Настраиваем прерывание в NVIC
  nvic_enable_irq(NVIC_TIM2_IRQ);
	/* Reset TIM2 peripheral to defaults. */
	rcc_periph_reset_pulse(RST_TIM2);
	timer_set_mode(TIM2, TIM_CR1_CKD_CK_INT,
		TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
timer_set_prescaler(TIM2, ((rcc_apb1_frequency * 2) / 5000));

	/* Disable preload. */
	timer_disable_preload(TIM2);
	timer_continuous_mode(TIM2);

	/* count full range, as we'll update compare value continuously */
	timer_set_period(TIM2, 65535);

	/* Set the initual output compare value for OC1. */
	timer_set_oc_value(TIM2, TIM_OC1, frequency_sequence[frequency_sel++]);

	/* Counter enable. */
	timer_enable_counter(TIM2);

	/* Enable Channel 1 compare interrupt to recalculate compare values */
	timer_enable_irq(TIM2, TIM_DIER_CC1IE);

}

/*void tim2_isr(void) {*/
/*  if (timer_get_flag(TIM2, TIM_SR_UIF)) {*/
/*    // Сбрасываем флаг прерывания*/
/*    timer_clear_flag(TIM2, TIM_SR_UIF);*/
/*  max_value = 0;*/
/*    hold_time_counter++;*/
/*    if (hold_time_counter > 10) max_value = 0;*/
/*  }*/
/*}*/

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

int main(void) {
	int i = 0;
  clock_setup();
  gpio_setup();
  adc_setup();
  tim2_setup();
 uint32_t max_value = 0;
 uint32_t min_value = 0;
 uint32_t max_amplitude = 0;
 uint32_t amplitude = 0;
 uint32_t delay = 100;

  adc_start_conversion_regular(ADC1); // Запуск преобразования
  while (true) {
		if (i++ > 1000) {
			i = 0;
			max_value = 0;
			min_value = 0;
			max_amplitude = 0;
		}
    gpio_toggle(GPIOC, LED_PIN); // Мигаем светодиодом
    while (!adc_eoc(ADC1)) {
    } // Ожидание завершения преобразования
    uint16_t adc_value = adc_read_regular(ADC1);          // Чтение результата
    if (adc_value > max_value) {  max_value = adc_value;}
    if (adc_value < min_value) {  min_value = adc_value;}
		amplitude = max_value - min_value;
		if (amplitude > max_amplitude) {max_amplitude = amplitude; i = 0;}
		if (max_amplitude < 100) delay = 100;
		else if (max_amplitude > 1000) delay = 1000;
		else delay = max_amplitude;
      gpio_toggle(GPIOC, LED_PIN); // Мигаем светодиодом
      wait(max_amplitude);                 // Задержка для стабилизации ADC
      gpio_toggle(GPIOC, LED_PIN); // Мигаем светодиодом
      wait(max_amplitude);                 // Задержка для стабилизации ADC
  }
}
