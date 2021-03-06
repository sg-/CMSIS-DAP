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
 
#include "virtual_fs.h"
#include "string.h"
#include "version.h"
#include "version_git.h"

// mbr is in RAM so the members can be updated at runtime to change drive capacity based
//  on target MCU that is attached
mbr_t mbr = {
    /*uint8_t[11]*/.boot_sector = {
        0xEB,0x3C, 0x90,
        'M','S','D','0','S','4','.','1' // OEM Name in text (8 chars max)
    },
    /*uint16_t*/.bytes_per_sector           = 0x0200,       // 512 bytes per sector
    /*uint8_t */.sectors_per_cluster        = 0x08,         // 4k cluser
    /*uint16_t*/.reserved_logical_sectors   = 0x0001,       // mbr is 1 sector
    /*uint8_t */.num_fats                   = 0x02,         // 2 FATs
    /*uint16_t*/.max_root_dir_entries       = 0x0020,       // 16 dir entries (max)
    /*uint16_t*/.total_logical_sectors      = 0x1f50,       // sector size * # of sectors = drive size
    /*uint8_t */.media_descriptor           = 0xf8,         // fixed disc = F8, removable = F0
    /*uint16_t*/.logical_sectors_per_fat    = 0x0001,       // FAT is 1k - ToDO:need to edit this
    /*uint16_t*/.physical_sectors_per_track = 0x0001,       // flat
    /*uint16_t*/.heads                      = 0x0001,       // flat
    /*uint32_t*/.hidden_sectors             = 0x00000000,   // before mbt, 0
    /*uint32_t*/.big_sectors_on_drive       = 0x00000000,   // 4k sector. not using large clusters
    /*uint8_t */.physical_drive_number      = 0x00,
    /*uint8_t */.not_used                   = 0x00,         // Current head. Linux tries to set this to 0x1
    /*uint8_t */.boot_record_signature      = 0x29,         // signature is present
    /*uint32_t*/.volume_id                  = 0x27021974,   // serial number
    // needs to match the root dir label
    /*char[11]*/.volume_label               = {'M','B','E','D',' ',' ',' ',' ','D','N','D'},
    // unused by msft - just a label (FAT, FAT12, FAT16)
    /*char[8] */.file_system_type           = {'F','A','T','1','2',' ',' ',' '},

    /* Executable boot code that starts the operating system */
    /*uint8_t[448]*/.bootstrap = {
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    },
    /*uint16_t*/.signature = 0xAA55,
};

// FAT is only valid for files on disc that are part of this file. Not writeable during
//  MSC transfer operations
static const file_allocation_table_t fat = {
    .f = {
        0xF8, 0xFF, 
        0xFF, 0xFF,
        0xFF, 0xFF,
        0xFF, 0xFF,
        0xFF, 0xFF,
        0xFF, 0xFF,
        0x00, 0x00,
    }
};

// Tool for release awareness and tracking
#if (GIT_LOCAL_MODS == 1)
  #warning "Building with local modifications."
  #define GIT_LOCAL_MODS_STR "Yes"
#else
  #define GIT_LOCAL_MODS_STR "No"
#endif

#define URL "<meta http-equiv=\"refresh\" content=\"0; url=http://mbed.org/device/?code=@A\"/>\r\n"
#define URL_NAME "MBED    HTM"
//#define URL "<meta http-equiv=\"refresh\" content=\"0; url=http://mbed.org/partnerdevice/ibmethernet/@M\"/>\r\n"
//#define URL_NAME "IBM     HTM"

const uint8_t mbed_redirect_file[512] =
    "<!-- mbed Platform Website and Authentication Shortcut -->\r\n"
    "<html>\r\n"
    "<head>\r\n"
    URL
    "<title>mbed Website Shortcut</title>\r\n"
    "</head>\r\n"
    "<body></body>\r\n"
    "</html>\r\n"
    "\r\n";

static const uint8_t details_file[512] =
    "Version: " FW_BUILD "\r\n"
    "Build:   " __DATE__ " " __TIME__ "\r\n"
    "Git Commit SHA: " GIT_COMMIT_SHA "\r\n"
    "Git Local mods: " GIT_LOCAL_MODS_STR "\r\n";

static const uint8_t fail_file[512] =
    "Placeholder for fail.txt data\r\n";

