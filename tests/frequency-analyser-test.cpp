#include "kiss_fft.h"
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <gtest/gtest.h>
#include <stdint.h>

extern "C" {
#include "frequency-analyser.h"
}

const int THRESHOLD = FFT_SIZE / 200;

void fill_zeros(kiss_fft_cpx *buffer) {
  for (int i = 0; i < FFT_SIZE; i++) {
    buffer[i].r = 0;
    buffer[i].i = 0.0;
  }
}

void print(kiss_fft_cpx *buffer) {
  for (int i = 0; i < FFT_SIZE; i++) {
    double val = sqrt(buffer[i + 1].r * buffer[i + 1].r +
                      buffer[i + 1].i * buffer[i + 1].i);
    /*printf("(%d, %d) %f = %f\n", buffer[i].r, buffer[i].i, val,*/
    /*       val / INT16_MAX);*/
    buffer[i].i = 0.0;
  }
}

double bin_value(kiss_fft_cpx *buffer, int bin) {
  return sqrt(buffer[bin].r * buffer[bin].r + buffer[bin].i * buffer[bin].i) /
         INT16_MAX;
}

void add_sinewave(kiss_fft_cpx *buffer, int16_t freq, int amplitude) {
  for (int i = 0; i < FFT_SIZE; i++) {
    double t = (double)i / SAMPLE_RATE;
    buffer[i].r += amplitude * sin(2.0 * M_PI * freq * t);
  }
}

int get_most_significant_bin(kiss_fft_cpx *buffer) {
  double max = 0;
  int maxi = 0;
  double v = 0;
  for (int i = 1; i < FFT_SIZE / 2; i++) {
    v = bin_value(buffer, i);
    if (v > max) {
      max = v;
      maxi = i;
    }
  }
  return maxi;
}

void get_peak_bins_sorted_by_amplitude(kiss_fft_cpx *buffer, int *sorted_bins,
                                       int &num_peaks) {
  num_peaks = 0;
  double bin_values[FFT_SIZE / 2];

  // Precompute bin values
  for (int i = 1; i < FFT_SIZE / 2; i++) {
    bin_values[i] = bin_value(buffer, i);
  }

  // Adaptive neighborhood size
  int N = FFT_SIZE / 100; // Пример: 2% от FFT_SIZE
  if (N < 1)
    N = 1;

  // Compute average amplitude to set a threshold
  double avg_amplitude = 0;
  for (int i = 1; i < FFT_SIZE / 2; i++) {
    avg_amplitude += bin_values[i];
  }
  avg_amplitude /= ((float)FFT_SIZE / 2);

  // Find peaks
  for (int i = 1; i < FFT_SIZE / 2 - 1; i++) {
    bool is_peak = true;
    for (int j = 1; j <= N; j++) {
      if (i - j > 0 && bin_values[i] <= bin_values[i - j]) {
        is_peak = false;
        break;
      }
      if (i + j < FFT_SIZE / 2 && bin_values[i] <= bin_values[i + j]) {
        is_peak = false;
        break;
      }
    }
    if (is_peak && bin_values[i] > avg_amplitude * 1.5) { // Фильтр по порогу
      sorted_bins[num_peaks++] = i;
    }
  }
}

void plot_with_gnuplot(const double data[], size_t size,
                       const std::string &name) {
  std::ofstream file(name + ".txt");
  if (!file) {
    std::cerr << "Error: Unable to open file for writing\n";
    return;
  }

  for (size_t i = 0; i < size; i++) {
    file << i << " " << data[i] << "\n"; // X = index, Y = value
  }
  file.close();

  std::string command = "gnuplot -p -e \"plot '" + name + ".txt" +
                        "' with lines title '" + name + "'\"";
  std::system(command.c_str());
}

void draw(kiss_fft_cpx *buffer, std::string name) {
  double amps[FFT_SIZE];
  for (int i = 0; i < FFT_SIZE; i++) {
    amps[i] = buffer[i].r;
  }
  plot_with_gnuplot(amps, FFT_SIZE, name);
}

void draw_fac(kiss_fft_cpx *buffer, std::string name) {
  double amps[FFT_SIZE - 1];
  for (int i = 1; i < FFT_SIZE / 2; i++) {
    amps[i - 1] = bin_value(buffer, i);
  }
  plot_with_gnuplot(amps, FFT_SIZE / 2 - 1, name);
}

class FrequencyAnalisys : public testing::Test {
protected:
  void SetUp() override { cfg = kiss_fft_alloc(FFT_SIZE, 0, NULL, NULL); }

  void TearDown() override {
    free(cfg);
    fill_zeros(in);
  }

  kiss_fft_cfg cfg;
  kiss_fft_cpx in[FFT_SIZE], out[FFT_SIZE];
  FrequencyBands bands;
};

TEST_F(FrequencyAnalisys, FreqPeak) {
  int freq = 6000;

  add_sinewave(in, freq, 1 * INT16_MAX);
  kiss_fft(cfg, in, out);
#ifdef PLOTS
  draw(in, "in1");
  draw_fac(out, "out1");
#endif
  int expected_bin = ceil(freq / BIN_WIDTH);
  int most_significant_bin = get_most_significant_bin(out);
  EXPECT_LT(abs(most_significant_bin - expected_bin), THRESHOLD);
}

