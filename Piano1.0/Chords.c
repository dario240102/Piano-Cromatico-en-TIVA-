#include <stdint.h>
#include "Notes.h"
#include "Chords.h"

/**************************************************************************************************
 *  Variable de depuracion
 */

volatile uint16_t chord_mask_debug = 0;

/**************************************************************************************************
 *  Mascaras de acordes
 */

#define CHORD_DO_MAYOR                  (NOTE_MASK_DO  | NOTE_MASK_MI   | NOTE_MASK_SOL)
#define CHORD_RE_MENOR                  (NOTE_MASK_RE  | NOTE_MASK_FA   | NOTE_MASK_LA)
#define CHORD_MI_MENOR                  (NOTE_MASK_MI  | NOTE_MASK_SOL  | NOTE_MASK_SI)
#define CHORD_FA_MAYOR                  (NOTE_MASK_FA  | NOTE_MASK_LA   | NOTE_MASK_DO)
#define CHORD_SOL_MAYOR                 (NOTE_MASK_SOL | NOTE_MASK_SI   | NOTE_MASK_RE)
#define CHORD_LA_MENOR                  (NOTE_MASK_LA  | NOTE_MASK_DO   | NOTE_MASK_MI)

#define CHORD_DO_MENOR                  (NOTE_MASK_DO  | NOTE_MASK_RES  | NOTE_MASK_SOL)
#define CHORD_RE_MAYOR                  (NOTE_MASK_RE  | NOTE_MASK_FAS  | NOTE_MASK_LA)
#define CHORD_MI_MAYOR                  (NOTE_MASK_MI  | NOTE_MASK_SOLS | NOTE_MASK_SI)
#define CHORD_LA_MAYOR                  (NOTE_MASK_LA  | NOTE_MASK_DOS  | NOTE_MASK_MI)

/**************************************************************************************************
 *  Chords_Detect
 *
 *  Detecta acordes basicos a partir de una mascara exacta.
 */

const char* Chords_Detect(uint16_t notes)
{
    chord_mask_debug = notes;

    switch(notes)
    {
        case CHORD_DO_MAYOR:
            return "DO Mayor";

        case CHORD_RE_MENOR:
            return "RE menor";

        case CHORD_MI_MENOR:
            return "MI menor";

        case CHORD_FA_MAYOR:
            return "FA Mayor";

        case CHORD_SOL_MAYOR:
            return "SOL Mayor";

        case CHORD_LA_MENOR:
            return "LA menor";

        case CHORD_DO_MENOR:
            return "DO menor";

        case CHORD_RE_MAYOR:
            return "RE Mayor";

        case CHORD_MI_MAYOR:
            return "MI Mayor";

        case CHORD_LA_MAYOR:
            return "LA Mayor";

        default:
            return "Sin acorde";
    }
}
