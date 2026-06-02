#ifndef GPIO_H_
#define GPIO_H_

#include <stdint.h>

void GPIO_Piano_Init(void);

uint16_t GPIO_Buttons_ReadRawPressedMask(void);

void GPIO_Buzzer_Write(uint8_t channel, uint8_t value);
void GPIO_Buzzer_Toggle(uint8_t channel);
void GPIO_Buzzers_OffAll(void);

#endif /* GPIO_H_ */
