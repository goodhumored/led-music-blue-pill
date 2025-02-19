#include "kiss_fft.h"
#include "adc-to-fft.h"

kiss_fft_scalar adc_to_kiss_fft_scalar(uint32_t value) {
  return ((int16_t)(value) - OFFSET)*16;
}
