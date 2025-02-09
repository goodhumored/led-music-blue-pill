#ifndef ADC_H
#define ADC_H
#include <stdint.h>
/**
 * Функция инициализации АЦП модуля
 */
void init_adc(void);
/**
 * Функция запуска преобразований АЦП
 */
void start_conversion(void);
/**
 * Функция получения очередного значения АЦП
 * @returns значение полученное с АЦП в диапазоне от 0-1
 */
uint16_t read_adc(void);
#endif
