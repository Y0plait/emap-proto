#ifndef EMAP_LOW_LEVEL_H
#define EMAP_LOW_LEVEL_H

#include "../include/types.h"
#include <sys/types.h>

ssize_t emap_send_all();
uint8_t emap_receive_packet();


#endif
