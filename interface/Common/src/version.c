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
#include <string.h>

#include "main.h"
#include "version.h"
#include "target_config.h"
#include "board.h"
#include "read_uid.h"
#include "virtual_fs.h"

char mac_string[16] = {0};
char uuid_string[33] = {0};
// Pointers to substitution strings
const char *fw_version = (const char *)FW_BUILD;

static uint32_t unique_id[4];
static uint8_t already_unique_id = 0;
static uint32_t auth;
uint8_t string_auth[49 + 4]; //25 + 4
uint8_t string_auth_descriptor[2+49*2]; // 2+25*2
static const char nybble_chars[] = "0123456789abcdef";

static void setup_string_id_auth(void);

static void get_byte_hex( uint8_t b, uint8_t *ch1, uint8_t *ch2 ) {
    *ch1 = nybble_chars[ ( b >> 4 ) & 0x0F ];
    *ch2 = nybble_chars[ b & 0x0F ];
}

void build_uuid_string(uint32_t *uuid_data)
{
    uint32_t i = 0, j = 0, k = 0;
    uint8_t b = 0;
    // format UUID
    for (; j<32; j+=8) {
        for (; i<4; i++) {
            b = uuid_data[k] >> (24-(i*8));
            get_byte_hex(b, (uint8_t *)&uuid_string[j+2*i], (uint8_t *)&uuid_string[j+2*i+1]);
        }
        i = 0;
        k++;
    }
    uuid_string[32] = '\0';
}

void build_mac_string(uint32_t *uuid_data)
{
    uint32_t i = 0;
    uint8_t b = 0;
    // patch for MAC use. Make sure MSB bits are set correctly
    //uuid_data[2] |=  (0x2 << 8);
    //uuid_data[2] &= ~(0x1 << 8);
    for (; i<2; i++) {
        b = uuid_data[1] >> (8-(i*8));
        get_byte_hex(b, (uint8_t *)&mac_string[i*2], (uint8_t *)&mac_string[i*2+1]);
    }
    for (; i<6; i++) {
        b = uuid_data[0] >> (24-((i-2)*8));
        get_byte_hex(b, (uint8_t *)&mac_string[i*2], (uint8_t *)&mac_string[i*2+1]);
    }
    mac_string[15] = '\0';
}

static uint32_t atoi(uint8_t * str, uint8_t size, uint8_t base)
{
    uint32_t k = 0;
    uint8_t i = 0, idx = 0;
    for (i = 0; i < size; i++) {
        if ((uint8_t)(*str) >= '0' && (uint8_t)(*str) <= '9') {
            idx = '0';
        }
        else if ((uint8_t)(*str) >= 'A' && (uint8_t)(*str) <= 'F') {
            idx = 'A' - 10;
        }
        else if ((uint8_t)(*str) >= 'a' && (uint8_t)(*str) <= 'f') {
            idx = 'a' - 10;
        }
        k = k*base + (uint8_t)(*str) - idx;
        str++;
    }
    return k;
}

static void setup_string_id_auth()
{
    uint8_t i = 0, j = 0, idx = 0;

    string_auth[0] = '$';
    string_auth[1] = '$';
    string_auth[2] = '$';
    string_auth[3] = 24;
    idx += 4;
    // string id
    for (i = 0; i < 4; i++) {
        string_auth[idx++] = target_device.board_id[i];
    }
    for (i = 0; i < 4; i++) {
        string_auth[idx++] = fw_version[i];
    }
    for (i = 0; i < 4; i++) {
        for(j = 0; j < 4; j++) {
            get_byte_hex((unique_id[j] >> 8*(3 - i)) & 0xff, &string_auth[idx + 2*i + 8*j], &string_auth[idx + 2*i + 1] + 8*j);
        }
    }
    idx+=32; //8

    //string auth
    for (i = 0; i < 4; i++) {
        get_byte_hex((auth >> 8*(3 - i)) & 0xff, &string_auth[idx + 2*i], &string_auth[idx + 2*i + 1]);
    }
    idx+=8;
    string_auth[idx] = 0;
}

