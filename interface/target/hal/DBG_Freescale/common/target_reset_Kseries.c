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
#include "target_reset.h"
#include "swd_host.h"

#define MDM_STATUS 0x01000000
#define MDM_CTRL   0x01000004
#define MDM_IDR    0x010000fc

#define MCU_ID     0x001c0000

void target_before_init_debug(void) {
    swd_set_target_reset(1);
}

uint8_t target_unlock_sequence(void) {
    uint32_t val;

    // read the device ID
    if (!swd_read_ap(MDM_IDR, &val)) {
        return 0;
    }
    // verify the result
    if (val != MCU_ID) {
        return 0;
    }

    if (!swd_read_ap(MDM_STATUS, &val)) {
        return 0;
    }

    // flash in secured mode
    if (val & (1 << 2)) {
        // hold the device in reset
        target_set_state(RESET_HOLD);
        // write the mass-erase enable bit
        if (!swd_write_ap(MDM_CTRL, 1)) {
            return 0;
        }
        while (1) {
            // wait until mass erase is started
            if (!swd_read_ap(MDM_STATUS, &val)) {
                return 0;
            }

            if (val & 1) {
                break;
            }
        }
        // mass erase in progress
        while (1) {            
            // keep reading until procedure is complete
            if (!swd_read_ap(MDM_CTRL, &val)) {
                return 0;
            }

            if (val == 0) {
                break;
            }
        }
    }

    return 1;
}

uint8_t target_set_state(TARGET_RESET_STATE state) {
    return swd_set_target_state(state);
}


/////////////////////////////////////////////////////////////////////////
// target known state testing

#include <stdio.h>

#define ITM_Port8(n)    (*((volatile unsigned char *)(0xE0000000+4*n)))
#define ITM_Port16(n)   (*((volatile unsigned short*)(0xE0000000+4*n)))
#define ITM_Port32(n)   (*((volatile unsigned long *)(0xE0000000+4*n)))
#define DEMCR           (*((volatile unsigned long *)(0xE000EDFC)))
#define TRCENA          (0x01000000)
 
struct __FILE { int handle; /* Add whatever needed */ };
FILE __stdout;
FILE __stdin;

int fputc(int ch, FILE *f) 
{
    if (DEMCR & TRCENA)
    {
        while (ITM_Port32(0) == 0);
        ITM_Port8(0) = ch;
    }
    return(ch);
}

uint32_t check_range(const uint32_t test, const uint32_t min, const uint32_t max)
{
    return ((test < min) || (test > max)) ? 0 : 1;
}

uint32_t target_validate_nvic(void)
{
    uint32_t data = 0, ret_val = 0;
    if (swd_read_word(0x0, &data) == 1) {
        printf("SP 0x%08x\n", data);
        ret_val = check_range(data, 0x1FFF0000, 0x20030000); // test RAM ranges
    }
    return ret_val;
}

uint32_t get_target_uuid(uint8_t* data, uint32_t size)
{
    uint32_t tmp = 0;
    uint32_t address;
    if (swd_read_word(address, &tmp) == 1) {
        data[0] = (uint8_t) tmp & 0xff;
        data[1] = (uint8_t) (tmp>>8 ) & 0xff;
        data[2] = (uint8_t) (tmp>>16) & 0xff;
        data[3] = (uint8_t) (tmp>>24) & 0xff;
    }
    return 1;
}
