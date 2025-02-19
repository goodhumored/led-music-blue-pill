#include "adc-to-fft.h"
#include "adc.h"
#include "blink.h"
#include "frequency-analyser.h"
#include "led-strip-controller.h"
#include "pin-mapping.h"
#include "pwm.h"
#include "usart.h"
#include <kiss_fft.h>
#include <stdint.h>
#include <stdio.h>

#define FFT_CONVERSION_PERIOD FFT_SIZE

void push_value(kiss_fft_cpx buffer[FFT_SIZE], int16_t value) {
  for (int i = 0; i < FFT_SIZE - 1; i++) {
    buffer[i].r = buffer[i + 1].r;
  }
  buffer[FFT_SIZE - 1].r = value;
}

void fill_fft_buffer(kiss_fft_cpx in[FFT_SIZE], uint16_t *max_value) {
  uint16_t value = 0;
  for (int i = 0; i < FFT_CONVERSION_PERIOD; i++) {
    value = read_adc();
    if (value > *max_value) {
      *max_value = value;
    }
    in[i].r = adc_to_kiss_fft_scalar(value); // Пример для 12-bit АЦП
    in[i].i = 0;
  }
}

#ifndef UNIT_TEST
int main(void) {
  FrequencyBands bands;
  // инициализация fft
  kiss_fft_cfg cfg = kiss_fft_alloc(FFT_SIZE, 0, NULL, NULL);
  kiss_fft_cpx in[FFT_SIZE], out[FFT_SIZE];

  uint32_t another_value;
  int16_t fft_value;

  // Инициализация всей аппаратной части
  init_gpio();
  usart_setup();
  init_adc();
  init_pwm();

  blink(3, 50, 25);

  start_conversion();

  blink(2, 25, 10);

  int16_t red = 0;
  int16_t green = 0;
  int16_t blue = 0;
  int16_t bins[FFT_SIZE];
  uint16_t max_val;

  while (1) {
    max_val = 0;
    fill_fft_buffer(in, &max_val);
    blink(1, 10, 0);

    kiss_fft(cfg, in, out);

    calculate_bin_amps(out, bins);
    send_fft_data(bins, FFT_SIZE/2);
    calculate_bands(bins, &bands, PWM_MAX_VALUE);

    red = bands.low;
    green = bands.mid;
    blue = bands.high;

    /*printf("(%d, %d, %d)\r\n", red, green, blue);*/

    set_led_color(red, green, blue);
  }

  return 0;
}
#endif
