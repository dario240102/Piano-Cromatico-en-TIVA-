#include <stdint.h>
#include "Config.h"
#include "GPIO.h"

/**************************************************************************************************
 *  System Control registers
 */

#define SYSCTL_RCGCGPIO_R               (*((volatile uint32_t *)0x400FE608))
#define SYSCTL_PRGPIO_R                 (*((volatile uint32_t *)0x400FEA08))

#define SYSCTL_RCGCGPIO_PORTK           (1 << 9)
#define SYSCTL_RCGCGPIO_PORTL           (1 << 10)
#define SYSCTL_RCGCGPIO_PORTM           (1 << 11)
#define SYSCTL_RCGCGPIO_PORTP           (1 << 13)
#define SYSCTL_RCGCGPIO_PORTQ           (1 << 14)

/**************************************************************************************************
 *  GPIO Port K
 *
 *  PK1-PK7 = botones canales 1 a 7.
 *  PK0 ya NO se usa.
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
 *  PP0-PP3 = botones canales 8 a 11.
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
 *  PQ0 = nuevo boton del canal 0 / Si.
 */

#define GPIO_PORTQ_DATA_R               (*((volatile uint32_t *)0x400663FC))
#define GPIO_PORTQ_DIR_R                (*((volatile uint32_t *)0x40066400))
#define GPIO_PORTQ_AFSEL_R              (*((volatile uint32_t *)0x40066420))
#define GPIO_PORTQ_PUR_R                (*((volatile uint32_t *)0x40066510))
#define GPIO_PORTQ_DEN_R                (*((volatile uint32_t *)0x4006651C))
#define GPIO_PORTQ_AMSEL_R              (*((volatile uint32_t *)0x40066528))
#define GPIO_PORTQ_PCTL_R               (*((volatile uint32_t *)0x4006652C))

/**************************************************************************************************
 *  GPIO Port L
 *
 *  PL0-PL5 = buzzers canales 0 a 5.
 */

#define GPIO_PORTL_DATA_R               (*((volatile uint32_t *)0x400623FC))
#define GPIO_PORTL_DIR_R                (*((volatile uint32_t *)0x40062400))
#define GPIO_PORTL_AFSEL_R              (*((volatile uint32_t *)0x40062420))
#define GPIO_PORTL_DEN_R                (*((volatile uint32_t *)0x4006251C))
#define GPIO_PORTL_AMSEL_R              (*((volatile uint32_t *)0x40062528))
#define GPIO_PORTL_PCTL_R               (*((volatile uint32_t *)0x4006252C))

/**************************************************************************************************
 *  GPIO Port M
 *
 *  PM0-PM5 = buzzers canales 6 a 11.
 */

#define GPIO_PORTM_DATA_R               (*((volatile uint32_t *)0x400633FC))
#define GPIO_PORTM_DIR_R                (*((volatile uint32_t *)0x40063400))
#define GPIO_PORTM_AFSEL_R              (*((volatile uint32_t *)0x40063420))
#define GPIO_PORTM_DEN_R                (*((volatile uint32_t *)0x4006351C))
#define GPIO_PORTM_AMSEL_R              (*((volatile uint32_t *)0x40063528))
#define GPIO_PORTM_PCTL_R               (*((volatile uint32_t *)0x4006352C))

/**************************************************************************************************
 *  Mascaras
 */

#define BUTTONS_PORTK_MASK              0xFE        /* PK1-PK7 */
#define BUTTONS_PORTP_MASK              0x0F        /* PP0-PP3 */
#define BUTTONS_PORTQ_MASK              0x01        /* PQ0 */

#define BUZZERS_PORTL_MASK              0x3F        /* PL0-PL5 */
#define BUZZERS_PORTM_MASK              0x3F        /* PM0-PM5 */

/**************************************************************************************************
 *  GPIO_Piano_Init
 */

