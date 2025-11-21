#ifndef EMAP_CONSTANTS_H
#define EMAP_CONSTANTS_H

/**
 * @file constants.h
 * @brief Compile-time constants shared by the EMAP protocol implementation.
 */

/**
 * @brief Name of the transport protocol used with @c getprotobyname().
 */
#define EMAP_BASE_TRANSPORT_PROTOCOL "TCP"

/**
 * @brief Maximum payload size accepted by the protocol (bytes).
 */
#define EMAP_MAX_PAYLOAD_SIZE 1024

/**
 * @brief Number of cosmetic skins available to a player profile.
 */
#define TOTAL_SKIN_AMOUNT 6

/**
 * @brief Maximum number of characters stored for a username on the wire.
 */
#define MAX_PLAYER_USERNAME_SIZE 16

/**
 * @brief First synchronization byte that prefixes all EMAP packets.
 */
#define EMAP_MAGIC_BYTE_0 'E'

/**
 * @brief Second synchronization byte that prefixes all EMAP packets.
 */
#define EMAP_MAGIC_BYTE_1 'M'

/**
 * @brief Current protocol version encoded in EMAP packet headers.
 */
#define EMAP_PROTO_VERSION 1

#endif