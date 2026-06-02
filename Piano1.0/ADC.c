#include <stdint.h>
#include "Config.h"
#include "ADC.h"

/**************************************************************************************************
 *  System Control registers
 */

#define SYSCTL_RCGCGPIO_R               (*((volatile uint32_t *)0x400FE608))
#define SYSCTL_PRGPIO_R                 (*((volatile uint32_t *)0x400FEA08))

#define SYSCTL_RCGCADC_R                (*((volatile uint32_t *)0x400FE638))
#define SYSCTL_PRADC_R                  (*((volatile uint32_t *)0x400FEA38))

#define SYSCTL_RCGCGPIO_PORTE           (1 << 4)
#define SYSCTL_RCGCADC_ADC0             (1 << 0)

/**************************************************************************************************
 *  GPIO Port E
 *
 *  PE3 = AIN0 = eje X
 *  PE2 = AIN1 = eje Y
 */

#define GPIO_PORTE_DIR_R                (*((volatile uint32_t *)0x4005C400))
#define GPIO_PORTE_AFSEL_R              (*((volatile uint32_t *)0x4005C420))
#define GPIO_PORTE_DEN_R                (*((volatile uint32_t *)0x4005C51C))
#define GPIO_PORTE_AMSEL_R              (*((volatile uint32_t *)0x4005C528))
#define GPIO_PORTE_PCTL_R               (*((volatile uint32_t *)0x4005C52C))

#define JOYSTICK_ADC_PINS               0x0C        /* PE3 + PE2 */

/**************************************************************************************************
 *  ADC0 registers
 */

#define ADC0_ACTSS_R                    (*((volatile uint32_t *)0x40038000))
#define ADC0_RIS_R                      (*((volatile uint32_t *)0x40038004))
#define ADC0_IM_R                       (*((volatile uint32_t *)0x40038008))
#define ADC0_ISC_R                      (*((volatile uint32_t *)0x4003800C))
#define ADC0_EMUX_R                     (*((volatile uint32_t *)0x40038014))
#define ADC0_SSPRI_R                    (*((volatile uint32_t *)0x40038020))
#define ADC0_PSSI_R                     (*((volatile uint32_t *)0x40038028))
#define ADC0_SAC_R                      (*((volatile uint32_t *)0x40038030))
#define ADC0_PC_R                       (*((volatile uint32_t *)0x40038FC4))
#define ADC0_CC_R                       (*((volatile uint32_t *)0x40038FC8))

/**************************************************************************************************
 *  ADC0 Sample Sequencer 1
 */

#define ADC0_SSMUX1_R                   (*((volatile uint32_t *)0x40038060))
#define ADC0_SSCTL1_R                   (*((volatile uint32_t *)0x40038064))
#define ADC0_SSFIFO1_R                  (*((volatile uint32_t *)0x40038068))
#define ADC0_SSFSTAT1_R                 (*((volatile uint32_t *)0x4003806C))
#define ADC0_SSEMUX1_R                  (*((volatile uint32_t *)0x40038078))

/**************************************************************************************************
 *  NVIC
 *
 *  ADC0 SS1 corresponde a IRQ 15.
 */

#define NVIC_EN0_R                      (*((volatile uint32_t *)0xE000E100))
#define NVIC_PRI3_R                     (*((volatile uint32_t *)0xE000E40C))

#define ADC0SS1_IRQ_NUMBER              15

/**************************************************************************************************
 *  Bits y mascaras
 */

#define ADC_SS1_MASK                    0x02

#define ADC_CC_CS_PLL                   0x0
#define ADC_CC_CLKDIV_30                (29 << 4)

/**************************************************************************************************
 *  Variables internas
 */

static volatile uint8_t adc_initialized = 0;
static volatile uint8_t adc_conversion_busy = 0;
static volatile uint16_t adc_tick_counter = 0;

/**************************************************************************************************
 *  Estados internos del joystick por pasos
 */

