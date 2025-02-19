#include "frequency-analyser.h"
#include <stdint.h>

void calculate_bin_amps(kiss_fft_cpx *buffer, int16_t *bins) {
  for(int i = 1; i < FFT_SIZE/2; i++) {
    bins[i] = bin_value(buffer, i);
  }
}

inline int16_t bin_value(kiss_fft_cpx *buffer, int bin) {
  return (int16_t)sqrt(buffer[bin].r * buffer[bin].r + buffer[bin].i * buffer[bin].i);
}

void calculate_bands(const int16_t *bins, FrequencyBands *bands, int16_t output_max) {
  uint32_t sum_low = 0, sum_mid = 0, sum_high = 0;
  for (uint16_t i = START_LOW; i <= END_LOW; i++) {
    sum_low += bins[i];
  }
  for (uint16_t i = START_MID; i <= END_MID; i++) {
    sum_mid += bins[i];
  }
  for (uint16_t i = START_HIGH; i <= END_HIGH; i++) {
    sum_high += bins[i];
  }

  uint32_t max_sum = sum_low;
  /*if (sum_mid > max_sum)*/
  /*  max_sum = sum_mid;*/
  /*if (sum_high > max_sum)*/
  /*  max_sum = sum_high;*/
  max_sum += sum_mid + sum_high;

  bands->low = sum_low * output_max / max_sum;
  bands->mid = sum_mid * output_max / max_sum;
  bands->high = sum_high * output_max / max_sum;
}