TEST_F(FrequencyAnalisys, TWO_FREQS) {
  int freq1 = 200;
  int freq2 = 5000;

  fill_zeros(in);
  add_sinewave(in, freq1, INT16_MAX / 2);
  add_sinewave(in, freq2, INT16_MAX / 2);
  kiss_fft(cfg, in, out);
#ifdef PLOTS
  draw(in, "in2");
  draw_fac(out, "out2");
#endif

  int expected_bin1 = ceil(freq1 / BIN_WIDTH);
  int expected_bin2 = ceil(freq2 / BIN_WIDTH);
  int bins_sorted[FFT_SIZE / 2];
  int peaks_count = 0;
  get_peak_bins_sorted_by_amplitude(out, bins_sorted, peaks_count);
  EXPECT_LT(abs(bins_sorted[0] - expected_bin1), THRESHOLD);
  EXPECT_LT(abs(bins_sorted[1] - expected_bin2), THRESHOLD);
}

TEST_F(FrequencyAnalisys, FIVE_FREQS) {
  int freq1 = 200;
  int freq2 = 1000;
  int freq3 = 5000;
  int freq4 = 15000;
  int freq5 = 20000;

  fill_zeros(in);
  add_sinewave(in, freq1, INT16_MAX / 5);
  add_sinewave(in, freq2, INT16_MAX / 5);
  add_sinewave(in, freq3, INT16_MAX / 5);
  add_sinewave(in, freq4, INT16_MAX / 5);
  add_sinewave(in, freq5, INT16_MAX / 5);
  kiss_fft(cfg, in, out);
#ifdef PLOTS
  draw(in, "in3");
  draw_fac(out, "out3");
#endif

  int expected_bin1 = ceil(freq1 / BIN_WIDTH);
  int expected_bin2 = ceil(freq2 / BIN_WIDTH);
  int expected_bin3 = ceil(freq3 / BIN_WIDTH);
  int expected_bin4 = ceil(freq4 / BIN_WIDTH);
  int expected_bin5 = ceil(freq5 / BIN_WIDTH);
  int bins_sorted[FFT_SIZE / 2];
  int peaks_count = 0;
  get_peak_bins_sorted_by_amplitude(out, bins_sorted, peaks_count);
  EXPECT_LT(abs(bins_sorted[0] - expected_bin1), THRESHOLD);
  EXPECT_LT(abs(bins_sorted[1] - expected_bin2), THRESHOLD);
  EXPECT_LT(abs(bins_sorted[2] - expected_bin3), THRESHOLD);
  EXPECT_LT(abs(bins_sorted[3] - expected_bin4), THRESHOLD);
  EXPECT_LT(abs(bins_sorted[4] - expected_bin5), THRESHOLD);
}

TEST_F(FrequencyAnalisys, BANDS__HIGH_FREQ__HIGH_AMP) {
  int freq = 6000;

  add_sinewave(in, freq, 1 * INT16_MAX);
  kiss_fft(cfg, in, out);

  calculate_bands(out, &bands, 1000);
  EXPECT_LT(bands.low, 100);
  EXPECT_LT(bands.mid, 100);
  EXPECT_GT(bands.high, 900);
}

TEST_F(FrequencyAnalisys, BANDS__MID_FREQ__LOW_AMP) {
  int freq = 2000;

  add_sinewave(in, freq, 0.1 * INT16_MAX);
  kiss_fft(cfg, in, out);

  calculate_bands(out, &bands, 1000);
  EXPECT_LT(bands.low, 1000 * 0.1);
  EXPECT_GT(bands.mid, 1000 * 0.9);
  EXPECT_LT(bands.high, 1000 * 0.1);
}

TEST_F(FrequencyAnalisys, BANDS__LOW_FREQ__HIGH_AMP) {
  int freq = 300;

  add_sinewave(in, freq, 1 * INT16_MAX);
  kiss_fft(cfg, in, out);

  calculate_bands(out, &bands, 1000);
  EXPECT_EQ(bands.low, 1000);
  EXPECT_LT(bands.mid, 1000 * 0.5);
  EXPECT_LT(bands.high, 1000 * 0.2);
}

TEST_F(FrequencyAnalisys, BANDS__LOW_HIGH_FREQ__HIGH_AMP) {
  int freq1 = 300;
  int freq2 = 6000;

  add_sinewave(in, freq1, 0.5 * INT16_MAX);
  add_sinewave(in, freq2, 0.5 * INT16_MAX);
  kiss_fft(cfg, in, out);

  calculate_bands(out, &bands, 1000);
  EXPECT_EQ(bands.low, 1000);
  EXPECT_LT(bands.mid, 1000 * 0.5);
  EXPECT_GT(bands.high, 1000 * 0.7);
}

TEST_F(FrequencyAnalisys, BANDS__LOW_MID_FREQ__LOW_AMP) {
  int freq1 = 300;
  int freq2 = 2000;

  add_sinewave(in, freq1, 0.05 * INT16_MAX);
  add_sinewave(in, freq2, 0.05 * INT16_MAX);
  kiss_fft(cfg, in, out);

  calculate_bands(out, &bands, 1000);
  EXPECT_EQ(bands.low, 1000);
  EXPECT_GT(bands.mid, 1000 * 0.8);
  EXPECT_LT(bands.high, 1000 * 0.2);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
