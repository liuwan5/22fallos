#ifndef READ_BOOT_H
#define READ_BOOT_H
#include <cstdint>
#include <string>
#include <sys/types.h>

struct boot_msg{
    uint16_t bytes_per_sector;
    uint8_t sectors_per_clustor;
    uint16_t boot_record_sectors;
    uint16_t directories_entries_number;
    uint16_t sectors_per_FAT;
};

boot_msg* read_boot(std::string image_name);

#endif