static uint8_t joystick_x_ready = 1;
static uint8_t joystick_y_ready = 1;

static uint8_t joystick_volume = DEFAULT_VOLUME_PERCENT;
static uint8_t joystick_octave = DEFAULT_OCTAVE;

/**************************************************************************************************
 *  Variables de depuracion
 */

volatile uint16_t adc_x_debug = 2048;
volatile uint16_t adc_y_debug = 2048;

volatile uint8_t adc_volume_debug = DEFAULT_VOLUME_PERCENT;
volatile uint8_t adc_octave_debug = DEFAULT_OCTAVE;

volatile uint8_t adc_x_ready_debug = 1;
volatile uint8_t adc_y_ready_debug = 1;

volatile uint32_t adc_isr_counter_debug = 0;
volatile uint32_t adc_start_counter_debug = 0;

volatile uint32_t adc_ris_debug = 0;
volatile uint32_t adc_actss_debug = 0;
volatile uint32_t adc_ssfstat_debug = 0;

/**************************************************************************************************
 *  Prototipos internos
 */

static void ADC_Joystick_StartConversion(void);
static void ADC_ProcessJoystick(uint16_t x, uint16_t y);

/**************************************************************************************************
 *  ADC_Joystick_Init
 *
 *  Configura:
 *      ADC0 SS1
 *      Muestra 0: AIN0 / PE3 / X
 *      Muestra 1: AIN1 / PE2 / Y
 *
 *  El ADC usa PLL VCO dividido entre 30:
 *      480 MHz / 30 = 16 MHz
 *
 *  No se apaga el PLL.
 */

void ADC_Joystick_Init(void)
{
    /**********************************************************************************************
     *  1. Habilitar reloj de GPIOE y ADC0.
     */

    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_PORTE;
    SYSCTL_RCGCADC_R  |= SYSCTL_RCGCADC_ADC0;

    while ((SYSCTL_PRGPIO_R & SYSCTL_RCGCGPIO_PORTE) == 0)
    {
    }

    while ((SYSCTL_PRADC_R & SYSCTL_RCGCADC_ADC0) == 0)
    {
    }

    /**********************************************************************************************
     *  2. Configurar PE3 y PE2 como entradas analogicas.
     *
     *  PE3 = AIN0 = eje X
     *  PE2 = AIN1 = eje Y
     */

    GPIO_PORTE_DIR_R   &= ~JOYSTICK_ADC_PINS;
    GPIO_PORTE_AFSEL_R |=  JOYSTICK_ADC_PINS;
    GPIO_PORTE_DEN_R   &= ~JOYSTICK_ADC_PINS;
    GPIO_PORTE_AMSEL_R |=  JOYSTICK_ADC_PINS;

    GPIO_PORTE_PCTL_R  &= ~0x0000FF00;

    /**********************************************************************************************
     *  3. Deshabilitar SS1 antes de configurarlo.
     */

    ADC0_ACTSS_R &= ~ADC_SS1_MASK;

    /**********************************************************************************************
     *  4. Reloj del ADC.
     *
     *  ADCCC:
     *      CS = 0x0       -> PLL VCO dividido
     *      CLKDIV = 29    -> divide entre 30
     *
     *  fADC = 480 MHz / 30 = 16 MHz.
     */

    ADC0_CC_R = ADC_CC_CLKDIV_30 | ADC_CC_CS_PLL;

    /**********************************************************************************************
     *  5. Disparo por software para SS1.
     */

    ADC0_EMUX_R &= ~0x000000F0;

    /**********************************************************************************************
     *  6. Prioridad de secuenciadores.
     */

    ADC0_SSPRI_R = 0x00000123;

    /**********************************************************************************************
     *  7. Sin promediado por hardware.
     */

    ADC0_SAC_R = 0x00000000;

    /**********************************************************************************************
     *  8. Tasa de conversion segura.
     */

    ADC0_PC_R = 0x00000001;

    /**********************************************************************************************
     *  9. Configurar SS1 para dos muestras.
     *
     *  Muestra 0:
     *      AIN0 = PE3 = X
     *
     *  Muestra 1:
     *      AIN1 = PE2 = Y
     */

    ADC0_SSMUX1_R = 0x00000010;
    ADC0_SSEMUX1_R = 0x00000000;

    /**********************************************************************************************
     *  10. Control del secuenciador.
     *
     *  Muestra 0:
     *      sin IE, sin END
     *
     *  Muestra 1:
     *      IE1  = 1
     *      END1 = 1
     *
     *  IE1  = bit 5
     *  END1 = bit 6
     */

    ADC0_SSCTL1_R = 0x00000060;

    /**********************************************************************************************
     *  11. Limpiar bandera y habilitar interrupcion local del ADC.
     */

    ADC0_ISC_R = ADC_SS1_MASK;
    ADC0_IM_R |= ADC_SS1_MASK;

    /**********************************************************************************************
     *  12. Configurar NVIC para ADC0 SS1, IRQ 15.
     *
     *  NVIC_PRI3 controla IRQ 12 a 15.
     *  IRQ 15 usa bits 31:29.
     */

    NVIC_PRI3_R = (NVIC_PRI3_R & ~0xE0000000) | (2 << 29);
    NVIC_EN0_R |= (1 << ADC0SS1_IRQ_NUMBER);

    /**********************************************************************************************
     *  13. Inicializar estados del joystick.
     */

    joystick_x_ready = 1;
    joystick_y_ready = 1;

    joystick_volume = DEFAULT_VOLUME_PERCENT;
    joystick_octave = DEFAULT_OCTAVE;

    adc_volume_debug = joystick_volume;
    adc_octave_debug = joystick_octave;

    adc_x_ready_debug = joystick_x_ready;
    adc_y_ready_debug = joystick_y_ready;

    /**********************************************************************************************
     *  14. Habilitar SS1.
     */

    ADC0_ACTSS_R |= ADC_SS1_MASK;

    adc_initialized = 1;
    adc_conversion_busy = 0;
    adc_tick_counter = 0;
}

