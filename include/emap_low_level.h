#ifndef EMAP_LOW_LEVEL_H
#define EMAP_LOW_LEVEL_H

#include "../include/types.h"
#include <sys/types.h>
#include <sys/socket.h>

ssize_t send_all(int sockfd, const void *buf, size_t len);



#endif
