#ifndef OLED_H_
#define OLED_H_

#include <stdint.h>

/**************************************************************************************************
 *  Variables de diagnostico
 */

extern volatile uint8_t oled_ready_debug;

/**************************************************************************************************
 *  Prototipos
 */

uint8_t OLED_Init(void);
uint8_t OLED_IsReady(void);

void OLED_Clear(void);
void OLED_SetCursor(uint8_t page, uint8_t col);

void OLED_Print(const char *text);
void OLED_PrintUInt(uint32_t number);
void OLED_PrintNotes(uint16_t notes);

void OLED_ShowPianoStatus(const char *chord,
                          uint16_t notes,
                          uint8_t volume_percent,
                          uint8_t octave);

#endif /* OLED_H_ */
