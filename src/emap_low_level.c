#include <sys/types.h>
#include <sys/socket.h>

#include "../include/emap_low_level.h"

/**
 * @brief Sends all elements of a buffer
 *
 * Sends all the elements of a buffer through a socket (TCP) even if 
 * the transport layer fragments the message in multiple packets.
 *
 * @param[in]  socket  A system network socket (AF_INET, SOCK_STREAM preferably)
 * @param[in] buf  pointer 
 * @param[in] len [Description of parameter modified by the function]
 *
 * @return Return an integer associated to the state of the send process
 * @retval 0    Success, all data was sent
 * @retval <0   Error, not all data was sent
 *
 * @note  [Any additional notes, performance characteristics, or pitfalls]
 * @warning [Any important warnings about misuse or limitations]
 *
 *
 * @see https://beej.us/guide/bgnet/html/split/slightly-advanced-techniques.html
 */
int emap_send_all(int socket, char *buf, int *len) {
    int total_sent = 0; /* How many bytes we have sent */
    int left_bytes = *len; /* How many bytes are left to send */
    int counter; /* Counter for the amount of bytes sent */

    /*
     *
     * While we have not sent all the bytes of the buffer, don't stop sending.
     * As we have the message length in the EMAP header we can decode the message
     * even if it is distributed on multiple packets.
     */
    while (total_sent < *len) {
        counter = send(socket, buf+total_sent, left_bytes, 0);
        if (counter == -1) {
            break;
        }
        total_sent += counter;
        left_bytes -= counter;
    }

    *len = total_sent;

    return counter==-1 ? -1 : 0; /* return -1 on failure, 0 on success (ternary operators) */
}