/**************************************************************************************************
 *  ADC_Joystick_Tick1ms
 *
 *  Se llama desde SysTick_Handler cada 1 ms.
 *  Cada ADC_JOYSTICK_SAMPLE_MS inicia una conversion.
 */

void ADC_Joystick_Tick1ms(void)
{
    if (adc_initialized == 0)
    {
        return;
    }

    if (adc_tick_counter < ADC_JOYSTICK_SAMPLE_MS)
    {
        adc_tick_counter++;
        return;
    }

    adc_tick_counter = 0;

    ADC_Joystick_StartConversion();
}

/**************************************************************************************************
 *  ADC_Joystick_StartConversion
 */

static void ADC_Joystick_StartConversion(void)
{
    if (adc_conversion_busy != 0)
    {
        return;
    }

    adc_conversion_busy = 1;
    adc_start_counter_debug++;

    ADC0_ISC_R = ADC_SS1_MASK;

    ADC0_PSSI_R = ADC_SS1_MASK;
}

/**************************************************************************************************
 *  ADC0SS1_Handler
 *
 *  Rutina de interrupcion del ADC0 Sample Sequencer 1.
 */

void ADC0SS1_Handler(void)
{
    uint16_t x;
    uint16_t y;

    adc_ris_debug = ADC0_RIS_R;
    adc_actss_debug = ADC0_ACTSS_R;
    adc_ssfstat_debug = ADC0_SSFSTAT1_R;

    /**********************************************************************************************
     *  Leer FIFO en el mismo orden configurado:
     *      1. X
     *      2. Y
     */

    x = (uint16_t)(ADC0_SSFIFO1_R & 0x0FFF);
    y = (uint16_t)(ADC0_SSFIFO1_R & 0x0FFF);

    adc_x_debug = x;
    adc_y_debug = y;

    /**********************************************************************************************
     *  Procesar joystick como control por pasos.
     */

    ADC_ProcessJoystick(x, y);

    adc_isr_counter_debug++;

    /**********************************************************************************************
     *  Limpiar bandera de interrupcion.
     */

    ADC0_ISC_R = ADC_SS1_MASK;

    adc_conversion_busy = 0;
}