void GPIO_Piano_Init(void)
{
    uint32_t ready_mask;

    ready_mask = SYSCTL_RCGCGPIO_PORTK |
                 SYSCTL_RCGCGPIO_PORTL |
                 SYSCTL_RCGCGPIO_PORTM |
                 SYSCTL_RCGCGPIO_PORTP |
                 SYSCTL_RCGCGPIO_PORTQ;

    SYSCTL_RCGCGPIO_R |= ready_mask;

    while ((SYSCTL_PRGPIO_R & ready_mask) != ready_mask)
    {
    }

    /**********************************************************************************************
     *  Port K: PK1-PK7 como entradas digitales con pull-up.
     *
     *  PK0 queda fuera porque esta fallando.
     */

    GPIO_PORTK_AFSEL_R &= ~BUTTONS_PORTK_MASK;
    GPIO_PORTK_AMSEL_R &= ~BUTTONS_PORTK_MASK;
    GPIO_PORTK_PCTL_R  &= ~0xFFFFFFF0;
    GPIO_PORTK_DIR_R   &= ~BUTTONS_PORTK_MASK;
    GPIO_PORTK_PUR_R   |=  BUTTONS_PORTK_MASK;
    GPIO_PORTK_DEN_R   |=  BUTTONS_PORTK_MASK;

    /**********************************************************************************************
     *  Desactivar PK0 para que ya no participe en nada.
     */

    GPIO_PORTK_AFSEL_R &= ~0x01;
    GPIO_PORTK_AMSEL_R &= ~0x01;
    GPIO_PORTK_PCTL_R  &= ~0x0000000F;
    GPIO_PORTK_PUR_R   &= ~0x01;
    GPIO_PORTK_DEN_R   &= ~0x01;

    /**********************************************************************************************
     *  Port P: PP0-PP3 como entradas digitales con pull-up.
     */

    GPIO_PORTP_AFSEL_R &= ~BUTTONS_PORTP_MASK;
    GPIO_PORTP_AMSEL_R &= ~BUTTONS_PORTP_MASK;
    GPIO_PORTP_PCTL_R  &= ~0x0000FFFF;
    GPIO_PORTP_DIR_R   &= ~BUTTONS_PORTP_MASK;
    GPIO_PORTP_PUR_R   |=  BUTTONS_PORTP_MASK;
    GPIO_PORTP_DEN_R   |=  BUTTONS_PORTP_MASK;

    /**********************************************************************************************
     *  Port Q: PQ0 como nuevo boton del canal 0 / Si.
     */

    GPIO_PORTQ_AFSEL_R &= ~BUTTONS_PORTQ_MASK;
    GPIO_PORTQ_AMSEL_R &= ~BUTTONS_PORTQ_MASK;
    GPIO_PORTQ_PCTL_R  &= ~0x0000000F;
    GPIO_PORTQ_DIR_R   &= ~BUTTONS_PORTQ_MASK;
    GPIO_PORTQ_PUR_R   |=  BUTTONS_PORTQ_MASK;
    GPIO_PORTQ_DEN_R   |=  BUTTONS_PORTQ_MASK;

    /**********************************************************************************************
     *  Port L: salidas digitales para buzzers canales 0 a 5.
     */

    GPIO_PORTL_AFSEL_R &= ~BUZZERS_PORTL_MASK;
    GPIO_PORTL_AMSEL_R &= ~BUZZERS_PORTL_MASK;
    GPIO_PORTL_PCTL_R  &= ~0x00FFFFFF;
    GPIO_PORTL_DATA_R  &= ~BUZZERS_PORTL_MASK;
    GPIO_PORTL_DIR_R   |=  BUZZERS_PORTL_MASK;
    GPIO_PORTL_DEN_R   |=  BUZZERS_PORTL_MASK;

    /**********************************************************************************************
     *  Port M: salidas digitales para buzzers canales 6 a 11.
     */

    GPIO_PORTM_AFSEL_R &= ~BUZZERS_PORTM_MASK;
    GPIO_PORTM_AMSEL_R &= ~BUZZERS_PORTM_MASK;
    GPIO_PORTM_PCTL_R  &= ~0x00FFFFFF;
    GPIO_PORTM_DATA_R  &= ~BUZZERS_PORTM_MASK;
    GPIO_PORTM_DIR_R   |=  BUZZERS_PORTM_MASK;
    GPIO_PORTM_DEN_R   |=  BUZZERS_PORTM_MASK;
}

