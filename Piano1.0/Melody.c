#include <stdint.h>

#include "Config.h"
#include "Notes.h"
#include "SysTick.h"
#include "Melody.h"

/**************************************************************************************************
 *  Tipo de dato para eventos de melodia
 */

typedef struct
{
    uint16_t notes;
    uint16_t duration_ms;
} MelodyEvent_t;

/**************************************************************************************************
 *  Variables de depuracion
 */

volatile uint8_t melody_playing_debug = 0;
volatile uint8_t melody_id_debug = 0;
volatile uint16_t melody_channels_debug = 0;
volatile uint8_t melody_index_debug = 0;

/**************************************************************************************************
 *  Variables internas
 */

static const MelodyEvent_t * volatile current_melody = 0;
static volatile uint8_t current_length = 0;
static volatile uint8_t current_index = 0;
static volatile uint16_t current_notes = 0;
static volatile uint16_t remaining_ms = 0;
static volatile uint8_t is_playing = 0;

/**************************************************************************************************
 *  Variables para combinaciones secretas
 */

static uint16_t secret_candidate = 0;
static uint32_t secret_start_ms = 0;
static uint8_t secret_locked = 0;

/**************************************************************************************************
 *  Combinaciones secretas
 */

#define SECRET_1_MASK                   (NOTE_MASK_DO  | NOTE_MASK_RE   | NOTE_MASK_MI)
#define SECRET_2_MASK                   (NOTE_MASK_FA  | NOTE_MASK_SOL  | NOTE_MASK_LA)
#define SECRET_3_MASK                   (NOTE_MASK_FAS | NOTE_MASK_SOLS | NOTE_MASK_LAS)

/**************************************************************************************************
 *  Melodias programadas
 *
 *  0 en notes representa silencio.
 */

/**************************************************************************************************
 *  Melodia 1
 *
 *  Adaptacion de "Oda a la Alegria" de Beethoven.
 *  Muy estable para una sola octava.
 */

