#ifndef USART_H
#define USART_H
#include <stdint.h>

void send_fft_data(int16_t *bins, uint16_t size);
int _write(int file, char *ptr, int len);
void usart_setup(void);

#endif
