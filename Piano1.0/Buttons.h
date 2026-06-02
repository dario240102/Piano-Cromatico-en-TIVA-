#ifndef BUTTONS_H_
#define BUTTONS_H_

#include <stdint.h>

/**************************************************************************************************
 *  Variables de depuracion
 */

extern volatile uint16_t buttons_raw_debug;
extern volatile uint16_t buttons_stable_debug;
extern volatile uint16_t buttons_last_raw_debug;
extern volatile uint16_t buttons_debounce_counter_debug;

/**************************************************************************************************
 *  Prototipos
 */

void Buttons_Init(void);
void Buttons_UpdateDebounce(void);

uint16_t Buttons_GetStableMask(void);

#endif /* BUTTONS_H_ */
