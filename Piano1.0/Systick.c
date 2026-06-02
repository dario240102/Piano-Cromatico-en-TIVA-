#include <stdint.h>
#include "Config.h"
#include "Buttons.h"
#include "Melody.h"
#include "ADC.h"
#include "SysTick.h"

/**************************************************************************************************
 *  SysTick registers
 */

#define NVIC_ST_CTRL_R                  (*((volatile uint32_t *)0xE000E010))
#define NVIC_ST_RELOAD_R                (*((volatile uint32_t *)0xE000E014))
#define NVIC_ST_CURRENT_R               (*((volatile uint32_t *)0xE000E018))

#define NVIC_ST_CTRL_ENABLE             0x00000001
#define NVIC_ST_CTRL_INTEN              0x00000002
#define NVIC_ST_CTRL_CLK_SRC            0x00000004

/**************************************************************************************************
 *  Variable global de tiempo
 */

volatile uint32_t systick_ms = 0;

/**************************************************************************************************
 *  SysTick_Init
 */

void SysTick_Init(void)
{
    NVIC_ST_CTRL_R = 0;

    NVIC_ST_RELOAD_R = (SYS_CLOCK_HZ / SYSTICK_TICK_HZ) - 1;
    NVIC_ST_CURRENT_R = 0;

    NVIC_ST_CTRL_R = NVIC_ST_CTRL_CLK_SRC |
                     NVIC_ST_CTRL_INTEN   |
                     NVIC_ST_CTRL_ENABLE;
}

/**************************************************************************************************
 *  SysTick_GetMs
 */

uint32_t SysTick_GetMs(void)
{
    return systick_ms;
}

/**************************************************************************************************
 *  SysTick_Handler
 */

void SysTick_Handler(void)
{
    systick_ms++;

    Buttons_UpdateDebounce();

    Melody_Update1ms();

    ADC_Joystick_Tick1ms();
}
