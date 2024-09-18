/*
 * G0ORX New Front Panel.
 *
 * i2c interface to Raspberry Pi Pico acting as a i2c slave device.
 *
 * Supports 4 Encoders, 4 Encoder Switches, Touch Screen
 *
 */

#ifndef BEENHERE
#include "SDT.h"
#endif

#ifdef G0ORX_FRONTPANEL_2

#include <Wire.h>

#define I2C_ADDRESS 0x22
bool found_pico = false;
bool configured_pico = false;

#define CONFIG_REG 0x00
#define RESET_REG 0x01
#define READ_INTERRUPT_MASK_REG 0x02
#define READ_ENCODER_REG 0x03
#define READ_SWITCH_REG 0x04
#define READ_TOUCH_REG 0x05
#define SET_LED_REG 0x06

#define EN0_INT 0x01
#define EN1_INT 0x02
#define EN2_INT 0x04
#define EN3_INT 0x08
#define SW0_INT 0x10
#define SW1_INT 0x20
#define SW2_INT 0x40
#define SW3_INT 0x80
#define TS_INT 0x100

#define LED0 0x01
#define LED1 0x02
#define LED_ON 0x04
#define LED_OFF 0x00

#define PICO_INTERRUPT_PIN 15

volatile uint16_t config=0x0F0;  // Active Low Interrupt, Invert SW, Invert SW, Invert SW, Invert SW, FWD, FWD, FWD, FWD

volatile uint16_t interrupt_mask;

#define TOUCH_PRESSED 0x02
#define TOUCH_RELEASED 0x01
volatile uint8_t touch_status;
volatile int16_t touch_X;
volatile int16_t touch_Y;
volatile bool pressed = false;
volatile bool has_moved = false;
volatile long start_freq;
volatile long new_freq;

static long zoomBandwidth[5]= {192000L,96000L,48000L,24000L,12000L};

volatile int16_t last_X;

volatile int16_t encoder_value[4] = {0,0,0,0};
int G0ORXButtonPressed = -1;
int touchPrimaryMenuIndex = -1;
int touchSecondaryMenuIndex = -1;

static const uint8_t my_keymap[22] = { // note 18 and 19 are not connected
    0,
    0,  1,  2, 3, 4,
    5,  6,  7, 8, 9,
    10,  11, 12, 13, 14,
    15,  16, 17, 18, 19,
    255
};

static bool keyPressed=false;

FASTRUN
void KeypadInterrupt() {
  //__disable_irq();
  keyPressed=true;
  //__enable_irq();
}

static void KeypadInit() {
  tft.KeypadInit(true, false, 0, 0, 0, true, false);
  tft.SetKeyMap(my_keymap);
  attachInterrupt(digitalPinToInterrupt(TFT_INTERRUPT), KeypadInterrupt, FALLING);
}

FASTRUN
void getKeypad() {
/*
  if(keyPressed) {
    int val=tft.KeypadGetKey();
    if(val!=0xFF) {
      G0ORXButtonPressed=val;
    } else {
      G0ORXButtonPressed=-1;
    }
    keyPressed=false;
  }
*/

  if(G0ORXButtonPressed==-1) {
    int val=tft.KeypadGetKey();
    if(val!=0xFF) {
      G0ORXButtonPressed=val;
    } else {
      G0ORXButtonPressed=-1;
    }
  }
}

// initial processing only to handle frequency change

static void update_frequency(long new_freq) {
  Debug(String(__FUNCTION__)+": "+String(new_freq));
  if (T41State == CW_XMIT && decoderFlag == DECODE_ON) {
    ResetHistograms();
  }
  centerFreq = new_freq;
  TxRxFreq = centerFreq + NCOFreq;
  centerTuneFlag = 1;
}

