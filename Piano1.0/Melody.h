#ifndef MELODY_H_
#define MELODY_H_

#include <stdint.h>

#define MELODY_NONE                     0
#define MELODY_1                        1
#define MELODY_2                        2
#define MELODY_3                        3

extern volatile uint8_t melody_playing_debug;
extern volatile uint8_t melody_id_debug;
extern volatile uint16_t melody_channels_debug;
extern volatile uint8_t melody_index_debug;

void Melody_Init(void);
void Melody_Update1ms(void);

void Melody_Start(uint8_t melody_id);
void Melody_Stop(void);

uint8_t Melody_IsPlaying(void);
uint16_t Melody_GetCurrentChannels(void);

void Melody_CheckSecretCombination(uint16_t stable_notes);

#endif /* MELODY_H_ */
