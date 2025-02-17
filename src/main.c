#include "adc.h"
#include "blink.h"
#include "frequency-analyser.h"
#include "led-strip-controller.h"
#include "pin-mapping.h"
#include "pwm.h"
#include "usart.h"
#include "util.h"
#include <kiss_fft.h>
#include <stdint.h>
#include <stdio.h>

#define OFFSET                                                                 \
  0 // половина от 4095 (макс значение АЦП)  это если в идеале сигнал
    // крутится вокруг 1.65v

#define FFT_CONVERSION_PERIOD 64

void push_value(kiss_fft_cpx buffer[FFT_SIZE], int16_t value) {
  for (int i = 0; i < FFT_SIZE - 1; i++) {
    buffer[i].r = buffer[i + 1].r;
  }
  buffer[FFT_SIZE - 1].r = value;
}

kiss_fft_scalar adc_to_kiss_fft_scalar(uint32_t value) {
  return ((int16_t)(value) - OFFSET)*16;
}

void fill_fft_buffer(kiss_fft_cpx in[FFT_SIZE]) {
  for (int i = 0; i < FFT_SIZE; i++) {
    in[i].r = adc_to_kiss_fft_scalar(read_adc()); // Пример для 12-bit АЦП
    in[i].i = 0;
  }
}

kiss_fft_scalar get_amplitude(kiss_fft_cpx afr) {}

int main(void) {
  FrequencyBands bands;
  // инициализация fft
  kiss_fft_cfg cfg = kiss_fft_alloc(FFT_SIZE, 0, NULL, NULL);
  kiss_fft_cpx in[FFT_SIZE], out[FFT_SIZE];

  uint32_t another_value;
  int16_t fft_value;
  uint32_t i = 0;

  // Инициализация всей аппаратной части
  init_gpio();
  usart_setup();
  init_adc();
  init_pwm();

  blink(3, 100, 50);

  start_conversion();

  blink(2, 50, 50);

  fill_fft_buffer(in);
  double red = 0;
  double green = 0;

  double blue = 0;
  uint16_t mxv = 0; 
  uint16_t mnv = 65535;
  uint32_t sum = 0;
  while (1) {
    another_value = read_adc();
    /*if (another_value > mxv) mxv = another_value;*/
    /*if (another_value < mnv) mnv = another_value;*/
    /*sum += another_value;*/
    /*printf("min: %d; max: %d; avg: %d\r\n", mnv, mxv, sum/i);*/
    fft_value = adc_to_kiss_fft_scalar(another_value);
    push_value(in, another_value);
    i++;
    if (i % FFT_CONVERSION_PERIOD == 0) {
      kiss_fft(cfg, in, out);
      printf("dc_offset: %d\r\n",out[0]);
      printf("2: %d; 10: %d; 20: %d; 32: %d\r\n",out[2], out[10], out[20], out[32]);
      calculate_bands(out, &bands);
      red = bands.low;
      green = bands.mid;
      blue = bands.high;
      wait(50);
      set_led_color(red, green, blue);
    }
    if (i % FFT_SIZE == 0) {
      blink(1, 10, 0);
    }
  }

  return 0;
}
