#ifndef BEENHERE
#include "SDT.h"
#endif

#ifdef FAST_TUNE
// Fine frequency tune control with variable speed by Harry Brash GM3RVL
bool FastTune = true;    //  HMB
uint32_t FT_last_time;   // millis() of last fine tune step   HMB
bool FT_ON = false;      // In fast tunung mode HMB
int FT_step_counter = 0; // how many fast steps have there been continuously HMB
int last_FT_step_size = 1; // so can go back HMB
const unsigned long FT_on_ms = 30;  // time between FTsteps below which increases the step size
const unsigned long FT_cancel_ms = 400;  // time between steps above which FT is cancelled
const int FT_trig = 4;  // number of short steps to trigger fast tune,
const int FT_step = 500;  // Hz step in Fast Tune
unsigned long MS_temp;    // HMB
unsigned long FT_delay;   // HMB
#endif // FAST_TUNE

/*****
  Purpose: EncoderFilter

  Parameter list:
    void

  Return value;
    void
    Modified AFP21-12-15
*****/
void FilterSetSSB() {
  long filter_change;

  // SSB
  if (filter_pos != last_filter_pos) {
    tft.writeTo(L2);  // Clear layer 2.  KF5N July 31, 2023
    tft.clearMemory();
    if (xmtMode == CW_MODE) BandInformation();
    tft.fillRect((MAX_WATERFALL_WIDTH + SPECTRUM_LEFT_X) / 2 - filterWidth, SPECTRUM_TOP_Y + 17, filterWidth, SPECTRUM_HEIGHT - 20, RA8875_BLACK);  // Erase old filter background
    filter_change = (filter_pos - last_filter_pos);
    if (filter_change >= 1) {
      filterWidth--;
      if (filterWidth < 10)
        filterWidth = 10;
    }
    if (filter_change <= -1) {
      filterWidth++;
      if (filterWidth > 100)
        filterWidth = 50;
    }
    last_filter_pos = filter_pos;
    // =============  AFP 10-27-22
    switch (bands[currentBand].mode) {
      case DEMOD_LSB:
        if (switchFilterSideband == 0)  // "0" = normal, "1" means change opposite filter
        {
          bands[currentBand].FLoCut = bands[currentBand].FLoCut + filter_change * 50 * ENCODER_FACTOR;
          //fHiCutOld= bands[currentBand].FHiCut;
          FilterBandwidth();
        } else if (switchFilterSideband == 1) {
          //if (abs(bands[currentBand].FHiCut) < 500) {
          bands[currentBand].FHiCut = bands[currentBand].FHiCut + filter_change * 50 * ENCODER_FACTOR;
          fLoCutOld = bands[currentBand].FLoCut;
        }
        break;
      case DEMOD_USB:
        if (switchFilterSideband == 0) {
          bands[currentBand].FHiCut = bands[currentBand].FHiCut - filter_change * 50 * ENCODER_FACTOR;
          //bands[currentBand].FLoCut= fLoCutOld;
          FilterBandwidth();
        } else if (switchFilterSideband == 1) {
          bands[currentBand].FLoCut = bands[currentBand].FLoCut - filter_change * 50 * ENCODER_FACTOR;
          // bands[currentBand].FHiCut= fHiCutOld;
        }
        break;
      case DEMOD_AM:
        bands[currentBand].FHiCut = bands[currentBand].FHiCut - filter_change * 50 * ENCODER_FACTOR;
        bands[currentBand].FLoCut = -bands[currentBand].FHiCut;
        FilterBandwidth();
        InitFilterMask();
        break;
      case DEMOD_SAM:  // AFP 11-03-22
        bands[currentBand].FHiCut = bands[currentBand].FHiCut - filter_change * 50 * ENCODER_FACTOR;
        bands[currentBand].FLoCut = -bands[currentBand].FHiCut;
        FilterBandwidth();
        InitFilterMask();
        break;
    }
    // =============  AFP 10-27-22

    //ControlFilterF();
    Menu2 = MENU_F_LO_CUT;  // set Menu2 to MENU_F_LO_CUT
    FilterBandwidth();
    ShowBandwidth();
    DrawFrequencyBarValue();
    //    centerTuneFlag = 1; //AFP 10-03-22
    //SetFreq();               //  AFP 10-04-22
    //  ShowFrequency();
  }
  //notchPosOld = filter_pos;
  tft.writeTo(L1);  // Exit function in layer 1.  KF5N August 3, 2023
}


