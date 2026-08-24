/*
 * ============================================================================
 * Controle de Servomotores por PWM - Raspberry Pi Pico / RP2040
 * ============================================================================
 *
 * Integrantes:
 *   João Pedro de Jesus Cândido Silva
 *   R.A. 23.01416-4
 *
 *   Erich Abreu Serafim
 *   R.A. 23.10022-2
 *
 * Repositório:
 *   https://github.com/Jujupedro5052005/Controle-Servomotores-PWM
 *
 * Plataforma:
 *   Raspberry Pi Pico / RP2040
 *
 * Linguagem:
 *   C/C++ utilizando Raspberry Pi Pico SDK
 *
 * Descrição:
 *   Implementação progressiva do controle de servomotores utilizando o
 *   periférico PWM do RP2040.
 *
 *   Nível 1:
 *     Controle automático de posição nas posições:
 *     0°, 45°, 90°, 135°, 180°, 135°, 90°, 45° e 0°.
 *
 *   Nível 2:
 *     Controle contínuo da posição de um servomotor através de um
 *     potenciômetro conectado ao ADC0 / GPIO26.
 *
 *   Nível 3:
 *     Controle de dois servomotores a partir de um único potenciômetro.
 *
 * Hardware:
 *   Servo PWM : GPIO0
 *   ADC0      : GPIO26
 *
 * Frequência PWM:
 *   Aproximadamente 50 Hz (período de 20 ms).
 *
 * Calibração experimental do servomotor:
 *   0°   -> 0,5 ms
 *   90°  -> 1,5 ms
 *   180° -> 2,5 ms
 *
 * ============================================================================
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "hardware/clocks.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"

// ==========================
// Level selection
// ==========================

#define LEVEL 2   // 1 = automatic sequence, 2 = potentiometer control

#if LEVEL < 1 || LEVEL > 3
#error "LEVEL must be 1, 2 or 3"
#endif

// ==========================
// GPIO configuration
// ==========================

#define SERVO_PIN 0
#define POT_PIN 26              // GPIO26 = ADC0

// ==========================
// PWM / Servo configuration
// ==========================

const float cf_TARGET_FREQ = 50.0f;
const float cf_TARGET_PERIOD = 1.0f / cf_TARGET_FREQ;

float limits_ang[2] = {0.0f, 180.0f};
float limits_ms[2] = {0.5f, 2.5f};   // Calibrated SG90 range

volatile uint16_t vu16Wrap;

// ==========================
// Level 2 configuration
// ==========================

#define ADC_MAX_VALUE 4095
#define ADC_NUM_SAMPLES 8
#define ANGLE_DEADBAND 1.0f
#define ADC_TIMER_MS 20

/**
 * @brief Calculate clock divider and wrap value for desired PWM frequency
 */
bool bPwmCalculateFreq(float fFrequency, float *pfClkdiv, uint16_t *pu16Wrap)
{
    if (fFrequency <= 0.0f || pfClkdiv == NULL || pu16Wrap == NULL)
        return false;

    uint32_t sys_clk_hz = clock_get_hz(clk_sys);
    float fDivider = (float)sys_clk_hz / fFrequency;

    if (fDivider <= 65535.0f) {
        *pu16Wrap = (uint16_t)roundf(fDivider);
        *pfClkdiv = 1.0f;
        return true;
    }

    float fClkDiv = fDivider / 65535.0f;

    if (fClkDiv <= 255.9375f) {
        *pu16Wrap = 65535;
        *pfClkdiv = fClkDiv;
        return true;
    }

    return false;
}

/**
 * @brief Convert duty cycle to PWM level
 */
uint16_t u16PwmCalculateLevel(float fDutyCycle, uint16_t u16Wrap)
{
    if (fDutyCycle < 0.0f) fDutyCycle = 0.0f;
    if (fDutyCycle > 1.0f) fDutyCycle = 1.0f;

    return (uint16_t)(fDutyCycle * (float)u16Wrap);
}

/**
 * @brief Set servo position in degrees
 */
