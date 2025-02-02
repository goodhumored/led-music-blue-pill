#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H
#include <stdint.h>

enum ColorChannel { RED, GREEN, BLUE };

/**
 * Функция устанавливающая цвет конкретного канала
 * @param c - color channel enum (RED, GREEN, BLUE)
 * @param value - значение для канала в диапазоне от 0 до 1
 */
void set_chanel_color(enum ColorChannel c, double value);

/**
 * Функция устанавливающая цвет всей ленты
 * @param r - значение для красного канала в диапазоне от 0 до 1
 * @param g - значение для зелёного канала в диапазоне от 0 до 1
 * @param b - значение для синего канала в диапазоне от 0 до 1
 */
void set_led_color(double red, double green, double blue);
#endif