/*****
  Purpose: EncoderCenterTune
  Parameter list:
    void
  Return value;
    void
*****/
void EncoderCenterTune() {

  long tuneChange = 0L;
  //  long oldFreq    = centerFreq;

#if defined(G0ORX_FRONTPANEL)
  int result = tuneEncoder.process();  // Read the encoder
#else
  unsigned char result = tuneEncoder.process();  // Read the encoder
#endif // G0ORX_FRONTPANEL


  if (result == 0)  // Nothing read
    return;

  //centerTuneFlag = 1;  //AFP 10-03-22  Not used in revised tuning scheme.  KF5N July 22, 2023

  if (xmtMode == CW_MODE && decoderFlag == DECODE_ON) {  // No reason to reset if we're not doing decoded CW AFP 09-27-22
    ResetHistograms();
  }

#if defined(G0ORX_FRONTPANEL)
  tuneChange = result;
#else
  switch (result) {
    case DIR_CW:  // Turned it clockwise, 16
      tuneChange = 1L;
      break;

    case DIR_CCW:  // Turned it counter-clockwise
      tuneChange = -1L;
      break;
  }
#endif // G0ORX_FRONTPANEL
//=================== AFP 03-30-24 V012 Bode Plot start
#if defined(BODE)
  if (BodePlotFlag == 1) {  //AFP 03-30-24

    freqStartBode += (-1000000L * tuneChange);  //AFP 03-30-24
    freqBodeChangeFlag = 1;
    //tuneChange = 0L;
  } else {//if (BodePlotFlag != 1) {
#endif // BODE
    centerFreq += ((long)freqIncrement * tuneChange);  // tune the master vfo

//=================== AFP 03-30-24 V012 Bode Plot end

    TxRxFreq = centerFreq + NCOFreq;
    SetFreq();  //  Change to receiver tuning process.  KF5N July 22, 2023
    //currentFreqA= centerFreq + NCOFreq;
    DrawBandWidthIndicatorBar();  // AFP 10-20-22
    //FilterOverlay(); // AFP 10-20-22
    ShowFrequency();
    BandInformation();
#if defined(BODE)
  }
#endif // BODE
  //  }
}