void set_servo_angle(float angle)
{
    if (angle < limits_ang[0]) angle = limits_ang[0];
    if (angle > limits_ang[1]) angle = limits_ang[1];

    float m = (limits_ms[1] - limits_ms[0]) / (limits_ang[1] - limits_ang[0]);
    float pulse_ms = limits_ms[0] + m * (angle - limits_ang[0]);
    float duty = pulse_ms / (1000.0f * cf_TARGET_PERIOD);

    uint16_t pwm_level = u16PwmCalculateLevel(duty, vu16Wrap);
    pwm_set_gpio_level(SERVO_PIN, pwm_level);

    printf("angle=%.1f, pulse=%.3f ms, duty=%.2f%%, pwm=%u\n",
           angle, pulse_ms, duty * 100.0f, pwm_level);
}

/**
 * @brief Initialize servo PWM
 */
void servo_pwm_init(void)
{
    gpio_set_function(SERVO_PIN, GPIO_FUNC_PWM);

    uint slice = pwm_gpio_to_slice_num(SERVO_PIN);

    float fClkdiv = 125.0f;
    vu16Wrap = 20000;

    pwm_config cfg = pwm_get_default_config();
    pwm_config_set_clkdiv(&cfg, fClkdiv);
    pwm_config_set_wrap(&cfg, vu16Wrap);
    pwm_init(slice, &cfg, true);

    printf("PWM Settings: clkdiv=%.1f, wrap=%u, freq≈50Hz\n",
           fClkdiv, vu16Wrap);
}

// ============================================================
// LEVEL 1
// ============================================================

#if LEVEL == 1

float sequence[9] = {
    0.0f, 45.0f, 90.0f, 135.0f, 180.0f,
    135.0f, 90.0f, 45.0f, 0.0f
};

int cont = 0;

bool timer1_cb(repeating_timer_t *t)
{
    set_servo_angle(sequence[cont]);

    if (cont < 8)
        cont++;
    else
        cont = 0;

    return true;
}

void run_level_1(void)
{
    gpio_set_function(SERVO_PIN, GPIO_FUNC_PWM);

    uint slice = pwm_gpio_to_slice_num(SERVO_PIN);

    float fClkdiv = 125.0f;
    vu16Wrap = 20000;

    printf("PWM Settings: clkdiv = %.4f, wrap = %u\n",
           fClkdiv, vu16Wrap);

    pwm_config cfg = pwm_get_default_config();
    pwm_config_set_clkdiv(&cfg, fClkdiv);
    pwm_config_set_wrap(&cfg, vu16Wrap);

    pwm_init(slice, &cfg, true);

    set_servo_angle(sequence[0]);
    cont = 1;

    repeating_timer_t timer1;
    add_repeating_timer_ms(2000, timer1_cb, NULL, &timer1);

    while (true)
        tight_loop_contents();
}

#endif

// ============================================================
// LEVEL 2
// ============================================================

#if LEVEL == 2

uint16_t read_adc_average(void)
{
    uint32_t sum = 0;

    for (int i = 0; i < ADC_NUM_SAMPLES; i++)
        sum += adc_read();

    return (uint16_t)(sum / ADC_NUM_SAMPLES);
}

bool timer2_cb(repeating_timer_t *t)
{
    static float previous_angle = -1000.0f;

    uint16_t raw = read_adc_average();
    float percent = (float)raw / ADC_MAX_VALUE;
    float angle = limits_ang[0] + percent * (limits_ang[1] - limits_ang[0]);

    if (fabsf(angle - previous_angle) >= ANGLE_DEADBAND) {
        set_servo_angle(angle);
        printf("ADC=%u, percent=%.3f\n", raw, percent);
        previous_angle = angle;
    }

    return true;
}

void run_level_2(void)
{
    printf("\n=== LEVEL 2 - Potentiometer control ===\n");

    servo_pwm_init();

    adc_init();
    adc_gpio_init(POT_PIN);
    adc_select_input(0);

    set_servo_angle(90.0f);

    repeating_timer_t timer2;
    add_repeating_timer_ms(ADC_TIMER_MS, timer2_cb, NULL, &timer2);

    while (true)
        tight_loop_contents();
}

#endif

// ============================================================
// LEVEL 3
// ============================================================

#if LEVEL == 3

void run_level_3(void)
{
    printf("\n=== LEVEL 3 - Not implemented yet ===\n");

    while (true)
        tight_loop_contents();
}

#endif

// ============================================================
// MAIN
// ============================================================

int main(void)
{
    stdio_init_all();

#if LEVEL == 1
    run_level_1();
#elif LEVEL == 2
    run_level_2();
#elif LEVEL == 3
    run_level_3();
#endif

    return 0;
}