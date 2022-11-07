#include <cstdint>
#include <cstdlib>
#include<fstream>
#include <ios>
#include"read_boot.h"

boot_msg* read_boot(std::string image_name){
    boot_msg* msg = (boot_msg*)malloc(sizeof(boot_msg));
    std::ifstream image;
    char buf[32];
    image.open(image_name, std::ios::in);
    
    image.read(buf, 32);
    image.close();
    msg->bytes_per_sector = *(uint16_t*)(buf+11);
    msg->directories_entries_number = *(uint16_t*)(buf+17);
    return msg;
}
