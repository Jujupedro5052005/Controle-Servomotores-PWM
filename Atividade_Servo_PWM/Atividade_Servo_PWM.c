#include <stdio.h>
#include "pico/stdlib.h"
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "hardware/clocks.h" // for clock_get_hz
#include "hardware/pwm.h"
#include "hardware/adc.h"

#define LED_PIN 16            // LED on-board do Pico (25)
#define POT_PIN 26            // GPIO26 = ADC0

const float cf_TARGET_FREQ = 50.0f;
const float cf_TARGET_PERIOD = 1/cf_TARGET_FREQ;

/**
 * @brief Calculate clock divider and wrap value for desired PWM frequency
 *
 * @param fFrequency Desired PWM frequency (in Hz)
 * @param pfClkdiv Pointer to store calculated clock divider (range: 1.0 - 255.9375)
 * @param pu16Wrap Pointer to store calculated wrap value (TOP) (range: 1 - 65535)
 * @return true If valid clock divider and wrap value were found
 * @return false If frequency too low, can't generate using PWM
   */
bool bPwmCalculateFreq(float fFrequency, float *pfClkdiv, uint16_t *pu16Wrap)
{
    // Check for valid input
    if (fFrequency <= 0.0f || pfClkdiv == NULL || pu16Wrap == NULL) {
        return false;
    }

    // Get the system clock frequency for PWM (typically 125 MHz)
    uint32_t sys_clk_hz = clock_get_hz(clk_sys);  // Usually 125,000,000 Hz

    // Calculate initial divider = sys_clk / desired PWM frequency
    float fDivider = (float)sys_clk_hz / fFrequency;

    // Case 1: If divider fits within 16-bit counter range (65535)
    if (fDivider <= 65535.0f) {
        *pu16Wrap = (uint16_t)roundf(fDivider);  // Set TOP
        *pfClkdiv = 1.0f;                        // Use no additional divider
        return true;
    }

    // Case 2: Divider is too big, must scale down using clock divider
    float fClkDiv = fDivider / 65535.0f;

    // Clock divider must be between 1.0 and 255.9375 (8.4 fixed-point)
    if (fClkDiv <= 255.9375f) {
        *pu16Wrap = 65535;         // Set maximum TOP
        *pfClkdiv = fClkDiv;       // Set scaled clock divider
        return true;
    }

    // Frequency too low; can't represent with PWM limits
    return false;
}

/**
 * @brief Calculate the PWM level value to set the desired duty cycle
 *
 * @param fDutyCycle Desired duty cycle in percentage (0.0 to 1.0)
 * @param u16Wrap PWM wrap (TOP) value
 * @return uint16_t Value to pass to pwm_set_gpio_level()
 */
uint16_t u16PwmCalculateLevel(float fDutyCycle, uint16_t u16Wrap)
{
    // Clamp duty cycle to valid range (0.0% to 100.0%)
    if (fDutyCycle < 0.0f) fDutyCycle = 0.0f;
    if (fDutyCycle > 1.0f) fDutyCycle = 1.0f;

    // Convert duty cycle percentage to level (0 to wrap)
    return (uint16_t)((fDutyCycle * (float)u16Wrap));
}

volatile uint16_t vu16Wrap;

// Callback do repetidor de timer (chamado a cada 200 ms)
bool timer_cb(repeating_timer_t *t) {
    uint16_t raw = adc_read();
    float percent = raw / 4095.0f;
    uint16_t pwm_level = u16PwmCalculateLevel(percent, vu16Wrap);
    pwm_set_gpio_level(LED_PIN, pwm_level);
    printf("%u,%.3f,%u\n", raw, percent, pwm_level);
    return true;                       // Repetir indefinidamente
}

