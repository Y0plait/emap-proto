/* emap_message.h */

#ifndef EMAP_MESSAGE_H
#define EMAP_MESSAGE_H

#include <stdint.h>
#include "types.h"  /* for EMAP_MessageType, EMAP_Payload* etc. */

/* High-level EMAP message, used by the game logic */
typedef struct EMAP_Message {
    EMAP_MessageType type;  /* One of EMAP_MSG_* */

    union {
        EMAP_PayloadGeneralACK   general_ack;   /* EMAP_MSG_GENERAL_ACK */
        EMAP_PayloadConnect      connect;       /* EMAP_MSG_CONNECT */
        EMAP_PayloadGameJoin     game_join;     /* EMAP_MSG_GAME_JOIN */
        EMAP_PayloadGameStart    game_start;    /* EMAP_MSG_GAME_START */
        EMAP_PayloadGameEnd      game_end;      /* EMAP_MSG_GAME_END */
        EMAP_PayloadGameWait     game_wait;     /* EMAP_MSG_GAME_WAIT */
        EMAP_PayloadGameState    game_state;    /* EMAP_MSG_GAME_STATE */
        EMAP_PayloadMove         move;          /* EMAP_MSG_MOVE */
        EMAP_PayloadPlay         play;          /* EMAP_MSG_PLAY */
        EMAP_PayloadPlayerInfo   player_info;   /* EMAP_MSG_PLAYER_INFO */
    } data;

} EMAP_Message;

#endif /* EMAP_MESSAGE_H */
