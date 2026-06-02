#ifndef CHORDS_H_
#define CHORDS_H_

#include <stdint.h>

extern volatile uint16_t chord_mask_debug;

const char* Chords_Detect(uint16_t notes);

#endif /* CHORDS_H_ */
