/* CMSIS-DAP Interface Firmware
 * Copyright (c) 2009-2013 ARM Limited
 *
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
 */
#ifndef VERSION_H
#define VERSION_H

#include "stdint.h"

// built for bootloader 1xxx
//#define FW_BUILD "1203"
// build for bootloader 0xxx
#define FW_BUILD "0226"

void update_html_file(uint8_t *buf, uint32_t bufsize);
uint8_t * get_uid_string          (void);
uint8_t   get_len_string_interface(void);
uint8_t * get_uid_string_interface(void);
void      init_auth_config        (void);
void build_mac_string(uint32_t *uuid_data);

#endif
