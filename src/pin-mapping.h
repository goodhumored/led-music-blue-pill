#ifndef PIN_MAPPING_H
#define PIN_MAPPING_H

//          ╭─────────────────────────────────────────────────────────╮
//          │                          Пины                           │
//          ╰─────────────────────────────────────────────────────────╯
//  ────────────────────────────── Микрофон ───────────────────────────
#define MIC_ADC ADC1     // Первый АЦП
#define MIC_PIN GPIO0    // НУЛЕВОЙ ПИН
#define MIC_RCC RCC_ADC1 // НОМЕР ПЕРЕФЕРИЙНОГО ЧЕГО-ТО ТАМ ДЛЯ ТАКТИРОВАНИЯ
#define MIC_CHANNEL 0    // КАНАЛ МИКРОФОНА (0 потому что A0)

//  ──────────────────────────────── Лента ────────────────────────────────
#define RED_PIN GPIO7
#define GREEN_PIN GPIO6
#define BLUE_PIN GPIO9

//  ─────────────────────── Каналы таймеров цветов ────────────────────
#define RED_OC TIM_OC2
#define GREEN_OC TIM_OC1
#define BLUE_OC TIM_OC4

//  ─────────────────────────── Таймеры цветов ────────────────────────
#define RED_TIM TIM4
#define GREEN_TIM TIM4
#define BLUE_TIM TIM4

/**
 * Функция инициализации и настройки пинов
 */
void init_gpio(void);

#endif