FASTRUN
static void process_touch() {
  Debug(String(__FUNCTION__));
  if(menuStatus==NO_MENUS_ACTIVE) {
    long hzPerPixel = zoomBandwidth[spectrum_zoom]/SPECTRUM_RES;
    if(touch_status==TOUCH_PRESSED) {
      if(!pressed) {
        if(touch_X<SPECTRUM_RES) {
          pressed = true;
          has_moved = false;
          last_X = touch_X;
        }
      } else {
        long moved = (long)last_X - (long)touch_X;
        if(has_moved || moved>=10 || moved <=-10) {
          has_moved = true;
          last_X = touch_X;
          if (activeVFO == VFO_A) {
            start_freq = currentFreqA;
          } else {
            start_freq = currentFreqB;
          }
          new_freq = start_freq + (moved * hzPerPixel);
          update_frequency(new_freq);
        }
      }
    } else if(touch_status==TOUCH_RELEASED) {
      if(pressed) {
        if(has_moved) {
          if (activeVFO == VFO_A) {
            start_freq = currentFreqA;
          } else {
            start_freq = currentFreqB;
          }
          new_freq = (start_freq/freqIncrement)*freqIncrement;
          update_frequency(new_freq);
        } else {
          int x=last_X-SPECTRUM_LEFT_X;
          if (activeVFO == VFO_A) {
            start_freq = currentFreqA - (zoomBandwidth[spectrum_zoom]/2);
          } else {
            start_freq = currentFreqB - (zoomBandwidth[spectrum_zoom]/2);
          }
          new_freq = start_freq + (x * hzPerPixel);
          new_freq = (new_freq/freqIncrement)*freqIncrement;
          update_frequency(new_freq);
      //centerTuneFlag = 1; //AFP 10-03-22
        }
        pressed = false;
        has_moved = false;
      }
    }
  } else {
    if(touch_status==TOUCH_RELEASED) {
      // ignore touch pressed - activate when released
      int i=(touch_Y-115)/25;
      if(touch_X<250) {
        //touchSelectPrimaryMenuIndex(i);
        touchPrimaryMenuIndex=i;
      } else if(touch_X>300) {
        touchSecondaryMenuIndex=i;
      //centerTuneFlag = 1; //AFP 10-03-22
      }
    }
  }
}


FASTRUN
uint8_t get_byte() {
  uint8_t byte=0;
  while(!Wire1.available()) {
    delay(10);
  }
  byte = Wire1.read();
  return byte;
}

FASTRUN
uint16_t get_interrupt_mask() {

  uint16_t mask;
  uint8_t res;
  
  Debug(String(__FUNCTION__));
  mask=0;
  Wire1.beginTransmission(I2C_ADDRESS);
  Wire1.write(READ_INTERRUPT_MASK_REG);
  res=Wire1.endTransmission(true);
  if(res==0) {
    Wire1.requestFrom(I2C_ADDRESS, 2);
    mask=get_byte()&0xFF;
    mask|=(get_byte()&0xFF)<<8;
  } else {
    Debug(String(__FUNCTION__)+": endTransmission: "+String(res,HEX));
  }
  Debug(String(__FUNCTION__)+": 0x"+String(mask,HEX));
  return mask;
}

FASTRUN
int16_t get_encoder_value(uint8_t encoder) {
  int16_t result=0;
  uint8_t res;

  Debug(String(__FUNCTION__)+": encoder="+String(encoder));
  Wire1.beginTransmission(I2C_ADDRESS);
  Wire1.write(READ_ENCODER_REG);
  Wire1.write(encoder);
  res=Wire1.endTransmission(true);
  if(res==0) {
    Wire1.requestFrom(I2C_ADDRESS, 2);  // request 2 bytes from Pico}
    result=get_byte()&0xFF;
    result|=(get_byte()&0xFF)<<8;

  } else {
    Debug(String(__FUNCTION__)+": endTransmission: "+String(res,HEX));
  }

  Debug(String(__FUNCTION__)+": "+String(result));
  return result;
}

FASTRUN
uint8_t get_switch_state(uint8_t sw) {
  uint8_t state=0;
  uint8_t res;
  Debug(String(__FUNCTION__)+": sw="+String(sw));
  Wire1.beginTransmission(I2C_ADDRESS);
  Wire1.write(READ_SWITCH_REG);
  Wire1.write(sw);
  res=Wire1.endTransmission(true);

  if(res==0) {
    Wire1.requestFrom(I2C_ADDRESS, 1);  // request 1 bytes from Pico
    state = get_byte();
  } else {
    Debug(String(__FUNCTION__)+": endTransmission: "+String(res,HEX));
  }
  return state;
}

FASTRUN
void get_touch() {
  uint8_t res;
  Wire1.beginTransmission(I2C_ADDRESS);
  Wire1.write(READ_TOUCH_REG);
  res=Wire1.endTransmission(true);
  if(res==0) {
    Wire1.requestFrom(I2C_ADDRESS, 5);
    touch_status = get_byte();
    touch_X=get_byte()&0xFF;
    touch_X|=(get_byte()&0xFF)<<8;
    touch_Y=get_byte()&0xFF;
    touch_Y|=(get_byte()&0xFF)<<8;
  }
  Debug(String(__FUNCTION__)+": 0x"+String(touch_status,HEX)+" X="+String(touch_X)+" Y="+String(touch_Y));
}

