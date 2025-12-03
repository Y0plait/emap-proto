#ifndef EMAP_MESSAGE_H
#define EMAP_MESSAGE_H

#include <stdint.h>
#include "types.h"

/**
 * @file emap_message.h
 * @brief High-level container for decoded EMAP messages.
 */

/**
 * @brief Decoded EMAP message routed to the gameplay layer.
 */
typedef struct EMAP_Message {
    EMAP_MessageType type; /**< One of @ref EMAP_MessageType describing @ref data. */

    /**
     * @brief Variant view of the message payload.
     */
    union {
        EMAP_PayloadGeneralACK general_ack; /**< Payload for @ref EMAP_MSG_GENERAL_ACK. */
        EMAP_PayloadConnect connect;        /**< Payload for @ref EMAP_MSG_CONNECT. */
        EMAP_PayloadGameJoin game_join;     /**< Payload for @ref EMAP_MSG_GAME_JOIN. */
        EMAP_PayloadGameStart game_start;   /**< Payload for @ref EMAP_MSG_GAME_START. */
        EMAP_PayloadGameEnd game_end;       /**< Payload for @ref EMAP_MSG_GAME_END. */
        EMAP_PayloadGameWait game_wait;     /**< Payload for @ref EMAP_MSG_GAME_WAIT. */
        EMAP_PayloadGameState game_state;   /**< Payload for @ref EMAP_MSG_GAME_STATE. */
        EMAP_PayloadMove move;              /**< Payload for @ref EMAP_MSG_MOVE. */
        EMAP_PayloadPlay play;              /**< Payload for @ref EMAP_MSG_PLAY. */
        EMAP_PayloadPlayerInfo player_info; /**< Payload for @ref EMAP_MSG_PLAYER_INFO. */
        EMAP_PayloadShopAction shop_action; /**< Payload for @ref EMAP_MSG_SHOP_ACTION. */
        EMAP_PayloadPlayerDataUpdate player_update; /**< Payload for @ref EMAP_MSG_PLAYER_DATA_UPDATE. */
    } data;

} EMAP_Message;

#endif /* EMAP_MESSAGE_H */
