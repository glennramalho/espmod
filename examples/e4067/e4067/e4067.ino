/*******************************************************************************
 * e4067.ino -- Copyright 2019 (c) Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   This is the INO file for the e4067 example. It is just a simple file to
 * test the cd4067 model.
 *******************************************************************************
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *******************************************************************************
 */

#include "driver/adc.h"
int sel;

#define D0 4
#define D1 5
#define D2 13
#define D3 14
#define ANALOG_PIN_0 36
#define ANALOG_PIN_1 39
#define ANALOG_PIN_2 34

/***********************
 * drive():
 * inputs: integer to drive
 * return: none
 * globals:
 *
 * Drives the D pins
 */
void drive(int z) {
   digitalWrite(D0, (z & 0x1)?HIGH:LOW);
   digitalWrite(D1, (z & 0x2)?HIGH:LOW);
   digitalWrite(D2, (z & 0x4)?HIGH:LOW);
   digitalWrite(D3, (z & 0x8)?HIGH:LOW);
}

/***********************
 * setup():
 * inputs: none
 * return: none
 * globals:
 *
 * Main Arduino Setup function, run once after POR or reset.
 */
void setup() {
   Serial.begin(115200);
   pinMode(D0, OUTPUT);
   pinMode(D1, OUTPUT);
   pinMode(D2, OUTPUT);
   pinMode(D3, OUTPUT);
   drive(0);
   pinMode(ANALOG_PIN_0, INPUT);
   adc1_config_width(ADC_WIDTH_BIT_10);
   adc1_config_channel_atten(ADC1_CHANNEL_0,ADC_ATTEN_DB_11);  //ADC_ATTEN_DB_11 = 0-3,6V
}

/***********************
 * loop():
 * inputs: none
 * return: none
 * globals:
 *
 * Main Arduino infinite loop.
 */
void loop() {
   int f = adc1_get_raw( ADC1_CHANNEL_0 );
   int z = (millis() / 1000) % 4;
   drive(z);
   Serial.printf("For sel @ %d got ADC1 raw to be %d\r\n", z, f);
   delay(200);
}
