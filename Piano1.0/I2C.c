#include <stdint.h>
#include "Config.h"
#include "I2C.h"

/**************************************************************************************************
 *  System Control registers
 */

#define SYSCTL_RCGCGPIO_R               (*((volatile uint32_t *)0x400FE608))
#define SYSCTL_PRGPIO_R                 (*((volatile uint32_t *)0x400FEA08))

#define SYSCTL_RCGCI2C_R                (*((volatile uint32_t *)0x400FE620))
#define SYSCTL_PRI2C_R                  (*((volatile uint32_t *)0x400FEA20))

#define SYSCTL_RCGCGPIO_PORTB           (1 << 1)
#define SYSCTL_RCGCI2C_I2C0             (1 << 0)

/**************************************************************************************************
 *  GPIO Port B
 *
 *  PB2 = I2C0SCL
 *  PB3 = I2C0SDA
 */

#define GPIO_PORTB_AFSEL_R              (*((volatile uint32_t *)0x40059420))
#define GPIO_PORTB_ODR_R                (*((volatile uint32_t *)0x4005950C))
#define GPIO_PORTB_DEN_R                (*((volatile uint32_t *)0x4005951C))
#define GPIO_PORTB_AMSEL_R              (*((volatile uint32_t *)0x40059528))
#define GPIO_PORTB_PCTL_R               (*((volatile uint32_t *)0x4005952C))

#define GPIO_PB2_PB3_MASK               0x0C

/**************************************************************************************************
 *  I2C0 registers
 */

#define I2C0_MSA_R                      (*((volatile uint32_t *)0x40020000))
#define I2C0_MCS_R                      (*((volatile uint32_t *)0x40020004))
#define I2C0_MDR_R                      (*((volatile uint32_t *)0x40020008))
#define I2C0_MTPR_R                     (*((volatile uint32_t *)0x4002000C))
#define I2C0_MIMR_R                     (*((volatile uint32_t *)0x40020010))
#define I2C0_MRIS_R                     (*((volatile uint32_t *)0x40020014))
#define I2C0_MICR_R                     (*((volatile uint32_t *)0x4002001C))
#define I2C0_MCR_R                      (*((volatile uint32_t *)0x40020020))

/**************************************************************************************************
 *  I2C bits
 */

#define I2C_MCS_RUN                     0x01
#define I2C_MCS_START                   0x02
#define I2C_MCS_STOP                    0x04

#define I2C_MCS_BUSY                    0x01
#define I2C_MCS_ERROR                   0x02
#define I2C_MCS_ADRACK                  0x04
#define I2C_MCS_DATACK                  0x08
#define I2C_MCS_ARBLST                  0x10
#define I2C_MCS_BUSBSY                  0x40

#define I2C_MCR_MFE                     0x10

/**************************************************************************************************
 *  Variables de diagnostico
 */

volatile uint8_t i2c_error_debug = 0;
volatile uint32_t i2c_timeout_debug = 0;
volatile uint8_t i2c_last_mcs_debug = 0;
volatile uint8_t i2c_probe_3c_debug = 0;
volatile uint8_t i2c_probe_3d_debug = 0;

/**************************************************************************************************
 *  I2C0_Wait
 *
 *  Espera a que termine una operacion del maestro.
 *  Usa MRIS para evitar depender solamente de BUSY.
 */

static uint8_t I2C0_Wait(void)
{
    uint32_t timeout;

    timeout = 1000000;

    while ((I2C0_MRIS_R & 0x01) == 0)
    {
        timeout--;

        if (timeout == 0)
        {
            i2c_timeout_debug++;
            i2c_last_mcs_debug = (uint8_t)(I2C0_MCS_R & 0xFF);
            return 0;
        }
    }

    i2c_last_mcs_debug = (uint8_t)(I2C0_MCS_R & 0xFF);

    I2C0_MICR_R = 0x01;

    if ((I2C0_MCS_R & I2C_MCS_ERROR) != 0)
    {
        i2c_error_debug = (uint8_t)(I2C0_MCS_R & 0xFF);
        return 0;
    }

    return 1;
}

/**************************************************************************************************
 *  I2C0_Init
 *
 *  Configura I2C0 en PB2/PB3.
 *
 *  Conexion:
 *      PB2 -> SCL
 *      PB3 -> SDA
 */

