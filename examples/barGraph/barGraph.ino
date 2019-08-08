/*
  LED bar graph

  Turns on a series of LEDs based on the value of an analog sensor.
  This is a simple way to make a bar graph display. Though this graph uses 10
  LEDs, you can use any number by changing the LED count and the pins in the
  array.

  This method can be used to control any series of digital outputs that depends
  on an analog input.

  The circuit:
  - LEDs from pins 2 through 11 to ground

  created 4 Sep 2010
  by Tom Igoe

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/BarGraph

  Modified 31 July 2019 by Glenn Ramalho
*/

// these constants won't change:
const int analogPin = A0;   // the pin that the potentiometer is attached to
const int ledCount = 10;    // the number of LEDs in the bar graph

int ledPins[] = {
  15, 2, 4, 16, 5, 18, 19, 21, 3, 1
};   // an array of pin numbers to which LEDs are attached


void setup() {
  // loop over the pin array and set them all to output:
  for (int thisLed = 0; thisLed < ledCount; thisLed++) {
    pinMode(ledPins[thisLed], OUTPUT);
  }
}

void loop() {
  // read the potentiometer:
  int sensorReading = analogRead(analogPin);
  // map the result to a range from 0 to the number of LEDs. Note that the
  // ADC is set to 12 bits, so the range is from 0 to 4095 and the DB range
  // is 0-3.9V.
  int ledLevel = map(sensorReading, 0, 1023, 0, ledCount);

  // loop over the LED array:
  for (int thisLed = 0; thisLed < ledCount; thisLed++) {
    // if the array element's index is less than ledLevel,
    // turn the pin for this element on:
    if (thisLed < ledLevel) {
      digitalWrite(ledPins[thisLed], HIGH);
    }
    // turn off all pins higher than the ledLevel:
    else {
      digitalWrite(ledPins[thisLed], LOW);
    }
  }
#ifdef SYSCMOD
  /* The simulation time only advances with delay statements. Therefore, to get
   * the model to work there must be at least some delays periodically. This
   * also then allowes the other threads to advance. So, if the loop() function
   * has no delays a small delay will work.
   * Note that the ifdef SYSCMOD will make this delay apply only to the
   * simulation, it will not be visible on the real system.
   */
  delay(1);
#endif
}
