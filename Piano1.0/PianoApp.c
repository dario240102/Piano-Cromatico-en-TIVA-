#include <stdint.h>

#include "Config.h"
#include "Clock.h"
#include "GPIO.h"
#include "Buttons.h"
#include "SysTick.h"
#include "Audio.h"
#include "GPTM.h"
#include "Chords.h"
#include "Melody.h"
#include "PianoApp.h"

#if ADC_ENABLE
#include "ADC.h"
#endif

#if OLED_ENABLE
#include "I2C.h"
#include "OLED.h"
#endif

#if UART_ENABLE
#include "UART.h"
#endif

/**************************************************************************************************
 *  Variables de depuracion
 */

volatile uint16_t piano_notes_debug = 0;
volatile uint16_t piano_output_debug = 0;
volatile uint32_t piano_update_counter = 0;

volatile uint8_t piano_volume_debug = DEFAULT_VOLUME_PERCENT;
volatile uint8_t piano_octave_debug = DEFAULT_OCTAVE;

const char *piano_chord_debug = "Sin acorde";

/**************************************************************************************************
 *  Variable interna para actualizar OLED sin saturarla.
 */

#if OLED_ENABLE
static uint32_t last_oled_update_ms = 0;
#endif

/**************************************************************************************************
 *  Prototipos internos
 */

#if OLED_ENABLE
static void PianoApp_UpdateOLED(uint16_t notes, const char *chord);
#endif

#if UART_ENABLE
static void PianoApp_ProcessUART(uint16_t notes, const char *chord);
#endif

/**************************************************************************************************
 *  PianoApp_Init
 */

void PianoApp_Init(void)
{
    Clock_Init120MHz();

    GPIO_Piano_Init();

    Buttons_Init();

    Audio_Init();

    Melody_Init();

#if ADC_ENABLE
    ADC_Joystick_Init();
#endif

    SysTick_Init();

    GPTM0A_Audio_Init();

#if OLED_ENABLE
    I2C0_Init();

    if (OLED_Init() != 0)
    {
        OLED_ShowPianoStatus("LISTO",
                             0,
                             piano_volume_debug,
                             piano_octave_debug);
    }
#endif

#if UART_ENABLE
    UART4_Init();

    UART4_SendString("\r\nPiano TIVA listo por Bluetooth.\r\n");
    UART4_SendHelp();
#endif
}

/**************************************************************************************************
 *  PianoApp_Update
 *
 *  Logica principal:
 *
 *      1. Actualiza volumen/octava desde ADC.
 *      2. Procesa botones.
 *      3. Revisa combinaciones secretas.
 *      4. Revisa si hay melodia activa.
 *      5. Actualiza audio.
 *      6. Actualiza OLED.
 *      7. Procesa comandos UART.
 */

void PianoApp_Update(void)
{
    uint16_t notes;
    uint16_t output_notes;

    piano_update_counter++;

    /**********************************************************************************************
     *  1. Leer parametros actualizados por interrupcion del ADC.
     */

#if ADC_ENABLE
    piano_volume_debug = ADC_Joystick_GetVolumePercent();
    piano_octave_debug = ADC_Joystick_GetOctave();
#endif

    Audio_SetVolumePercent(piano_volume_debug);
    Audio_SetOctave(piano_octave_debug);

    /**********************************************************************************************
     *  2. Leer botones estables.
     */

    notes = Buttons_GetStableMask();

    piano_notes_debug = notes;

    /**********************************************************************************************
     *  3. Revisar combinaciones secretas.
     */

    Melody_CheckSecretCombination(notes);

    /**********************************************************************************************
     *  4. Decidir que controla el audio:
     *      - Si hay melodia activa, manda la melodia.
     *      - Si no, mandan los botones.
     */

    if (Melody_IsPlaying() != 0)
    {
        output_notes = Melody_GetCurrentChannels();

        piano_chord_debug = "MELODIA";
    }
    else
    {
        output_notes = notes;

        piano_chord_debug = Chords_Detect(notes);
    }

    piano_output_debug = output_notes;

    /**********************************************************************************************
     *  5. Actualizar canales activos de audio.
     */

    Audio_SetActiveChannels(output_notes);

    /**********************************************************************************************
     *  6. Actualizar OLED.
     */

#if OLED_ENABLE
    PianoApp_UpdateOLED(output_notes, piano_chord_debug);
#endif

    /**********************************************************************************************
     *  7. Procesar comandos UART.
     */

#if UART_ENABLE
    PianoApp_ProcessUART(output_notes, piano_chord_debug);
#endif
}

/**************************************************************************************************
 *  PianoApp_UpdateOLED
 *
 *  Actualiza la OLED solo si:
 *      - La pantalla esta lista.
 *      - Paso OLED_REFRESH_MS.
 *      - Cambio alguna informacion.
 */

#if OLED_ENABLE

static void PianoApp_UpdateOLED(uint16_t notes, const char *chord)
{
    uint32_t now_ms;

    static uint16_t last_notes = 0xFFFF;
    static const char *last_chord = 0;
    static uint8_t last_volume = 255;
    static uint8_t last_octave = 255;

    if (OLED_IsReady() == 0)
    {
        return;
    }

    now_ms = SysTick_GetMs();

    if ((now_ms - last_oled_update_ms) < OLED_REFRESH_MS)
    {
        return;
    }

    /**********************************************************************************************
     *  Si no cambio nada, no redibujar.
     */

    if ((notes == last_notes) &&
        (chord == last_chord) &&
        (piano_volume_debug == last_volume) &&
        (piano_octave_debug == last_octave))
    {
        return;
    }

    last_oled_update_ms = now_ms;

    last_notes = notes;
    last_chord = chord;
    last_volume = piano_volume_debug;
    last_octave = piano_octave_debug;

    OLED_ShowPianoStatus(chord,
                         notes,
                         piano_volume_debug,
                         piano_octave_debug);
}

#endif

/**************************************************************************************************
 *  PianoApp_ProcessUART
 *
 *  Procesa solicitudes recibidas por Bluetooth.
 *
 *  La ISR de UART solo guarda banderas.
 *  Aqui se ejecutan los comandos reales.
 */

#if UART_ENABLE

static void PianoApp_ProcessUART(uint16_t notes, const char *chord)
{
    uint8_t melody_command;

    /**********************************************************************************************
     *  Comando de melodia:
     *      '1' -> melodia 1
     *      '2' -> melodia 2
     *      '3' -> melodia 3
     */

    melody_command = UART4_TakeMelodyRequest();

    if (melody_command != 0)
    {
        Melody_Start(melody_command);

        UART4_SendString("\r\nReproduciendo melodia ");
        UART4_SendUInt(melody_command);
        UART4_SendString(".\r\n");
    }

    /**********************************************************************************************
     *  Comando de detener melodia:
     *      'X' o 'x'
     */

    if (UART4_TakeStopRequest() != 0)
    {
        Melody_Stop();

        UART4_SendString("\r\nMelodia detenida.\r\n");
    }

    /**********************************************************************************************
     *  Comando de ayuda:
     *      '?'
     */

    if (UART4_TakeHelpRequest() != 0)
    {
        UART4_SendHelp();
    }

    /**********************************************************************************************
     *  Comando de estado:
     *      'S' o 's'
     */

    if (UART4_TakeStatusRequest() != 0)
    {
        UART4_SendPianoStatus(piano_volume_debug,
                              piano_octave_debug,
                              chord,
                              notes);
    }
}

#endif
