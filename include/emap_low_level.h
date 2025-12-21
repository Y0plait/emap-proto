#ifndef EMAP_LOW_LEVEL_H
#define EMAP_LOW_LEVEL_H

/**
 * @file emap_low_level.h
 * @brief Blocking send/receive primitives used by the EMAP transport layer.
 */

#include <sys/socket.h>
#include <sys/types.h>

/**
 * @brief Send the entirety of a buffer over a stream socket.
 *
 * The routine retries on @c EINTR and only returns once @p len bytes have been
 * written, an error occurs, or the remote endpoint closes the connection.
 *
 * @param sockfd Connected stream socket descriptor.
 * @param buf Buffer containing the bytes to transmit.
 * @param len Number of bytes that must be transmitted before returning.
 * @return Total number of bytes written on success, 0 if the peer closed the
 *         connection, or -1 on error (errno is preserved).
 */
ssize_t emap_send_all(int sockfd, const void* buf, size_t len);

/**
 * @brief Receive an exact number of bytes from a stream socket.
 *
 * The function blocks until @p buf_len bytes have been read, the remote peer
 * shuts down the connection, or an error occurs. It retries on @c EINTR.
 *
 * @param sock Connected stream socket descriptor.
 * @param buf Destination buffer where the incoming payload is stored.
 * @param buf_len Exact number of bytes to read before returning.
 * @return Total number of bytes read on success, 0 if the peer closed the
 *         connection, or -1 on error (errno is preserved).
 */
ssize_t emap_recv_all(int sock, void* buf, size_t buf_len);

#endif
