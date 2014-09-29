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

#include "vector_table.h"
#include "firmware_cfg.h"
#include "MK20D5.h"

void relocate_vector_table_ram(void)
{
    uint32_t *vectors;
    uint32_t i;
    // Copy and switch to dynamic vectors if the first time called
    if (SCB->VTOR != app.ram_start) {
        uint32_t *old_vectors = (uint32_t *)app.flash_start;
        vectors = (uint32_t *)app.ram_start;
        for (i = 0; i < (16+46); i++) {
            vectors[i] = old_vectors[i];
        }
        SCB->VTOR = app.ram_start;
    }
}

void relocate_vector_table_app(void)
{
    SCB->VTOR = app.flash_start;
}
