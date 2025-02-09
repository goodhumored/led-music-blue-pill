#include "frequency-analyser.h"

void calculate_bands(const kiss_fft_cpx* fft_output, FrequencyBands* bands) {
    uint32_t sum_low = 0, sum_mid = 0, sum_high = 0;
    uint16_t count_low = 0, count_mid = 0, count_high = 0;

    // Низкие частоты
    for(uint16_t i = START_LOW; i <= END_LOW; i++) {
        int32_t re = fft_output[i].r;
        int32_t im = fft_output[i].i;
        sum_low += (uint32_t)sqrtf(re*re + im*im);
        count_low++;
    }

    // Средние частоты
    for(uint16_t i = START_MID; i <= END_MID; i++) {
        int32_t re = fft_output[i].r;
        int32_t im = fft_output[i].i;
        sum_mid += (uint32_t)sqrtf(re*re + im*im);
        count_mid++;
    }

    // Высокие частоты
    for(uint16_t i = START_HIGH; i <= END_HIGH; i++) {
        int32_t re = fft_output[i].r;
        int32_t im = fft_output[i].i;
        sum_high += (uint32_t)sqrtf(re*re + im*im);
        count_high++;
    }

    // Усреднение с приведением к int16_t
    bands->low  = (count_low)  ? (int16_t)(sum_low / count_low)  : 0;
    bands->mid  = (count_mid)  ? (int16_t)(sum_mid / count_mid)  : 0;
    bands->high = (count_high) ? (int16_t)(sum_high / count_high) : 0;
}
