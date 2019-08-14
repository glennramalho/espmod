#include "driver/pcnt.h"

// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(115200);
   pinMode(LED_BUILTIN, OUTPUT);
   pcnt_config_t pc;
  
   pc.pulse_gpio_num = 12;
   pc.ctrl_gpio_num = 13;
   pc.pos_mode = PCNT_COUNT_INC;
   pc.neg_mode = PCNT_COUNT_DIS;
   pc.lctrl_mode = PCNT_MODE_KEEP;
   pc.hctrl_mode = PCNT_MODE_KEEP;
   pc.counter_h_lim = 25;
   pc.counter_l_lim = 0;
   pc.unit = PCNT_UNIT_0;
   pc.channel = PCNT_CHANNEL_0;
   pcnt_unit_config(&pc);
   pcnt_counter_pause(PCNT_UNIT_0);
   pcnt_counter_clear(PCNT_UNIT_0);
   pcnt_event_enable(PCNT_UNIT_0, PCNT_EVT_H_LIM);
   pcnt_counter_resume(PCNT_UNIT_0);
}

// the loop function runs over and over again forever
void loop() {
   int16_t count;
   pcnt_get_counter_value(PCNT_UNIT_0, &count);
   Serial.printf("Counter = %d\r\n", count);
   delay(10);
}
