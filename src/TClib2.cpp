/* TClib2.cpp
	A set of functions used with my Thermocouple_datalogger_2 project
	on RevA hardware (ATmega328p)
	Luke Miller June 2017

*/


#include "TClib2.h"

void printTimeSerial(DateTime now){
//------------------------------------------------
// printTime function takes a DateTime object from
// the real time clock and prints the date and time 
// to the serial monitor. 
	Serial.print(now.year(), DEC);
    Serial.print('-');
	if (now.month() < 10) {
		Serial.print(F("0"));
	}
    Serial.print(now.month(), DEC);
    Serial.print('-');
    if (now.day() < 10) {
		Serial.print(F("0"));
	}
	Serial.print(now.day(), DEC);
    Serial.print(' ');
	if (now.hour() < 10){
		Serial.print(F("0"));
	}
    Serial.print(now.hour(), DEC);
    Serial.print(':');
	if (now.minute() < 10) {
		Serial.print("0");
	}
    Serial.print(now.minute(), DEC);
    Serial.print(':');
	if (now.second() < 10) {
		Serial.print(F("0"));
	}
    Serial.print(now.second(), DEC);
	// You may want to print a newline character
	// after calling this function i.e. Serial.println();

}

//---------------printTimeToSD----------------------------------------
// printTimeToSD function. This formats the available data in the
// data arrays and writes them to the SD card file in a
// comma-separated value format.
void printTimeToSD (SdFile& mylogfile, DateTime tempTime) {
    // Write the date and time in a human-readable format
    // to the file on the SD card. 
    mylogfile.print(tempTime.year(), DEC);
    mylogfile.print(F("-"));
    if (tempTime.month() < 10) {
      mylogfile.print("0");
    }
    mylogfile.print(tempTime.month(), DEC);
    mylogfile.print(F("-"));
    if (tempTime.day() < 10) {
      mylogfile.print("0");
    }
    mylogfile.print(tempTime.day(), DEC);
    mylogfile.print(F(" "));
    if (tempTime.hour() < 10){
      mylogfile.print("0");
    }
    mylogfile.print(tempTime.hour(), DEC);
    mylogfile.print(F(":"));
    if (tempTime.minute() < 10) {
      mylogfile.print("0");
    }
    mylogfile.print(tempTime.minute(), DEC);
    mylogfile.print(F(":"));
    if (tempTime.second() < 10) {
      mylogfile.print("0");
    }
    mylogfile.print(tempTime.second(), DEC);
}

//--------------------goToSleep-----------------------------------------------
// goToSleep function. When called, this puts the AVR to
// sleep until it is awakened by an interrupt (TIMER2 in this case)
// This is a higher power sleep mode than the lowPowerSleep function uses.
void goToSleep() {
  // Create three variables to hold the current status register contents
  byte adcsra, mcucr1, mcucr2;
  // Cannot re-enter sleep mode within one TOSC cycle. 
  // This provides the needed delay.
  OCR2A = 0; // write to OCR2A, we're not using it, but no matter
  while (ASSR & _BV(OCR2AUB)) {} // wait for OCR2A to be updated
  // Set the sleep mode to PWR_SAVE, which allows TIMER2 to wake the AVR
  set_sleep_mode(SLEEP_MODE_PWR_SAVE);
  adcsra = ADCSRA; // save the ADC Control and Status Register A
  ADCSRA = 0; // disable ADC by zeroing out the ADC status register
  sleep_enable();
  // Do not interrupt before we go to sleep, or the
  // ISR will detach interrupts and we won't wake.
  noInterrupts ();
  
  wdt_disable(); // turn off the watchdog timer
  
  //ATOMIC_FORCEON ensures interrupts are enabled so we can wake up again
  ATOMIC_BLOCK(ATOMIC_FORCEON) { 
    // Turn off the brown-out detector
    mcucr1 = MCUCR | _BV(BODS) | _BV(BODSE); 
    mcucr2 = mcucr1 & ~_BV(BODSE);
    MCUCR = mcucr1; //timed sequence
    // BODS stays active for 3 cycles, sleep instruction must be executed 
    // while it's active
    MCUCR = mcucr2; 
  }
  // We are guaranteed that the sleep_cpu call will be done
  // as the processor executes the next instruction after
  // interrupts are turned on.
  interrupts ();  // one cycle, re-enables interrupts
  sleep_cpu(); //go to sleep
  //wake up here
  sleep_disable(); // upon wakeup (due to interrupt), AVR resumes here
  ADCSRA = adcsra; // re-apply the previous settings to the ADC status register

}