/*****
  Purpose: Encoder volume control

  Parameter list:
    void

  Return value;
    int               0 means encoder didn't move; otherwise it moved
*****/
void EncoderVolume()  //============================== AFP 10-22-22  Begin new
{
#if defined(G0ORX_FRONTPANEL)
  int result;
#else
  char result;
#endif // G0ORX_FRONTPANEL

  int increment [[maybe_unused]] = 0;

  result = volumeEncoder.process();  // Read the encoder

  if (result == 0) {  // Nothing read
    return;
  }
#if defined(G0ORX_FRONTPANEL)
  adjustVolEncoder = result;
#else
  switch (result) {
    case DIR_CW:  // Turned it clockwise, 16
      adjustVolEncoder = 1;
      break;

    case DIR_CCW:  // Turned it counter-clockwise
      adjustVolEncoder = -1;
      break;
  }
#endif // G0ORX_FROMNTPANEL

#if defined(G0ORX_FRONTPANEL)
  switch(volumeFunction) {
        case AUDIO_VOLUME:
          audioVolume += adjustVolEncoder;

          if (audioVolume > 100) {  // In range?
            audioVolume = 100;
          } else {
           if (audioVolume < 0) {
              audioVolume = 0;
            }
          }
          break;
        case AGC_GAIN:
          bands[currentBand].AGC_thresh += adjustVolEncoder;
          if(bands[currentBand].AGC_thresh < -20) {
            bands[currentBand].AGC_thresh = -20;
          } else if(bands[currentBand].AGC_thresh > 120) {
            bands[currentBand].AGC_thresh = 120;
          }
          AGCLoadValues();
          break;
        case MIC_GAIN:
          currentMicGain += adjustVolEncoder;
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
          sidetoneVolume += adjustVolEncoder;
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
          currentNoiseFloor[currentBand] += adjustVolEncoder;
          if(currentNoiseFloor[currentBand]<0) {
            currentNoiseFloor[currentBand]=0;
          } else if(currentNoiseFloor[currentBand]>100) {
            currentNoiseFloor[currentBand]=100;
          }
          break;
      }
#else
  audioVolume += adjustVolEncoder;
  // simulate log taper.  As we go higher in volume, the increment increases.

  if (audioVolume < (MIN_AUDIO_VOLUME + 10)) increment = 2;
  else if (audioVolume < (MIN_AUDIO_VOLUME + 20)) increment = 3;
  else if (audioVolume < (MIN_AUDIO_VOLUME + 30)) increment = 4;
  else if (audioVolume < (MIN_AUDIO_VOLUME + 40)) increment = 5;
  else if (audioVolume < (MIN_AUDIO_VOLUME + 50)) increment = 6;
  else if (audioVolume < (MIN_AUDIO_VOLUME + 60)) increment = 7;
  else increment = 8;


  if (audioVolume > MAX_AUDIO_VOLUME) {
    audioVolume = MAX_AUDIO_VOLUME;
  } else {
    if (audioVolume < MIN_AUDIO_VOLUME)
      audioVolume = MIN_AUDIO_VOLUME;
  }
#endif // G0ORX_FRONTPANEL

//=================== AFP 04-3-24 V012 Bode Plot start


#if defined(BODE)
    refLevelBode += adjustVolEncoder;  //AFP 03-30-24

    levelBodeChangeFlag = 1;
#endif // BODE
//=================== AFP 04-3-24 V012 Bode Plot end

  volumeChangeFlag = true;  // Need this because of unknown timing in display updating.

}  //============================== AFP 10-22-22  End new


/*****
  Purpose: Use the fine tune encoder to change the value of a number in some other function

  Parameter list:
    int minValue                the lowest value allowed
    int maxValue                the largest value allowed
    int startValue              the numeric value to begin the count
    int increment               the amount by which each increment changes the value
    char prompt[]               the input prompt
  Return value;
    int                         the new value
*****/
int GetFineTuneValueLive(int minValue, int maxValue, int startValue, int increment, char prompt[])
{
  int currentValue = startValue;
  tft.setFontScale((enum RA8875tsize)1);
  tft.setTextColor(RA8875_WHITE);
  tft.fillRect(250, 449, 285, CHAR_HEIGHT, RA8875_BLACK);
  tft.setCursor(257, 450);
  tft.print(prompt);
  tft.setCursor(440, 450);
  tft.print((float)startValue/2.0, 2);
  if (fineTuneEncoderMove != 0) {
    currentValue += fineTuneEncoderMove * increment;  // Bump up or down...
    if (currentValue < minValue)
      currentValue = minValue;
    else if (currentValue > maxValue)
      currentValue = maxValue;

    tft.setCursor(440, 450);
    tft.print((float)startValue/2.0, 2);
    fineTuneEncoderMove = 0;
  }
  return currentValue;
}

/*****
  Purpose: Use the encoder to change the value of a number in some other function

  Parameter list:
    int minValue                the lowest value allowed
    int maxValue                the largest value allowed
    int startValue              the numeric value to begin the count
    int increment               the amount by which each increment changes the value
    char prompt[]               the input prompt
  Return value;
    int                         the new value
*****/
float GetEncoderValueLive(float minValue, float maxValue, float startValue, float increment, char prompt[], int Ndecimals){
  float currentValue = startValue;
  tft.setFontScale((enum RA8875tsize)1);
  tft.setTextColor(RA8875_WHITE);
  tft.fillRect(250, 0, 285, CHAR_HEIGHT, RA8875_BLACK);  // Increased rectangle size to full erase value.  KF5N August 12, 2023
  tft.setCursor(257, 1);
  tft.print(prompt);
  tft.setCursor(440, 1);
  if (Ndecimals < 0){
    Ndecimals = 0;
  }
  if (Ndecimals > 3){
    Ndecimals = 3;
  }
  tft.print(startValue, Ndecimals);
  if (filterEncoderMove != 0) {
    currentValue += filterEncoderMove * increment;  // Bump up or down...
    if (currentValue < minValue)
      currentValue = minValue;
    else if (currentValue > maxValue)
      currentValue = maxValue;

    //  tft.fillRect(449, 0, 90, CHAR_HEIGHT, RA8875_BLACK);  // This is not required. KF5N August 12, 2023
    tft.setCursor(440, 1);
    if (abs(startValue) > 2) {
      tft.print(startValue, 0);
    } else {
      tft.print(startValue, 3);
    }
    filterEncoderMove = 0;
  }
  //tft.setTextColor(RA8875_WHITE);
  return currentValue;
}