static const MelodyEvent_t melody_1[] =
{
    { NOTE_MASK_MI,                 260 },
    { 0,                             40 },
    { NOTE_MASK_MI,                 260 },
    { 0,                             40 },
    { NOTE_MASK_FA,                 260 },
    { 0,                             40 },
    { NOTE_MASK_SOL,                260 },
    { 0,                             40 },

    { NOTE_MASK_SOL,                260 },
    { 0,                             40 },
    { NOTE_MASK_FA,                 260 },
    { 0,                             40 },
    { NOTE_MASK_MI,                 260 },
    { 0,                             40 },
    { NOTE_MASK_RE,                 260 },
    { 0,                             40 },

    { NOTE_MASK_DO,                 260 },
    { 0,                             40 },
    { NOTE_MASK_DO,                 260 },
    { 0,                             40 },
    { NOTE_MASK_RE,                 260 },
    { 0,                             40 },
    { NOTE_MASK_MI,                 260 },
    { 0,                             40 },

    { NOTE_MASK_MI,                 390 },
    { 0,                             50 },
    { NOTE_MASK_RE,                 130 },
    { 0,                             40 },
    { NOTE_MASK_RE,                 520 },
    { 0,                            140 },

    { NOTE_MASK_MI,                 260 },
    { 0,                             40 },
    { NOTE_MASK_MI,                 260 },
    { 0,                             40 },
    { NOTE_MASK_FA,                 260 },
    { 0,                             40 },
    { NOTE_MASK_SOL,                260 },
    { 0,                             40 },

    { NOTE_MASK_SOL,                260 },
    { 0,                             40 },
    { NOTE_MASK_FA,                 260 },
    { 0,                             40 },
    { NOTE_MASK_MI,                 260 },
    { 0,                             40 },
    { NOTE_MASK_RE,                 260 },
    { 0,                             40 },

    { NOTE_MASK_DO,                 260 },
    { 0,                             40 },
    { NOTE_MASK_DO,                 260 },
    { 0,                             40 },
    { NOTE_MASK_RE,                 260 },
    { 0,                             40 },
    { NOTE_MASK_MI,                 260 },
    { 0,                             40 },

    { NOTE_MASK_RE,                 390 },
    { 0,                             50 },
    { NOTE_MASK_DO,                 130 },
    { 0,                             40 },
    { NOTE_MASK_DO,                 600 },
    { 0,                            200 }
};
static const MelodyEvent_t melody_2[] =
{
    /**********************************************************************************************
     *  Melodia 2
     *
     *  Homenaje estilo chiptune rapido.
     *  Usa las notas propuestas:
     *
     *  Seccion 1: Re - Re - Re - La - Sol# - Sol - Fa - Re - Fa - Sol
     *  Seccion 2: Do - Do - Re - La - Sol# - Sol - Fa - Re - Fa - Sol
     *  Seccion 3: Si - Si - Re - La - Sol# - Sol - Fa - Re - Fa - Sol
     *  Seccion 4: Sib - Sib - Re - La - Sol# - Sol - Fa - Re - Fa - Sol
     *
     *  Ritmo original:
     *      golpes cortos repetidos,
     *      acentos medios,
     *      cierre largo por seccion.
     */

    /**********************************************************************************************
     *  Seccion 1
     */

    { NOTE_MASK_RE,                  95 },
    { 0,                             25 },
    { NOTE_MASK_RE,                  95 },
    { 0,                             25 },
    { NOTE_MASK_RE,                 180 },
    { 0,                             35 },
    { NOTE_MASK_LA,                 110 },
    { 0,                             25 },
    { NOTE_MASK_SOLS,               220 },
    { 0,                             40 },
    { NOTE_MASK_SOL,                110 },
    { 0,                             25 },
    { NOTE_MASK_FA,                 110 },
    { 0,                             25 },
    { NOTE_MASK_RE,                  95 },
    { 0,                             25 },
    { NOTE_MASK_FA,                 110 },
    { 0,                             25 },
    { NOTE_MASK_SOL,                320 },
    { 0,                            100 },

    /**********************************************************************************************
     *  Seccion 2
     */

    { NOTE_MASK_DO,                  95 },
    { 0,                             25 },
    { NOTE_MASK_DO,                  95 },
    { 0,                             25 },
    { NOTE_MASK_RE,                 180 },
    { 0,                             35 },
    { NOTE_MASK_LA,                 110 },
    { 0,                             25 },
    { NOTE_MASK_SOLS,               220 },
    { 0,                             40 },
    { NOTE_MASK_SOL,                110 },
    { 0,                             25 },
    { NOTE_MASK_FA,                 110 },
    { 0,                             25 },
    { NOTE_MASK_RE,                  95 },
    { 0,                             25 },
    { NOTE_MASK_FA,                 110 },
    { 0,                             25 },
    { NOTE_MASK_SOL,                320 },
    { 0,                            100 },

    /**********************************************************************************************
     *  Seccion 3
     */

    { NOTE_MASK_SI,                  95 },
    { 0,                             25 },
    { NOTE_MASK_SI,                  95 },
    { 0,                             25 },
    { NOTE_MASK_RE,                 180 },
    { 0,                             35 },
    { NOTE_MASK_LA,                 110 },
    { 0,                             25 },
    { NOTE_MASK_SOLS,               220 },
    { 0,                             40 },
    { NOTE_MASK_SOL,                110 },
    { 0,                             25 },
    { NOTE_MASK_FA,                 110 },
    { 0,                             25 },
    { NOTE_MASK_RE,                  95 },
    { 0,                             25 },
    { NOTE_MASK_FA,                 110 },
    { 0,                             25 },
    { NOTE_MASK_SOL,                320 },
    { 0,                            100 },

    /**********************************************************************************************
     *  Seccion 4
     *
     *  Sib = La# = NOTE_MASK_LAS
     */

    { NOTE_MASK_LAS,                 95 },
    { 0,                             25 },
    { NOTE_MASK_LAS,                 95 },
    { 0,                             25 },
    { NOTE_MASK_RE,                 180 },
    { 0,                             35 },
    { NOTE_MASK_LA,                 110 },
    { 0,                             25 },
    { NOTE_MASK_SOLS,               220 },
    { 0,                             40 },
    { NOTE_MASK_SOL,                110 },
    { 0,                             25 },
    { NOTE_MASK_FA,                 110 },
    { 0,                             25 },
    { NOTE_MASK_RE,                  95 },
    { 0,                             25 },
    { NOTE_MASK_FA,                 110 },
    { 0,                             25 },
    { NOTE_MASK_SOL,                420 },
    { 0,                            200 }
};

/**************************************************************************************************
 *  Melodia 3
 *
 *  Adaptacion transpuesta de "In the Hall of the Mountain King".
 *
 *  Solucion:
 *      Transponer el patron a RE menor.
 *
 *  Ventaja:
 *      El tema queda dentro de una zona estable de la octava:
 *          RE - MI - FA - SOL - LA
 *
 *  Esto evita cruces raros por el limite DO/SI del teclado.
 */

