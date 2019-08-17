/*******************************************************************************
 * update.h -- Copyright 2019 Glenn Ramalho - RFIDo Design
 *******************************************************************************
 * Description:
 *   Functions to notify the modules that the IO structures have changed. This
 *   allowes also C programs to talk to the SystemC C++ model.
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

#ifndef _UPDATE_H
#define _UPDATE_H

#ifdef __cplusplus
extern "C" {
#endif

/* After changing PCNT you must call this function to update the model. */
void update_pcnt();
/* After changing GPIO you must call one of these functions to update the model.
 */
void update_gpio();
void update_gpio_reg();
void update_gpio_oe();

#ifdef __cplusplus
}
#endif

#endif