int GetEncoderValueLiveInt(int minValue, int maxValue, int startValue, int increment, char prompt[]){
  int currentValue = startValue;
  tft.setFontScale((enum RA8875tsize)1);
  tft.setTextColor(RA8875_WHITE);
  tft.fillRect(250, 0, 285, CHAR_HEIGHT, RA8875_BLACK);  // Increased rectangle size to full erase value.  KF5N August 12, 2023
  tft.setCursor(257, 1);
  tft.print(prompt);
  tft.setCursor(440, 1);
  tft.print((float)startValue/2.0, 2);
  if (filterEncoderMove != 0) {
    currentValue += filterEncoderMove * increment;  // Bump up or down...
    if (currentValue < minValue)
      currentValue = minValue;
    else if (currentValue > maxValue)
      currentValue = maxValue;
    tft.setCursor(440, 1);
    tft.print((float)startValue/2.0, 2);
    filterEncoderMove = 0;
  }
  return currentValue;
}

float GetEncoderValueLive(float minValue, float maxValue, float startValue, float increment, char prompt[])  //AFP 10-22-22
{
  int N = 0;
  if (abs(startValue) > 2) {
    N = 0;
  } else {
    N = 3;
  }
  return GetEncoderValueLive(minValue, maxValue, startValue, increment, prompt, N);
}



/*****
  Purpose: Use the encoder to change the value of a number in some other function

  Parameter list:
    int minValue                the lowest value allowed
    int maxValue                the largest value allowed
    int startValue              the numeric value to begin the count
    int increment               the amount by which each increment changes the value
    char prompt[]               the input prompt
  Return value;
    int                         the new value
*****/
int GetEncoderValue(int minValue, int maxValue, int startValue, int increment, char prompt[]) {
  int currentValue = startValue;
  int val;

  tft.setFontScale((enum RA8875tsize)1);

  tft.setTextColor(RA8875_WHITE);
  tft.fillRect(250, 0, 280, CHAR_HEIGHT, RA8875_MAGENTA);
  tft.setCursor(257, 1);
  tft.print(prompt);
  tft.setCursor(470, 1);
  tft.print(startValue);

  while (true) {
    if (filterEncoderMove != 0) {
      currentValue += filterEncoderMove * increment;  // Bump up or down...
      if (currentValue < minValue)
        currentValue = minValue;
      else if (currentValue > maxValue)
        currentValue = maxValue;

      tft.fillRect(465, 0, 65, CHAR_HEIGHT, RA8875_MAGENTA);
      tft.setCursor(470, 1);
      tft.print(currentValue);
      filterEncoderMove = 0;
    }

    val = ReadSelectedPushButton();  // Read the ladder value
    //MyDelay(100L); //AFP 09-22-22
    if (val != -1 && val < (EEPROMData.switchValues[0] + WIGGLE_ROOM)) {
      val = ProcessButtonPress(val);    // Use ladder value to get menu choice
      if (val == MENU_OPTION_SELECT) {  // Make a choice??
        return currentValue;
      }
    }
  }
}

