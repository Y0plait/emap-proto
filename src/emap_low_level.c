#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>

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
 * @return Return an integer associated to the state of the send process
 * @retval >0    Success, all data was sent, return is size of data transmitted
 * @retval <0   Error, not all data was sent
 *
 * @note  [Any additional notes, performance characteristics, or pitfalls]
 * @warning [Any important warnings about misuse or limitations]
 *
 *
 * @see https://beej.us/guide/bgnet/html/split/slightly-advanced-techniques.html
 */
ssize_t send_all(int sock, const void *buf, size_t buf_len) {
    size_t total = 0;
    const uint8_t *ptr = buf;

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
            /* socket fermé proprement */
            return 0;
        }
        total += (size_t)n;
    }
    return (ssize_t)total;
}