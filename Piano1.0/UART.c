#include <stdint.h>

#include "Config.h"
#include "Notes.h"
#include "UART.h"

/**************************************************************************************************
 *  System Control registers
 */

#define SYSCTL_RCGCGPIO_R               (*((volatile uint32_t *)0x400FE608))
#define SYSCTL_PRGPIO_R                 (*((volatile uint32_t *)0x400FEA08))

#define SYSCTL_RCGCUART_R               (*((volatile uint32_t *)0x400FE618))
#define SYSCTL_PRUART_R                 (*((volatile uint32_t *)0x400FEA18))

#define SYSCTL_ALTCLKCFG_R              (*((volatile uint32_t *)0x400FE138))

#define SYSCTL_RCGCGPIO_PORTA           (1 << 0)
#define SYSCTL_RCGCUART_UART4           (1 << 4)

/**************************************************************************************************
 *  GPIO Port A
 *
 *  PA2 = U4Rx
 *  PA3 = U4Tx
 */

#define GPIO_PORTA_AFSEL_R              (*((volatile uint32_t *)0x40058420))
#define GPIO_PORTA_DEN_R                (*((volatile uint32_t *)0x4005851C))
#define GPIO_PORTA_AMSEL_R              (*((volatile uint32_t *)0x40058528))
#define GPIO_PORTA_PCTL_R               (*((volatile uint32_t *)0x4005852C))

#define UART4_PA2_PA3_MASK              0x0C

/**************************************************************************************************
 *  UART4 registers
 */

#define UART4_DR_R                      (*((volatile uint32_t *)0x40010000))
#define UART4_FR_R                      (*((volatile uint32_t *)0x40010018))
#define UART4_IBRD_R                    (*((volatile uint32_t *)0x40010024))
#define UART4_FBRD_R                    (*((volatile uint32_t *)0x40010028))
#define UART4_LCRH_R                    (*((volatile uint32_t *)0x4001002C))
#define UART4_CTL_R                     (*((volatile uint32_t *)0x40010030))
#define UART4_IFLS_R                    (*((volatile uint32_t *)0x40010034))
#define UART4_IM_R                      (*((volatile uint32_t *)0x40010038))
#define UART4_MIS_R                     (*((volatile uint32_t *)0x40010040))
#define UART4_ICR_R                     (*((volatile uint32_t *)0x40010044))
#define UART4_CC_R                      (*((volatile uint32_t *)0x40010FC8))

/**************************************************************************************************
 *  UART bits
 */

#define UART_FR_TXFF                    0x20
#define UART_FR_RXFE                    0x10

#define UART_CTL_UARTEN                 0x0001
#define UART_CTL_TXE                    0x0100
#define UART_CTL_RXE                    0x0200

#define UART_LCRH_FEN                   0x10
#define UART_LCRH_WLEN_8                0x60

#define UART_IM_RXIM                    0x10
#define UART_IM_RTIM                    0x40

#define UART_ICR_RXIC                   0x10
#define UART_ICR_RTIC                   0x40

/**************************************************************************************************
 *  NVIC
 *
 *  UART4 corresponde a IRQ 57.
 */

#define NVIC_EN1_R                      (*((volatile uint32_t *)0xE000E104))
#define NVIC_PRI14_R                    (*((volatile uint32_t *)0xE000E438))

#define UART4_IRQ_NUMBER                57

/**************************************************************************************************
 *  Variables de depuracion
 */

volatile uint8_t uart_last_char_debug = 0;
volatile uint32_t uart_rx_counter_debug = 0;
volatile uint32_t uart_tx_counter_debug = 0;

volatile uint8_t uart_status_request_debug = 0;
volatile uint8_t uart_help_request_debug = 0;
volatile uint8_t uart_melody_request_debug = 0;
volatile uint8_t uart_stop_request_debug = 0;

/**************************************************************************************************
 *  Prototipos internos
 */

static void UART4_ProcessReceivedChar(uint8_t data);

/**************************************************************************************************
 *  UART4_Init
 *
 *  UART4:
 *      PA2 = U4Rx
 *      PA3 = U4Tx
 *
 *  Configuracion:
 *      9600 baudios por defecto
 *      8 bits
 *      sin paridad
 *      1 bit de paro
 *      FIFO habilitada
 *      interrupcion por recepcion
 *
 *  Fuente de reloj:
 *      PIOSC = 16 MHz mediante UARTCC = 0x5.
 */

