#ifndef EMAP_TYPES_H
#define EMAP_TYPES_H

#include <stdint.h>

#include "../include/constants.h"

/* EMAP Play Types */
typedef enum EMAP_PlayType {
    EMAP_PLAY_NOT_YOUR_TURN = 0,
    EMAP_PLAY_YOUR_TURN = 1
} EMAP_PlayType;

/* EMAP End Game Types */
typedef enum EMAP_EndGameType {
    EMAP_END_GAME_LOSE = 0,
    EMAP_END_GAME_WIN = 1
} EMAP_EndGameType;

/* EMAP General ACK Types */
typedef enum EMAP_GeneralACKTypes {
    EMAP_MESSAGE_ACK_NOK = 0,
    EMAP_MESSAGE_ACK_OK = 1
} EMAP_GeneralACKTypes;

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
    uint8_t code; /* Use EMAP_MessageType, just hardcoding the type to be sure */
    uint16_t size; /* Packet payload size */
} EMAP_PacketHeader;

/* EMAP Payload Types */
/* EMAP Packet for custom message */
typedef struct EMAP_GeneralPacket {
    uint8_t payload;
} EMAP_GeneralPacket;

/* EMAP Payload for GENERAL_ACK message */
typedef struct  {
    uint8_t ack_type; /* EMAP_GeneralACKTypes (1: good, 0: not good)*/
} EMAP_PayloadGeneralACK;

/* EMAP Payload for CONNECT message */
typedef struct EMAP_PayloadConnect {
    uint8_t username[MAX_PLAYER_USERNAME_SIZE]; 
} EMAP_PayloadConnect;

/* EMAP Payload for GAME_JOIN message */
typedef struct EMAP_PayloadGameJoin {
    /* No payload needed */
} EMAP_PayloadGameJoin;

/* EMAP Payload for GAME_STATE message */
typedef struct EMAP_PayloadGameStart {
    uint8_t is_starting; /* EMAP_PlayType */
} EMAP_PayloadGameStart;

/* EMAP Payload for GAME_END message */
typedef struct EMAP_PayloadGameEnd {
    uint8_t has_won; /* EMAP_GameEndType */
} EMAP_PayloadGameEnd;

/* EMAP Payload for GAME_WAIT message */
typedef struct EMAP_PayloadGameWait {
    /* No payload needed */
} EMAP_PayloadGameWait;

/* EMAP Payload for GAME_STATE message */
typedef struct EMAP_PayloadGameState {
    /* No payload needed */
} EMAP_PayloadGameState;

/* EMAP Payload for MOVE message */
typedef struct EMAP_PayloadMove {
    uint8_t coin_index; /* Which coin are we moving ? */
    uint8_t coin_pos; /* Where do we move the coin ? */
} EMAP_PayloadMove;

/* EMAP Payload for PLAY message */
typedef struct EMAP_PayloadPlay {
    uint8_t play_type; /* Your turn or not (1/0) */
} EMAP_PayloadPlay;

/* EMAP Payload for PLAYER_INFO message */
typedef struct EMAP_PayloadPlayerInfo {
    uint8_t level; /* Level range: 0-255 */
    uint16_t progression; /* State of advancement in a level, max 1000 */
    uint16_t money; /* Money owned, max 2^16-1 */
    uint8_t possessed_skins[TOTAL_SKIN_AMOUNT]; /* Skin are in an array (of size TOTAL_SKIN_AMOUNT)*/
} EMAP_PayloadPlayerInfo;

#pragma pack(pop)

#endif
