#include <gtest/gtest.h>
extern "C" {
    #include "adc-to-fft.h"
}

TEST(ADCConverterTest, OffsetValue) {
    EXPECT_EQ(adc_to_kiss_fft_scalar(OFFSET), 0);
}

TEST(ADCConverterTest, MIN) {
    EXPECT_EQ(adc_to_kiss_fft_scalar(0), -32768);
}

TEST(ADCConverterTest, MAX) {
    EXPECT_EQ(adc_to_kiss_fft_scalar(4095), 32752);
}

TEST(ADCConverterTest, OFFSET_PLUS) {
    EXPECT_EQ(adc_to_kiss_fft_scalar(OFFSET+10), 160);
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
