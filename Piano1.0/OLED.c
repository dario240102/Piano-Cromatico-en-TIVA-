#include <stdint.h>
#include "Config.h"
#include "I2C.h"
#include "Notes.h"
#include "SysTick.h"
#include "OLED.h"

/**************************************************************************************************
 *  SSD1306 control bytes
 */

#define OLED_CONTROL_COMMAND            0x00
#define OLED_CONTROL_DATA               0x40

/**************************************************************************************************
 *  Variables de diagnostico
 */

volatile uint8_t oled_ready_debug = 0;

/**************************************************************************************************
 *  Prototipos internos
 */

static void OLED_DelayMs(uint32_t delay_ms);

static uint8_t OLED_CommandRaw(uint8_t command);
static uint8_t OLED_DataRaw(uint8_t data);

static void OLED_Command(uint8_t command);
static void OLED_Data(uint8_t data);

static void OLED_ClearLine(uint8_t page);

static const uint8_t* OLED_GetFont(char c);
static void OLED_WriteChar(char c);
static uint8_t OLED_StrLen(const char *text);

/**************************************************************************************************
 *  Fuente 5x7 parcial
 */

static const uint8_t font_space[5]   = {0x00,0x00,0x00,0x00,0x00};
static const uint8_t font_colon[5]   = {0x00,0x36,0x36,0x00,0x00};
static const uint8_t font_percent[5] = {0x62,0x64,0x08,0x13,0x23};
static const uint8_t font_dash[5]    = {0x08,0x08,0x08,0x08,0x08};
static const uint8_t font_hash[5]    = {0x14,0x7F,0x14,0x7F,0x14};

static const uint8_t font_0[5] = {0x3E,0x51,0x49,0x45,0x3E};
static const uint8_t font_1[5] = {0x00,0x42,0x7F,0x40,0x00};
static const uint8_t font_2[5] = {0x42,0x61,0x51,0x49,0x46};
static const uint8_t font_3[5] = {0x21,0x41,0x45,0x4B,0x31};
static const uint8_t font_4[5] = {0x18,0x14,0x12,0x7F,0x10};
static const uint8_t font_5[5] = {0x27,0x45,0x45,0x45,0x39};
static const uint8_t font_6[5] = {0x3C,0x4A,0x49,0x49,0x30};
static const uint8_t font_7[5] = {0x01,0x71,0x09,0x05,0x03};
static const uint8_t font_8[5] = {0x36,0x49,0x49,0x49,0x36};
static const uint8_t font_9[5] = {0x06,0x49,0x49,0x29,0x1E};

static const uint8_t font_A[5] = {0x7E,0x11,0x11,0x11,0x7E};
static const uint8_t font_B[5] = {0x7F,0x49,0x49,0x49,0x36};
static const uint8_t font_C[5] = {0x3E,0x41,0x41,0x41,0x22};
static const uint8_t font_D[5] = {0x7F,0x41,0x41,0x22,0x1C};
static const uint8_t font_E[5] = {0x7F,0x49,0x49,0x49,0x41};
static const uint8_t font_F[5] = {0x7F,0x09,0x09,0x09,0x01};
static const uint8_t font_G[5] = {0x3E,0x41,0x49,0x49,0x7A};
static const uint8_t font_H[5] = {0x7F,0x08,0x08,0x08,0x7F};
static const uint8_t font_I[5] = {0x00,0x41,0x7F,0x41,0x00};
static const uint8_t font_J[5] = {0x20,0x40,0x41,0x3F,0x01};
static const uint8_t font_K[5] = {0x7F,0x08,0x14,0x22,0x41};
static const uint8_t font_L[5] = {0x7F,0x40,0x40,0x40,0x40};
static const uint8_t font_M[5] = {0x7F,0x02,0x0C,0x02,0x7F};
static const uint8_t font_N[5] = {0x7F,0x04,0x08,0x10,0x7F};
static const uint8_t font_O[5] = {0x3E,0x41,0x41,0x41,0x3E};
static const uint8_t font_P[5] = {0x7F,0x09,0x09,0x09,0x06};
static const uint8_t font_Q[5] = {0x3E,0x41,0x51,0x21,0x5E};
static const uint8_t font_R[5] = {0x7F,0x09,0x19,0x29,0x46};
static const uint8_t font_S[5] = {0x46,0x49,0x49,0x49,0x31};
static const uint8_t font_T[5] = {0x01,0x01,0x7F,0x01,0x01};
static const uint8_t font_U[5] = {0x3F,0x40,0x40,0x40,0x3F};
static const uint8_t font_V[5] = {0x1F,0x20,0x40,0x20,0x1F};
static const uint8_t font_W[5] = {0x7F,0x20,0x18,0x20,0x7F};
static const uint8_t font_X[5] = {0x63,0x14,0x08,0x14,0x63};
static const uint8_t font_Y[5] = {0x07,0x08,0x70,0x08,0x07};
static const uint8_t font_Z[5] = {0x61,0x51,0x49,0x45,0x43};