void UART4_Init(void)
{
    uint32_t denominator;
    uint32_t integer_divisor;
    uint32_t fractional_divisor;
    uint32_t remainder;

    /**********************************************************************************************
     *  1. Habilitar reloj de UART4 y GPIOA.
     */

    SYSCTL_RCGCUART_R |= SYSCTL_RCGCUART_UART4;
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_PORTA;

    while ((SYSCTL_PRUART_R & SYSCTL_RCGCUART_UART4) == 0)
    {
    }

    while ((SYSCTL_PRGPIO_R & SYSCTL_RCGCGPIO_PORTA) == 0)
    {
    }

    /**********************************************************************************************
     *  2. Configurar PA2/PA3 como UART4.
     *
     *  PA2 = U4Rx, PCTL encoding 1.
     *  PA3 = U4Tx, PCTL encoding 1.
     */

    GPIO_PORTA_AFSEL_R |= UART4_PA2_PA3_MASK;
    GPIO_PORTA_AMSEL_R &= ~UART4_PA2_PA3_MASK;
    GPIO_PORTA_DEN_R   |= UART4_PA2_PA3_MASK;

    GPIO_PORTA_PCTL_R &= ~0x0000FF00;
    GPIO_PORTA_PCTL_R |=  0x00001100;

    /**********************************************************************************************
     *  3. Deshabilitar UART antes de configurar.
     */

    UART4_CTL_R &= ~UART_CTL_UARTEN;

    /**********************************************************************************************
     *  4. Fuente de reloj alternativa = PIOSC.
     *
     *  ALTCLKCFG = 0 -> PIOSC.
     *  UARTCC = 0x5 -> fuente alternativa.
     */

    SYSCTL_ALTCLKCFG_R = 0x00000000;
    UART4_CC_R = 0x00000005;

    /**********************************************************************************************
     *  5. Calcular divisor de baud rate.
     *
     *  BRD = UART_CLOCK_HZ / (16 * UART_BAUD_RATE)
     *
     *  Para 16 MHz y 9600:
     *      IBRD = 104
     *      FBRD = 11
     */

    denominator = 16 * UART_BAUD_RATE;

    integer_divisor = UART_CLOCK_HZ / denominator;
    remainder = UART_CLOCK_HZ % denominator;
    fractional_divisor = ((remainder * 64) + (denominator / 2)) / denominator;

    UART4_IBRD_R = integer_divisor;
    UART4_FBRD_R = fractional_divisor;

    /**********************************************************************************************
     *  6. Trama: 8 bits, sin paridad, 1 stop bit, FIFO habilitada.
     *
     *  La escritura a LCRH actualiza internamente los divisores.
     */

    UART4_LCRH_R = UART_LCRH_WLEN_8 | UART_LCRH_FEN;

    /**********************************************************************************************
     *  7. Interrupciones de recepcion.
     *
     *  RXIFLSEL = 0 -> RX FIFO >= 1/8.
     */

    UART4_IFLS_R &= ~0x00000038;

    UART4_ICR_R = UART_ICR_RXIC | UART_ICR_RTIC;
    UART4_IM_R |= UART_IM_RXIM | UART_IM_RTIM;

    /**********************************************************************************************
     *  8. NVIC UART4, IRQ 57.
     *
     *  IRQ 57 esta en EN1 bit 25.
     *  PRI14, campo de IRQ57: bits 15:13.
     */

    NVIC_PRI14_R = (NVIC_PRI14_R & ~0x0000E000) | (4 << 13);
    NVIC_EN1_R |= (1 << (UART4_IRQ_NUMBER - 32));

    /**********************************************************************************************
     *  9. Habilitar UART, TX y RX.
     */

    UART4_CTL_R |= UART_CTL_UARTEN | UART_CTL_TXE | UART_CTL_RXE;
}

/**************************************************************************************************
 *  UART4_SendChar
 */

void UART4_SendChar(char c)
{
    while ((UART4_FR_R & UART_FR_TXFF) != 0)
    {
    }

    UART4_DR_R = (uint8_t)c;
    uart_tx_counter_debug++;
}

/**************************************************************************************************
 *  UART4_SendString
 */

void UART4_SendString(const char *text)
{
    while (*text != '\0')
    {
        UART4_SendChar(*text);
        text++;
    }
}

/**************************************************************************************************
 *  UART4_SendUInt
 */

void UART4_SendUInt(uint32_t number)
{
    char buffer[11];
    int8_t index;

    if (number == 0)
    {
        UART4_SendChar('0');
        return;
    }

    index = 0;

    while ((number > 0) && (index < 10))
    {
        buffer[index] = (char)('0' + (number % 10));
        number = number / 10;
        index++;
    }

    while (index > 0)
    {
        index--;
        UART4_SendChar(buffer[index]);
    }
}

