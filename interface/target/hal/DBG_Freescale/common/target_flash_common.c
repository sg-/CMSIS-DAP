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

#include "target_flash_common.h"
#include "target_flash.h"

#include "target_flash.inc"
#include "string.h"

extern uint32_t usb_buffer[];

static uint32_t check_range(const uint32_t test, const uint32_t min, const uint32_t max)
{
    return ((test < min) || (test > max)) ? 0 : 1;
}

static uint32_t check_value(const uint32_t test, const uint32_t val)
{
    return (test == val) ? 1 : 0;
}

// only testing 
static uint32_t validate_bin_nvic(void)
{
    uint32_t i = 0;
    uint32_t mem;
    uint32_t test_val = 0;
    // Initial_SP
    for( ; i<(1*4); i+=4)
    {	
        mem = (uint32_t)usb_buffer[i];
        // check for a valid ram address.
        if (0 == check_range(mem, TGT_START_RAM, TGT_END_RAM)) {
            return 0;
        }
    }
    // Reset_Handler
    // NMI_Handler
    // HardFault_Handler
    // MemManage_Handler (Reserved on CM0+)
    // BusFault_Handler (Reserved on CM0+)
    // UsageFault_Handler (Reserved on CM0+)
    for( ; i<(4*4); i+=4)
    {	
        mem = (uint32_t)usb_buffer[i/4];
        // check for a valid flash address.
        if (0 == check_range(mem, TGT_START_FLASH, TGT_END_FLASH)) {
            return 0;
        }
    }
    i += 12;    // only testing M0 entries
    // RESERVED * 4
    for( ; i<(11*4); i+=4)
    {
        mem = (uint32_t)usb_buffer[i/4];
        // check for a known value.
        if (0 == check_value(mem, 0)) {
            return 0;
        }
    }
    // SVC_Handler
    // DebugMon_Handler (Reserved on CM0+)
    for( ; i<(12*4); i+=4)
    {	
        mem = (uint32_t)usb_buffer[i/4];    
        // check for a valid flash address.
        if (0 == check_range(mem, TGT_START_FLASH, TGT_END_FLASH)) {
            return 0;
        }
    }
    i += 4;    // only testing M0 entries
    // RESERVED * 1
    for( ; i<(14*4); i+=4)
    {
        mem = (uint32_t)usb_buffer[i/4];
        // check for a known value
        if (0 == check_value(mem, 0)) {
            return 0;
        }
    }
    // PendSV_Handler
    // SysTick_Handler
    for( ; i<(16*4); i+=4)
    {	
        mem = (uint32_t)usb_buffer[i/4];   
        // check for a valid flash address.
        if (0 == check_range(mem, TGT_START_FLASH, TGT_END_FLASH)) {
            return 0;
        }
    }
    return 1;
}

uint8_t target_flash_init(uint32_t clk) {
    // Download flash programming algorithm to target and initialise.
    if (!swd_write_memory(flash.algo_start, (uint8_t *)flash.image, flash.algo_size)) {
        return 0;
    }

    if (!swd_flash_syscall_exec(&flash.sys_call_param, flash.init, 0, 0 /* clk value is not used */, 0, 0)) {
        return 0;
        //return 1;
    }

    return 1;
}

uint32_t target_flash_uninit(void)
{
    return swd_flash_syscall_exec_return_value(&flash.sys_call_param, flash.uninit, 0, 0, 0, 0);  
}

uint8_t target_flash_erase_sector(unsigned int sector) {
    if (sector == 0) {
        // need to validate the NVIC table or do not erase
        if (validate_bin_nvic() == 0) {
            return 0;
        }
    }
    
    if (!swd_flash_syscall_exec(&flash.sys_call_param, flash.erase_sector, sector * FLASH_SECTOR_SIZE, 0, 0, 0)) {
        return 0;
    }

    return 1;
}

uint8_t target_flash_erase_chip(void) {
    // need to validate the NVIC table or do not erase
    if (validate_bin_nvic() == 0) {
        return 0;
    }
    if (!swd_flash_syscall_exec(&flash.sys_call_param, flash.erase_chip, 0, 0, 0, 0)) {
        return 0;
    }

    return 1;
}

// Check Flash Configuration Field bytes at address 0x400-0x40f to ensure that flash security
// won't be enabled.
//
// FCF bytes:
// [0x0-0x7]=backdoor key
// [0x8-0xb]=flash protection bytes
// [0xc]=FSEC:
//      [7:6]=KEYEN (2'b10 is backdoor key enabled, all others backdoor key disabled)
//      [5:4]=MEEN (2'b10 mass erase disabled, all other mass erase enabled)
//      [3:2]=FSLACC (2'b00 and 2'b11 factory access enabled, 2'b01 and 2'b10 factory access disabled)
//      [1:0]=SEC (2'b10 flash security disabled, all other flash security enabled)
// [0xd]=FOPT
// [0xe]=EEPROM protection bytes (FlexNVM devices only)
// [0xf]=data flash protection bytes (FlexNVM devices only)
//
// This function checks that:
// - 0x0-0xb==0xff
// - 0xe-0xf==0xff
// - FSEC=0xfe
//
// FOPT can be set to any value.
uint8_t check_security_bits(uint32_t flashAddr, uint8_t *data) {
    
    if (flashAddr == 0x400) {
        // make sure we can unsecure the device or dont program at all
        if ((data[0xc] & 0x30) == 0x20) {
            // Dont allow programming mass-erase disabled state
            return 0;
        }
        // Security is OK long as we can mass-erase (comment the following out to enable target security)
        if ((data[0xc] & 0x03) != 0x02) {
            return 0;
        }
    }
    return 1;
}