static void setup_string_descriptor()
{
    uint8_t i = 0, idx = 0, len = 0;
    len = strlen((const char *)(string_auth+4));
    string_auth_descriptor[0] = len*2 + 2;
    string_auth_descriptor[1] = 3;
    idx += 2;

    for (i = 0; i < len*2; i++) {
        if ((i % 2) == 0) {
            string_auth_descriptor[idx + i] = string_auth[4 + i/2];
        }
        else {
            string_auth_descriptor[idx + i] = 0;
        }
    }
    idx += len*2;

    string_auth_descriptor[idx] = 0;
}


uint8_t *get_uid_string(void)
{
    return string_auth;
}

uint8_t get_len_string_interface(void)
{
    return 2 + strlen((const char *)(string_auth+4))*2;
}

uint8_t *get_uid_string_interface(void)
{
    return string_auth_descriptor;
}

static void compute_auth()
{
    uint32_t id = 0, fw = 0, sec = 0;
    id = atoi((uint8_t *)target_device.board_id  , 4, 16);
    fw = atoi((uint8_t *)fw_version, 4, 16);
    auth = (id) | (fw << 16);
    auth ^= unique_id[0];
    sec = atoi((uint8_t *)(target_device.secret), 8, 16);
    auth ^= sec;
}

// HTML character reader context
typedef struct {
    uint8_t *phtml;        // Pointer to current position in HTML data in flash
    uint8_t substitute;    // TRUE if characters should be read from a substitution string, otherwise read from HTML data.
} HTMLCTX;


static void init_html(HTMLCTX *h, uint8_t *ptr)
{
    h->substitute = 0;
    h->phtml = ptr;
    return;
}

static uint8_t get_html_character(HTMLCTX *h)
{
    // Returns the next character from the HTML data at h->phtml.
    // Substitutes special sequences @V etc. with variable text.
    uint8_t c = 0, s = 0;     // Character from HTML data
    static uint8_t *sptr;     // Pointer to substitution string data
    uint8_t valid = 0;        // Set to false if we need to read an additional character

    do {
        valid = 1;
        if (h->substitute) {
            // Check next substitution character
            if ((uint8_t)(*sptr) == '\0') {
                // End of substituted string
                h->substitute = 0;
            }
        }
        if (!h->substitute) {
            // Get next HTML character
            c = (uint8_t)(*h->phtml++);
            // Indicates substitution
            if (c == '@') {
                // Check next HTML character
                s = (uint8_t)(*h->phtml);
                switch (s) {
                    case 'A':   // platform ID string
                        sptr = (uint8_t *)(string_auth+4); // auth string
                        h->substitute = 1;
                        break;

                    // Add any additional substitutions here
                    case 'M':   // MAC address
                        h->substitute = 1;
                        sptr = (uint8_t *)&mac_string;    // target UUID only so many bits...
                        break;
                    
                    case 'U':   // UUID
                        h->substitute = 1;
                        sptr = (uint8_t *)&uuid_string;    // target UUID only so many bits...
                        break;
                    
                    default:
                        break;
                }
                if (h->substitute) {
                    // If a vaild substitution sequence was found then discard this character
                    // Increment HTML pointer
                    h->phtml++;
                    // Check for the case of the substitution string being zero characters length
                    if ((uint8_t)(*sptr) == '\0') {
                        // Effectively the substitution is already completed
                        h->substitute = 0;
                        // Must read another character
                        valid = 0;
                    }
                }
            }
        }
        if (h->substitute) {
            // Get next substitution character
            c = (uint8_t)(*sptr++);
        }
    } while (!valid);

    return c;
}

void init_auth_config(void)
{
    read_unique_id(unique_id);
    compute_auth();
    setup_string_id_auth();
    setup_string_descriptor();
}

void update_html_file(uint8_t *buf, uint32_t bufsize)
{
    // Update a file containing the version information for this firmware
    // This assumes exclusive access to the file system (i.e. USB not enabled at this time)
    HTMLCTX html;                 // HTML reader context
    uint8_t c;                    // Current character from HTML reader
    uint32_t i = 0;
    
    if (already_unique_id == 0) {
        init_auth_config();
        already_unique_id = 1;
    }

    // Write file
    init_html(&html, (uint8_t *)mbed_redirect_file);
    do {
        c = get_html_character(&html);
        if (c != '\0') {
            buf[i++] = c;
        }
    } while (c != '\0');
    memset(buf+i, ' ', bufsize - i);
}
