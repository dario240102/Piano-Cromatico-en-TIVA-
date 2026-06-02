#include <stdint.h>
#include "Clock.h"

/**************************************************************************************************
 *  System Control registers
 */

#define SYSCTL_MEMTIM0_R                (*((volatile uint32_t *)0x400FE0C0))
#define SYSCTL_RSCLKCFG_R               (*((volatile uint32_t *)0x400FE0B0))
#define SYSCTL_PLLFREQ0_R               (*((volatile uint32_t *)0x400FE160))
#define SYSCTL_PLLFREQ1_R               (*((volatile uint32_t *)0x400FE164))
#define SYSCTL_PLLSTAT_R                (*((volatile uint32_t *)0x400FE168))

/**************************************************************************************************
 *  Mascaras
 */

#define RSCLKCFG_MEMTIMU                0x80000000
#define RSCLKCFG_NEWFREQ                0x40000000
#define RSCLKCFG_USEPLL                 0x10000000

#define PLLFREQ0_PLLPWR                 0x00800000

/**************************************************************************************************
 *  Variables de depuracion
 */

volatile uint32_t system_clock_debug = 16000000;
volatile uint32_t pll_lock_debug = 0;

/**************************************************************************************************
 *  Clock_Init120MHz
 *
 *  Configuracion:
 *
 *      Fuente PLL: PIOSC = 16 MHz
 *      Q = 0
 *      N = 0
 *      MINT = 30
 *      MFRAC = 0
 *
 *      fVCO = 16 MHz * 30 = 480 MHz
 *
 *      PSYSDIV = 3
 *      SysClk = 480 MHz / (3 + 1) = 120 MHz
 */

void Clock_Init120MHz(void)
{
    volatile uint32_t delay;

    /**********************************************************************************************
     *  1. Asegurar que por ahora no se este usando el PLL como reloj del sistema.
     */

    SYSCTL_RSCLKCFG_R &= ~RSCLKCFG_USEPLL;

    /**********************************************************************************************
     *  2. Configurar tiempos de Flash y EEPROM para 120 MHz.
     *
     *  Para 100 < f <= 120 MHz:
     *      EBCHT = 0x6
     *      EBCE  = 0
     *      EWS   = 0x5
     *      FBCHT = 0x6
     *      FBCE  = 0
     *      FWS   = 0x5
     *
     *  Valor usado:
     *      0x01950195
     */

    SYSCTL_MEMTIM0_R = 0x01950195;

    /**********************************************************************************************
     *  3. Apagar el PLL antes de reconfigurarlo.
     */

    SYSCTL_PLLFREQ0_R &= ~PLLFREQ0_PLLPWR;

    /**********************************************************************************************
     *  4. Configurar frecuencia del PLL.
     *
     *  PLLFREQ1:
     *      Q = 0
     *      N = 0
     *
     *  PLLFREQ0:
     *      PLLPWR = 1
     *      MFRAC  = 0
     *      MINT   = 30
     */

    SYSCTL_PLLFREQ1_R = 0x00000000;
    SYSCTL_PLLFREQ0_R = 0x0080001E;

    /**********************************************************************************************
     *  5. Aplicar la nueva configuracion de PLLFREQ0 y PLLFREQ1.
     */

    SYSCTL_RSCLKCFG_R |= RSCLKCFG_NEWFREQ;

    /**********************************************************************************************
     *  6. Esperar a que el PLL haga lock.
     */

    while ((SYSCTL_PLLSTAT_R & 0x01) == 0)
    {
    }

    pll_lock_debug = SYSCTL_PLLSTAT_R;

    /**********************************************************************************************
     *  7. Cambiar el reloj del sistema al PLL.
     *
     *  RSCLKCFG:
     *      MEMTIMU = 1  -> aplicar MEMTIM0
     *      USEPLL  = 1  -> usar PLL
     *      PLLSRC  = 0  -> PIOSC como fuente del PLL
     *      PSYSDIV = 3  -> 480 MHz / 4 = 120 MHz
     */

    SYSCTL_RSCLKCFG_R = RSCLKCFG_MEMTIMU |
                        RSCLKCFG_USEPLL  |
                        0x00000003;

    /**********************************************************************************************
     *  8. Pequeña espera de estabilizacion.
     */

    for (delay = 0; delay < 1000; delay++)
    {
    }

    system_clock_debug = 120000000;
}
