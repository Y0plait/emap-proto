#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

#define MAX_PACKET_SIZE 200

/* EMAP Packet */

enum EMAP_Code {
    GENERAL_ACK = 0x01,
    CONNECT = 0x02,
    GAME_JOIN = 0x03,
    GAME_START = 0x04,
    GAME_END = 0x05,
    GAME_WAIT = 0x06,
    GAME_STATE = 0x07,
    MOVE = 0x08,
    PLAY = 0x09,
    ILLEGAL_PLAY = 0x10,
    PLAYER_INFO = 0x11
};

/* Only pack the packet structs as they should be fixed size, not compiler/memory dependant */
#pragma pack(push, 1)
typedef struct {
    enum EMAP_Code code;
    uint16_t size;
} EMAP_PacketHeader;

typedef struct {
    EMAP_PacketHeader packet_header;
    uint8_t payload[MAX_PACKET_SIZE];
} EMAP_Packet;

/* payload structs — pack if they will be serialized as-is */
typedef struct {
    int8_t positionCoinInitiale;
    int8_t positionCoinFinale;
} MOVE_Payload;

/* Avoid pointers in serialized structs; use fixed-size or length+data */
typedef struct {
    /* example: length followed by data (not a pointer) */
    uint16_t length;
    int8_t data[MAX_PACKET_SIZE];
} PLAY_Payload;
#pragma pack(pop)

#endif