//-------------- initFileName --------------------------------------------------
// initFileName - a function to create a filename for the SD card based
// on the 4-digit year, month, day, hour, minutes and a 2-digit counter. 
// The character array 'filename' was defined as a global array 
// at the top of the sketch in the form "YYYYMMDD_HHMM_00.csv"
void initFileName(SdFat& sd, SdFile& logfile, DateTime time1, char *filename) {
	
	char buf[5];
	// integer to ascii function itoa(), supplied with numeric year value,
	// a buffer to hold output, and the base for the conversion (base 10 here)
	itoa(time1.year(), buf, 10);
	// copy the ascii year into the filename array
	for (byte i = 0; i < 4; i++){
		filename[i] = buf[i];
	}
	// Insert the month value
	if (time1.month() < 10) {
		filename[4] = '0';
		filename[5] = time1.month() + '0';
	} else if (time1.month() >= 10) {
		filename[4] = (time1.month() / 10) + '0';
		filename[5] = (time1.month() % 10) + '0';
	}
	// Insert the day value
	if (time1.day() < 10) {
		filename[6] = '0';
		filename[7] = time1.day() + '0';
	} else if (time1.day() >= 10) {
		filename[6] = (time1.day() / 10) + '0';
		filename[7] = (time1.day() % 10) + '0';
	}
	// Insert an underscore between date and time
	filename[8] = '_';
	// Insert the hour
	if (time1.hour() < 10) {
		filename[9] = '0';
		filename[10] = time1.hour() + '0';
	} else if (time1.hour() >= 10) {
		filename[9] = (time1.hour() / 10) + '0';
		filename[10] = (time1.hour() % 10) + '0';
	}
	// Insert minutes
		if (time1.minute() < 10) {
		filename[11] = '0';
		filename[12] = time1.minute() + '0';
	} else if (time1.minute() >= 10) {
		filename[11] = (time1.minute() / 10) + '0';
		filename[12] = (time1.minute() % 10) + '0';
	}
	// Insert another underscore after time
	filename[13] = '_';

	// Next change the counter on the end of the filename
	// (digits 14+15) to increment count for files generated on
	// the same day. This shouldn't come into play
	// during a normal data run, but can be useful when 
	// troubleshooting.
	for (uint8_t i = 0; i < 100; i++) {
		filename[14] = i / 10 + '0';
		filename[15] = i % 10 + '0';
		
		if (!sd.exists(filename)) {
			// when sd.exists() returns false, this block
			// of code will be executed to open the file
			if (!logfile.open(filename, O_RDWR | O_CREAT | O_AT_END)) {
				// If there is an error opening the file, notify the
				// user. Otherwise, the file is open and ready for writing
				// Turn both indicator LEDs on to indicate a failure
				// to create the log file
//				digitalWrite(ERRLED, !digitalRead(ERRLED)); // Toggle error led 
//				digitalWrite(GREENLED, !digitalRead(GREENLED)); // Toggle indicator led 
				delay(5);
			}
			break; // Break out of the for loop when the
			// statement if(!logfile.exists())
			// is finally false (i.e. you found a new file name to use).
		} // end of if(!sd.exists())
	} // end of file-naming for loop
	//------------------------------------------------------------
  // Write 1st header line
  logfile.println(F("POSIXt,DateTime,TC0,TC1,TC2,TC3,TC4,TC5,TC6,TC7"));
	// Update the file's creation date, modify date, and access date.
	logfile.timestamp(T_CREATE, time1.year(), time1.month(), time1.day(), 
			time1.hour(), time1.minute(), time1.second());
	logfile.timestamp(T_WRITE, time1.year(), time1.month(), time1.day(), 
			time1.hour(), time1.minute(), time1.second());
	logfile.timestamp(T_ACCESS, time1.year(), time1.month(), time1.day(), 
			time1.hour(), time1.minute(), time1.second());
	logfile.close(); // force the data to be written to the file by closing it
} // end of initFileName function


