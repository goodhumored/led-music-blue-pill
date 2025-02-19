#ifndef FREQ_ANALYSER_H
#define FREQ_ANALYSER_H
#include "kiss_fft.h"
#include <stdint.h>

#ifndef FFT_SIZE
#define FFT_SIZE 512
#endif
#define SAMPLE_RATE 47619
#define BIN_WIDTH (SAMPLE_RATE / FFT_SIZE) 

// Границы частот (настройте под свою задачу)
#define LOW_FREQ_MAX 300  // Верхняя граница низких частот (Гц)
#define MID_FREQ_MAX 3000 // Верхняя граница средних частот (Гц)

// Рассчет индексов бинов
#define BIN(freq) ((uint16_t)((freq) / BIN_WIDTH))

// Диапазоны бинов (ceil для верхних границ)
#define START_LOW 1 // Игнорируем DC (бин 0)
#define END_LOW BIN(LOW_FREQ_MAX)+1
#define START_MID (END_LOW + 1)
#define END_MID BIN(MID_FREQ_MAX)
#define START_HIGH (END_MID + 1)
#define END_HIGH (FFT_SIZE / 2) // Теорема Найквиста (23.8 кГц)

typedef struct {
    uint16_t low;
    uint16_t mid;
    uint16_t high;
} FrequencyBands;

void calculate_bands(const int16_t *bins, FrequencyBands* bands, int16_t output_max);
int16_t bin_value(kiss_fft_cpx *buffer, int bin);
void calculate_bin_amps(kiss_fft_cpx *buffer, int16_t *bins);
#endif
