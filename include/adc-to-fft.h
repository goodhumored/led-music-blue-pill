#ifndef ADC_TO_FFT_H
#define ADC_TO_FFT_H
#include "kiss_fft.h"

#define OFFSET                                                                 \
  2048 // половина от 4095 (макс значение АЦП)  это если в идеале сигнал
    // крутится вокруг 1.65v

kiss_fft_scalar adc_to_kiss_fft_scalar(uint32_t value);

#endif