/**************************************************************************************************
 *  GPIO_Buttons_ReadRawPressedMask
 *
 *  Lee los botones y regresa una mascara de 12 bits:
 *
 *      bit 0  = canal 0  = Si   = PQ0
 *      bit 1  = canal 1  = La#  = PK1
 *      bit 2  = canal 2  = La   = PK2
 *      bit 3  = canal 3  = Sol# = PK3
 *      bit 4  = canal 4  = Sol  = PK4
 *      bit 5  = canal 5  = Fa#  = PK5
 *      bit 6  = canal 6  = Fa   = PK6
 *      bit 7  = canal 7  = Mi   = PK7
 *      bit 8  = canal 8  = Re#  = PP0
 *      bit 9  = canal 9  = Re   = PP1
 *      bit 10 = canal 10 = Do#  = PP2
 *      bit 11 = canal 11 = Do   = PP3
 *
 *  Botones con pull-up:
 *      fisico 1 = suelto
 *      fisico 0 = presionado
 *
 *  Por eso se invierte con ~.
 */

uint16_t GPIO_Buttons_ReadRawPressedMask(void)
{
    uint32_t raw_k;
    uint32_t raw_p;
    uint32_t raw_q;

    uint16_t pressed_k;
    uint16_t pressed_p;
    uint16_t pressed_q;

    raw_k = GPIO_PORTK_DATA_R & BUTTONS_PORTK_MASK;
    raw_p = GPIO_PORTP_DATA_R & BUTTONS_PORTP_MASK;
    raw_q = GPIO_PORTQ_DATA_R & BUTTONS_PORTQ_MASK;

    /**********************************************************************************************
     *  PQ0 -> canal 0.
     */

    pressed_q = (uint16_t)((~raw_q) & BUTTONS_PORTQ_MASK);

    /**********************************************************************************************
     *  PK1-PK7 -> canales 1-7.
     *
     *  Se conserva la posicion de bits.
     */

    pressed_k = (uint16_t)((~raw_k) & BUTTONS_PORTK_MASK);

    /**********************************************************************************************
     *  PP0-PP3 -> canales 8-11.
     */

    pressed_p = (uint16_t)((~raw_p) & BUTTONS_PORTP_MASK);

    return (uint16_t)(pressed_q | pressed_k | (pressed_p << 8));
}

/**************************************************************************************************
 *  GPIO_Buzzer_Write
 */

void GPIO_Buzzer_Write(uint8_t channel, uint8_t value)
{
    uint32_t mask;

    if (channel < 6)
    {
        mask = (1 << channel);

        if (value != 0)
        {
            GPIO_PORTL_DATA_R |= mask;
        }
        else
        {
            GPIO_PORTL_DATA_R &= ~mask;
        }
    }
    else if (channel < NUM_CHANNELS)
    {
        mask = (1 << (channel - 6));

        if (value != 0)
        {
            GPIO_PORTM_DATA_R |= mask;
        }
        else
        {
            GPIO_PORTM_DATA_R &= ~mask;
        }
    }
    else
    {
    }
}

/**************************************************************************************************
 *  GPIO_Buzzer_Toggle
 */

void GPIO_Buzzer_Toggle(uint8_t channel)
{
    uint32_t mask;

    if (channel < 6)
    {
        mask = (1 << channel);
        GPIO_PORTL_DATA_R ^= mask;
    }
    else if (channel < NUM_CHANNELS)
    {
        mask = (1 << (channel - 6));
        GPIO_PORTM_DATA_R ^= mask;
    }
    else
    {
    }
}

/**************************************************************************************************
 *  GPIO_Buzzers_OffAll
 */

void GPIO_Buzzers_OffAll(void)
{
    GPIO_PORTL_DATA_R &= ~BUZZERS_PORTL_MASK;
    GPIO_PORTM_DATA_R &= ~BUZZERS_PORTM_MASK;
}
