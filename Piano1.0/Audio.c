#include <stdint.h>
#include "Config.h"
#include "GPIO.h"
#include "Audio.h"

/**************************************************************************************************
 *  Variables de audio
 */

static volatile uint16_t active_channels = 0;

volatile uint16_t audio_active_debug = 0;
volatile uint8_t audio_octave_debug = DEFAULT_OCTAVE;
volatile uint8_t audio_volume_debug = DEFAULT_VOLUME_PERCENT;

static uint32_t channel_counters[NUM_CHANNELS] = {0};

/**************************************************************************************************
 *  Tabla de limites por canal fisico
 *
 *  limite = AUDIO_TICK_HZ / (2 * frecuencia_nota)
 *
 *  Como la placa quedo invertida, KEYBOARD_REVERSED = 1 asigna:
 *
 *      Canal 0  -> Si
 *      Canal 1  -> La#
 *      Canal 2  -> La
 *      Canal 3  -> Sol#
 *      Canal 4  -> Sol
 *      Canal 5  -> Fa#
 *      Canal 6  -> Fa
 *      Canal 7  -> Mi
 *      Canal 8  -> Re#
 *      Canal 9  -> Re
 *      Canal 10 -> Do#
 *      Canal 11 -> Do
 */

#if KEYBOARD_REVERSED

static const uint32_t channel_limits[NUM_CHANNELS] =
{
    41,     /* Canal 0  -> Si   */
    43,     /* Canal 1  -> La#  */
    46,     /* Canal 2  -> La   */
    48,     /* Canal 3  -> Sol# */
    51,     /* Canal 4  -> Sol  */
    55,     /* Canal 5  -> Fa#  */
    58,     /* Canal 6  -> Fa   */
    61,     /* Canal 7  -> Mi   */
    65,     /* Canal 8  -> Re#  */
    69,     /* Canal 9  -> Re   */
    73,     /* Canal 10 -> Do#  */
    77      /* Canal 11 -> Do   */
};

#else

static const uint32_t channel_limits[NUM_CHANNELS] =
{
    76,     /* Canal 0  -> Do   */
    72,     /* Canal 1  -> Do#  */
    68,     /* Canal 2  -> Re   */
    64,     /* Canal 3  -> Re#  */
    61,     /* Canal 4  -> Mi   */
    57,     /* Canal 5  -> Fa   */
    54,     /* Canal 6  -> Fa#  */
    51,     /* Canal 7  -> Sol  */
    48,     /* Canal 8  -> Sol# */
    45,     /* Canal 9  -> La   */
    43,     /* Canal 10 -> La#  */
    40      /* Canal 11 -> Si   */
};

#endif
/**************************************************************************************************
 *  Audio_Init
 */

void Audio_Init(void)
{
    uint8_t i;

    active_channels = 0;
    audio_active_debug = 0;
    audio_octave_debug = DEFAULT_OCTAVE;
    audio_volume_debug = DEFAULT_VOLUME_PERCENT;

    for (i = 0; i < NUM_CHANNELS; i++)
    {
        channel_counters[i] = 0;
    }

    GPIO_Buzzers_OffAll();
}

/**************************************************************************************************
 *  Audio_SetActiveChannels
 */

void Audio_SetActiveChannels(uint16_t channels)
{
    active_channels = channels & 0x0FFF;
    audio_active_debug = active_channels;
}



/**************************************************************************************************
 *  Audio_GetActiveChannels
 */

uint16_t Audio_GetActiveChannels(void)
{
    return active_channels;
}

/**************************************************************************************************
 *  Audio_UpdateTick
 *
 *  Esta funcion se llama desde Timer0A_Handler cada AUDIO_TICK_US.
 *
 *  Cada canal tiene un contador.
 *  Cuando el contador alcanza su limite, se invierte el estado del buzzer.
 */

/**************************************************************************************************
 *  Audio_SetOctave
 */

void Audio_SetOctave(uint8_t octave)
{
    if (octave < MIN_OCTAVE)
    {
        octave = MIN_OCTAVE;
    }

    if (octave > MAX_OCTAVE)
    {
        octave = MAX_OCTAVE;
    }

    audio_octave_debug = octave;
}

/**************************************************************************************************
 *  Audio_GetOctave
 */

uint8_t Audio_GetOctave(void)
{
    return audio_octave_debug;
}

/**************************************************************************************************
 *  Audio_SetVolumePercent
 */

void Audio_SetVolumePercent(uint8_t volume_percent)
{
    if (volume_percent > 100)
    {
        volume_percent = 100;
    }

    audio_volume_debug = volume_percent;
}

/**************************************************************************************************
 *  Audio_GetVolumePercent
 */

uint8_t Audio_GetVolumePercent(void)
{
    return audio_volume_debug;
}

/**************************************************************************************************
 *  Audio_GetAdjustedLimit
 *
 *  Octava 3 -> frecuencia menor -> limite mayor
 *  Octava 4 -> limite normal
 *  Octava 5 -> frecuencia mayor -> limite menor
 */

static uint32_t Audio_GetAdjustedLimit(uint8_t channel)
{
    uint32_t limit;

    limit = channel_limits[channel];

    if (audio_octave_debug == MIN_OCTAVE)
    {
        limit = limit * 2;
    }
    else if (audio_octave_debug == MAX_OCTAVE)
    {
        if (limit > 2)
        {
            limit = limit / 2;
        }
    }
    else
    {
    }

    if (limit < 1)
    {
        limit = 1;
    }

    return limit;
}

void Audio_UpdateTick(void)
{
    uint8_t channel;
    uint16_t channel_mask;

    uint32_t limit;
    uint32_t period_ticks;
    uint32_t high_ticks;
    uint32_t duty_percent;

    for (channel = 0; channel < NUM_CHANNELS; channel++)
    {
        channel_mask = (uint16_t)(1 << channel);

        if (((active_channels & channel_mask) != 0) && (audio_volume_debug > 0))
        {
            limit = Audio_GetAdjustedLimit(channel);

            period_ticks = limit * 2;

            if (period_ticks < 2)
            {
                period_ticks = 2;
            }

            /******************************************************************************************
             *  Volumen:
             *      100% volumen -> 50% duty
             *       50% volumen -> 25% duty
             *       10% volumen ->  5% duty
             */

            duty_percent = audio_volume_debug / 2;

            if (duty_percent == 0)
            {
                duty_percent = 1;
            }

            high_ticks = (period_ticks * duty_percent) / 100;

            if (high_ticks == 0)
            {
                high_ticks = 1;
            }

            if (high_ticks >= period_ticks)
            {
                high_ticks = period_ticks - 1;
            }

            if (channel_counters[channel] < high_ticks)
            {
                GPIO_Buzzer_Write(channel, 1);
            }
            else
            {
                GPIO_Buzzer_Write(channel, 0);
            }

            channel_counters[channel]++;

            if (channel_counters[channel] >= period_ticks)
            {
                channel_counters[channel] = 0;
            }
        }
        else
        {
            GPIO_Buzzer_Write(channel, 0);
            channel_counters[channel] = 0;
        }
    }
}

/**************************************************************************************************
 *  Audio_StopAll
 */

void Audio_StopAll(void)
{
    uint8_t i;

    active_channels = 0;
    audio_active_debug = 0;

    for (i = 0; i < NUM_CHANNELS; i++)
    {
        channel_counters[i] = 0;
    }

    GPIO_Buzzers_OffAll();
}
