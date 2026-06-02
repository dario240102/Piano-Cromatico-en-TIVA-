#ifndef AUDIO_H_
#define AUDIO_H_

#include <stdint.h>

extern volatile uint16_t audio_active_debug;
extern volatile uint8_t audio_octave_debug;
extern volatile uint8_t audio_volume_debug;

void Audio_Init(void);

void Audio_SetActiveChannels(uint16_t channels);
uint16_t Audio_GetActiveChannels(void);

void Audio_SetOctave(uint8_t octave);
uint8_t Audio_GetOctave(void);

void Audio_SetVolumePercent(uint8_t volume_percent);
uint8_t Audio_GetVolumePercent(void);

void Audio_UpdateTick(void);
void Audio_StopAll(void);

#endif /* AUDIO_H_ */
