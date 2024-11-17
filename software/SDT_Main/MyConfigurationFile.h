//==============================================================
// This file is used to configure the specifics of your radio by 
// commenting/uncommenting/modifying particular #define statements
// to select which hardware you installed and any modifications to
// their addresses. You can also select which extra software capa-
// bilities you want to include in the same way. If you need to re-
// map Teensy pins or change the operation of the front panel buttons
// please edit SDT.h
//==============================================================

//====================== Hardware selections ===================

// Select either V11 hardware or V12 hardware
//#define V11HWR   // Selectes Si5351 clock frequencies for the 4x frequency method           
#define V12HWR     // Selects the V12 BPF, K9HZ LPF, V12 RF control, and affects Si5351

#ifdef V12HWR
  // Set the I2C addresses of the LPF, BPF, and RF boards
  #define K9HZ_LPF_MCP23017_ADDR  0x25
  #define BPF_MCP23017_ADDR       0x24
  #define RF_MCP23017_ADDR        0x27

  // K9HZ LPF options
  // Uncomment the appropriate line below to select which power detection option 
  // you populated in the SWR meter
  #define K9HZ_LPF_SWR_LOG_AMP
  //#define K9HZ_LPF_SWR_DIODE

  // Uncomment the appropriate line below to select which detected power readout 
  // option you populated in the SWR meter
  #define K9HZ_LPF_SWR_AD7991
  //#define K9HZ_LPF_SWR_ANALOG
#endif //V12HWR

// You can't change the Si5351 address, but it sometimes differs from the default 
// of 0x60. Change it here if needed.
#define SI5351_BUS_BASE_ADDR    0x60

// Uncomment if using G0ORX or K9HZ MCP23017 Front Panel
#define G0ORX_FRONTPANEL

// Set the I2C addresses of the front panel board
#define G0ORX_PANEL_MCP23017_ADDR_1 0x20
#define G0ORX_PANEL_MCP23017_ADDR_2 0x21

// Pick one of the following encoder configurations
#define                           NORM_ENCODER                              // Leave as is UNLESS...
//#define                           FOURSQRP                                // ...you are using the 4 States QRP kit

//====================== Software selections ===================

// Uncomment to exclude BEARING and/or BODE code
//#define EXCLUDE_BEARING
//#define EXCLUDE_BODE

// Uncomment if using G0ORX Kenwood TS-2000 CAT interface
#define G0ORX_CAT

// G0ORX_AUDIO_DISPLAY draws a Time Domain plot of the Microphone when transmitting
#define G0ORX_AUDIO_DISPLAY

// Fast tune function on fine tune knob from Harry Brash GM3RVL
#define FAST_TUNE

//====================== User Specific Preferences =============

#define DEBUG 		                1                                         // Uncommented for debugging, comment out for normal use
#define DECODER_STATE							0						                              // 0 = off, 1 = on
#define DEFAULT_KEYER_WPM   			15                                        // Startup value for keyer wpm
#define FREQ_SEP_CHARACTER  			'.'					                              // Some may prefer period, space, or combo
#define MAP_FILE_NAME   					"Cincinnati.bmp"                          // Name you gave to BMP map file. Max is 50 chars
#define MY_LAT										39.07466                                  // Coordinates for QTH
#define MY_LON										-84.42677
#define MY_CALL										"AI6YM"                                   // Default max is 10 chars
#define MY_TIMEZONE          			"PST: "                                   // Default max is 10 chars
//DB2OO, 29-AUG-23: TIME_24H to display 24 hour times (Utility.cpp). If this is not defined 12h display will be used
#define TIME_24H                  1
//DB2OO, 29-AUG-23: ITU_REGION to determine band borders: Upper band limits on 80m (3.8MHz vs 4.0MhHz) and 40m (7.2MHz vs. 7.3MHz)
// #define ITU_REGION                1  //for Europe
#define ITU_REGION                  2   // for USA
//#define ITU_REGION    3   // Asia/Oceania
// DB2OO, 29.823:. Analog Signal on this pin will be used for an analog S-Meter (250uA full scale) connected via 10kOhm to this output. 1uF capacitor paralle to the S-Meter. --> Display.cpp. 
// This might conflict with other hardware modifications, that might use Pin 33 for a different purpose --> please check, before defining this
//#define HW_SMETER              33
// DB2OO, 30-AUG-23: with TCVSDR_SMETER (TCVSDR=Teensy Convolution SDR) defined the S-Meter bar will be consistent with the dBm value and it will not go over the box for S9+40+
#define TCVSDR_SMETER             1
//DB2OO, 10-SEP-23: TCXO_25MHZ defined sets the default EEPROMData.freqCorrectionFactor = 0, as the TCXO is supposed to deliver 25.00000MHz
//#define TCXO_25MHZ                1

#define PADDLE_FLIP								0						                              // 0 = right paddle = DAH, 1 = DIT
#define STRAIGHT_KEY_OR_PADDLES		0						                              // 0 = straight, 1 = paddles
#define SDCARD_MESSAGE_LENGTH     3000L                                     // The number of milliseconds to leave error message on screen
//#define SD_CARD_PRESENT         0                                         //   JJP  7/18/23

//====================== System specific ===============
#define CURRENT_FREQ_A            7200000                                   // VFO_A
#define CURRENT_FREQ_B            7030000                                   // VFO_B
#define DEFAULTFREQINCREMENT 		  4                                         // Default: (10, 50, 100, 250, 1000, 10000Hz)
#define DEFAULT_POWER_LEVEL       10                                        // Startup power level. Probably 20 for most people
#define FAST_TUNE_INCREMENT    	  1			 		                                // Default from above for fine tune
#define SPLASH_DELAY              1000L                                     // How long to show Splash screen. Use 1000 for testing, 4000 normally
#define BIT_DELAY_LONG            10000L                                    // How long to show BIT screen with errors
#define BIT_DELAY_SHORT           1000L                                     // How long to show BIT screen when no error
#define STARTUP_BAND        			BAND_40M                                  // This is the 40M band. see around line 575 in SDT.h // G0ORX changed from 1

#define CENTER_SCREEN_X           400
#define CENTER_SCREEN_Y           245
#define IMAGE_CORNER_X            190                                       // ImageWidth = 378 Therefore 800 - 378 = 422 / 2 = 211
#define IMAGE_CORNER_Y            40                                        // ImageHeight = 302 Therefore 480 - 302 = 178 / 2 = 89
#define RAY_LENGTH                190

//=========== Applying hardware/software selections ============

// Set multiplication factors for your QSD and QSE boards.
#ifdef NORM_ENCODER
  #define MASTER_CLK_MULT_RX 2
  #define MASTER_CLK_MULT_TX 2  
#else
  #define MASTER_CLK_MULT_RX 4
  #define MASTER_CLK_MULT_TX 4
#endif 

#if defined(G0ORX_FRONTPANEL)
#include "G0ORX_FrontPanel.h"
#endif // G0ORX_FRONTPANEL

#if defined(G0ORX_CAT)
#include "G0ORX_CAT.h"
#endif // G0ORX_CAT

#ifdef V12HWR
  #define K9HZ_LPF           // Selects the K9HZ LPF board
  #define V12BPF             // Selects the V12 BPF board
#endif //V12HWR