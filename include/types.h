#ifndef EMAP_TYPES_H
#define EMAP_TYPES_H

#include <stdint.h>

#define EMAP_BASE_TRANSPORT_PROTOCOL "TCP" /* To be used with getprotobyname() */

#define EMAP_MAGIC_BYTE_0 'E'
#define EMAP_MAGIC_BYTE_1 'M'
#define EMAP_PROTO_VERSION 1

/* EMAP Play Types */
typedef enum EMAP_PlayType {
    EMAP_PLAY_NOT_YOUR_TURN = 0,
    EMAP_PLAY_YOUR_TURN = 1
} EMAP_PlayType;

/* EMAP Message Types */
typedef enum EMAP_MessageType {
    EMAP_MSG_GENERAL_ACK = 0x01,
    EMAP_MSG_CONNECT = 0x02,
    EMAP_MSG_GAME_JOIN = 0x03,
    EMAP_MSG_GAME_START = 0x04,
    EMAP_MSG_GAME_END = 0x05,
    EMAP_MSG_GAME_WAIT = 0x06,
    EMAP_MSG_GAME_STATE = 0x07,
    EMAP_MSG_MOVE = 0x08,
    EMAP_MSG_PLAY = 0x09,
    EMAP_MSG_ILLEGAL_PLAY = 0x0A,
    EMAP_MSG_PLAYER_INFO = 0x0B
} EMAP_MessageType;

/* EMAP Packet
 *
 * Note on endianess: 
 *  - Big-endian on the network, htons to encode over the network
 *  - Little-endian on the systemn, ntohs to decode over the network
 * 
 */

/* Only pack the packet structs as they should be fixed size, not compiler/memory dependant */
#pragma pack(push, 1)

typedef struct EMAP_PacketHeader {
    uint8_t magic_byte[2]; /* Used for packet synchronization, EMAP_MAGIC_0 (E) or EMAP_MAGIC_1 (M) */
    uint8_t version; /* EMAP_PROTO_VERSION */
    uint8_t code; /* Use EMAP_MessageType, just hardcoding the size to be sure */
    uint16_t size; /* Packet payload size */
} EMAP_PacketHeader;

typedef struct EMAP_Packet {
    EMAP_PacketHeader packet_header;
    uint8_t payload;
} EMAP_Packet;

/* Payload Types */
/* EMAP Payload for MOVE message */
typedef struct EMAP_PayloadMove {
    uint8_t coin_index; /* Which coin are we moving ? */
    uint8_t coin_pos; /* Where do we move the coin ? */
} EMAP_PayloadMove;

/* EMAP Payload for PLAY message */
typedef struct EMAP_PayloadPlay {
    EMAP_PlayType play_type; /* Your turn or not (1/0) */
} EMAP_PayloadPlay;

/* EMAP Payload for PLAYER_INFO message */
typedef struct EMAP_PayloadPlayerInfo {
    uint8_t level; /* Level range: 0-255 */
    uint16_t progression; /* State of advancement in a level, max 1000 */
    uint16_t money; /* Money owned, max 2^16-1 */
} EMAP_PayloadPlayerInfo;

#pragma pack(pop)

#endif
