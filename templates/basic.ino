/***********************
 * setup():
 * inputs: none
 * return: none
 * globals:
 *
 * Main Arduino Setup function, run once after POR or reset.
 */
void setup() {
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
#ifdef SYSCMOD
   /* This is needed for the model as TaskScheduler uses a busy wait and not
    * a regular sleep request. Other engironments might have the same busy
    * wait.
    */
   delay(1);
#endif
}
