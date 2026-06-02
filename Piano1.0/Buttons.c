#include <stdint.h>

#include "Config.h"
#include "Buttons.h"

/**************************************************************************************************
 *  System Control registers
 */

#define SYSCTL_RCGCGPIO_R               (*((volatile uint32_t *)0x400FE608))
#define SYSCTL_PRGPIO_R                 (*((volatile uint32_t *)0x400FEA08))

#define SYSCTL_RCGCGPIO_PORTK           (1 << 9)
#define SYSCTL_RCGCGPIO_PORTP           (1 << 13)
#define SYSCTL_RCGCGPIO_PORTQ           (1 << 14)

/**************************************************************************************************
 *  GPIO Port K
 *
 *  PK1-PK7 = botones canales 1 a 7
 *
 *  PK0 ya NO se usa porque quedo con falla/corto a GND.
 */

#define GPIO_PORTK_DATA_R               (*((volatile uint32_t *)0x400613FC))
#define GPIO_PORTK_DIR_R                (*((volatile uint32_t *)0x40061400))
#define GPIO_PORTK_AFSEL_R              (*((volatile uint32_t *)0x40061420))
#define GPIO_PORTK_PUR_R                (*((volatile uint32_t *)0x40061510))
#define GPIO_PORTK_DEN_R                (*((volatile uint32_t *)0x4006151C))
#define GPIO_PORTK_AMSEL_R              (*((volatile uint32_t *)0x40061528))
#define GPIO_PORTK_PCTL_R               (*((volatile uint32_t *)0x4006152C))

/**************************************************************************************************
 *  GPIO Port P
 *
 *  PP0-PP3 = botones canales 8 a 11
 */

#define GPIO_PORTP_DATA_R               (*((volatile uint32_t *)0x400653FC))
#define GPIO_PORTP_DIR_R                (*((volatile uint32_t *)0x40065400))
#define GPIO_PORTP_AFSEL_R              (*((volatile uint32_t *)0x40065420))
#define GPIO_PORTP_PUR_R                (*((volatile uint32_t *)0x40065510))
#define GPIO_PORTP_DEN_R                (*((volatile uint32_t *)0x4006551C))
#define GPIO_PORTP_AMSEL_R              (*((volatile uint32_t *)0x40065528))
#define GPIO_PORTP_PCTL_R               (*((volatile uint32_t *)0x4006552C))

/**************************************************************************************************
 *  GPIO Port Q
 *
 *  PQ0 = nuevo boton del canal 0 / Si
 */

#define GPIO_PORTQ_DATA_R               (*((volatile uint32_t *)0x400663FC))
#define GPIO_PORTQ_DIR_R                (*((volatile uint32_t *)0x40066400))
#define GPIO_PORTQ_AFSEL_R              (*((volatile uint32_t *)0x40066420))
#define GPIO_PORTQ_PUR_R                (*((volatile uint32_t *)0x40066510))
#define GPIO_PORTQ_DEN_R                (*((volatile uint32_t *)0x4006651C))
#define GPIO_PORTQ_AMSEL_R              (*((volatile uint32_t *)0x40066528))
#define GPIO_PORTQ_PCTL_R               (*((volatile uint32_t *)0x4006652C))

/**************************************************************************************************
 *  Mascaras de botones
 */

#define BUTTON_PORTK_MASK               0xFE        /* PK1-PK7 */
#define BUTTON_PORTP_MASK               0x0F        /* PP0-PP3 */
#define BUTTON_PORTQ_MASK               0x01        /* PQ0 */

/**************************************************************************************************
 *  Variables internas
 */

static volatile uint16_t buttons_stable_mask = 0;
static volatile uint16_t buttons_last_raw = 0;
static volatile uint16_t buttons_debounce_counter = 0;

/**************************************************************************************************
 *  Variables de depuracion
 */

volatile uint16_t buttons_raw_debug = 0;
volatile uint16_t buttons_stable_debug = 0;
volatile uint16_t buttons_last_raw_debug = 0;
volatile uint16_t buttons_debounce_counter_debug = 0;

/**************************************************************************************************
 *  Prototipo interno
 */

static uint16_t Buttons_ReadRaw(void);

/**************************************************************************************************
 *  Buttons_Init
 *
 *  Configura botones con pull-up interno.
 *
 *  Logica fisica:
 *      boton suelto     = 1
 *      boton presionado = 0
 *
 *  Logica de software:
 *      boton suelto     = 0
 *      boton presionado = 1
 */