/**************************************************************************************************
 *  UART4_SendNotes
 *
 *  Envia por UART los nombres de las notas activas.
 */

void UART4_SendNotes(uint16_t notes)
{
    uint8_t channel;
    uint8_t printed;
    const char *name;

    printed = 0;

    if (notes == 0)
    {
        UART4_SendString("Sin notas");
        return;
    }

    for (channel = 0; channel < NUM_CHANNELS; channel++)
    {
        if ((notes & (1 << channel)) != 0)
        {
            name = Notes_GetNameFromChannel(channel);

            if (printed != 0)
            {
                UART4_SendString(" ");
            }

            UART4_SendString(name);

            printed = 1;
        }
    }
}

/**************************************************************************************************
 *  UART4_ProcessReceivedChar
 *
 *  La ISR solo guarda solicitudes. El procesamiento real ocurre en PianoApp_Update().
 */

static void UART4_ProcessReceivedChar(uint8_t data)
{
    uart_last_char_debug = data;
    uart_rx_counter_debug++;

    if ((data == 'S') || (data == 's'))
    {
        uart_status_request_debug = 1;
    }
    else if (data == '?')
    {
        uart_help_request_debug = 1;
    }
    else if (data == '1')
    {
        uart_melody_request_debug = 1;
    }
    else if (data == '2')
    {
        uart_melody_request_debug = 2;
    }
    else if (data == '3')
    {
        uart_melody_request_debug = 3;
    }
    else if ((data == 'X') || (data == 'x'))
    {
        uart_stop_request_debug = 1;
    }
    else
    {
    }
}

/**************************************************************************************************
 *  UART4_Handler
 */

void UART4_Handler(void)
{
    uint32_t status;
    uint8_t data;

    status = UART4_MIS_R;

    UART4_ICR_R = status & (UART_ICR_RXIC | UART_ICR_RTIC);

    while ((UART4_FR_R & UART_FR_RXFE) == 0)
    {
        data = (uint8_t)(UART4_DR_R & 0xFF);

        UART4_ProcessReceivedChar(data);
    }
}

/**************************************************************************************************
 *  UART4_TakeStatusRequest
 */

uint8_t UART4_TakeStatusRequest(void)
{
    uint8_t request;

    request = uart_status_request_debug;
    uart_status_request_debug = 0;

    return request;
}

/**************************************************************************************************
 *  UART4_TakeHelpRequest
 */

uint8_t UART4_TakeHelpRequest(void)
{
    uint8_t request;

    request = uart_help_request_debug;
    uart_help_request_debug = 0;

    return request;
}

/**************************************************************************************************
 *  UART4_TakeMelodyRequest
 */

uint8_t UART4_TakeMelodyRequest(void)
{
    uint8_t request;

    request = uart_melody_request_debug;
    uart_melody_request_debug = 0;

    return request;
}

/**************************************************************************************************
 *  UART4_TakeStopRequest
 */

uint8_t UART4_TakeStopRequest(void)
{
    uint8_t request;

    request = uart_stop_request_debug;
    uart_stop_request_debug = 0;

    return request;
}

/**************************************************************************************************
 *  UART4_SendHelp
 */

void UART4_SendHelp(void)
{
    UART4_SendString("\r\nComandos Piano TIVA:\r\n");
    UART4_SendString("S = estado\r\n");
    UART4_SendString("1 = melodia 1\r\n");
    UART4_SendString("2 = melodia 2\r\n");
    UART4_SendString("3 = melodia 3\r\n");
    UART4_SendString("X = detener melodia\r\n");
    UART4_SendString("? = ayuda\r\n");
}

/**************************************************************************************************
 *  UART4_SendPianoStatus
 */

void UART4_SendPianoStatus(uint8_t volume,
                           uint8_t octave,
                           const char *chord,
                           uint16_t notes)
{
    UART4_SendString("\r\nEstado Piano TIVA\r\n");

    UART4_SendString("Acorde: ");
    UART4_SendString(chord);
    UART4_SendString("\r\n");

    UART4_SendString("Notas: ");
    UART4_SendNotes(notes);
    UART4_SendString("\r\n");

    UART4_SendString("Volumen: ");
    UART4_SendUInt(volume);
    UART4_SendString("%\r\n");

    UART4_SendString("Octava: ");
    UART4_SendUInt(octave);
    UART4_SendString("\r\n");
}
