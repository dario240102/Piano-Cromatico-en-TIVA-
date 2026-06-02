#include <stdint.h>
#include "Notes.h"

/**************************************************************************************************
 *  Notes_GetNameFromChannel
 */

const char* Notes_GetNameFromChannel(uint8_t channel)
{
    switch(channel)
    {
        case NOTE_CH_DO:
            return "DO";

        case NOTE_CH_DOS:
            return "DO#";

        case NOTE_CH_RE:
            return "RE";

        case NOTE_CH_RES:
            return "RE#";

        case NOTE_CH_MI:
            return "MI";

        case NOTE_CH_FA:
            return "FA";

        case NOTE_CH_FAS:
            return "FA#";

        case NOTE_CH_SOL:
            return "SOL";

        case NOTE_CH_SOLS:
            return "SOL#";

        case NOTE_CH_LA:
            return "LA";

        case NOTE_CH_LAS:
            return "LA#";

        case NOTE_CH_SI:
            return "SI";

        default:
            return "--";
    }
}