uint8_t target_flash_program_page(uint32_t addr, uint8_t * buf, uint32_t size)
{
    uint32_t bytes_written = 0;

    // call a target dependent function to check if
    // we don't want to write specific bits (flash security bits, ...)
    if (!check_security_bits(addr, buf)) {
        return 0;
    }

    // Program a page in target flash.
    if (!swd_write_memory(flash.program_buffer, buf, size)) {
        return 0;
    }

    while(bytes_written < size) {
        if (!swd_flash_syscall_exec(&flash.sys_call_param,
                                    flash.program_page,
                                    addr,
                                    flash.ram_to_flash_bytes_to_be_written,
                                    flash.program_buffer + bytes_written, 0)) {
            return 0;
        }

        bytes_written += flash.ram_to_flash_bytes_to_be_written;
        addr += flash.ram_to_flash_bytes_to_be_written;
    }

    return 1;
}

#include "DAP_config.h"
#include <string.h>
#include <stdio.h>

uint32_t target_validate_nvic(uint32_t address)
{
    uint32_t i = 0;
    uint32_t mem[1];
    uint32_t test_val = 0;
    // Initial_SP
    for( ; i<(1*4); i+=4)
    {	
        swd_read_word(address+i, mem);
        test_val = mem[0];
        // check for a valid ram address.
        if (0 == check_range(test_val, TGT_START_RAM, TGT_END_RAM)) {
            return 0;
        }
    }
    // Reset_Handler
    // NMI_Handler
    // HardFault_Handler
    // MemManage_Handler (Reserved on CM0+)
    // BusFault_Handler (Reserved on CM0+)
    // UsageFault_Handler (Reserved on CM0+)
    for( ; i<(7*4); i+=4)
    {	
        swd_read_word(address+i, mem);
        test_val = mem[0];    
        // check for a valid flash address.
        if (0 == check_range(test_val, TGT_START_FLASH, TGT_END_FLASH)) {
            return 0;
        }
    }
    // RESERVED * 4
    for( ; i<(11*4); i+=4)
    {
        swd_read_word(address+i, mem);
        test_val = mem[0];    
        // check for a known value.
        if (0 == check_value(test_val, 0)) {
            return 0;
        }
    }
    // SVC_Handler
    // DebugMon_Handler (Reserved on CM0+)
    for( ; i<(13*4); i+=4)
    {	
        swd_read_word(address+i, mem);
        test_val = mem[0];    
        // check for a valid flash address.
        if (0 == check_range(test_val, TGT_START_FLASH, TGT_END_FLASH)) {
            return 0;
        }
    }
    // RESERVED * 1
    for( ; i<(14*4); i+=4)
    {
        swd_read_word(address+i, mem);
        test_val = mem[0];    
        // check for a known value
        if (0 == check_value(test_val, 0)) {
            return 0;
        }
    }
    // PendSV_Handler
    // SysTick_Handler
    for( ; i<(16*4); i+=4)
    {	
        swd_read_word(address+i, mem);
        test_val = mem[0];    
        // check for a valid flash address.
        if (0 == check_range(test_val, TGT_START_FLASH, TGT_END_FLASH)) {
            return 0;
        }
    }
    return 1;
}

uint32_t valid_binary_present = 0;
extern uint32_t uuid_data[4];

void pre_run_config(void)
{    
    //int32_t i = 3, j = 0;
    puts("pre-config");
    
    while(!target_set_state(RESET_PROGRAM)) {
        puts("fail!");
    }

    while(!target_set_state(DEBUG)) {
        puts("fail!");
    }
    
    // read and do mass-erase
    target_unlock_sequence();
    
    //////////////////////////////////////////////
    // start target ID patch
    ///////////////////////////////////////////
    // get target ID
//    target_set_state(RESET_PROGRAM);
//    if (target_flash_init(SystemCoreClock)) {
//        j = target_flash_uninit();
//        if (j > 0x1FFF0000) {   // look for valid RAM address
//            // here we can read the ID
//            for(; i>=0; --i){
//                swd_read_word(j, &uuid_data[i]);
//                printf("REG:%08x\tVAL:%08x\r\n", j, uuid_data[i]);
//                j +=4;
//            }
//        }
//    }
    //////////////////////////////////////////////
    // end the target ID patch
    
    // verify vector table and decide what state to leave target in
    valid_binary_present = target_validate_nvic(0x0);
    printf("NVIC is %s\n", (valid_binary_present) ? "valid" : "invalid" );
    target_set_state(NO_DEBUG);
    if (!valid_binary_present) {
        // should just have to hold the target in reset
        target_set_state(RESET_HOLD);
    } else {
        target_set_state(RESET_RUN);
    }
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

//uint32_t get_target_uuid(uint8_t* data, uint32_t size)
//{
//    uint32_t tmp = 0;
//    uint32_t address;
//    if (swd_read_word(address, &tmp) == 1) {
//        data[0] = (uint8_t) tmp & 0xff;
//        data[1] = (uint8_t) (tmp>>8 ) & 0xff;
//        data[2] = (uint8_t) (tmp>>16) & 0xff;
//        data[3] = (uint8_t) (tmp>>24) & 0xff;
//        return 1;
//    }
//    return 0;
//}