static FatDirectoryEntry_t const fail_txt_dir_entry = {
/*uint8_t[11] */ .filename = "FAIL    TXT",
/*uint8_t */ .attributes = 0x01,
/*uint8_t */ .reserved = 0x00,
/*uint8_t */ .creation_time_ms = 0x00,
/*uint16_t*/ .creation_time = 0x0000,
/*uint16_t*/ .creation_date = 0x0021,
/*uint16_t*/ .accessed_date = 0xbb32,
/*uint16_t*/ .first_cluster_high_16 = 0x0000,
/*uint16_t*/ .modification_time = 0x83dc,
/*uint16_t*/ .modification_date = 0x34bb,
/*uint16_t*/ .first_cluster_low_16 = 0x0004,    // always must be last sector
/*uint32_t*/ .filesize = sizeof(fail_file)
};

static FatDirectoryEntry_t const empty_dir_entry = {
/*uint8_t[11] */ .filename = {0},
/*uint8_t */ .attributes = 0x00,
/*uint8_t */ .reserved = 0x00,
/*uint8_t */ .creation_time_ms = 0x00,
/*uint16_t*/ .creation_time = 0x0000,
/*uint16_t*/ .creation_date = 0x0000,
/*uint16_t*/ .accessed_date = 0x0000,
/*uint16_t*/ .first_cluster_high_16 = 0x0000,
/*uint16_t*/ .modification_time = 0x0000,
/*uint16_t*/ .modification_date = 0x0000,
/*uint16_t*/ .first_cluster_low_16 = 0x0000,
/*uint32_t*/ .filesize = 0x0000
};


// Root directory in RAM to allow run-time modifications to contents. All files on disc must 
//  be accounted for in dir1
static root_dir_t dir1 = {
    .dir = {
    /*uint8_t[11] */ .filename = "MBED       ",
    /*uint8_t */ .attributes = 0x28,
    /*uint8_t */ .reserved = 0x00,
    /*uint8_t */ .creation_time_ms = 0x00,
    /*uint16_t*/ .creation_time = 0x0000,
    /*uint16_t*/ .creation_date = 0x0000,
    /*uint16_t*/ .accessed_date = 0x0000,
    /*uint16_t*/ .first_cluster_high_16 = 0x0000,
    /*uint16_t*/ .modification_time = 0x8E41,
    /*uint16_t*/ .modification_date = 0x32bb,
    /*uint16_t*/ .first_cluster_low_16 = 0x0000,
    /*uint32_t*/ .filesize = 0x00000000
    },
    .f1  = {
    /*uint8_t[11] */ .filename = URL_NAME,
    /*uint8_t */ .attributes = 0x01,
    /*uint8_t */ .reserved = 0x00,
    /*uint8_t */ .creation_time_ms = 0x00,
    /*uint16_t*/ .creation_time = 0x0000,
    /*uint16_t*/ .creation_date = 0x0021,
    /*uint16_t*/ .accessed_date = 0xbb32,
    /*uint16_t*/ .first_cluster_high_16 = 0x0000,
    /*uint16_t*/ .modification_time = 0x83dc,
    /*uint16_t*/ .modification_date = 0x34bb,
    /*uint16_t*/ .first_cluster_low_16 = 0x0002,
    /*uint32_t*/ .filesize = sizeof(mbed_redirect_file)
    },
    .f2 = {
    /*uint8_t[11] */ .filename = "DETAILS TXT",
    /*uint8_t */ .attributes = 0x01,
    /*uint8_t */ .reserved = 0x00,
    /*uint8_t */ .creation_time_ms = 0x00,
    /*uint16_t*/ .creation_time = 0x0000,
    /*uint16_t*/ .creation_date = 0x0021,
    /*uint16_t*/ .accessed_date = 0xbb32,
    /*uint16_t*/ .first_cluster_high_16 = 0x0000,
    /*uint16_t*/ .modification_time = 0x83dc,
    /*uint16_t*/ .modification_date = 0x30bb,
    /*uint16_t*/ .first_cluster_low_16 = 0x0003,
    /*uint32_t*/ .filesize = sizeof(details_file)
    },
    .f3  = {0},
    .f4  = {0},
    .f5  = {0},    
    .f6  = {0},
    .f7  = {0},
    .f8  = {0},
    .f9  = {0},
    .f10 = {0},
    .f11 = {0},
    .f12 = {0},
    .f13 = {0},
    .f14 = {0},
    .f15 = {0}
};

