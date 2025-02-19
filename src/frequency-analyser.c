#include "frequency-analyser.h"

void calculate_bands(const kiss_fft_cpx* fft_output, FrequencyBands* bands) {
    uint32_t sum_low = 0, sum_mid = 0, sum_high = 0;

    printf("%d-%d; %d-%d; %d-%d\n", START_LOW, END_LOW, START_MID, END_MID, START_HIGH, END_HIGH);
    // Низкие частоты
    for(uint16_t i = START_LOW; i <= END_LOW; i++) {
        int32_t re = fft_output[i].r;
        int32_t im = fft_output[i].i;
        sum_low += (uint32_t)sqrtf(re*re + im*im);
    }

    // Средние частоты
    for(uint16_t i = START_MID; i <= END_MID; i++) {
        int32_t re = fft_output[i].r;
        int32_t im = fft_output[i].i;
        sum_mid += (uint32_t)sqrtf(re*re + im*im);
    }

    // Высокие частоты
    for(uint16_t i = START_HIGH; i <= END_HIGH; i++) {
        int32_t re = fft_output[i].r;
        int32_t im = fft_output[i].i;
        sum_high += (uint32_t)sqrtf(re*re + im*im);
    }

    uint32_t sum = sum_low + sum_mid + sum_high;
    printf("sl: %d, sm: %d, sh: %d; sum: %d\n", sum_low, sum_mid, sum_high, sum);
    // Усреднение с приведением к int16_t
    bands->low  = (float)sum_low/sum;
    bands->mid  = (float)sum_mid/sum;
    bands->high = (float)sum_high/sum;
}