//---------- startTIMER2 ----------------------------------------------------
// startTIMER2 function
// Starts the 32.768kHz clock signal being fed into XTAL1 from the
// real time clock to drive the
// quarter-second interrupts used during data-collecting periods. 
// Supply a current DateTime time value. 
// This function returns a DateTime value that can be used to show the 
// current time when returning from this function. 
DateTime startTIMER2(DateTime currTime, RTC_DS3231& rtc, byte SPS){
	TIMSK2 = 0; // stop timer 2 interrupts

	rtc.enable32K();
	ASSR = _BV(EXCLK); // Set EXCLK external clock bit in ASSR register
	// The EXCLK bit should only be set if you're trying to feed the
	// 32.768 clock signal from the Chronodot into XTAL1. 

	ASSR = ASSR | _BV(AS2); // Set the AS2 bit, using | (OR) to avoid
	// clobbering the EXCLK bit that might already be set. This tells 
	// TIMER2 to take its clock signal from XTAL1/2
	TCCR2A = 0; //override arduino settings, ensure WGM mode 0 (normal mode)
	
	// Set up TCCR2B register (Timer Counter Control Register 2 B) to use the 
	// desired prescaler on the external 32.768kHz clock signal. Depending on 
	// which bits you set high among CS22, CS21, and CS20, different 
	// prescalers will be used. See Table 18-9 on page 158 of the AVR 328P 
	// datasheet.
	//  TCCR2B = 0;  // No clock source (Timer/Counter2 stopped)
	// no prescaler -- TCNT2 will overflow once every 0.007813 seconds (128Hz)
	//  TCCR2B = _BV(CS20) ; 
	// prescaler clk/8 -- TCNT2 will overflow once every 0.0625 seconds (16Hz)
	//  TCCR2B = _BV(CS21) ; 

	if (SPS == 4){
		// prescaler clk/32 -- TCNT2 will overflow once every 0.25 seconds
		TCCR2B = _BV(CS21) | _BV(CS20); 
	} else if (SPS == 2) {
		TCCR2B = _BV(CS22) ; // prescaler clk/64 -- TCNT2 will overflow once every 0.5 seconds
	} else if (SPS == 1){
		TCCR2B = _BV(CS22) | _BV(CS20); // prescaler clk/128 -- TCNT2 will overflow once every 1 seconds
	}


	// Pause briefly to let the RTC roll over a new second
	DateTime starttime = currTime;
	// Cycle in a while loop until the RTC's seconds value updates
	while (starttime.second() == currTime.second()) {
		delay(1);
		currTime = rtc.now(); // check time again
	}

	TCNT2 = 0; // start the timer at zero
	// wait for the registers to be updated
	while (ASSR & (_BV(TCN2UB) | _BV(TCR2AUB) | _BV(TCR2BUB))) {} 
	TIFR2 = _BV(OCF2B) | _BV(OCF2A) | _BV(TOV2); // clear the interrupt flags
	TIMSK2 = _BV(TOIE2); // enable the TIMER2 interrupt on overflow
	// TIMER2 will now create an interrupt every time it rolls over,
	// which should be every 0.25, 0.5 or 1 seconds (depending on value 
	// of SAMPLES_PER_SECOND) regardless of whether the AVR is awake or asleep.
	return currTime;
}

 
//-----------printTempToOLEDs--------------------------------------------------
// Update temperature values on the 2 OLED screens. This function only updates
// elements that have changed, and leaves the rest of the screen static. 
void printTempToOLEDs (SSD1306AsciiWire& oled1, SSD1306AsciiWire& oled2, double *tempAverages, double *prevAverages){
	// Print stuff to screens
          oled1.home();
          oled1.set2X();
//          oled1.clearToEOL();
          for (byte j = 0; j < 4; j++){
            // Check to see if the value has changed and
            // only update if it's changed, and check that the 
			// new value isn't also NAN. 
			if ( isnan(prevAverages[j]) & isnan(tempAverages[j]) ){
				// If previous value was nan and current value
				// value is also nan, no need to update
				oled1.setRow(oled1.row()+2); // move to next row
			} else if ( (tempAverages[j] != prevAverages[j]) & !isnan(prevAverages[j]) ){
				// If old and new values don't match, and the previous value
				// wasn't nan, then the display value needs to be updated
              oled1.clear(60,128,oled1.row(),(oled1.row()+1));
              oled1.print(tempAverages[j]);
              oled1.println(F("C"));
            } else {
			   // If the two values match exactly, no update needed.
              // Skip to next row. At 2x size, each row is 2units tall
              oled1.setRow(oled1.row()+2);
            }
          }
          oled2.home();
          oled2.set2X();
          for (byte j = 4; j < 8; j++){
            // Check to see if the value has changed and
            // only update if it's changed, and check that the 
			// new value isn't also NAN. 
			if ( isnan(prevAverages[j]) & isnan(tempAverages[j]) ){
				// If previous value was nan and current value
				// value is also nan, no need to update
				oled2.setRow(oled2.row()+2); // move to next row
			} else if ( (tempAverages[j] != prevAverages[j]) & !isnan(prevAverages[j]) ){
				// If old and new values don't match, and the previous value
				// wasn't nan, then the display value needs to be updated
              oled2.clear(60,128,oled2.row(),(oled2.row()+1));
              oled2.print(tempAverages[j]);
              oled2.println(F("C"));
            } else {
			   // If the two values match exactly, no update needed.
              // Skip to next row. At 2x size, each row is 2units tall
              oled2.setRow(oled2.row()+2);
            }
          } 
}  


