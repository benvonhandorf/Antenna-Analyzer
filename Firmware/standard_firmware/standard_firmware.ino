// This file is part of the K6BEZ Antenna Analyzer project.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
//   This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

// include the library code:
#include <LiquidCrystal.h>
#include "si5351.h"
#include "Wire.h"

// * LCD RS pin to digital pin 8
// * LCD Enable pin to digital pin 9
// * LCD D4 pin to digital pin 15
// * LCD D5 pin to digital pin 14
// * LCD D6 pin to digital pin 16
// * LCD D7 pin to digital pin 10

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(8, 9, 15, 14, 16, 10);


long Fstart = 1000000;  // Start Frequency for sweep
long Fstop = 30000000;  // Stop Frequency for sweep
unsigned long current_freq; // Temp variable used during sweep
long serial_input_number; // Used to build number from serial stream
long num_steps = 101; // Number of steps to use in the sweep
char incoming_char; // Character read from serial stream
byte mode_pressed = 0;
int mode = 1;

// CALIBRATION VALUES
// set signal generator to zero output, measure FWD and REV over a long period for these values. If not, set both equal to 0.
int fwdOffset = 0; //6 compensates for idle noise on the fwd diode and amplifier
int revOffset = 0; //4 compensates for idle noise on the fwd diode and amplifier
// Short the antenna port to ground, measure FWD and REV over a sweep for this ratio (REV/FWD). If not, set  equal to 1.
// see comments in PerformSweep() about alternative compensation schemes....there is a frequency dependent noise component.
double diodeComp = 1;
// Short the diode cathodes together, measure FWD and REV over a sweep for this ratio (FWD/REV). If not, set both equal to 1.
// see comments in PerformSweep() about alternative compensation schemes....there is a frequency dependent noise component.
double gainComp = 1;

//Look at the Si5351 library to determine the calibration value for your Si5351 - https://github.com/etherkit/Si5351Arduino
#define SI5351_CORRECTION 0 

Si5351 si5351;

void setup() {
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);

  // Print a message to the LCD.
  lcd.print("Antenna Analyzer");

  //Initialize the DDS module
  si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, SI5351_CORRECTION);

  //Set a 2MA drive strength - 3dBm.  This can be adjusted to 4, 6 or 8 mA, per your needs
  si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_2MA);

  // Set up analog inputs on A0 and A1, internal reference voltage
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  analogReference(DEFAULT);

  // initialize serial communication
  Serial.begin(115200);
  Serial.println("Anteanna Analyzer");

  //Initialise the incoming serial number to zero
  serial_input_number = 0;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("1-30 MHz");

  Fstart = 6000000;
  Fstop = 8000000;
}

void loop() {
  //Check for character
  if (Serial.available() > 0) {
    mode = 0;
    lcd.clear();
    lcd.setCursor(14, 0);
    lcd.print("PC");
    incoming_char = Serial.read();
    switch (incoming_char) {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        serial_input_number = serial_input_number * 10 + (incoming_char - '0');
        break;
      case 'A':
        //Turn frequency into FStart
        Fstart = serial_input_number;
        serial_input_number = 0;
        break;
      case 'B':
        //Turn frequency into FStop
        Fstop = serial_input_number;
        serial_input_number = 0;
        break;
      case 'C':
        //Turn frequency into FStart and set DDS output to single frequency
        Fstart = serial_input_number;
        SetDDSFreq(Fstart);
        serial_input_number = 0;
        break;
      case 'N':
        // Set number of steps in the sweep
        num_steps = serial_input_number;
        serial_input_number = 0;
        break;
      case 'S':
      case 's':
        Perform_sweep();
        break;
      case '?':
        // Report current configuration to PC
        Serial.print("Start Freq:");
        Serial.println(Fstart);
        Serial.print("Stop Freq:");
        Serial.println(Fstop);
        Serial.print("Num Steps:");
        Serial.println(num_steps);
        break;
    }
    Serial.flush();
  } else {
    //No serial data was received
    if (mode > 0) {
      Perform_sweep();
    }
  }

  if (mode_pressed == 1) {
    mode_pressed = 0;
    mode += 1;
    if (mode == 12) {
      mode = 1;
    }
    num_steps = 1000;

    switch (mode) {
      case 1:
        // Full sweep 1-30 MHz
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("1-30 MHz");
        Fstart = 1000000;
        Fstop = 30000000;
        break;
      case 2:
        // 160m
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("160m");
        Fstart = 1500000;
        Fstop = 2300000;
        break;
      case 3:
        // 80m
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("80m");
        Fstart = 2000000;
        Fstop = 5000000;
        break;
      case 4:
        // 60m
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("60m");
        Fstart = 5000000;
        Fstop = 6000000;
        break;
      case 5:
        // 40m
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("40m");
        Serial.print("40m");
        Fstart = 6000000;
        Fstop = 8000000;
        break;
      case 6:
        // 30m
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("30m");
        Fstart = 9000000;
        Fstop = 11000000;
        break;
      case 7:
        // 20m
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("20m");
        Fstart = 13000000;
        Fstop = 16000000;
        break;
      case 8:
        // 17m
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("17m");
        Fstart = 17000000;
        Fstop = 19000000;
        break;
      case 9:
        // 15m
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("15m");
        Fstart = 20000000;
        Fstop = 23000000;
        break;
      case 10:
        // 12m
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("12m");
        Fstart = 24000000;
        Fstop = 26000000;
        break;
      case 11:
        // 10m
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("10m");
        Fstart = 28000000;
        Fstop = 30000000;
        break;
    }
  }
}

