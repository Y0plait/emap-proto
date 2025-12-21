/**
 * @file wrappers.h
 * @brief High-level helper functions to build and send EMAP protocol messages.
 *
 * These wrappers provide convenient routines to serialize and transmit EMAP
 * messages over a connected file descriptor (socket or pipe). They hide the
 * details of message framing and payload encoding used by the EMAP high-level
 * layer.
 *
 * General behavior:
 * - All functions send a single EMAP message corresponding to their purpose.
 * - On success the function returns a non-negative value (typically 0). On
 *   failure they return a negative value.
 * - Caller is responsible for providing a valid, connected file descriptor.
 * - Constants referenced in parameters (e.g. EMAP_MESSAGE_ACK_OK,
 *   EMAP_PLAY_YOUR_TURN, EMAP_END_GAME_WIN) are defined in emap_high_level.h.
 */

#pragma once
#include "emap_high_level.h"
#include <stdint.h>

/**
 * @brief Send an ACK/NACK message.
 *
 * @param fd    File descriptor to send the message on (e.g. socket).
 * @param ok    Acknowledgement value: EMAP_MESSAGE_ACK_OK or
 * EMAP_MESSAGE_ACK_NOK.
 *
 * @return Non-negative on success, negative on error.
 */
int emap_helper_send_ack(
	int fd, uint8_t ok); // ok = EMAP_MESSAGE_ACK_OK / EMAP_MESSAGE_ACK_NOK

/**
 * @brief Send a connect request with a username.
 *
 * This message is used to announce or register the client identity to the peer.
 *
 * @param fd        File descriptor to send the message on.
 * @param username  NULL-terminated username string. Must not be NULL.
 *
 * @return Non-negative on success, negative on error.
 */

int emap_helper_send_connect(int fd, const char* username);

/**
 * @brief Send player information.
 *
 * Sends player-related metadata such as level, progression, money and available
 * skins.
 *
 * @param fd            File descriptor to send the message on.
 * @param level         Player level.
 * @param progression   Player progression value.
 * @param money         Player money amount.
 * @param selected_skin Currently equipped skin index.
 * @param skins         Pointer to an array of skin identifiers. If skins_len is
 * 0, this pointer may be NULL.
 * @param skins_len     Number of bytes (or elements) in the skins array.
 *
 * @return Non-negative on success, negative on error.
 */
int emap_helper_send_player_info(int fd, uint8_t level, uint16_t progression,
								 uint16_t money, uint8_t selected_skin,
								 const uint8_t* skins, uint16_t skins_len);

/**
 * @brief Notify the peer that the client is joining a game.
 *
 * Sends a game-join request/message; typically used to indicate the client
 * wants to participate in matchmaking or join a lobby.
 *
 * @param fd    File descriptor to send the message on.
 *
 * @return Non-negative on success, negative on error.
 */
int emap_helper_send_game_join(int fd);

/**
 * @brief Notify the peer that the client is waiting for a game.
 *
 * Use this to indicate a waiting/lobby state.
 *
 * @param fd    File descriptor to send the message on.
 *
 * @return Non-negative on success, negative on error.
 */
int emap_helper_send_game_wait(int fd);

/**
 * @brief Send a game start notification.
 *
 * Informs the peer whether the game is starting and (optionally) which side or
 * turn state applies.
 *
 * @param fd           File descriptor to send the message on.
 * @param is_starting  EMAP_PLAY_YOUR_TURN if this client starts / prepares to
 * play, otherwise NOT_YOUR_TURN (or similar defined constant).
 *
 * @return Non-negative on success, negative on error.
 */
int emap_helper_send_game_start(
	int fd, uint8_t is_starting); // EMAP_PLAY_YOUR_TURN / NOT_YOUR_TURN

/**
 * @brief Inform the peer about play/turn state.
 *
 * Use to indicate whether it is the recipient's turn or the sender's turn.
 *
 * @param fd         File descriptor to send the message on.
 * @param your_turn  EMAP_PLAY_YOUR_TURN if it is the recipient's turn,
 * otherwise NOT_YOUR_TURN (or the appropriate constant).
 *
 * @return Non-negative on success, negative on error.
 */
int emap_helper_send_play(
	int fd, uint8_t your_turn); // EMAP_PLAY_YOUR_TURN / NOT_YOUR_TURN

/**
 * @brief Send an illegal-move or protocol-illegal notification.
 *
 * Indicates that an illegal action or protocol violation occurred.
 *
 * @param fd    File descriptor to send the message on.
 *
 * @return Non-negative on success, negative on error.
 */
int emap_helper_send_illegal(int fd);

/**
 * @brief Send a move action.
 *
 * Encodes and transmits a move from start_pos to new_pos.
 *
 * @param fd          File descriptor to send the message on.
 * @param start_pos   Starting position/index for the move.
 * @param new_pos     Destination position/index for the move.
 *
 * @return Non-negative on success, negative on error.
 */
int emap_helper_send_move(int fd, uint8_t start_pos, uint8_t new_pos);

/**
 * @brief Send current game state.
 *
 * Transmits an arbitrary game-state payload. If there is no state to send,
 * the caller may pass payload = NULL and len = 0.
 *
 * @param fd     File descriptor to send the message on.
 * @param payload Pointer to state data to send. May be NULL when len == 0.
 * @param len     Length in bytes of the payload.
 *
 * @return Non-negative on success, negative on error.
 */
int emap_helper_send_game_state(
	int fd, const void* payload,
	uint16_t len); // si état vide: payload=NULL,len=0

/**
 * @brief Send game end/result notification.
 *
 * Informs the peer of the game outcome (win/lose/tie etc.) using defined
 * EMAP end-game constants.
 *
 * @param fd      File descriptor to send the message on.
 * @param result  Result code (e.g. EMAP_END_GAME_WIN or EMAP_END_GAME_LOSE).
 *
 * @return Non-negative on success, negative on error.
 */
int emap_helper_send_game_end(int fd,
							  uint8_t result); // EMAP_END_GAME_WIN / LOSE

/**
 * @brief Send a shop action requested by the client.
 *
 * @param fd          File descriptor to send on.
 * @param action      Value from @ref EMAP_ShopActionType.
 * @param skin_index  Target skin.
 *
 * @return Non-negative on success, negative on error.
 */
int emap_helper_send_shop_action(int fd, uint8_t action, uint8_t skin_index);

/**
 * @brief Send a lightweight player data update.
 *
 * Broadcast refreshed player stats (level, progression, money and skins) when
 * only incremental changes occur.
 *
 * @param fd             File descriptor to send the message on.
 * @param level          Player level value.
 * @param progression    Player progression amount.
 * @param money          Player money balance.
 * @param selected_skin  Currently equipped skin identifier.
 * @param skins          Pointer to array describing owned skins; may be NULL if
 * len==0.
 * @param skins_len      Number of bytes available in skins array.
 *
 * @return Non-negative on success, negative on error.
 */
int emap_helper_send_player_data_update(int fd, uint8_t level,
										uint16_t progression, uint16_t money,
										uint8_t selected_skin,
										const uint8_t* skins,
										uint16_t skins_len);