/*****
  Purpose: Allows quick setting of WPM without going through a menu

  Parameter list:
    void

  Return value;
    int           the current WPM
*****/
int SetWPM() {
  int val;
  long lastWPM = currentWPM;

  tft.setFontScale((enum RA8875tsize)1);

  tft.fillRect(SECONDARY_MENU_X, MENUS_Y, EACH_MENU_WIDTH, CHAR_HEIGHT, RA8875_MAGENTA);
  tft.setTextColor(RA8875_WHITE);
  tft.setCursor(SECONDARY_MENU_X + 1, MENUS_Y + 1);
  tft.print("current WPM:");
  tft.setCursor(SECONDARY_MENU_X + 200, MENUS_Y + 1);
  tft.print(currentWPM);

  while (true) {
    if (filterEncoderMove != 0) {       // Changed encoder?
      currentWPM += filterEncoderMove;  // Yep
      lastWPM = currentWPM;
      if (lastWPM < 5)  // Set minimum keyer speed to 5 wpm.  KF5N August 20, 2023
        lastWPM = 5;
      else if (lastWPM > MAX_WPM)
        lastWPM = MAX_WPM;

      tft.fillRect(SECONDARY_MENU_X + 200, MENUS_Y + 1, 50, CHAR_HEIGHT, RA8875_MAGENTA);
      tft.setCursor(SECONDARY_MENU_X + 200, MENUS_Y + 1);
      tft.print(lastWPM);
      filterEncoderMove = 0;
    }

    val = ReadSelectedPushButton();   // Read pin that controls all switches
    val = ProcessButtonPress(val);    ///////////////////////////////////////////////////
    if (val == MENU_OPTION_SELECT) {  // Make a choice??
      currentWPM = lastWPM;
      EEPROMData.currentWPM = currentWPM;
      EEPROMWrite();
      UpdateWPMField();
      break;
    }
  }
  return currentWPM;
}