/**************************************************************************************************
 *  OLED_DelayMs
 */

static void OLED_DelayMs(uint32_t delay_ms)
{
    uint32_t start;

    start = SysTick_GetMs();

    while ((SysTick_GetMs() - start) < delay_ms)
    {
    }
}

/**************************************************************************************************
 *  OLED_CommandRaw
 */

static uint8_t OLED_CommandRaw(uint8_t command)
{
    return I2C0_Write2Bytes(OLED_I2C_ADDRESS, OLED_CONTROL_COMMAND, command);
}

/**************************************************************************************************
 *  OLED_DataRaw
 */

static uint8_t OLED_DataRaw(uint8_t data)
{
    return I2C0_Write2Bytes(OLED_I2C_ADDRESS, OLED_CONTROL_DATA, data);
}

/**************************************************************************************************
 *  OLED_Command
 */

static void OLED_Command(uint8_t command)
{
    if (oled_ready_debug != 0)
    {
        OLED_CommandRaw(command);
    }
}

/**************************************************************************************************
 *  OLED_Data
 */

static void OLED_Data(uint8_t data)
{
    if (oled_ready_debug != 0)
    {
        OLED_DataRaw(data);
    }
}

/**************************************************************************************************
 *  OLED_Init
 *
 *  Inicializacion para OLED SSD1306 128x32.
 */

uint8_t OLED_Init(void)
{
    OLED_DelayMs(50);

    if (OLED_CommandRaw(0xAE) == 0)
    {
        oled_ready_debug = 0;
        return 0;
    }

    oled_ready_debug = 1;

    OLED_Command(0xD5);
    OLED_Command(0x80);

    OLED_Command(0xA8);
    OLED_Command(0x1F);      /* 128x32 */

    OLED_Command(0xD3);
    OLED_Command(0x00);

    OLED_Command(0x40);

    OLED_Command(0x8D);
    OLED_Command(0x14);

    OLED_Command(0x20);
    OLED_Command(0x02);      /* Page addressing mode */

    OLED_Command(0xA1);
    OLED_Command(0xC8);

    OLED_Command(0xDA);
    OLED_Command(0x02);      /* 128x32 COM pins */

    OLED_Command(0x81);
    OLED_Command(0x7F);

    OLED_Command(0xD9);
    OLED_Command(0xF1);

    OLED_Command(0xDB);
    OLED_Command(0x40);

    OLED_Command(0xA4);
    OLED_Command(0xA6);

    OLED_Command(0x2E);

    OLED_Command(0xAF);

    OLED_Clear();

    return 1;
}

/**************************************************************************************************
 *  OLED_IsReady
 */

uint8_t OLED_IsReady(void)
{
    return oled_ready_debug;
}

/**************************************************************************************************
 *  OLED_Clear
 */

void OLED_Clear(void)
{
    uint8_t page;
    uint8_t col;

    if (oled_ready_debug == 0)
    {
        return;
    }

    for (page = 0; page < OLED_PAGES; page++)
    {
        OLED_SetCursor(page, 0);

        for (col = 0; col < OLED_WIDTH; col++)
        {
            OLED_Data(0x00);
        }
    }

    OLED_SetCursor(0, 0);
}

/**************************************************************************************************
 *  OLED_ClearLine
 *
 *  Borra una sola pagina de texto.
 *  Esto evita borrar toda la pantalla en cada actualizacion.
 */

static void OLED_ClearLine(uint8_t page)
{
    uint8_t col;

    if (oled_ready_debug == 0)
    {
        return;
    }

    if (page >= OLED_PAGES)
    {
        page = OLED_PAGES - 1;
    }

    OLED_SetCursor(page, 0);

    for (col = 0; col < OLED_WIDTH; col++)
    {
        OLED_Data(0x00);
    }

    OLED_SetCursor(page, 0);
}

/**************************************************************************************************
 *  OLED_SetCursor
 *
 *  page:
 *      0 a 3 para pantalla 128x32.
 */

void OLED_SetCursor(uint8_t page, uint8_t col)
{
    if (oled_ready_debug == 0)
    {
        return;
    }

    if (page >= OLED_PAGES)
    {
        page = OLED_PAGES - 1;
    }

    if (col >= OLED_WIDTH)
    {
        col = OLED_WIDTH - 1;
    }

    OLED_Command(0xB0 | (page & 0x07));
    OLED_Command(0x00 | (col & 0x0F));
    OLED_Command(0x10 | ((col >> 4) & 0x0F));
}

/**************************************************************************************************
 *  OLED_GetFont
 */

