/**
 * @file wrappers.c
 * @brief Convenience wrapper functions for EMAP packet construction.
 *
 * Provides helper functions that simplify creation and sending of common
 * EMAP message types (ACK, CONNECT, MOVE, GAME_STATE, etc.). Each wrapper
 * constructs the appropriate payload structure and invokes emap_send_packet.
 */

#include "wrappers.h"
#include <arpa/inet.h>
#include <string.h>

/**
 * @brief Send a general acknowledgment message.
 *
 * Construct and send an EMAP general ACK packet containing a single
 * acknowledgment byte.
 *
 * @param fd   File descriptor (socket) to send the packet on.
 * @param ok   ACK value (e.g., EMAP_MESSAGE_ACK_OK or EMAP_MESSAGE_ACK_NOK).
 *
 * @return The value returned by emap_send_packet (propagates success/error).
 *
 * @note The function wraps the ACK value in an EMAP_PayloadGeneralACK and
 *       sends it using EMAP_MSG_GENERAL_ACK message type.
 */
int emap_helper_send_ack(int fd, uint8_t ok) {
	EMAP_PayloadGeneralACK p = {.ack_type = ok};
	return emap_send_packet(fd, EMAP_MSG_GENERAL_ACK, &p, 1);
}

/**
 * @brief Send a connect request with a username.
 *
 * Populate an EMAP_PayloadConnect structure with the provided username
 * (NUL-terminated/truncated to fit) and send an EMAP_MSG_CONNECT packet.
 *
 * @param fd        File descriptor (socket) to send the packet on.
 * @param username  Pointer to a NUL-terminated username string. If NULL,
 *                  an empty username is sent.
 *
 * @return The value returned by emap_send_packet (propagates success/error).
 *
 * @note The username is copied using strncpy and the payload is zeroed
 *       beforehand to ensure any unused bytes are NULs.
 */
int emap_helper_send_connect(int fd, const char* username) {
	EMAP_PayloadConnect p;
	memset(&p, 0, sizeof p);
	if (username) {
		strncpy((char*)p.username, username, sizeof p.username - 1);
	}
	return emap_send_packet(fd, EMAP_MSG_CONNECT, &p, sizeof p);
}

/**
 * @brief Send player information to the peer.
 *
 * Build and send an EMAP_MSG_PLAYER_INFO packet that contains level,
 * progression, money and a list of possessed skins.
 *
 * @param fd          File descriptor (socket) to send the packet on.
 * @param level       Player level (uint8_t).
 * @param progression Player progression (uint16_t). This value is converted
 *                    to network byte order (big-endian) before being sent.
 * @param money       Player money (uint16_t). This value is converted to
 *                    network byte order (big-endian) before being sent.
 * @param skins       Pointer to an array of skin identifiers (may be NULL).
 * @param skins_len   Number of bytes in the skins array; if greater than
 *                    TOTAL_SKIN_AMOUNT it will be truncated to
 *                    TOTAL_SKIN_AMOUNT.
 *
 * @return The value returned by emap_send_packet (propagates success/error).
 *
 * @note Progression and money are converted with htons before being copied
 *       into the payload to ensure network byte order.
 */
int emap_helper_send_player_info(int fd, uint8_t level, uint16_t progression,
								 uint16_t money, uint8_t selected_skin,
								 const uint8_t* skins, uint16_t skins_len) {
	EMAP_PayloadPlayerInfo pi;
	memset(&pi, 0, sizeof pi);
	pi.level = level;
	uint16_t prog_n = htons(progression);
	uint16_t money_n = htons(money);
	memcpy(&pi.progression, &prog_n, 2);
	memcpy(&pi.money, &money_n, 2);
	pi.selected_skin = selected_skin;
	if (skins && skins_len > 0) {
		if (skins_len > TOTAL_SKIN_AMOUNT)
			skins_len = TOTAL_SKIN_AMOUNT;
		memcpy(pi.possessed_skins, skins, skins_len);
	}
	return emap_send_packet(fd, EMAP_MSG_PLAYER_INFO, &pi, sizeof pi);
}

/**
 * @brief Send a game-join request.
 *
 * Send an EMAP_MSG_GAME_JOIN packet. The packet has no payload.
 *
 * @param fd  File descriptor (socket) to send the packet on.
 *
 * @return The value returned by emap_send_packet (propagates success/error).
 */
int emap_helper_send_game_join(int fd) {
	// Payload vide (si struct vide)
	return emap_send_packet(fd, EMAP_MSG_GAME_JOIN, NULL, 0);
}

/**
 * @brief Notify the peer that the sender is waiting for a game.
 *
 * Send an EMAP_MSG_GAME_WAIT packet. The packet has no payload.
 *
 * @param fd  File descriptor (socket) to send the packet on.
 *
 * @return The value returned by emap_send_packet (propagates success/error).
 */
int emap_helper_send_game_wait(int fd) {
	return emap_send_packet(fd, EMAP_MSG_GAME_WAIT, NULL, 0);
}

/**
 * @brief Notify peers that a game is starting or stopping.
 *
 * Send an EMAP_MSG_GAME_START packet containing a single byte indicating
 * whether the game is starting.
 *
 * @param fd           File descriptor (socket) to send the packet on.
 * @param is_starting  Non-zero if the game is starting, zero otherwise.
 *
 * @return The value returned by emap_send_packet (propagates success/error).
 */
int emap_helper_send_game_start(int fd, uint8_t is_starting) {
	EMAP_PayloadGameStart p = {.is_starting = is_starting};
	return emap_send_packet(fd, EMAP_MSG_GAME_START, &p, 1);
}

