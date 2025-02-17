#ifndef ADC_TO_FFT_H
#define ADC_TO_FFT_H
#include "kiss_fft.h"


kiss_fft_scalar adc_to_kiss_fft_scalar(uint32_t value);

#endif
