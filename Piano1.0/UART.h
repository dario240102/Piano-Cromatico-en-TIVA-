#ifndef UART_H_
#define UART_H_

#include <stdint.h>

/**************************************************************************************************
 *  Variables de depuracion
 */

extern volatile uint8_t uart_last_char_debug;
extern volatile uint32_t uart_rx_counter_debug;
extern volatile uint32_t uart_tx_counter_debug;

extern volatile uint8_t uart_status_request_debug;
extern volatile uint8_t uart_help_request_debug;
extern volatile uint8_t uart_melody_request_debug;
extern volatile uint8_t uart_stop_request_debug;

/**************************************************************************************************
 *  Prototipos
 */

void UART4_Init(void);

void UART4_SendChar(char c);
void UART4_SendString(const char *text);
void UART4_SendUInt(uint32_t number);
void UART4_SendNotes(uint16_t notes);

uint8_t UART4_TakeStatusRequest(void);
uint8_t UART4_TakeHelpRequest(void);
uint8_t UART4_TakeMelodyRequest(void);
uint8_t UART4_TakeStopRequest(void);

void UART4_SendHelp(void);

void UART4_SendPianoStatus(uint8_t volume,
                           uint8_t octave,
                           const char *chord,
                           uint16_t notes);

void UART4_Handler(void);

#endif /* UART_H_ */
