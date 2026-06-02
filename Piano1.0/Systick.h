#ifndef SYSTICK_H_
#define SYSTICK_H_

#include <stdint.h>

extern volatile uint32_t systick_ms;

void SysTick_Init(void);
uint32_t SysTick_GetMs(void);

#endif /* SYSTICK_H_ */