void I2C0_Init(void)
{
    uint32_t tpr;

    /**********************************************************************************************
     *  1. Habilitar reloj del modulo I2C0 y GPIOB.
     */

    SYSCTL_RCGCI2C_R  |= SYSCTL_RCGCI2C_I2C0;
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_PORTB;

    while ((SYSCTL_PRI2C_R & SYSCTL_RCGCI2C_I2C0) == 0)
    {
    }

    while ((SYSCTL_PRGPIO_R & SYSCTL_RCGCGPIO_PORTB) == 0)
    {
    }

    /**********************************************************************************************
     *  2. Limpiar configuracion previa de PB2/PB3.
     */

    GPIO_PORTB_AFSEL_R &= ~GPIO_PB2_PB3_MASK;
    GPIO_PORTB_AMSEL_R &= ~GPIO_PB2_PB3_MASK;
    GPIO_PORTB_DEN_R   &= ~GPIO_PB2_PB3_MASK;
    GPIO_PORTB_ODR_R   &= ~GPIO_PB2_PB3_MASK;
    GPIO_PORTB_PCTL_R  &= ~0x0000FF00;

    /**********************************************************************************************
     *  3. Configurar PB2/PB3 como funcion alterna I2C0.
     *
     *  PB2 = I2C0SCL, PCTL encoding 2.
     *  PB3 = I2C0SDA, PCTL encoding 2.
     */

    GPIO_PORTB_AFSEL_R |= GPIO_PB2_PB3_MASK;
    GPIO_PORTB_PCTL_R  |= 0x00002200;

    /**********************************************************************************************
     *  4. Solo PB3/SDA debe ser open-drain.
     */

    GPIO_PORTB_ODR_R |= (1 << 3);

    /**********************************************************************************************
     *  5. Habilitar funcion digital en PB2/PB3.
     */

    GPIO_PORTB_DEN_R |= GPIO_PB2_PB3_MASK;

    /**********************************************************************************************
     *  6. Reiniciar diagnosticos.
     */

    i2c_error_debug = 0;
    i2c_timeout_debug = 0;
    i2c_last_mcs_debug = 0;
    i2c_probe_3c_debug = 0;
    i2c_probe_3d_debug = 0;

    /**********************************************************************************************
     *  7. Habilitar I2C0 como maestro.
     */

    I2C0_MCR_R = 0x00;
    I2C0_MCR_R = I2C_MCR_MFE;

    /**********************************************************************************************
     *  8. No usaremos interrupciones NVIC de I2C, solo polling.
     */

    I2C0_MIMR_R = 0x00;
    I2C0_MICR_R = 0x01;

    /**********************************************************************************************
     *  9. Configurar velocidad I2C.
     *
     *  TPR = SYS_CLOCK_HZ / (20 * I2C_SPEED_HZ) - 1
     *
     *  Si SYS_CLOCK_HZ = 120 MHz e I2C_SPEED_HZ = 100 kHz:
     *      TPR = 59
     */

    tpr = (SYS_CLOCK_HZ / (20 * I2C_SPEED_HZ)) - 1;

    I2C0_MTPR_R = tpr;
}

/**************************************************************************************************
 *  I2C0_WriteByte
 *
 *  Escribe un byte en una transaccion.
 */

uint8_t I2C0_WriteByte(uint8_t slave_address, uint8_t data)
{
    I2C0_MICR_R = 0x01;

    I2C0_MSA_R = (slave_address << 1);
    I2C0_MDR_R = data;

    I2C0_MCS_R = I2C_MCS_START | I2C_MCS_RUN | I2C_MCS_STOP;

    if (I2C0_Wait() == 0)
    {
        I2C0_MCS_R = I2C_MCS_STOP;
        return 0;
    }

    return 1;
}

/**************************************************************************************************
 *  I2C0_Write2Bytes
 *
 *  Escribe dos bytes en la misma transaccion:
 *
 *      byte1 = byte de control
 *      byte2 = comando o dato
 *
 *  Para SSD1306:
 *      byte1 = 0x00 -> comando
 *      byte1 = 0x40 -> dato
 */

uint8_t I2C0_Write2Bytes(uint8_t slave_address, uint8_t byte1, uint8_t byte2)
{
    I2C0_MICR_R = 0x01;

    I2C0_MSA_R = (slave_address << 1);

    /**********************************************************************************************
     *  Primer byte.
     */

    I2C0_MDR_R = byte1;
    I2C0_MCS_R = I2C_MCS_START | I2C_MCS_RUN;

    if (I2C0_Wait() == 0)
    {
        I2C0_MCS_R = I2C_MCS_STOP;
        return 0;
    }

    /**********************************************************************************************
     *  Segundo byte.
     */

    I2C0_MDR_R = byte2;
    I2C0_MCS_R = I2C_MCS_RUN | I2C_MCS_STOP;

    if (I2C0_Wait() == 0)
    {
        I2C0_MCS_R = I2C_MCS_STOP;
        return 0;
    }

    return 1;
}

/**************************************************************************************************
 *  I2C0_Probe
 *
 *  Prueba si un dispositivo responde en una direccion.
 */

uint8_t I2C0_Probe(uint8_t slave_address)
{
    return I2C0_WriteByte(slave_address, 0x00);
}
