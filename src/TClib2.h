/* TClib2.h

	Luke Miller June 2017

*/

#ifndef TClib2_H
#define TClib2_H

#include <Arduino.h> // to get access to pinMode, digitalRead etc functions
#include "SdFat.h"	// https://github.com/greiman/SdFat
#include "RTClib.h" // https://github.com/millerlp/RTClib
// Various additional libraries for access to sleep mode functions
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/atomic.h>
#include <wiring_private.h>
#include <avr/wdt.h>


//--------- Public functions


// Print formatted Date + Time to Serial monitor
void printTimeSerial(DateTime now); 

// Print formatted Date + Time to SD card csv file. Notice that this passes the
// SdFile object by reference (SdFile& mylogfile) instead of making a copy and passing by value
// (which SdFile mylogfile would do).
void printTimeToSD(SdFile& mylogfile, DateTime now); 

// Put the AVR to sleep until a TIMER2 interrupt fires to awaken it
void goToSleep();

void initFileName(SdFat& sd, SdFile& logfile, DateTime time1, char *filename);

DateTime startTIMER2(DateTime currTime, RTC_DS3231& rtc);

#endif