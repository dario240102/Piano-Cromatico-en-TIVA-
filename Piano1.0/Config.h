#ifndef CONFIG_H_
#define CONFIG_H_

#include <stdint.h>

/**************************************************************************************************
 *  Configuracion general
 */

#define SYS_CLOCK_HZ                    120000000

/**************************************************************************************************
 *  Audio
 */

#define AUDIO_TICK_US                   25
#define AUDIO_TICK_HZ                   (1000000 / AUDIO_TICK_US)

/**************************************************************************************************
 *  SysTick
 */

#define SYSTICK_TICK_HZ                 1000
#define BUTTON_DEBOUNCE_MS              20

/**************************************************************************************************
 *  Canales fisicos
 */

#define NUM_CHANNELS                    12

#define CH_0                            0
#define CH_1                            1
#define CH_2                            2
#define CH_3                            3
#define CH_4                            4
#define CH_5                            5
#define CH_6                            6
#define CH_7                            7
#define CH_8                            8
#define CH_9                            9
#define CH_10                           10
#define CH_11                           11

/**************************************************************************************************
 *  Frecuencias aproximadas de la octava 4
 */

#define FREQ_DO4                        262
#define FREQ_DOS4                       277
#define FREQ_RE4                        294
#define FREQ_RES4                       311
#define FREQ_MI4                        330
#define FREQ_FA4                        349
#define FREQ_FAS4                       370
#define FREQ_SOL4                       392
#define FREQ_SOLS4                      415
#define FREQ_LA4                        440
#define FREQ_LAS4                       466
#define FREQ_SI4                        494

/**************************************************************************************************
 *  Orientacion del teclado
 *
 *  1 = teclado invertido por software.
 */

#define KEYBOARD_REVERSED               1

/**************************************************************************************************
 *  Melodias secretas
 */

#define SECRET_HOLD_MS                  700

/**************************************************************************************************
 *  OLED SSD1306 I2C
 *
 *  Tu modulo indica 0x78 / 0x7A como direccion de 8 bits.
 *  En el codigo se usa direccion de 7 bits:
 *
 *      0x78 >> 1 = 0x3C
 *      0x7A >> 1 = 0x3D
 */

#define OLED_ENABLE                     1
#define OLED_I2C_ADDRESS                0x3C
#define OLED_REFRESH_MS                 100

#define OLED_WIDTH                      128
#define OLED_HEIGHT                     32
#define OLED_PAGES                      4

/**************************************************************************************************
 *  I2C
 */

#define I2C_SPEED_HZ                    400000

/**************************************************************************************************
 *  Valores provisionales de interfaz
 */

#define DEFAULT_VOLUME_PERCENT          75
#define DEFAULT_OCTAVE                  4

/**************************************************************************************************
 *  Joystick ADC por pasos
 */
#define ADC_ENABLE                      1
#define ADC_JOYSTICK_SAMPLE_MS          50

#define ADC_MAX_VALUE                   4095

/**************************************************************************************************
 *  Umbrales eje X
 *
 *  X controla octava:
 *      izquierda -> baja octava
 *      derecha   -> sube octava
 */

#define JOYSTICK_X_LOW                  1400
#define JOYSTICK_X_HIGH                 2700

#define JOYSTICK_X_RETURN_LOW           1700
#define JOYSTICK_X_RETURN_HIGH          2400

/**************************************************************************************************
 *  Umbrales eje Y
 *
 *  Y controla volumen:
 *      abajo  -> baja volumen
 *      arriba -> sube volumen
 */

#define JOYSTICK_Y_LOW                  1400
#define JOYSTICK_Y_HIGH                 2700

#define JOYSTICK_Y_RETURN_LOW           1700
#define JOYSTICK_Y_RETURN_HIGH          2400

#define VOLUME_STEP_PERCENT             5

/**************************************************************************************************
 *  Limites de octava y volumen
 */

#define MIN_OCTAVE                      3
#define DEFAULT_OCTAVE                  4
#define MAX_OCTAVE                      5

#define DEFAULT_VOLUME_PERCENT          75

/**************************************************************************************************
 *  UART Bluetooth HC-05
 */

#define UART_ENABLE                     1

#define UART_BAUD_RATE                  9600
#define UART_CLOCK_HZ                   16000000

#endif /* CONFIG_H_ */
