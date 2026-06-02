#ifndef NOTES_H_
#define NOTES_H_

#include <stdint.h>
#include "Config.h"

/**************************************************************************************************
 *  Asignacion musical segun orientacion del teclado
 */

#if KEYBOARD_REVERSED

#define NOTE_CH_SI                      CH_0
#define NOTE_CH_LAS                     CH_1
#define NOTE_CH_LA                      CH_2
#define NOTE_CH_SOLS                    CH_3
#define NOTE_CH_SOL                     CH_4
#define NOTE_CH_FAS                     CH_5
#define NOTE_CH_FA                      CH_6
#define NOTE_CH_MI                      CH_7
#define NOTE_CH_RES                     CH_8
#define NOTE_CH_RE                      CH_9
#define NOTE_CH_DOS                     CH_10
#define NOTE_CH_DO                      CH_11

#else

#define NOTE_CH_DO                      CH_0
#define NOTE_CH_DOS                     CH_1
#define NOTE_CH_RE                      CH_2
#define NOTE_CH_RES                     CH_3
#define NOTE_CH_MI                      CH_4
#define NOTE_CH_FA                      CH_5
#define NOTE_CH_FAS                     CH_6
#define NOTE_CH_SOL                     CH_7
#define NOTE_CH_SOLS                    CH_8
#define NOTE_CH_LA                      CH_9
#define NOTE_CH_LAS                     CH_10
#define NOTE_CH_SI                      CH_11

#endif

/**************************************************************************************************
 *  Mascaras de notas
 */

#define NOTE_MASK_DO                    (1 << NOTE_CH_DO)
#define NOTE_MASK_DOS                   (1 << NOTE_CH_DOS)
#define NOTE_MASK_RE                    (1 << NOTE_CH_RE)
#define NOTE_MASK_RES                   (1 << NOTE_CH_RES)
#define NOTE_MASK_MI                    (1 << NOTE_CH_MI)
#define NOTE_MASK_FA                    (1 << NOTE_CH_FA)
#define NOTE_MASK_FAS                   (1 << NOTE_CH_FAS)
#define NOTE_MASK_SOL                   (1 << NOTE_CH_SOL)
#define NOTE_MASK_SOLS                  (1 << NOTE_CH_SOLS)
#define NOTE_MASK_LA                    (1 << NOTE_CH_LA)
#define NOTE_MASK_LAS                   (1 << NOTE_CH_LAS)
#define NOTE_MASK_SI                    (1 << NOTE_CH_SI)

/**************************************************************************************************
 *  Prototipos
 */

const char* Notes_GetNameFromChannel(uint8_t channel);

#endif /* NOTES_H_ */
