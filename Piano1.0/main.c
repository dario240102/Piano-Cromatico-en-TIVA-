#include <stdint.h>
#include "PianoApp.h"

/**************************************************************************************************
 *  main
 */

int main(void)
{
    PianoApp_Init();

    while (1)
    {
        PianoApp_Update();
    }
}
