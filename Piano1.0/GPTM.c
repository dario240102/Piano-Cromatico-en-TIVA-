#include <stdint.h>
#include "Config.h"
#include "Audio.h"
#include "GPTM.h"

/**************************************************************************************************
 *  System Control registers
 */

#define SYSCTL_RCGCTIMER_R              (*((volatile uint32_t *)0x400FE604))
#define SYSCTL_PRTIMER_R                (*((volatile uint32_t *)0x400FEA04))

/**************************************************************************************************
 *  GPTM0 registers
 */

#define TIMER0_CFG_R                    (*((volatile uint32_t *)0x40030000))
#define TIMER0_TAMR_R                   (*((volatile uint32_t *)0x40030004))
#define TIMER0_CTL_R                    (*((volatile uint32_t *)0x4003000C))
#define TIMER0_IMR_R                    (*((volatile uint32_t *)0x40030018))
#define TIMER0_ICR_R                    (*((volatile uint32_t *)0x40030024))
#define TIMER0_TAILR_R                  (*((volatile uint32_t *)0x40030028))

/**************************************************************************************************
 *  NVIC registers
 *
 *  Timer0A corresponde a IRQ 19.
 */

#define NVIC_EN0_R                      (*((volatile uint32_t *)0xE000E100))
#define NVIC_PRI4_R                     (*((volatile uint32_t *)0xE000E410))

#define TIMER0A_IRQ_NUMBER              19

/**************************************************************************************************
 *  GPTM0A_Audio_Init
 */

void GPTM0A_Audio_Init(void)
{
    uint32_t load_value;

    load_value = (SYS_CLOCK_HZ / AUDIO_TICK_HZ) - 1;

    SYSCTL_RCGCTIMER_R |= 0x01;

    while ((SYSCTL_PRTIMER_R & 0x01) == 0)
    {
    }

    TIMER0_CTL_R &= ~0x01;

    TIMER0_CFG_R = 0x00000000;
    TIMER0_TAMR_R = 0x00000002;
    TIMER0_TAILR_R = load_value;

    TIMER0_ICR_R = 0x01;
    TIMER0_IMR_R |= 0x01;

    NVIC_PRI4_R = (NVIC_PRI4_R & ~0xE0000000) | (0 << 29);
    NVIC_EN0_R |= (1 << TIMER0A_IRQ_NUMBER);

    TIMER0_CTL_R |= 0x01;
}

/**************************************************************************************************
 *  Timer0A_Handler
 */

void Timer0A_Handler(void)
{
    TIMER0_ICR_R = 0x01;

    Audio_UpdateTick();
}