/*****
  Purpose: Determines how long the transmit relay remains on after last CW atom is sent.

  Parameter list:
    void

  Return value;
    long            the delay length in milliseconds
*****/
long SetTransmitDelay()  // new function JJP 9/1/22
{
  int val;
  long lastDelay = cwTransmitDelay;
  long increment = 250;  // Means a quarter second change per detent

  tft.setFontScale((enum RA8875tsize)1);

  tft.fillRect(SECONDARY_MENU_X - 150, MENUS_Y, EACH_MENU_WIDTH + 150, CHAR_HEIGHT, RA8875_MAGENTA);  // scoot left cuz prompt is long
  tft.setTextColor(RA8875_WHITE);
  tft.setCursor(SECONDARY_MENU_X - 149, MENUS_Y + 1);
  tft.print("current delay:");
  tft.setCursor(SECONDARY_MENU_X + 79, MENUS_Y + 1);
  tft.print(cwTransmitDelay);

  while (true) {
    if (filterEncoderMove != 0) {                  // Changed encoder?
      lastDelay += filterEncoderMove * increment;  // Yep
      if (lastDelay < 0L)
        lastDelay = 250L;

      tft.fillRect(SECONDARY_MENU_X + 80, MENUS_Y + 1, 200, CHAR_HEIGHT, RA8875_MAGENTA);
      tft.setCursor(SECONDARY_MENU_X + 79, MENUS_Y + 1);
      tft.print(lastDelay);
      filterEncoderMove = 0;
    }

    val = ReadSelectedPushButton();  // Read pin that controls all switches
    val = ProcessButtonPress(val);
    //MyDelay(150L);  //ALF 09-22-22
    if (val == MENU_OPTION_SELECT) {  // Make a choice??
      cwTransmitDelay = lastDelay;
      EEPROMData.cwTransmitDelay = cwTransmitDelay;
      EEPROMWrite();
      break;
    }
  }
  tft.setTextColor(RA8875_WHITE);
  EraseMenus();
  return cwTransmitDelay;
}
/*****
  Purpose: Fine tune control.

  Parameter list:
    void

  Return value;
    void               cannot return value from interrupt
*****/
FASTRUN  // Causes function to be allocated in RAM1 at startup for fastest performance.
  void
  EncoderFineTune() {
#if defined(G0ORX_FRONTPANEL)
  int result;
#else
  char result;
#endif // G0ORX_FRONTPANEL


  result = fineTuneEncoder.process();  // Read the encoder
  if (result == 0) {                   // Nothing read
    fineTuneEncoderMove = 0L;
    return;
  } else {
#if defined(G0ORX_FRONTPANEL)
    fineTuneEncoderMove=result;
#else
    if (result == DIR_CW) {  // 16 = CW, 32 = CCW
      fineTuneEncoderMove = 1L;
    } else {
      fineTuneEncoderMove = -1L;
    }
#endif // G0ORX_FRONTPANEL
  }
  // stop function execution at this point if we're using the fine tune
  // encoder as part of the calibration process
  if (calOnFlag) return; 
  // if not, then continue using the fine tune encoder for its named purpose

  #ifdef FAST_TUNE
  //----------------------------------------------------
  // Fine frequency tune control with variable speed by Harry Brash GM3RVL
  MS_temp = millis();   // HMB...
  FT_delay = MS_temp - FT_last_time;
  FT_last_time = MS_temp;
  if (FT_ON) {           // Check if FT should be cancelled (FT_delay>=FT_cancel_ms)
    if (FT_delay>=FT_cancel_ms) {
      FT_ON = false;
      EEPROMData.stepFineTune = last_FT_step_size;
    } 
  }
  else {      //  FT is off so check for short delays
    if (FT_delay<=FT_on_ms) {
      FT_step_counter +=1;
    }
    if (FT_step_counter>=FT_trig) {
      last_FT_step_size = EEPROMData.stepFineTune;
      EEPROMData.stepFineTune = FT_step;
      FT_step_counter = 0;
      FT_ON = true;
    }
  }
  //----------------------------------------------------
  #endif // FAST_TUNE

  NCOFreq += EEPROMData.stepFineTune * fineTuneEncoderMove;  //AFP 11-01-22
  //freqStopBode += 1000000 * fineTuneEncoderMove;
  centerTuneFlag = 1;
  // ============  AFP 10-28-22
  if (activeVFO == VFO_A) {
    currentFreqA = centerFreq + NCOFreq;  //AFP 10-05-22
  } else {
    currentFreqB = centerFreq + NCOFreq;  //AFP 10-05-22
  }
  // ===============  Recentering at band edges ==========
  if (spectrum_zoom != 0) {
    if (NCOFreq >= (95000 / (1 << spectrum_zoom)) || NCOFreq < (-93000 / (1 << spectrum_zoom))) {  // 47500 with 2x zoom.
      centerTuneFlag = 0;
      resetTuningFlag = 1;
      return;
    }
  } else {
    if (NCOFreq > 142000 || NCOFreq < -43000) {  // Offset tuning window in zoom 1x
      centerTuneFlag = 0;
      resetTuningFlag = 1;
      return;
    }
  }

  TxRxFreq = centerFreq + NCOFreq;  // KF5N
#if defined(BODE)
  freqStopBode += (1000000L * fineTuneEncoderMove); //=================== AFP 03-30-24 V012 Bode Plot
  freqBodeChangeFlag = 1; //=================== AFP 03-30-24 V012 Bode Plot
  fineTuneEncoderMove = 0L;
#endif // BODE
}


FASTRUN  // Causes function to be allocated in RAM1 at startup for fastest performance.
  void
  EncoderFilter() {
#if defined(G0ORX_FRONTPANEL)
  int result;
#else
  char result;
#endif // G0ORX_FRONTPANEL

  result = filterEncoder.process();  // Read the encoder

  if (result == 0) {
    //    filterEncoderMove = 0;// Nothing read
    return;
  }

#if defined(G0ORX_FRONTPANEL)
  filterEncoderMove = result;
#else
  switch (result) {
    case DIR_CW:  // Turned it clockwise, 16
      filterEncoderMove = 1;
      //filter_pos = last_filter_pos - 5 * filterEncoderMove;  // AFP 10-22-22
      break;

    case DIR_CCW:  // Turned it counter-clockwise
      filterEncoderMove = -1;
      // filter_pos = last_filter_pos - 5 * filterEncoderMove;   // AFP 10-22-22
      break;
  }
#endif // G0ORX_FRONTPANEL
  if (calibrateFlag == 0 && !mainMenuWindowActive) {                                // AFP 10-22-22
    filter_pos = last_filter_pos - 5 * filterEncoderMove;  // AFP 10-22-22
  }                                                        // AFP 10-22-22
}