static const MelodyEvent_t melody_3[] =
{
    /**********************************************************************************************
     *  Frase 1: lenta
     */

    { NOTE_MASK_RE,                 220 },
    { 0,                             40 },
    { NOTE_MASK_MI,                 220 },
    { 0,                             40 },
    { NOTE_MASK_FA,                 220 },
    { 0,                             40 },
    { NOTE_MASK_SOL,                220 },
    { 0,                             40 },
    { NOTE_MASK_LA,                 220 },
    { 0,                             40 },
    { NOTE_MASK_FA,                 220 },
    { 0,                             40 },
    { NOTE_MASK_LA,                 440 },
    { 0,                            100 },

    { NOTE_MASK_SOLS,               220 },
    { 0,                             40 },
    { NOTE_MASK_MI,                 220 },
    { 0,                             40 },
    { NOTE_MASK_SOLS,               220 },
    { 0,                             40 },
    { NOTE_MASK_LA,                 220 },
    { 0,                             40 },
    { NOTE_MASK_FA,                 220 },
    { 0,                             40 },
    { NOTE_MASK_RE,                 220 },
    { 0,                             40 },
    { NOTE_MASK_FA,                 440 },
    { 0,                            120 },

    /**********************************************************************************************
     *  Frase 2: un poco mas rapida
     */

    { NOTE_MASK_RE,                 170 },
    { 0,                             35 },
    { NOTE_MASK_MI,                 170 },
    { 0,                             35 },
    { NOTE_MASK_FA,                 170 },
    { 0,                             35 },
    { NOTE_MASK_SOL,                170 },
    { 0,                             35 },
    { NOTE_MASK_LA,                 170 },
    { 0,                             35 },
    { NOTE_MASK_FA,                 170 },
    { 0,                             35 },
    { NOTE_MASK_LA,                 340 },
    { 0,                             85 },

    { NOTE_MASK_SOLS,               170 },
    { 0,                             35 },
    { NOTE_MASK_MI,                 170 },
    { 0,                             35 },
    { NOTE_MASK_SOLS,               170 },
    { 0,                             35 },
    { NOTE_MASK_LA,                 170 },
    { 0,                             35 },
    { NOTE_MASK_FA,                 170 },
    { 0,                             35 },
    { NOTE_MASK_RE,                 170 },
    { 0,                             35 },
    { NOTE_MASK_FA,                 340 },
    { 0,                            100 },

    /**********************************************************************************************
     *  Frase 3: rapida
     */

    { NOTE_MASK_RE,                 130 },
    { 0,                             25 },
    { NOTE_MASK_MI,                 130 },
    { 0,                             25 },
    { NOTE_MASK_FA,                 130 },
    { 0,                             25 },
    { NOTE_MASK_SOL,                130 },
    { 0,                             25 },
    { NOTE_MASK_LA,                 130 },
    { 0,                             25 },
    { NOTE_MASK_FA,                 130 },
    { 0,                             25 },
    { NOTE_MASK_LA,                 260 },
    { 0,                             65 },

    { NOTE_MASK_SOLS,               130 },
    { 0,                             25 },
    { NOTE_MASK_MI,                 130 },
    { 0,                             25 },
    { NOTE_MASK_SOLS,               130 },
    { 0,                             25 },
    { NOTE_MASK_LA,                 130 },
    { 0,                             25 },
    { NOTE_MASK_FA,                 130 },
    { 0,                             25 },
    { NOTE_MASK_RE,                 130 },
    { 0,                             25 },
    { NOTE_MASK_FA,                 260 },
    { 0,                             80 },

    /**********************************************************************************************
     *  Frase 4: cierre rapido
     */

    { NOTE_MASK_RE,                 100 },
    { 0,                             20 },
    { NOTE_MASK_MI,                 100 },
    { 0,                             20 },
    { NOTE_MASK_FA,                 100 },
    { 0,                             20 },
    { NOTE_MASK_SOL,                100 },
    { 0,                             20 },
    { NOTE_MASK_LA,                 100 },
    { 0,                             20 },
    { NOTE_MASK_FA,                 100 },
    { 0,                             20 },
    { NOTE_MASK_LA,                 220 },
    { 0,                             50 },

    { NOTE_MASK_RE,                 520 },
    { 0,                            200 }
};

