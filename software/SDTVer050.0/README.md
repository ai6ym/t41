# SDTVer050.0

This version has the Bearing and BodePlotter code included. It also supports the shutdown routine
using the ATTiny85 on the external boards, the V12 BPF board, and the K9HZ LPF board. It implements
only one of the features on the K9HZ LPF board, namely the band select feature. Support for the SWR
meter, transverter selection, 100W amp selection, and antenna selection will be future work.

Latest SDTVer050.0 with G0ORX MCP23017 Front Panel Code and Kenwood TS-2000 CAT interface.

MyConfigurationFile.h has define of G0ORX_FRONTPANEL and G0ORX_CAT.

Note that this MCP23017 Front Panel Code supports my Front Panel and the K9HZ Version of the Front Panel and encoders.

I have also made some other small changes to get some small changes to increase the stack memory.

To build this version configure the IDE as:

Tools->Optimize->Faster with LTO

Tools->USB Type->Dual Serial

## Update

Latest version includes compile options in MyConfigurationFile.h for some additional new code that is still under deveopment.

G0ORX_FRONTPANEL_2 - A complete front panel for the 7" display with a i2c connected Raspberry Pi Pico handling the encoders, switches and touch screen. See https://github.com/g0orx/T41-EP/wiki for more information.

M0JTS_USB_AUDIO - sends and receives audio over USB using patches by M0JTS to the Audio Library. See https://github.com/laiudm/laiudm-Teensy4-192k-USB-Audio for more information.



