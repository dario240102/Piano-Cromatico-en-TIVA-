#ifndef CLOCK_H_
#define CLOCK_H_

#include <stdint.h>

extern volatile uint32_t system_clock_debug;
extern volatile uint32_t pll_lock_debug;

void Clock_Init120MHz(void);

#endif /* CLOCK_H_ */