//----------correctTemp-------------------------------
// Apply NIST correction to MAX31855 temperatures. Based on code from
// https://github.com/heypete/MAX31855-Linearization
// This code isn't necessary when using MAX31856 chips
double correctTemp(float rawTemp, float internalTemp){
	       // Initialize variables.
       int i = 0; // Counter for arrays
       double thermocoupleVoltage= 0;
       double internalVoltage = 0;
       double correctedTemp = 0;
       double totalVoltage = 0;
		// Steps 1 & 2. Subtract cold junction temperature from the raw 
		// thermocouple temperature.
          thermocoupleVoltage = (rawTemp - internalTemp)*0.041276;  // C * mv/C = mV
 
          // Step 3. Calculate the cold junction equivalent thermocouple voltage.
 
          if (internalTemp >= 0) { // For positive temperatures use appropriate NIST coefficients
             // Coefficients and equations available from http://srdata.nist.gov/its90/download/type_k.tab
 
             double c[] = {-0.176004136860E-01,  0.389212049750E-01,  0.185587700320E-04, -0.994575928740E-07,  0.318409457190E-09, -0.560728448890E-12,  0.560750590590E-15, -0.320207200030E-18,  0.971511471520E-22, -0.121047212750E-25};
 
             // Count the the number of coefficients. There are 10 coefficients for positive temperatures (plus three exponential coefficients),
             // but there are 11 coefficients for negative temperatures.
             int cLength = sizeof(c) / sizeof(c[0]);
 
             // Exponential coefficients. Only used for positive temperatures.
             double a0 =  0.118597600000E+00;
             double a1 = -0.118343200000E-03;
             double a2 =  0.126968600000E+03;
 
 
             // From NIST: E = sum(i=0 to n) c_i t^i + a0 exp(a1 (t - a2)^2), 
			 // where E is the thermocouple voltage in mV and t is the temperature
			// in degrees C.
             // In this case, E is the cold junction equivalent thermocouple 
			 // voltage.
             // Alternative form: C0 + C1*internalTemp + C2*internalTemp^2 +
			// C3*internalTemp^3 + ... + C10*internaltemp^10 +
			// A0*e^(A1*(internalTemp - A2)^2)
             // This loop sums up the c_i t^i components.
             for (i = 0; i < cLength; i++) {
                internalVoltage += c[i] * pow(internalTemp, i);
             }
                // This section adds the a0 exp(a1 (t - a2)^2) components.
                internalVoltage += a0 * exp(a1 * pow((internalTemp - a2), 2));
          }
          else if (internalTemp < 0) { // for negative temperatures
             double c[] = {0.000000000000E+00,  0.394501280250E-01,  0.236223735980E-04, -0.328589067840E-06, -0.499048287770E-08, -0.675090591730E-10, -0.574103274280E-12, -0.310888728940E-14, -0.104516093650E-16, -0.198892668780E-19, -0.163226974860E-22};
             // Count the number of coefficients.
             int cLength = sizeof(c) / sizeof(c[0]);
 
             // Below 0 degrees Celsius, the NIST formula is simpler and has no
			 // exponential components: E = sum(i=0 to n) c_i t^i
             for (i = 0; i < cLength; i++) {
                internalVoltage += c[i] * pow(internalTemp, i) ;
             }
          }
 
          // Step 4. Add the cold junction equivalent thermocouple voltage
		  // calculated in step 3 to the thermocouple voltage calculated in step 2.
          totalVoltage = thermocoupleVoltage + internalVoltage;
 
          // Step 5. Use the result of step 4 and the NIST voltage-to-temperature
		  // (inverse) coefficients to calculate the cold junction compensated,
		  // linearized temperature value.
          // The equation is in the form correctedTemp = d_0 + d_1*E + d_2*E^2 + ... + d_n*E^n, 
		  // where E is the totalVoltage in mV and correctedTemp is in degrees C.
          // NIST uses different coefficients for different temperature subranges: (-200 to 0C), (0 to 500C) and (500 to 1372C).
          if (totalVoltage < 0) { // Temperature is between -200 and 0C.
             double d[] = {0.0000000E+00, 2.5173462E+01, -1.1662878E+00, -1.0833638E+00, -8.9773540E-01, -3.7342377E-01, -8.6632643E-02, -1.0450598E-02, -5.1920577E-04, 0.0000000E+00};
 
             int dLength = sizeof(d) / sizeof(d[0]);
             for (i = 0; i < dLength; i++) {
                correctedTemp += d[i] * pow(totalVoltage, i);
             }
          }
          else if (totalVoltage < 20.644) { // Temperature is between 0C and 500C.
             double d[] = {0.000000E+00, 2.508355E+01, 7.860106E-02, -2.503131E-01, 8.315270E-02, -1.228034E-02, 9.804036E-04, -4.413030E-05, 1.057734E-06, -1.052755E-08};
             int dLength = sizeof(d) / sizeof(d[0]);
             for (i = 0; i < dLength; i++) {
                correctedTemp += d[i] * pow(totalVoltage, i);
             }
          }
          else if (totalVoltage < 54.886 ) { // Temperature is between 500C and 1372C.
             double d[] = {-1.318058E+02, 4.830222E+01, -1.646031E+00, 5.464731E-02, -9.650715E-04, 8.802193E-06, -3.110810E-08, 0.000000E+00, 0.000000E+00, 0.000000E+00};
             int dLength = sizeof(d) / sizeof(d[0]);
             for (i = 0; i < dLength; i++) {
                correctedTemp += d[i] * pow(totalVoltage, i);
             }
          } else { // NIST only has data for K-type thermocouples from -200C to +1372C. If the temperature is not in that range, set temp to impossible value.
             // Error handling should be improved.
             // Serial.print("Temperature is out of range. This should never happen.");
             correctedTemp = NAN;
          }
		  return correctedTemp;
}

//**********EEPROM_WriteFloat***************************
// Function to write a float value (4 bytes) to EEPROM
void EEPROM_WriteFloat(float *num, int MemPos)
{
 byte ByteArray[4];
 memcpy(ByteArray, num, 4);
 for(int x = 0; x < 4; x++)
 {
   EEPROM.write((MemPos * 4) + x, ByteArray[x]);
 }  
}
//*********EEPROM_ReadFloat*********************************
// Function to read back a float value (4 bytes) from EEPROM
void EEPROM_ReadFloat(float *num, int MemPos)
{
 byte ByteArray[4];
 for(int x = 0; x < 4; x++)
 {
   ByteArray[x] = EEPROM.read((MemPos * 4) + x);    
 }
 memcpy(num, ByteArray, 4);
}