void Buttons_Init(void)
{
    /**********************************************************************************************
     *  1. Habilitar reloj de puertos K, P y Q.
     */

    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_PORTK |
                         SYSCTL_RCGCGPIO_PORTP |
                         SYSCTL_RCGCGPIO_PORTQ;

    while ((SYSCTL_PRGPIO_R & SYSCTL_RCGCGPIO_PORTK) == 0)
    {
    }

    while ((SYSCTL_PRGPIO_R & SYSCTL_RCGCGPIO_PORTP) == 0)
    {
    }

    while ((SYSCTL_PRGPIO_R & SYSCTL_RCGCGPIO_PORTQ) == 0)
    {
    }

    /**********************************************************************************************
     *  2. Puerto K: PK1-PK7 como entradas digitales con pull-up.
     *
     *  PK0 queda fuera.
     */

    GPIO_PORTK_DIR_R   &= ~BUTTON_PORTK_MASK;
    GPIO_PORTK_AFSEL_R &= ~BUTTON_PORTK_MASK;
    GPIO_PORTK_AMSEL_R &= ~BUTTON_PORTK_MASK;
    GPIO_PORTK_PCTL_R  &= ~0xFFFFFFF0;
    GPIO_PORTK_DEN_R   |=  BUTTON_PORTK_MASK;
    GPIO_PORTK_PUR_R   |=  BUTTON_PORTK_MASK;

    /**********************************************************************************************
     *  3. Puerto P: PP0-PP3 como entradas digitales con pull-up.
     */

    GPIO_PORTP_DIR_R   &= ~BUTTON_PORTP_MASK;
    GPIO_PORTP_AFSEL_R &= ~BUTTON_PORTP_MASK;
    GPIO_PORTP_AMSEL_R &= ~BUTTON_PORTP_MASK;
    GPIO_PORTP_PCTL_R  &= ~0x0000FFFF;
    GPIO_PORTP_DEN_R   |=  BUTTON_PORTP_MASK;
    GPIO_PORTP_PUR_R   |=  BUTTON_PORTP_MASK;

    /**********************************************************************************************
     *  4. Puerto Q: PQ0 como nuevo boton del canal 0 / Si.
     */

    GPIO_PORTQ_DIR_R   &= ~BUTTON_PORTQ_MASK;
    GPIO_PORTQ_AFSEL_R &= ~BUTTON_PORTQ_MASK;
    GPIO_PORTQ_AMSEL_R &= ~BUTTON_PORTQ_MASK;
    GPIO_PORTQ_PCTL_R  &= ~0x0000000F;
    GPIO_PORTQ_DEN_R   |=  BUTTON_PORTQ_MASK;
    GPIO_PORTQ_PUR_R   |=  BUTTON_PORTQ_MASK;

    /**********************************************************************************************
     *  5. Inicializar variables de debounce.
     */

    buttons_stable_mask = 0;
    buttons_last_raw = Buttons_ReadRaw();
    buttons_debounce_counter = 0;

    buttons_raw_debug = buttons_last_raw;
    buttons_stable_debug = buttons_stable_mask;
    buttons_last_raw_debug = buttons_last_raw;
    buttons_debounce_counter_debug = buttons_debounce_counter;
}

/**************************************************************************************************
 *  Buttons_ReadRaw
 *
 *  Lee los botones y regresa una mascara de 12 bits:
 *
 *      bit 0  = canal 0  = Si  = PQ0
 *      bit 1  = canal 1  = La# = PK1
 *      bit 2  = canal 2  = La  = PK2
 *      bit 3  = canal 3  = Sol#= PK3
 *      bit 4  = canal 4  = Sol = PK4
 *      bit 5  = canal 5  = Fa# = PK5
 *      bit 6  = canal 6  = Fa  = PK6
 *      bit 7  = canal 7  = Mi  = PK7
 *      bit 8  = canal 8  = Re# = PP0
 *      bit 9  = canal 9  = Re  = PP1
 *      bit 10 = canal 10 = Do# = PP2
 *      bit 11 = canal 11 = Do  = PP3
 *
 *  Como los botones usan pull-up:
 *      fisico 1 = suelto
 *      fisico 0 = presionado
 *
 *  Por eso se invierte con ~.
 */

static uint16_t Buttons_ReadRaw(void)
{
    uint16_t raw;
    uint16_t portq_button;
    uint16_t portk_buttons;
    uint16_t portp_buttons;

    /**********************************************************************************************
     *  PQ0 -> canal 0.
     */

    portq_button = (uint16_t)((~GPIO_PORTQ_DATA_R) & 0x01);

    /**********************************************************************************************
     *  PK1-PK7 -> canales 1-7.
     *
     *  Se conserva la posicion de bits:
     *      PK1 -> bit 1
     *      PK2 -> bit 2
     *      ...
     *      PK7 -> bit 7
     */

    portk_buttons = (uint16_t)((~GPIO_PORTK_DATA_R) & 0xFE);

    /**********************************************************************************************
     *  PP0-PP3 -> canales 8-11.
     */

    portp_buttons = (uint16_t)(((~GPIO_PORTP_DATA_R) & 0x0F) << 8);

    raw = portq_button | portk_buttons | portp_buttons;

    raw &= 0x0FFF;

    return raw;
}

/**************************************************************************************************
 *  Buttons_UpdateDebounce
 *
 *  Debounce por software.
 *  Debe llamarse cada 1 ms desde SysTick_Handler.
 */

void Buttons_UpdateDebounce(void)
{
    uint16_t current_raw;

    current_raw = Buttons_ReadRaw();

    buttons_raw_debug = current_raw;

    if (current_raw != buttons_last_raw)
    {
        buttons_last_raw = current_raw;
        buttons_debounce_counter = 0;
    }
    else
    {
        if (buttons_debounce_counter < BUTTON_DEBOUNCE_MS)
        {
            buttons_debounce_counter++;
        }
        else
        {
            buttons_stable_mask = current_raw;
        }
    }

    buttons_last_raw_debug = buttons_last_raw;
    buttons_stable_debug = buttons_stable_mask;
    buttons_debounce_counter_debug = buttons_debounce_counter;
}

/**************************************************************************************************
 *  Buttons_GetStableMask
 */

uint16_t Buttons_GetStableMask(void)
{
    return buttons_stable_mask;
}