static const uint8_t* OLED_GetFont(char c)
{
    if ((c >= 'a') && (c <= 'z'))
    {
        c = c - 32;
    }

    switch(c)
    {
        case ' ': return font_space;
        case ':': return font_colon;
        case '%': return font_percent;
        case '-': return font_dash;
        case '#': return font_hash;

        case '0': return font_0;
        case '1': return font_1;
        case '2': return font_2;
        case '3': return font_3;
        case '4': return font_4;
        case '5': return font_5;
        case '6': return font_6;
        case '7': return font_7;
        case '8': return font_8;
        case '9': return font_9;

        case 'A': return font_A;
        case 'B': return font_B;
        case 'C': return font_C;
        case 'D': return font_D;
        case 'E': return font_E;
        case 'F': return font_F;
        case 'G': return font_G;
        case 'H': return font_H;
        case 'I': return font_I;
        case 'J': return font_J;
        case 'K': return font_K;
        case 'L': return font_L;
        case 'M': return font_M;
        case 'N': return font_N;
        case 'O': return font_O;
        case 'P': return font_P;
        case 'Q': return font_Q;
        case 'R': return font_R;
        case 'S': return font_S;
        case 'T': return font_T;
        case 'U': return font_U;
        case 'V': return font_V;
        case 'W': return font_W;
        case 'X': return font_X;
        case 'Y': return font_Y;
        case 'Z': return font_Z;

        default:  return font_space;
    }
}

/**************************************************************************************************
 *  OLED_WriteChar
 */

static void OLED_WriteChar(char c)
{
    const uint8_t *bitmap;
    uint8_t i;

    bitmap = OLED_GetFont(c);

    for (i = 0; i < 5; i++)
    {
        OLED_Data(bitmap[i]);
    }

    OLED_Data(0x00);
}

/**************************************************************************************************
 *  OLED_Print
 */

void OLED_Print(const char *text)
{
    if (oled_ready_debug == 0)
    {
        return;
    }

    while (*text != '\0')
    {
        OLED_WriteChar(*text);
        text++;
    }
}

/**************************************************************************************************
 *  OLED_PrintUInt
 */

void OLED_PrintUInt(uint32_t number)
{
    char buffer[11];
    int8_t index;

    if (oled_ready_debug == 0)
    {
        return;
    }

    if (number == 0)
    {
        OLED_WriteChar('0');
        return;
    }

    index = 0;

    while ((number > 0) && (index < 10))
    {
        buffer[index] = (char)('0' + (number % 10));
        number = number / 10;
        index++;
    }

    while (index > 0)
    {
        index--;
        OLED_WriteChar(buffer[index]);
    }
}

/**************************************************************************************************
 *  OLED_StrLen
 */

static uint8_t OLED_StrLen(const char *text)
{
    uint8_t len;

    len = 0;

    while (text[len] != '\0')
    {
        len++;
    }

    return len;
}

/**************************************************************************************************
 *  OLED_PrintNotes
 */

void OLED_PrintNotes(uint16_t notes)
{
    uint8_t channel;
    uint8_t used_cols;
    uint8_t len;
    const char *name;

    if (oled_ready_debug == 0)
    {
        return;
    }

    used_cols = 0;

    if (notes == 0)
    {
        OLED_Print("SIN NOTAS");
        return;
    }

    for (channel = 0; channel < NUM_CHANNELS; channel++)
    {
        if ((notes & (1 << channel)) != 0)
        {
            name = Notes_GetNameFromChannel(channel);
            len = OLED_StrLen(name);

            if ((used_cols + len) > 20)
            {
                return;
            }

            OLED_Print(name);
            used_cols += len;

            if (used_cols < 21)
            {
                OLED_Print(" ");
                used_cols++;
            }
        }
    }
}

/**************************************************************************************************
 *  OLED_ShowPianoStatus
 *
 *  Formato para OLED 128x32:
 *
 *      AC:DO MAYOR
 *      NOT:DO MI SOL
 *      VOL:75% OCT:4
 *      PIANO TIVA
 */

void OLED_ShowPianoStatus(const char *chord,
                          uint16_t notes,
                          uint8_t volume_percent,
                          uint8_t octave)
{
    if (oled_ready_debug == 0)
    {
        return;
    }

    OLED_ClearLine(0);
    OLED_Print("AC:");
    OLED_Print(chord);

    OLED_ClearLine(1);
    OLED_Print("NOT:");
    OLED_PrintNotes(notes);

    OLED_ClearLine(2);
    OLED_Print("VOL:");
    OLED_PrintUInt(volume_percent);
    OLED_Print("%");

    OLED_SetCursor(2, 72);
    OLED_Print("OCT:");
    OLED_PrintUInt(octave);

    OLED_ClearLine(3);
    OLED_Print("PIANO TIVA");
}