// Callback do repetidor de timer (chamado a cada 2000 ms)
float sequence[10] = {0.0, 45.0, 90.0, 135.0, 180.0, 135.0, 90.0, 45.0, 0.0};
float limits_ang[2] = {0.0, 180.0};
float limits_ms[2] = {1.0, 2.0};  // entre 5% e 10%
int cont = 0;
bool timer1_cb(repeating_timer_t *t) {
    float m = (limits_ms[1]-limits_ms[0])/(limits_ang[1]-limits_ang[0]);
    float period_ms = limits_ms[0] + m*(sequence[cont] - limits_ang[0]);
    float percent = period_ms/(100*cf_TARGET_PERIOD);

    uint16_t pwm_level = u16PwmCalculateLevel(percent, vu16Wrap);
    pwm_set_gpio_level(LED_PIN, pwm_level);
    printf("%u,%.3f,%u\n", vu16Wrap, percent, pwm_level);

    if(cont<9){
        cont++;
    } else{
        cont=0;
    }
    return true;                       // Repetir indefinidamente
}

int main1(void) {
    stdio_init_all();         // opcional, só se precisar de USB-serial

    /* 1. Seleciona a função PWM no pino */
    gpio_set_function(LED_PIN, GPIO_FUNC_PWM);

    /* 2. Descobre qual slice controla esse GPIO */
    uint slice = pwm_gpio_to_slice_num(LED_PIN);

    /* 3. Cria configuração padrão e ajusta frequência/resolução          *
     *    Frequência alvo: 50 Hz                                         */

    float fClkdiv;
    if (bPwmCalculateFreq(cf_TARGET_FREQ, &fClkdiv, (uint16_t *)(&vu16Wrap))) {
        printf("PWM Settings: clkdiv = %.4f, wrap = %u\n", fClkdiv, vu16Wrap);
    } else {
        printf("Unable to calculate PWM settings for %.2f Hz\n", cf_TARGET_FREQ);
        fClkdiv = 1.00f;
        vu16Wrap = 255;
    }

    pwm_config cfg = pwm_get_default_config();
    pwm_config_set_clkdiv(&cfg, fClkdiv);
    pwm_config_set_wrap  (&cfg, vu16Wrap);

    /* 4. Inicializa o slice e já o habilita */
    pwm_init(slice, &cfg, true);

    /* 5. Configura Timer de 2000ms */
    repeating_timer_t timer1;
    add_repeating_timer_ms(2000, timer1_cb, NULL, &timer1);

    /* 6. Loop principal vazio */
    while (true) {
        tight_loop_contents();
    }
}

int main(void) {

    main1();

    /*
    stdio_init_all();         // opcional, só se precisar de USB-serial

    // 1. Seleciona a função PWM no pino 
    gpio_set_function(LED_PIN, GPIO_FUNC_PWM);

    // 2. Descobre qual slice controla esse GPIO 
    uint slice = pwm_gpio_to_slice_num(LED_PIN);

    // 3. Cria configuração padrão e ajusta frequência/resolução         
    //    Frequência alvo: 50 Hz                                         

    const float cf_TARGET_FREQ = 50.0f;
    float fClkdiv;
    if (bPwmCalculateFreq(cf_TARGET_FREQ, &fClkdiv, (uint16_t *)(&vu16Wrap))) {
        printf("PWM Settings: clkdiv = %.4f, wrap = %u\n", fClkdiv, vu16Wrap);
    } else {
        printf("Unable to calculate PWM settings for %.2f Hz\n", cf_TARGET_FREQ);
        fClkdiv = 1.00f;
        vu16Wrap = 255;
    }

    pwm_config cfg = pwm_get_default_config();
    pwm_config_set_clkdiv(&cfg, fClkdiv);
    pwm_config_set_wrap  (&cfg, vu16Wrap);

    // 4. Inicializa o slice e já o habilita
    pwm_init(slice, &cfg, true);

    // 5. Configura ADC
    adc_init();
    adc_gpio_init(POT_PIN);
    adc_select_input(0);

    // 6. Configura Timer de 20ms
    repeating_timer_t timer;
    add_repeating_timer_ms(20, timer_cb, NULL, &timer);

    // 5. Loop principal vazio 
    while (true) {
        tight_loop_contents();
    }
    */
}