void Perform_sweep() {
  int FWD = 0;
  int REV = 0;
  double VSWR;
  double minVSWR;
  long minFreq;
  minVSWR = 999;
  minFreq = Fstart;

  // Start loop
  for (long i = 0; i <= num_steps; i++) {
    FWD = 0;
    REV = 0;
    
    // Calculate current frequency
    current_freq = Fstart + i * ((Fstop - Fstart) / num_steps);

    // Set DDS to current frequency
    SetDDSFreq(current_freq);
    delay(1);

    //discard a few measurements
    for (int j = 0; j < 100; j++) {
      analogRead(A1);
      analogRead(A0);
    }

    
    delay(10);

    // Average the reverse and foward voltages and calibrate them
    for (int k = 0; k < 50; k++) {
      REV += (analogRead(A0) - revOffset);
      FWD += (analogRead(A1) - fwdOffset);
      delay(1);
    }
    FWD /= 50;
    REV /= 50;

    // Mean compensation values
    // FWD = diodeComp * FWD;
    // REV = gainComp * REV;

    // Linear compensation equatiions
    // FWD = FWD * (2e-9 * current_freq + 0.9231) ;
    // REC = REV *(2e-10 * current_freq + 0.9916);

    // Polynomial compensation equations
    FWD = FWD * (8.9070e-17 * current_freq * current_freq - 8.1425e-10 * current_freq + 0.9832);
    REV = REV * (-2.7626e-18 * current_freq * current_freq + 3.1516e-10 * current_freq + 0.9912);

    if (REV >= FWD) {
      // To avoid a divide by zero or negative VSWR then set to max 999
      Serial.print("reverse - ");
      Serial.print(FWD);
      Serial.print(", ");
      Serial.println(REV);
      VSWR = 999;
    } else {
      // Calculate VSWR
      VSWR = ((double)FWD + (double)REV) / ((double)FWD - (double)REV);
    }

    if (VSWR <= minVSWR) {
      minFreq = current_freq;
      minVSWR = VSWR;
    }

    if (mode == 0) {
      // Send current line back to PC over serial bus
      Serial.print(current_freq);
      Serial.print(",");
      Serial.print(long(VSWR * 1000)); // This *1000 is to make the system compatible with the PIC version
      Serial.print(",");
      Serial.print(FWD);
      Serial.print(",");
      Serial.println(REV);
    }
  }

  // Send "End" to PC to indicate end of sweep
  if (mode == 0) {
    Serial.println("End");
    Serial.print("Freq ");
    Serial.print(minFreq);
    Serial.print(", VSWR ");
    Serial.println(minVSWR, 3);
    Serial.flush();
  }

  // display results
  lcd.setCursor(0, 1);
  lcd.print("F:");
  if (minFreq < 10e6) {
    lcd.setCursor(3, 1);
  }
  else
    lcd.setCursor(2, 1);
  lcd.print(minFreq / 1e6, 3);
  lcd.setCursor(9, 1);
  lcd.print("V:");
  if (minVSWR > 1)
    lcd.setCursor(11, 1);
  else
    lcd.setCursor(10, 1);
  lcd.print(minVSWR, 3);

  si5351.output_enable(SI5351_CLK0, 0);
}

void SetDDSFreq(long Freq_Hz) {
  unsigned long long frequency_cHz = Freq_Hz * 100ULL;
  si5351.set_freq(frequency_cHz, SI5351_CLK0);
  si5351.output_enable(SI5351_CLK0, 1);
}
