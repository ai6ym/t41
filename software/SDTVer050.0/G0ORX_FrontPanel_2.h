#ifndef G0ORX_FRONTPANEL_2_H
#define G0ORX_FRONTPANEL_2_H

#include <stdint.h>

#define AUDIO_VOLUME 0
#define MIC_GAIN 1
#define AGC_GAIN 2
#define SIDETONE_VOLUME 3
#define NOISE_FLOOR_LEVEL 4
#define RF_GAIN 5
#define SQUELCH_LEVEL 6

extern int G0ORXButtonPressed;
extern int touchPrimaryMenuIndex;
extern int touchSecondaryMenuIndex;
extern int volumeFunction;

extern void FrontPanel2Init();
extern uint16_t get_interrupt_mask();
extern void getKeypad();
extern int16_t get_encoder_value(uint8_t encoder);
extern uint8_t get_switch_state(uint8_t sw);
extern void get_touch();
extern void FrontPanelSetLed(int led, bool state);

#endif // G0ORX_FRONTPANEL_2_H