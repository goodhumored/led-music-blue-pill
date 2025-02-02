#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>
#include <stdint.h>

#define ADC_PIN GPIO0
#define RED_PIN GPIO7
#define GREEN_PIN GPIO6
#define BLUE_PIN GPIO9
#define RED_OC TIM_OC2
#define GREEN_OC TIM_OC1
#define BLUE_OC TIM_OC4

// Максимальные значения для цветов 0-1
#define RED_MAX 0.7
#define GREEN_MAX 0.7
#define BLUE_MAX 0.9

// минимальные значения
#define RED_TRESHOLD 0.05
#define GREEN_TRESHOLD 0.05
#define BLUE_TRESHOLD 0.05

void wait(int time) {
  for (int i = 0; i < 3600 * time; i++) __asm__("nop");
}

void gpio_setup(void) {
  rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_GPIOB);

  gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, RED_PIN | GREEN_PIN | BLUE_PIN);
  gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_ANALOG, ADC_PIN);
}

void adc_setup(void) {
  rcc_periph_clock_enable(RCC_ADC1); // Включаем тактирование ADC1
  adc_power_off(ADC1);               // Выключаем ADC перед настройкой
  adc_set_sample_time(ADC1, 0, ADC_SMPR_SMP_239DOT5CYC); // Время выборки
  adc_set_continuous_conversion_mode(ADC1);
  adc_set_regular_sequence(ADC1, 1, (uint8_t[]){0}); // Канал 0
  adc_power_on(ADC1);                                // Включаем ADC
}

uint16_t read_adc(void) {
    while (!adc_eoc(ADC1)) {} // Ожидание завершения преобразования
    return adc_read_regular(ADC1);          // Чтение результата
}

void pwm_setup(void) {
  rcc_periph_clock_enable(RCC_TIM4);

  timer_set_prescaler(TIM4, 72);         // Делитель для получения частоты
  timer_set_period(TIM4, 1000);          // Период ШИМ

  timer_set_oc_mode(TIM4, RED_OC, TIM_OCM_PWM1);
  timer_enable_oc_output(TIM4, RED_OC);

  timer_set_oc_mode(TIM4, GREEN_OC, TIM_OCM_PWM1);
  timer_enable_oc_output(TIM4, GREEN_OC);

  timer_set_oc_mode(TIM4, BLUE_OC, TIM_OCM_PWM1);
  timer_enable_oc_output(TIM4, BLUE_OC);

  timer_enable_preload(TIM4);
  timer_enable_counter(TIM4);
}

uint16_t get_amplitude(void) {
  uint16_t adc_value;
  uint16_t max_value = 0;
  uint16_t min_value = 4095;

  // Считываем несколько значений ADC для получения амплитуды
  for (int i = 0; i < 500; i++) { // 100 выборок для точности
    adc_value = read_adc();
    if (adc_value > max_value) {
      max_value = adc_value;
    }
    if (adc_value < min_value) {
      min_value = adc_value;
    }
  }

  // Вычисляем амплитуду как разницу между максимальным и минимальным значением
  return max_value - min_value;
}

/**
 * sets led color
 * @param r - red color amount (0-255)
 * @param g - green color amount (0-255)
 * @param b - blue color amount (0-255)
 */
void set_color(uint8_t r, uint8_t g, uint8_t b) {
  int red_duty_cycle = (r*1000*RED_MAX)/255;
  int green_duty_cycle = (g*1000*GREEN_MAX)/255;
  int blue_duty_cycle = (b*1000*BLUE_MAX)/255;
  if (red_duty_cycle < RED_TRESHOLD*1000) red_duty_cycle = 0;
  if (green_duty_cycle < GREEN_TRESHOLD*1000) green_duty_cycle = 0;
  if (blue_duty_cycle < BLUE_TRESHOLD*1000) blue_duty_cycle = 0;
  /*if (red_duty_cycle > 1000*RED_MAX) red_duty_cycle = 1000*RED_MAX;*/
  /*if (green_duty_cycle > 1000*GREEN_MAX) green_duty_cycle = 1000*GREEN_MAX;*/
  /*if (blue_duty_cycle > 1000*BLUE_MAX) blue_duty_cycle = 1000*BLUE_MAX;*/
  timer_set_oc_value(TIM4, RED_OC, red_duty_cycle);
  timer_set_oc_value(TIM4, GREEN_OC, green_duty_cycle);
  timer_set_oc_value(TIM4, BLUE_OC, blue_duty_cycle);
}

int main(void) {
  gpio_setup();
  adc_setup();
  pwm_setup();

  uint16_t adc_value;
  uint32_t pwm_duty_cycle;
  uint16_t amplitude;
  uint32_t i = 0;
  /*bool up = true;*/

  adc_start_conversion_regular(ADC1); // Запуск преобразования
  while (1) {
    /*if (amplitude >= 256){*/
    /*  up = false;*/
    /*}*/
    /*if (amplitude <= 0){*/
    /*  up = true;*/
    /*}*/
    /*if (up) amplitude++;*/
    /*else amplitude--;*/
    amplitude = get_amplitude();
    amplitude = (amplitude * 256) / 4095;
    set_color(amplitude, amplitude, amplitude);
    /*wait(1000);*/
  }

  return 0;
}