/**
 * @brief Inform the peer whether it is their turn to play.
 *
 * Send an EMAP_MSG_PLAY packet containing a single byte indicating the
 * play type / whether it is the recipient's turn.
 *
 * @param fd         File descriptor (socket) to send the packet on.
 * @param your_turn  Value indicating play type / turn ownership.
 *
 * @return The value returned by emap_send_packet (propagates success/error).
 */
int emap_helper_send_play(int fd, uint8_t your_turn) {
	EMAP_PayloadPlay p = {.play_type = your_turn};
	return emap_send_packet(fd, EMAP_MSG_PLAY, &p, 1);
}

/**
 * @brief Send an illegal-move notification.
 *
 * Send an EMAP_MSG_ILLEGAL_PLAY packet with a general NOK ACK payload to
 * indicate that the last attempted move was illegal.
 *
 * @param fd  File descriptor (socket) to send the packet on.
 *
 * @return The value returned by emap_send_packet (propagates success/error).
 */
int emap_helper_send_illegal(int fd) {
	EMAP_PayloadGeneralACK p = {.ack_type = EMAP_MESSAGE_ACK_NOK};
	return emap_send_packet(fd, EMAP_MSG_ILLEGAL_PLAY, &p, 1);
}

/**
 * @brief Send a move action describing a coin move.
 *
 * Build and send an EMAP_MSG_MOVE packet containing the index of the coin
 * moved and its new position.
 *
 * @param fd         File descriptor (socket) to send the packet on.
 * @param start_pos  Index of the coin being moved (uint8_t).
 * @param new_pos    New position of the coin (uint8_t).
 *
 * @return The value returned by emap_send_packet (propagates success/error).
 */
int emap_helper_send_move(int fd, uint8_t start_pos, uint8_t new_pos) {
	EMAP_PayloadMove m = {.coin_index = start_pos, .coin_pos = new_pos};
	return emap_send_packet(fd, EMAP_MSG_MOVE, &m, sizeof m);
}

/**
 * @brief Send an arbitrary game state payload.
 *
 * Forward the provided raw payload as the payload of an EMAP_MSG_GAME_STATE
 * packet. This function does not interpret or validate the payload contents.
 *
 * @param fd     File descriptor (socket) to send the packet on.
 * @param payload Pointer to the raw payload data to send (may be NULL if
 * len==0).
 * @param len     Length in bytes of payload.
 *
 * @return The value returned by emap_send_packet (propagates success/error).
 *
 * @note The caller is responsible for ensuring the payload format and length
 *       conform to the EMAP game-state specification.
 */
int emap_helper_send_game_state(int fd, const void* payload, uint16_t len) {
	return emap_send_packet(fd, EMAP_MSG_GAME_STATE, payload, len);
}

/**
 * @brief Notify peers about the end of a game and whether the sender won.
 *
 * Send an EMAP_MSG_GAME_END packet containing a single byte indicating
 * whether the sender has won the game.
 *
 * @param fd      File descriptor (socket) to send the packet on.
 * @param result  Non-zero if the sender won, zero otherwise.
 *
 * @return The value returned by emap_send_packet (propagates success/error).
 */
int emap_helper_send_game_end(int fd, uint8_t result) {
	EMAP_PayloadGameEnd p = {.has_won = result};
	return emap_send_packet(fd, EMAP_MSG_GAME_END, &p, 1);
}

/**
 * @brief Send a shop action packet.
 *
 * Wraps the given action and skin index into an EMAP_MSG_SHOP_ACTION payload
 * so the server can process buy/equip requests.
 *
 * @param fd         File descriptor (socket) to send the packet on.
 * @param action     Shop action type, see EMAP_ShopActionType.
 * @param skin_index Target skin identifier.
 *
 * @return The value returned by emap_send_packet (propagates success/error).
 */
int emap_helper_send_shop_action(int fd, uint8_t action, uint8_t skin_index) {
	EMAP_PayloadShopAction payload = {.action = action,
									  .skin_index = skin_index};
	return emap_send_packet(fd, EMAP_MSG_SHOP_ACTION, &payload, sizeof payload);
}

/**
 * @brief Notify the peer about updated player data.
 *
 * Sends EMAP_MSG_PLAYER_DATA_UPDATE so the client can refresh level, progress,
 * money, selected skin, and owned skins without going back to the initial state
 * of askin the initial player info.
 * @param fd             File descriptor (socket) to send the packet on.
 * @param level          Player level (uint8_t).
 * @param progression    Player progression (uint16_t, converted to network
 * order).
 * @param money          Player money amount (uint16_t, converted to network
 * order).
 * @param selected_skin  Currently equipped skin id.
 * @param skins          Pointer to skin ownership array (may be NULL when
 * len==0).
 * @param skins_len      Number of bytes in skins array; truncated to
 * TOTAL_SKIN_AMOUNT.
 *
 * @return The value returned by emap_send_packet (propagates success/error).
 */
int emap_helper_send_player_data_update(int fd, uint8_t level,
										uint16_t progression, uint16_t money,
										uint8_t selected_skin,
										const uint8_t* skins,
										uint16_t skins_len) {
	EMAP_PayloadPlayerDataUpdate pi;
	memset(&pi, 0, sizeof pi);
	pi.level = level;
	uint16_t prog_n = htons(progression);
	uint16_t money_n = htons(money);
	memcpy(&pi.progression, &prog_n, 2);
	memcpy(&pi.money, &money_n, 2);
	pi.selected_skin = selected_skin;
	if (skins && skins_len > 0) {
		if (skins_len > TOTAL_SKIN_AMOUNT)
			skins_len = TOTAL_SKIN_AMOUNT;
		memcpy(pi.possessed_skins, skins, skins_len);
	}
	return emap_send_packet(fd, EMAP_MSG_PLAYER_DATA_UPDATE, &pi, sizeof pi);
}