// dir2 is for hidden file and folder compatibility during OS writes (extended entry compatibility)
static const root_dir_t dir2 = {
    .dir = {0},
    .f1  = {0},
    .f2  = {0},
    .f3  = {0},
    .f4  = {0},
    .f5  = {0},    
    .f6  = {0},
    .f7  = {0},
    .f8  = {0},
    .f9  = {0},
    .f10 = {0},
    .f11 = {0},
    .f12 = {0},
    .f13 = {0},
    .f14 = {0},
    .f15 = {0}
};

// dummy blank reigon used to padd the space between files
static const uint8_t blank_reigon[512] = {0};

// this is the composite file system. It's looked at by ready operations to determine what to send
//  when the host OS requests data
virtual_media_t fs[] = {
    // fs setup
    {(uint8_t *)&mbr, sizeof(mbr)},
    {(uint8_t *)&fat, sizeof(fat)},
    {(uint8_t *)&fat, sizeof(fat)},
    
    // root dir
    {(uint8_t *)&dir1, sizeof(dir1)},
    {(uint8_t *)&dir2, sizeof(dir2)},
    
    //start of file contents
    {(uint8_t *)&mbed_redirect_file, sizeof(mbed_redirect_file)},    
    //empty area between every file (8*512 is start of data reigion need to pad between files - 1
    {(uint8_t *)&blank_reigon, sizeof(blank_reigon)},
    {(uint8_t *)&details_file, sizeof(details_file)},
    {(uint8_t *)&blank_reigon, sizeof(blank_reigon)},
    {(uint8_t *)&fail_file,    sizeof(fail_file)},
    
    // add other meaningful file data entries here
    
    // end of fs data
    {(uint8_t *)0, 0},
};

// when a fail condition occurs we need to update the data stored on disc and also
//  the directory entry. fs[] entry and dir entry need to be looked at and may need
//  modification if/when more files are added to the file-system
void configure_fail_txt(target_flash_status_t reason)
{
    // set the dir entry to a file or empty it
    dir1.f3 = (TARGET_OK == reason) ? empty_dir_entry : fail_txt_dir_entry;
    // update the filesize (pass/fail)
    dir1.f3.filesize = strlen((const char *)fail_txt_contents[reason]);
    // and the memory that we point to (file contents)
    fs[9].sect = (uint8_t *)fail_txt_contents[reason];
}

// Update known entries and mbr data when the program boots
void virtual_fs_init(void)
{
    // 64KB is mbr, FATs, root dir, ect...
    //uint32_t wanted_size_in_bytes   = (target_device.disc_size + KB(64);
    //uint32_t number_sectors_needed  = (wanted_size_in_bytes / mbr.bytes_per_sector);
    //uint32_t number_clusters_needed = (number_sectors_needed / mbr.sectors_per_cluster);
    //uint32_t fat_sector_size =        (((number_clusters_needed / 1023) / 1024) * 3);
    // number of sectors = (media size in bytes) / bytes per sector
    mbr.total_logical_sectors = ((target_device.disc_size + kB(64)) / mbr.bytes_per_sector);
    // number of cluster = ((number of sectors) / sectors per cluster)
    // secotrs per fat   = (3 x ((number of clusters + 1023) / 1024))
    mbr.logical_sectors_per_fat = (3 * (((mbr.total_logical_sectors / mbr.sectors_per_cluster) + 1023) / 1024));
    // patch root direcotry entries
    //dir1.f1.filesize = strlen((const char *)mbed_redirect_file);
    dir1.f2.filesize = strlen((const char *)details_file);
    // patch fs entries (fat sizes and all blank regions)
    fs[1].length = sizeof(fat) * mbr.logical_sectors_per_fat;
    fs[2].length = fs[1].length;
    fs[6].length = sizeof(blank_reigon)*(mbr.sectors_per_cluster - 1);
    fs[8].length = sizeof(blank_reigon)*(mbr.sectors_per_cluster - 1);
}

file_transfer_state_t file_transfer_state;

void reset_file_transfer_state(void)
{
    static const file_transfer_state_t default_transfer_state = {0,0,0,0,0,0,UNKNOWN};
    file_transfer_state = default_transfer_state;
}
