#include <systemc.h>
#include "Arduino.h"
#include "main.h"

/**********************
 * Function: setup()
 *
 * Main setup. Does nothing. Used mainly for compatabvility with the SystemC
 * model.
 */
void setup() { }

/**********************
 * Function: loop()
 *
 * Calls the firmware. Then it stops simulations.
 */
void loop() {
   app_main();
   sc_stop();
}
