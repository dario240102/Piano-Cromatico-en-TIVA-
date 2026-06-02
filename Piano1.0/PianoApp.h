#ifndef PIANOAPP_H_
#define PIANOAPP_H_

#include <stdint.h>

extern volatile uint16_t piano_notes_debug;
extern volatile uint16_t piano_output_debug;
extern volatile uint32_t piano_update_counter;
extern volatile uint8_t piano_volume_debug;
extern volatile uint8_t piano_octave_debug;
extern const char *piano_chord_debug;

void PianoApp_Init(void);
void PianoApp_Update(void);

#endif /* PIANOAPP_H_ */
