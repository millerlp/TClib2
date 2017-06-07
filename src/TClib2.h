/* TClib2.h
	A set of functions used with my Thermocouple_datalogger_2 project
	on RevA hardware (ATmega328p)
	Luke Miller June 2017

*/

#ifndef TClib2_H
#define TClib2_H

#include <Arduino.h> // to get access to pinMode, digitalRead etc functions
#include "SdFat.h"	// https://github.com/greiman/SdFat
#include "RTClib.h" // https://github.com/millerlp/RTClib
#include "SSD1306Ascii.h" // https://github.com/greiman/SSD1306Ascii
#include "SSD1306AsciiWire.h" // https://github.com/greiman/SSD1306Ascii
// Various additional libraries for access to sleep mode functions
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/atomic.h>
#include <wiring_private.h>
#include <avr/wdt.h>
#include <math.h>


//--------- Public functions


// Print formatted Date + Time to Serial monitor
void printTimeSerial(DateTime now); 

// Print formatted Date + Time to SD card csv file. Notice that this passes the
// SdFile object by reference (SdFile& mylogfile) instead of making a copy and passing by value
// (which SdFile mylogfile would do).
void printTimeToSD(SdFile& mylogfile, DateTime now); 

// Put the AVR to sleep until a TIMER2 interrupt fires to awaken it
void goToSleep();

// Initialize a new output csv file. Note that this writes a header row
// to the file, so you may want to tweak the column labels in this function.
void initFileName(SdFat& sd, SdFile& logfile, DateTime time1, char *filename);

// Start the TIMER2 timer, using a 32.768kHz input from a DS3231M 
// real time clock as the signal. 
DateTime startTIMER2(DateTime currTime, RTC_DS3231& rtc, byte SPS);

// Update temperature values on the 2 OLED screens. This function only updates
// elements that have changed, and leaves the rest of the screen static. 
void printTempToOLEDs (SSD1306AsciiWire& oled1, SSD1306AsciiWire& oled2, double *tempAverages, double *prevAverages);

#endif