FASTRUN
void FrontPanelSetLed(int led,bool state) {
  uint8_t res;
  Debug(String(__FUNCTION__)+": led+"+String(led)+" state="+String(state));

  Wire1.beginTransmission(I2C_ADDRESS);
  Wire1.write(SET_LED_REG);
  Wire1.write((uint8_t)led);
  Wire1.write((uint8_t)state);
  res=Wire1.endTransmission(true);
  if(res!=0) {
    Debug(String(__FUNCTION__)+": endTransmission: "+String(res,HEX));
  }
}

FASTRUN
static void send_config() {
  uint8_t res;

  Debug("Configure Pico config=0x"+String(config,HEX));
  configured_pico=false;
  while(!configured_pico) {
    Wire1.beginTransmission(I2C_ADDRESS);
    Wire1.write(CONFIG_REG);
    Wire1.write(config&0xFF);
    Wire1.write((config>>8)&0xFF);
    res=Wire1.endTransmission(true);
    if(res!=0) {
      Debug(String(__FUNCTION__)+": endTransmission: "+String(res,HEX));
    } else {
      configured_pico = true;
    }
    delay(100);
  }
}

FASTRUN
void front_panel_interrupt() {
  int16_t encoder_val;
  long tuneChange = 0L;
  uint8_t switch_state;

  Debug(String(__FUNCTION__));
  __disable_irq();
  uint16_t interrupt_mask=get_interrupt_mask();
  Debug(String(__FUNCTION__)+": mask=0x"+String(interrupt_mask,HEX));
  switch(interrupt_mask) {
    case 0: // nothing
      break;
    case 0x8000: // ready
      //send_config();
      break;
    case 0x01: // Volume
      encoder_value[0]=get_encoder_value(0);
      switch(volumeFunction) {
        case AUDIO_VOLUME:
          audioVolume += encoder_value[0];

          if (audioVolume > 100) {  // In range?
            audioVolume = 100;
          } else {
           if (audioVolume < 0) {
              audioVolume = 0;
            }
          }
          break;
        case AGC_GAIN:
          bands[currentBand].AGC_thresh += encoder_value[0];
          if(bands[currentBand].AGC_thresh < -20) {
            bands[currentBand].AGC_thresh = -20;
          } else if(bands[currentBand].AGC_thresh > 120) {
            bands[currentBand].AGC_thresh = 120;
          }
          AGCLoadValues();
          break;
        case MIC_GAIN:
          currentMicGain += encoder_value[0];
          if(currentMicGain < -40) {
            currentMicGain = -40;
          } else if(currentMicGain > 30) {
            currentMicGain = 30;
          }
          if(radioState == SSB_TRANSMIT_STATE ) {
            comp1.setPreGain_dB(currentMicGain);
            comp2.setPreGain_dB(currentMicGain);
          }
          break;
        case SIDETONE_VOLUME:
          sidetoneVolume += encoder_value[0];
          if(sidetoneVolume < 0.0 ) {
            sidetoneVolume = 0.0;
          } else if(sidetoneVolume > 100.0) {
            sidetoneVolume = 100.0;
          }
          if(radioState == CW_TRANSMIT_STRAIGHT_STATE || radioState == CW_TRANSMIT_KEYER_STATE) {
            modeSelectOutL.gain(1, sidetoneVolume/100.0);
            modeSelectOutR.gain(1, sidetoneVolume/100.0);
          }
          break;
        case NOISE_FLOOR_LEVEL:
          currentNoiseFloor[currentBand] += encoder_value[0];
          if(currentNoiseFloor[currentBand]<0) {
            currentNoiseFloor[currentBand]=0;
          } else if(currentNoiseFloor[currentBand]>100) {
            currentNoiseFloor[currentBand]=100;
          }
          break;
        case RF_GAIN:
          bands[currentBand].RFgain += encoder_value[0];
          if(bands[currentBand].RFgain<0) {
            bands[currentBand].RFgain=0;
          } else if(bands[currentBand].RFgain>15) {
            bands[currentBand].RFgain=15;
          }
          break;
/*
        case SQUELCH_LEVEL:
          squelch += adjustVolEncoder;
          if(squelch<0) {
            squelch=0;
          } else if(squelch>99) {
            squelch=99;
          }
          break;
*/
      }
      volumeChangeFlag = true;
      break;
    case 0x02: // Filter/Menu
      encoder_value[1]=get_encoder_value(1);
      if (calibrateFlag == 0) {
        filter_pos = last_filter_pos - 5 * encoder_value[1];
      } else {
        filterEncoderMove=encoder_value[1];
      }
      break;
    case 0x04: // Center Freq
      encoder_val=get_encoder_value(2);
      if (encoder_val == 0)                                // Nothing read
        break;
      if (T41State == CW_XMIT && decoderFlag == DECODE_ON) {        // No reason to reset if we're not doing decoded CW AFP 09-27-22
        ResetHistograms();
      }
      tuneChange=(long)encoder_val;
      centerFreq += ((long)freqIncrement * tuneChange);
      TxRxFreq = centerFreq + NCOFreq;
      centerTuneFlag = 1;
      break;
    case 0x08: // Fine Tune
      encoder_value[3]=get_encoder_value(3);
      fineTuneEncoderMove=encoder_value[3];
      if (calibrateFlag==0) {
        NCOFreq += stepFineTune * fineTuneEncoderMove; //AFP 11-01-22
        centerTuneFlag = 1;

        // ============  AFP 10-28-22
        if (activeVFO == VFO_A) {
          currentFreqA = centerFreq + NCOFreq;   //AFP 10-05-22
        } else {
          currentFreqB = centerFreq + NCOFreq;  //AFP 10-05-22
        }
        // ===============  Recentering at band edges ==========
        if (spectrum_zoom != 0) {
          if (NCOFreq > (95000 / (1 << spectrum_zoom)) || NCOFreq < (-93000 / (1 << spectrum_zoom))) {
            NCOFreq    = 0L;
            centerFreq = TxRxFreq = currentFreqA;
          }
        } else {
          if (NCOFreq > (142000) || NCOFreq < (-43000)) {  // Offset tuning window in zoom 1x
            NCOFreq    = 0L;
            centerFreq = TxRxFreq = currentFreqA;  //AFP 10-28-22
          }
          centerTuneFlag = 1;
        }
        fineTuneEncoderMove = 0L;
        TxRxFreq = centerFreq + NCOFreq;  // KF5N
      }
      break;
    case 0x10:
      switch_state=get_switch_state(0);
      if(switch_state==1) {
        G0ORXButtonPressed = NUMBER_OF_SWITCHES+0;
      }
      break;
    case 0x20:
      switch_state=get_switch_state(1);
      if(switch_state==1) {
        G0ORXButtonPressed = NUMBER_OF_SWITCHES+1;
      }
      break;
    case 0x40:
      switch_state=get_switch_state(2);
      if(switch_state==1) {
        G0ORXButtonPressed = NUMBER_OF_SWITCHES+2;
      }
      break;
    case 0x80:
      switch_state=get_switch_state(3);
      if(switch_state==1) {
        G0ORXButtonPressed = NUMBER_OF_SWITCHES+3;
      }
      break;
    case 0x100:
      get_touch();
      process_touch();
      break;
    default:
      Debug(String(__FUNCTION__)+": "+String(interrupt_mask,HEX)+"!");
      break;
  }
  __enable_irq();
}

void FrontPanel2Init() {
  uint8_t res;
  int count=0;

  KeypadInit();

  Debug(String(__FUNCTION__));

  Debug("Setup Wire1");
  Wire1.setClock(100000);
  Wire1.begin();

  Debug("Finding Pico...");
  found_pico=false;
  while(!found_pico && count<10) {
    Wire1.beginTransmission(I2C_ADDRESS);
    res=Wire1.endTransmission(true);
    if(res!=0) {
      Debug("endTransmission: "+String(res,HEX));
      ++count;
    } else {
      found_pico = true;
    }
    delay(50);
  }
  if(found_pico) {
    Debug("Found Pico @0x"+String(I2C_ADDRESS,HEX));
    delay(100);
    send_config();
    // Setup Pico interrupt
    pinMode(PICO_INTERRUPT_PIN,INPUT);
    attachInterrupt(digitalPinToInterrupt(PICO_INTERRUPT_PIN), front_panel_interrupt, FALLING);
  } else {
    Debug("Pico Not Found @0x"+String(I2C_ADDRESS,HEX));
    ShowMessageOnWaterfall("Pico Not Found at 0x"+String(I2C_ADDRESS,HEX));
  }


}

#endif