#include "kiss_fft.h"

#define OFFSET                                                                 \
  0 // половина от 4095 (макс значение АЦП)  это если в идеале сигнал
    // крутится вокруг 1.65v

kiss_fft_scalar adc_to_kiss_fft_scalar(uint32_t value) {
  return ((int16_t)(value) - OFFSET)*16;
}