/**************************************************************************************************
 *  ADC_ProcessJoystick
 *
 *  Procesa el joystick como control por pasos.
 *
 *  Eje X:
 *      izquierda -> baja una octava
 *      derecha   -> sube una octava
 *
 *  Eje Y:
 *      abajo  -> baja volumen 5%
 *      arriba -> sube volumen 5%
 *
 *  Para evitar cambios repetidos, cada eje debe regresar al centro antes de aceptar otro cambio.
 */

static void ADC_ProcessJoystick(uint16_t x, uint16_t y)
{
    /**********************************************************************************************
     *  EJE X: OCTAVA
     */

    if ((x > JOYSTICK_X_RETURN_LOW) && (x < JOYSTICK_X_RETURN_HIGH))
    {
        joystick_x_ready = 1;
    }
    else if (joystick_x_ready != 0)
    {
        if (x < JOYSTICK_X_LOW)
        {
            if (joystick_octave > MIN_OCTAVE)
            {
                joystick_octave--;
            }

            joystick_x_ready = 0;
        }
        else if (x > JOYSTICK_X_HIGH)
        {
            if (joystick_octave < MAX_OCTAVE)
            {
                joystick_octave++;
            }

            joystick_x_ready = 0;
        }
        else
        {
        }
    }
    else
    {
    }

    /**********************************************************************************************
     *  EJE Y: VOLUMEN
     *
     *  Joystick arriba  -> sube volumen
     *  Joystick abajo   -> baja volumen
     *
     *  Se invierte la accion porque el eje vertical del joystick entrega valores
     *  opuestos a la direccion fisica esperada.
     */

    if ((y > JOYSTICK_Y_RETURN_LOW) && (y < JOYSTICK_Y_RETURN_HIGH))
    {
        joystick_y_ready = 1;
    }
    else if (joystick_y_ready != 0)
    {
        if (y < JOYSTICK_Y_LOW)
        {
            /******************************************************************************************
             *  Arriba: subir volumen.
             */

            if (joystick_volume <= (100 - VOLUME_STEP_PERCENT))
            {
                joystick_volume = joystick_volume + VOLUME_STEP_PERCENT;
            }
            else
            {
                joystick_volume = 100;
            }

            joystick_y_ready = 0;
        }
        else if (y > JOYSTICK_Y_HIGH)
        {
            /******************************************************************************************
             *  Abajo: bajar volumen.
             */

            if (joystick_volume >= VOLUME_STEP_PERCENT)
            {
                joystick_volume = joystick_volume - VOLUME_STEP_PERCENT;
            }
            else
            {
                joystick_volume = 0;
            }

            joystick_y_ready = 0;
        }
        else
        {
        }
    }
    else
    {
    }

    /**********************************************************************************************
     *  Actualizar variables de salida y depuracion.
     */

    adc_volume_debug = joystick_volume;
    adc_octave_debug = joystick_octave;

    adc_x_ready_debug = joystick_x_ready;
    adc_y_ready_debug = joystick_y_ready;
}

/**************************************************************************************************
 *  ADC_Joystick_GetVolumePercent
 */

uint8_t ADC_Joystick_GetVolumePercent(void)
{
    return adc_volume_debug;
}

/**************************************************************************************************
 *  ADC_Joystick_GetOctave
 */

uint8_t ADC_Joystick_GetOctave(void)
{
    return adc_octave_debug;
}
