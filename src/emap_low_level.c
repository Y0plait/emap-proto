#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdint.h>

#include "../include/emap_low_level.h"

/**
 * @brief Sends all elements of a buffer
 *
 * Sends all the elements of a buffer through a socket (TCP) even if 
 * the transport layer fragments the message in multiple packets.
 *
 * @param[in]  socket  A system network socket (AF_INET, SOCK_STREAM preferably)
 * @param[in] buf  pointer to a buffer
 * @param[in] len size of the buffer
 *
 * @return Return an integer associated to the state of the send process, or 
 * size of transmitted data
 * @retval >0   Success, all data was sent, return is size of data received
 * @retval <0   Error, not all data was sent
 *
 * @warning Used as base layer for all the messaging for EMAP, no message
 * type consideration here
 * 
 * @see https://beej.us/guide/bgnet/html/split/slightly-advanced-techniques.html
 */
ssize_t send_all(int sock, void *buf, size_t buf_len) {
    size_t total = 0;
    uint8_t *ptr = buf;

    while (total < buf_len) {
        ssize_t n = send(sock, ptr + total, buf_len - total, 0);
        if (n < 0) {
            if (errno == EINTR) {
                continue; /* interrup­tion, on réessaie */
            }
            return -1;    /* vraie erreur */
        }
        if (n == 0) {
            /* socket fermé */
            return 0;
        }
        total += (size_t)n;
    }
    return (ssize_t)total;
}

/**
 * @brief Receive elements of network socket
 *
 * Receives elements through a socket (TCP), stores them in a
 * buffer.
 *
 * @param[in] socket  A system network socket (AF_INET, SOCK_STREAM preferably)
 * @param[in] buf  pointer to a buffer, will contain the received data
 * @param[in] len size of the buffer, 0 initialy.
 *
 * @return Return an integer associated to the state of the send process
 * @retval >0    Success, all data was sent, return is size of data transmitted
 * @retval <0   Error, not all data was sent
 * 
 * @warning Used as base layer for all the messaging for EMAP, no message
 * type consideration here
 * 
 * @see 
 */
ssize_t emap_recv_all(int sock, void *buf, size_t buf_len) {
    size_t total = 0;
    uint8_t *ptr = buf;

    while (total < buf_len) {
        ssize_t n = recv(sock, ptr + total, buf_len - total, 0);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        if (n == 0) {
            /* Properly closed socket */
            return 0;
        }
        total += (size_t)n;
    }
    return (ssize_t)total;
}