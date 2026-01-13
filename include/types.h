#ifndef EMAP_TYPES_H
#define EMAP_TYPES_H

/**
 * @file types.h
 * @brief Canonical EMAP protocol types shared across the stack.
 */

#include <stdint.h>

#include "constants.h"

/**
 * @brief Player turn state used inside @ref EMAP_PayloadGameStart and @ref
 * EMAP_PayloadPlay.
 */
typedef enum {
	EMAP_PLAY_NOT_YOUR_TURN = 0, /**< Another participant is expected to act. */
	EMAP_PLAY_YOUR_TURN = 1		 /**< The local player must act now. */
} EMAP_PlayType;

/**
 * @brief Result of a finished game session.
 */
typedef enum {
	EMAP_END_GAME_LOSE = 0, /**< The local player lost. */
	EMAP_END_GAME_WIN = 1	/**< The local player won. */
} EMAP_EndGameType;

/**
 * @brief Generic acknowledgement payload values shared by most request/response
 * pairs.
 */
typedef enum {
	EMAP_MESSAGE_ACK_NOK = 0, /**< Command rejected or failed. */
	EMAP_MESSAGE_ACK_OK = 1	  /**< Command accepted or succeeded. */
} EMAP_GeneralACKTypes;

/**
 * @brief On-the-wire message identifiers.
 */
typedef enum {
	EMAP_MSG_GENERAL_ACK = 0x01, /**< Generic acknowledgement. */
	EMAP_MSG_CONNECT = 0x02, /**< Initial client handshake carrying identity. */
	EMAP_MSG_GAME_JOIN = 0x03,	/**< Join matchmaking queue or lobby. */
	EMAP_MSG_GAME_START = 0x04, /**< Game start notification. */
	EMAP_MSG_GAME_END = 0x05,	/**< Game end notification. */
	EMAP_MSG_GAME_WAIT = 0x06,	/**< Waiting state prior to match start. */
	EMAP_MSG_GAME_STATE = 0x07, /**< Serialized snapshot of the match state. */
	EMAP_MSG_MOVE = 0x08,		/**< Player move command. */
	EMAP_MSG_PLAY = 0x09,		/**< Turn notification. */
	EMAP_MSG_ILLEGAL_PLAY = 0x0A,	   /**< Invalid move notification. */
	EMAP_MSG_PLAYER_INFO = 0x0B,	   /**< Player profile synchronization. */
	EMAP_MSG_SHOP_ACTION = 0x0C,	   /**< Shop purchase/equip action. */
	EMAP_MSG_PLAYER_DATA_UPDATE = 0x0D /**< Incremental player data refresh. */
} EMAP_MessageType;

/**
 * @brief Shop action semantic encoded in @ref EMAP_PayloadShopAction.
 */
typedef enum {
	EMAP_SHOP_ACTION_PURCHASE = 0, /**< Client wants to purchase a skin. */
	EMAP_SHOP_ACTION_EQUIP =
		1 /**< Client wants to equip a skin already owned. */
} EMAP_ShopActionType;

/**
 * @brief Packet header prepended to every EMAP frame.
 *
 * Values are encoded in network byte order on the wire and decoded using the
 * helper routines in @ref emap_high_level.c.
 */
#pragma pack(push, 1)
typedef struct {
	uint8_t magic_byte[2]; /**< Synchronization bytes: @ref EMAP_MAGIC_BYTE_0,
							  @ref EMAP_MAGIC_BYTE_1. */
	uint8_t version; /**< Protocol version, see @ref EMAP_PROTO_VERSION. */
	uint8_t code;	 /**< Message code, one of @ref EMAP_MessageType. */
	uint16_t size;	 /**< Payload size in bytes (network byte order). */
} EMAP_PacketHeader;

/**
 * @brief Generic one-byte payload used for tests and very small messages.
 */
typedef struct {
	uint8_t payload; /**< Single-byte payload. */
} EMAP_GeneralPacket;

/**
 * @brief Payload for @ref EMAP_MSG_GENERAL_ACK.
 */
typedef struct {
	uint8_t ack_type; /**< Value from @ref EMAP_GeneralACKTypes. */
} EMAP_PayloadGeneralACK;

/**
 * @brief Payload for @ref EMAP_MSG_CONNECT.
 */
typedef struct {
	uint8_t
		username[MAX_PLAYER_USERNAME_SIZE]; /**< Raw username (not
											   null-terminated on the wire). */
} EMAP_PayloadConnect;

/**
 * @brief Payload for @ref EMAP_MSG_GAME_JOIN.
 */
typedef struct {
	/* Intentionally empty; keep payload-less semantic. */
} EMAP_PayloadGameJoin;

/**
 * @brief Payload for @ref EMAP_MSG_GAME_START.
 */
typedef struct {
	uint8_t is_starting; /**< Value from @ref EMAP_PlayType indicating turn
							order. */
} EMAP_PayloadGameStart;

/**
 * @brief Payload for @ref EMAP_MSG_GAME_END.
 */
typedef struct {
	uint8_t has_won; /**< Value from @ref EMAP_EndGameType describing the
						outcome. */
} EMAP_PayloadGameEnd;

/**
 * @brief Payload for @ref EMAP_MSG_GAME_WAIT.
 */
typedef struct {
	/* Intentionally empty; keep payload-less semantic. */
} EMAP_PayloadGameWait;

/**
 * @brief Payload for @ref EMAP_MSG_GAME_STATE.
 */
typedef struct {
	/* Intentionally empty; future game state fields will extend this struct. */
} EMAP_PayloadGameState;

/**
 * @brief Payload for @ref EMAP_MSG_MOVE.
 */
typedef struct {
	uint8_t coin_index; /**< Selected coin index. */
	uint8_t coin_pos;	/**< Target board position. */
} EMAP_PayloadMove;

/**
 * @brief Payload for @ref EMAP_MSG_PLAY.
 */
typedef struct {
	uint8_t play_type; /**< Value from @ref EMAP_PlayType indicating turn
						  ownership. */
} EMAP_PayloadPlay;

/**
 * @brief Payload for @ref EMAP_MSG_PLAYER_INFO.
 */
typedef struct {
	uint8_t level;		   /**< Player level [0,255]. */
	uint16_t progression;  /**< Progress toward next level (network order). */
	uint16_t money;		   /**< Currency owned (network order). */
	uint8_t selected_skin; /**< Currently equipped skin id. */
	uint8_t possessed_skins[TOTAL_SKIN_AMOUNT]; /**< Owned skin flags. */
} EMAP_PayloadPlayerInfo;

/**
 * @brief Payload for @ref EMAP_MSG_SHOP_ACTION.
 */
typedef struct {
	uint8_t action;		/**< Value from @ref EMAP_ShopActionType. */
	uint8_t skin_index; /**< Target skin identifier. */
} EMAP_PayloadShopAction;

/**
 * @brief Payload for @ref EMAP_MSG_PLAYER_DATA_UPDATE.
 *
 * Reuses the same layout as @ref EMAP_PayloadPlayerInfo so that both
 * initial sync and subsequent updates share a common wire format.
 */
typedef EMAP_PayloadPlayerInfo EMAP_PayloadPlayerDataUpdate;

/**
 * @brief Payload for @ref EMAP_MSG_ILLEGAL_PLAY
 */
typedef struct {
	/* Intentionally empty; */
} EMAP_PayloadIllegalPlay;

#pragma pack(pop)

#endif
