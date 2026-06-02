#ifndef ADC_H_
#define ADC_H_

#include <stdint.h>

/**************************************************************************************************
 *  Variables de depuracion
 */

extern volatile uint16_t adc_x_debug;
extern volatile uint16_t adc_y_debug;

extern volatile uint8_t adc_volume_debug;
extern volatile uint8_t adc_octave_debug;

extern volatile uint32_t adc_isr_counter_debug;
extern volatile uint32_t adc_start_counter_debug;

extern volatile uint32_t adc_ris_debug;
extern volatile uint32_t adc_actss_debug;
extern volatile uint32_t adc_ssfstat_debug;

/**************************************************************************************************
 *  Prototipos
 */

void ADC_Joystick_Init(void);
void ADC_Joystick_Tick1ms(void);

uint8_t ADC_Joystick_GetVolumePercent(void);
uint8_t ADC_Joystick_GetOctave(void);

void ADC0SS1_Handler(void);

#endif /* ADC_H_ */
