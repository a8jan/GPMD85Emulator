# SAASound

- Email: Dave Hooper <dave@beermex.com>
- Homepage: https://github.com/stripwax/SAASound

SAASound is a software emulation library for the Philips SAA-1099 sound chip device (popular in computing and gaming devices in the late 1980's / early 1990's).  Development of SAA SOUND began around 1996 with completion around 1999 and active development ending around 2001 (aside from an update in 2004 to address an issue with frequency generator SYNC behaviour that was exploited to generate digital audio in a demo from Fred 59 magazine, which was failing to be emulated correctly!)

Simon Owen (author of SimCoupe) made additional infrastructure changes to support compilation on a variety of different platforms.  The latest releases of SAASound.dll continue to be compatible with the released version (1.0) of SimCoupe .

After a considerable time, the original author revisited the project in August 2018 following a conversation about emulation accuracy, which he intends to continue to improve following some recent reverse-engineering efforts of original hardware by the DosBox team, in conjunction with some crowd-sourced test cases comparing emulated behaviour to real original devices.

It has been incorporated into standalone chiptune players, computer hardware emulators (e.g. SimCoupe), and has even been the basis of SAA-1099 hardware emulators implemented in FPGA devices (such as MiST / MiSTer)
