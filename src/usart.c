#include "usart.h"
#include <errno.h>
#ifndef UNIT_TEST
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/usart.h>
#include <stdio.h>
#endif

void usart_setup(void) {
#ifndef UNIT_TEST
  usart_set_baudrate(USART1, 115200);
  usart_set_databits(USART1, 8);
  usart_set_stopbits(USART1, USART_STOPBITS_1);
  usart_set_parity(USART1, USART_PARITY_NONE);
  usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);
  usart_set_mode(USART1, USART_MODE_TX);

  usart_enable(USART1);
#endif
}

void send_fft_data(int16_t *bins, uint16_t size) {
#ifndef UNIT_TEST
  for (uint16_t i = 1; i < size; i++) {
    char buffer[10];
    int len = snprintf(buffer, sizeof(buffer), "%d ", bins[i]);
    _write(1, buffer, len);
  }
  _write(1, "\r\n", 2); // Новый кадр данных
#endif
}

#ifndef UNIT_TEST
int _write(int file, char *ptr, int len) {
  int i;

  if (file == 1) {
    for (i = 0; i < len; i++)
      usart_send_blocking(USART1, ptr[i]);
    return i;
  }

  errno = EIO;
  return -1;
}
#endif
