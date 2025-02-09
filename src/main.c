#include "adc.h"
#include "blink.h"
#include "frequency-analyser.h"
#include "led-strip-controller.h"
#include "pin-mapping.h"
#include "pwm.h"
#include <kiss_fft.h>
#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>


#define OFFSET                                                                 \
  0 // половина от 4095 (макс значение АЦП)  это если в идеале сигнал
       // крутится вокруг 1.65v

void push_value(kiss_fft_cpx buffer[FFT_SIZE], int16_t value) {
  for (int i = 0; i < FFT_SIZE - 1; i++) {
    buffer[i].r = buffer[i + 1].r;
  }
  buffer[FFT_SIZE - 1].r = value;
}

kiss_fft_scalar adc_to_kiss_fft_scalar(uint16_t value) {
  return (value - OFFSET) * 16;
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

  uint16_t another_value;
  int i = 0;

  // Инициализация всей аппаратной части
  init_gpio();
  init_adc();
  init_pwm();

  blink(3, 100, 50);

  start_conversion();

  blink(2, 50, 50);

  fill_fft_buffer(in);

  while (1) {
    another_value = adc_to_kiss_fft_scalar(read_adc());
    /*printf("Value = %d\n", another_value);*/
    push_value(in, another_value);

    kiss_fft(cfg, in, out);
    calculate_bands(out, &bands);
    /*blink(1, bands.low * 10, 200);*/
    double red = bands.low/( (double)INT16_MAX/10 );
    double green = bands.mid/( (double)INT16_MAX/10 );
    double blue = bands.high/( (double)INT16_MAX/10 );
    /*red = 0.5;*/
    /*green = 0.5;*/
    /*blue = 0.5;*/
    if (i++ == FFT_SIZE) {
      i = 0;
      blink(1, 1000, 0);
    }

    set_led_color(red, green, blue);
  }

  return 0;
}
