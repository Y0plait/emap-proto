#ifndef EMAP_LOW_LEVEL_H
#define EMAP_LOW_LEVEL_H

#include "../include/types.h"
#include <sys/types.h>
#include <sys/socket.h>

ssize_t emap_send_all(int socket, char *buf, int *len);
uint8_t emap_receive_packet();


#endif
