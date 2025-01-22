#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/timer.h>
#include <stdint.h>

#define PWM_PIN GPIO8   // PA8 для ШИМ
#define ADC_PIN GPIO0   // PA0 для ADC (микрофон)

void gpio_setup(void)
{
    /* Включаем тактирование для GPIO */
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_GPIOC);

    /* Настроим PA8 как альтернативный функционал для ШИМ */
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, PWM_PIN);

    /* Настроим PA0 как аналоговый вход для микрофона */
    gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_ANALOG, ADC_PIN);
}

void adc_setup(void)
{
    /* Включаем тактирование для ADC1 */
    rcc_periph_clock_enable(RCC_ADC1);

    /* Настроим ADC */
    adc_power_on(ADC1);
    adc_set_regular_sequence(ADC1, 1, (uint8_t[]){0});  // Один канал, PA0
    adc_set_sample_time_on_all_channels(ADC1, ADC_SMPR_SMP_55DOT5CYC);  // Длительное время выборки
    adc_enable_scan_mode(ADC1);  // Для сканирования нескольких каналов, если необходимо
    adc_power_on(ADC1);
}

uint16_t read_adc(void)
{
    /* Запускаем преобразование */
    adc_start_conversion_regular(ADC1);

    /* Ожидаем завершения преобразования */
    while (!(ADC1_SR & ADC_SR_EOC)) {
        // Ждем, пока не завершится преобразование
    }

    /* Читаем результат */
    return adc_read_regular(ADC1);
}

void pwm_setup(void)
{
    /* Включаем тактирование таймера 1 (для ШИМ) */
    rcc_periph_clock_enable(RCC_TIM1);

    /* Настроим таймер для ШИМ на PA8 */
    timer_set_prescaler(TIM1, 72); // Таймер с предделителем 72 (для 1 МГц)
    timer_set_period(TIM1, 1000);   // Период 1000 (для 1 кГц)
    timer_enable_oc_output(TIM1, TIM_OC1);
    timer_set_oc_mode(TIM1, TIM_OC1, TIM_OCM_PWM1);
    timer_enable_counter(TIM1);
}

int main(void)
{
    /* Настроим порты, ADC и ШИМ */
    gpio_setup();
    adc_setup();
    pwm_setup();

    uint16_t adc_value;
    uint32_t pwm_duty_cycle;
    uint8_t i = 0;

    while (1) {
        i += 1;
        i = i % 1000;
        /* Считываем данные с ADC */
        /*adc_value = read_adc();*/

        /* Преобразуем значение ADC в значение ШИМ */
        pwm_duty_cycle = (i * 1000) / 4095; // Преобразуем в диапазон 0-1000

        /* Устанавливаем коэффициент заполнения ШИМ в зависимости от уровня звука */
        timer_set_oc_value(TIM1, TIM_OC1, pwm_duty_cycle);

        // Можно добавить задержку, если необходимо
    }

    return 0;
}