/**************************************************************************************************
 *  Melody_Init
 */

void Melody_Init(void)
{
    current_melody = 0;
    current_length = 0;
    current_index = 0;
    current_notes = 0;
    remaining_ms = 0;
    is_playing = 0;

    secret_candidate = 0;
    secret_start_ms = 0;
    secret_locked = 0;

    melody_playing_debug = 0;
    melody_id_debug = MELODY_NONE;
    melody_channels_debug = 0;
    melody_index_debug = 0;
}

/**************************************************************************************************
 *  Melody_Start
 */

void Melody_Start(uint8_t melody_id)
{
    switch(melody_id)
    {
        case MELODY_1:
            current_melody = melody_1;
            current_length = sizeof(melody_1) / sizeof(melody_1[0]);
            melody_id_debug = MELODY_1;
            break;

        case MELODY_2:
            current_melody = melody_2;
            current_length = sizeof(melody_2) / sizeof(melody_2[0]);
            melody_id_debug = MELODY_2;
            break;

        case MELODY_3:
            current_melody = melody_3;
            current_length = sizeof(melody_3) / sizeof(melody_3[0]);
            melody_id_debug = MELODY_3;
            break;

        default:
            return;
    }

    current_index = 0;
    current_notes = current_melody[0].notes;
    remaining_ms = current_melody[0].duration_ms;
    is_playing = 1;

    melody_playing_debug = 1;
    melody_channels_debug = current_notes;
    melody_index_debug = current_index;
}

/**************************************************************************************************
 *  Melody_Stop
 */

void Melody_Stop(void)
{
    current_melody = 0;
    current_length = 0;
    current_index = 0;
    current_notes = 0;
    remaining_ms = 0;
    is_playing = 0;

    melody_playing_debug = 0;
    melody_id_debug = MELODY_NONE;
    melody_channels_debug = 0;
    melody_index_debug = 0;
}

/**************************************************************************************************
 *  Melody_Update1ms
 *
 *  Debe llamarse cada 1 ms desde SysTick_Handler.
 */

void Melody_Update1ms(void)
{
    if (is_playing == 0)
    {
        return;
    }

    if (remaining_ms > 0)
    {
        remaining_ms--;
    }

    if (remaining_ms == 0)
    {
        current_index++;

        if (current_index >= current_length)
        {
            Melody_Stop();
        }
        else
        {
            current_notes = current_melody[current_index].notes;
            remaining_ms = current_melody[current_index].duration_ms;

            melody_channels_debug = current_notes;
            melody_index_debug = current_index;
        }
    }
}

/**************************************************************************************************
 *  Melody_IsPlaying
 */

uint8_t Melody_IsPlaying(void)
{
    return is_playing;
}

/**************************************************************************************************
 *  Melody_GetCurrentChannels
 */

uint16_t Melody_GetCurrentChannels(void)
{
    return current_notes;
}

/**************************************************************************************************
 *  Melody_GetSecretId
 */

static uint8_t Melody_GetSecretId(uint16_t notes)
{
    if (notes == SECRET_1_MASK)
    {
        return MELODY_1;
    }

    if (notes == SECRET_2_MASK)
    {
        return MELODY_2;
    }

    if (notes == SECRET_3_MASK)
    {
        return MELODY_3;
    }

    return MELODY_NONE;
}

/**************************************************************************************************
 *  Melody_CheckSecretCombination
 *
 *  Si una combinacion secreta se mantiene durante SECRET_HOLD_MS,
 *  se reproduce la melodia correspondiente.
 */

void Melody_CheckSecretCombination(uint16_t stable_notes)
{
    uint8_t secret_id;
    uint32_t now_ms;

    if (is_playing != 0)
    {
        return;
    }

    if (stable_notes == 0)
    {
        secret_candidate = 0;
        secret_start_ms = 0;
        secret_locked = 0;
        return;
    }

    if (secret_locked != 0)
    {
        return;
    }

    secret_id = Melody_GetSecretId(stable_notes);

    if (secret_id == MELODY_NONE)
    {
        secret_candidate = 0;
        secret_start_ms = 0;
        return;
    }

    now_ms = SysTick_GetMs();

    if (stable_notes != secret_candidate)
    {
        secret_candidate = stable_notes;
        secret_start_ms = now_ms;
        return;
    }

    if ((now_ms - secret_start_ms) >= SECRET_HOLD_MS)
    {
        Melody_Start(secret_id);
        secret_locked = 1;
    }
}
