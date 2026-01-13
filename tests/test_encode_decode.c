/**
 * @file test_encode_decode.c
 * @brief Encode/Decode round-trip tests for EMAP protocol messages.
 */

#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>

#include "../include/libemap.h"
#include "test_encode_decode.h"
#include "test_framework.h"

/**
 * @brief Test EMAP_MSG_GENERAL_ACK encoding and decoding (ACK_OK).
 */
void test_general_ack_ok(void) {
	EMAP_Message msg_in, msg_out;
	void* encoded = NULL;
	uint16_t encoded_len = 0;

	memset(&msg_in, 0, sizeof(msg_in));
	msg_in.type = EMAP_MSG_GENERAL_ACK;
	msg_in.data.general_ack.ack_type = EMAP_MESSAGE_ACK_OK;

	int ret =
		emap_encode_msg(EMAP_MSG_GENERAL_ACK, &msg_in, &encoded, &encoded_len);
	TEST_ASSERT_EQ(ret, 0, "emap_encode_msg should succeed");
	TEST_ASSERT(encoded != NULL, "Encoded buffer should not be NULL");
	TEST_ASSERT_EQ(encoded_len, sizeof(EMAP_PayloadGeneralACK),
				   "Encoded length mismatch");

	memset(&msg_out, 0, sizeof(msg_out));
	ret = emap_decode_msg(EMAP_MSG_GENERAL_ACK, encoded, encoded_len, &msg_out);
	TEST_ASSERT_EQ(ret, 0, "emap_decode_msg should succeed");
	TEST_ASSERT_EQ(msg_out.type, EMAP_MSG_GENERAL_ACK, "Message type mismatch");
	TEST_ASSERT_EQ(msg_out.data.general_ack.ack_type, EMAP_MESSAGE_ACK_OK,
				   "ACK type mismatch");

	free(encoded);
	TEST_PASS();
}

/**
 * @brief Test EMAP_MSG_GENERAL_ACK encoding and decoding (ACK_NOK).
 */
void test_general_ack_nok(void) {
	EMAP_Message msg_in, msg_out;
	void* encoded = NULL;
	uint16_t encoded_len = 0;

	memset(&msg_in, 0, sizeof(msg_in));
	msg_in.type = EMAP_MSG_GENERAL_ACK;
	msg_in.data.general_ack.ack_type = EMAP_MESSAGE_ACK_NOK;

	int ret =
		emap_encode_msg(EMAP_MSG_GENERAL_ACK, &msg_in, &encoded, &encoded_len);
	TEST_ASSERT_EQ(ret, 0, "emap_encode_msg should succeed");

	memset(&msg_out, 0, sizeof(msg_out));
	ret = emap_decode_msg(EMAP_MSG_GENERAL_ACK, encoded, encoded_len, &msg_out);
	TEST_ASSERT_EQ(ret, 0, "emap_decode_msg should succeed");
	TEST_ASSERT_EQ(msg_out.data.general_ack.ack_type, EMAP_MESSAGE_ACK_NOK,
				   "ACK type mismatch");

	free(encoded);
	TEST_PASS();
}

/**
 * @brief Test EMAP_MSG_CONNECT encoding and decoding.
 */
void test_connect_message(void) {
	EMAP_Message msg_in, msg_out;
	void* encoded = NULL;
	uint16_t encoded_len = 0;
	const char* test_username = "TestPlayer42";

	memset(&msg_in, 0, sizeof(msg_in));
	msg_in.type = EMAP_MSG_CONNECT;
	strncpy((char*)msg_in.data.connect.username, test_username,
			MAX_PLAYER_USERNAME_SIZE - 1);

	int ret =
		emap_encode_msg(EMAP_MSG_CONNECT, &msg_in, &encoded, &encoded_len);
	TEST_ASSERT_EQ(ret, 0, "emap_encode_msg should succeed");
	TEST_ASSERT_EQ(encoded_len, MAX_PLAYER_USERNAME_SIZE,
				   "Encoded length should match username size");

	memset(&msg_out, 0, sizeof(msg_out));
	ret = emap_decode_msg(EMAP_MSG_CONNECT, encoded, encoded_len, &msg_out);
	TEST_ASSERT_EQ(ret, 0, "emap_decode_msg should succeed");
	TEST_ASSERT_EQ(msg_out.type, EMAP_MSG_CONNECT, "Message type mismatch");
	TEST_ASSERT(memcmp(msg_out.data.connect.username, test_username,
					   strlen(test_username)) == 0,
				"Username mismatch");

	free(encoded);
	TEST_PASS();
}

/**
 * @brief Test EMAP_MSG_CONNECT with max length username.
 */
void test_connect_max_username(void) {
	EMAP_Message msg_in, msg_out;
	void* encoded = NULL;
	uint16_t encoded_len = 0;

	memset(&msg_in, 0, sizeof(msg_in));
	msg_in.type = EMAP_MSG_CONNECT;
	memset(msg_in.data.connect.username, 'A', MAX_PLAYER_USERNAME_SIZE);

	int ret =
		emap_encode_msg(EMAP_MSG_CONNECT, &msg_in, &encoded, &encoded_len);
	TEST_ASSERT_EQ(ret, 0, "emap_encode_msg should succeed");

	memset(&msg_out, 0, sizeof(msg_out));
	ret = emap_decode_msg(EMAP_MSG_CONNECT, encoded, encoded_len, &msg_out);
	TEST_ASSERT_EQ(ret, 0, "emap_decode_msg should succeed");
	TEST_ASSERT(memcmp(msg_out.data.connect.username,
					   msg_in.data.connect.username,
					   MAX_PLAYER_USERNAME_SIZE) == 0,
				"Username mismatch");

	free(encoded);
	TEST_PASS();
}

/**
 * @brief Test EMAP_MSG_MOVE encoding and decoding.
 */
void test_move_message(void) {
	EMAP_Message msg_in, msg_out;
	void* encoded = NULL;
	uint16_t encoded_len = 0;

	memset(&msg_in, 0, sizeof(msg_in));
	msg_in.type = EMAP_MSG_MOVE;
	msg_in.data.move.coin_index = 3;
	msg_in.data.move.coin_pos = 15;

	int ret = emap_encode_msg(EMAP_MSG_MOVE, &msg_in, &encoded, &encoded_len);
	TEST_ASSERT_EQ(ret, 0, "emap_encode_msg should succeed");
	TEST_ASSERT_EQ(encoded_len, sizeof(EMAP_PayloadMove),
				   "Encoded length mismatch");

	memset(&msg_out, 0, sizeof(msg_out));
	ret = emap_decode_msg(EMAP_MSG_MOVE, encoded, encoded_len, &msg_out);
	TEST_ASSERT_EQ(ret, 0, "emap_decode_msg should succeed");
	TEST_ASSERT_EQ(msg_out.type, EMAP_MSG_MOVE, "Message type mismatch");
	TEST_ASSERT_EQ(msg_out.data.move.coin_index, 3, "coin_index mismatch");
	TEST_ASSERT_EQ(msg_out.data.move.coin_pos, 15, "coin_pos mismatch");

	free(encoded);
	TEST_PASS();
}

/**
 * @brief Test EMAP_MSG_MOVE with boundary values.
 */
void test_move_boundary_values(void) {
	EMAP_Message msg_in, msg_out;
	void* encoded = NULL;
	uint16_t encoded_len = 0;

	memset(&msg_in, 0, sizeof(msg_in));
	msg_in.type = EMAP_MSG_MOVE;
	msg_in.data.move.coin_index = 255;
	msg_in.data.move.coin_pos = 0;

	int ret = emap_encode_msg(EMAP_MSG_MOVE, &msg_in, &encoded, &encoded_len);
	TEST_ASSERT_EQ(ret, 0, "emap_encode_msg should succeed");

	memset(&msg_out, 0, sizeof(msg_out));
	ret = emap_decode_msg(EMAP_MSG_MOVE, encoded, encoded_len, &msg_out);
	TEST_ASSERT_EQ(ret, 0, "emap_decode_msg should succeed");
	TEST_ASSERT_EQ(msg_out.data.move.coin_index, 255,
				   "coin_index boundary mismatch");
	TEST_ASSERT_EQ(msg_out.data.move.coin_pos, 0, "coin_pos boundary mismatch");

	free(encoded);
	TEST_PASS();
}

/**
 * @brief Test EMAP_MSG_PLAY encoding and decoding (YOUR_TURN).
 */
void test_play_your_turn(void) {
	EMAP_Message msg_in, msg_out;
	void* encoded = NULL;
	uint16_t encoded_len = 0;

	memset(&msg_in, 0, sizeof(msg_in));
	msg_in.type = EMAP_MSG_PLAY;
	msg_in.data.play.play_type = EMAP_PLAY_YOUR_TURN;

	int ret = emap_encode_msg(EMAP_MSG_PLAY, &msg_in, &encoded, &encoded_len);
	TEST_ASSERT_EQ(ret, 0, "emap_encode_msg should succeed");
	TEST_ASSERT_EQ(encoded_len, sizeof(EMAP_PayloadPlay),
				   "Encoded length mismatch");

	memset(&msg_out, 0, sizeof(msg_out));
	ret = emap_decode_msg(EMAP_MSG_PLAY, encoded, encoded_len, &msg_out);
	TEST_ASSERT_EQ(ret, 0, "emap_decode_msg should succeed");
	TEST_ASSERT_EQ(msg_out.type, EMAP_MSG_PLAY, "Message type mismatch");
	TEST_ASSERT_EQ(msg_out.data.play.play_type, EMAP_PLAY_YOUR_TURN,
				   "play_type mismatch");

	free(encoded);
	TEST_PASS();
}

/**
 * @brief Test EMAP_MSG_PLAY encoding and decoding (NOT_YOUR_TURN).
 */
void test_play_not_your_turn(void) {
	EMAP_Message msg_in, msg_out;
	void* encoded = NULL;
	uint16_t encoded_len = 0;

	memset(&msg_in, 0, sizeof(msg_in));
	msg_in.type = EMAP_MSG_PLAY;
	msg_in.data.play.play_type = EMAP_PLAY_NOT_YOUR_TURN;

	int ret = emap_encode_msg(EMAP_MSG_PLAY, &msg_in, &encoded, &encoded_len);
	TEST_ASSERT_EQ(ret, 0, "emap_encode_msg should succeed");

	memset(&msg_out, 0, sizeof(msg_out));
	ret = emap_decode_msg(EMAP_MSG_PLAY, encoded, encoded_len, &msg_out);
	TEST_ASSERT_EQ(ret, 0, "emap_decode_msg should succeed");
	TEST_ASSERT_EQ(msg_out.data.play.play_type, EMAP_PLAY_NOT_YOUR_TURN,
				   "play_type mismatch");

	free(encoded);
	TEST_PASS();
}

/**
 * @brief Test EMAP_MSG_PLAYER_INFO encoding and decoding.
 */
void test_player_info_message(void) {
	EMAP_Message msg_in, msg_out;
	void* encoded = NULL;
	uint16_t encoded_len = 0;

	memset(&msg_in, 0, sizeof(msg_in));
	msg_in.type = EMAP_MSG_PLAYER_INFO;
	msg_in.data.player_info.level = 42;
	msg_in.data.player_info.progression = 1500;
	msg_in.data.player_info.money = 9999;
	msg_in.data.player_info.selected_skin = 2;
	msg_in.data.player_info.possessed_skins[0] = 1;
	msg_in.data.player_info.possessed_skins[1] = 1;
	msg_in.data.player_info.possessed_skins[2] = 1;
	msg_in.data.player_info.possessed_skins[3] = 0;
	msg_in.data.player_info.possessed_skins[4] = 0;
	msg_in.data.player_info.possessed_skins[5] = 1;

	int ret =
		emap_encode_msg(EMAP_MSG_PLAYER_INFO, &msg_in, &encoded, &encoded_len);
	TEST_ASSERT_EQ(ret, 0, "emap_encode_msg should succeed");
	TEST_ASSERT_EQ(encoded_len, 1 + 2 + 2 + 1 + TOTAL_SKIN_AMOUNT,
				   "Encoded length mismatch");

	memset(&msg_out, 0, sizeof(msg_out));
	ret = emap_decode_msg(EMAP_MSG_PLAYER_INFO, encoded, encoded_len, &msg_out);
	TEST_ASSERT_EQ(ret, 0, "emap_decode_msg should succeed");
	TEST_ASSERT_EQ(msg_out.type, EMAP_MSG_PLAYER_INFO, "Message type mismatch");
	TEST_ASSERT_EQ(msg_out.data.player_info.level, 42, "level mismatch");
	TEST_ASSERT_EQ(msg_out.data.player_info.progression, 1500,
				   "progression mismatch");
	TEST_ASSERT_EQ(msg_out.data.player_info.money, 9999, "money mismatch");
	TEST_ASSERT_EQ(msg_out.data.player_info.selected_skin, 2,
				   "selected_skin mismatch");
	TEST_ASSERT(memcmp(msg_out.data.player_info.possessed_skins,
					   msg_in.data.player_info.possessed_skins,
					   TOTAL_SKIN_AMOUNT) == 0,
				"possessed_skins mismatch");

	free(encoded);
	TEST_PASS();
}

/**
 * @brief Test EMAP_MSG_PLAYER_DATA_UPDATE (same format as PLAYER_INFO).
 */
void test_player_data_update(void) {
	EMAP_Message msg_in, msg_out;
	void* encoded = NULL;
	uint16_t encoded_len = 0;

	memset(&msg_in, 0, sizeof(msg_in));
	msg_in.type = EMAP_MSG_PLAYER_DATA_UPDATE;
	msg_in.data.player_update.level = 99;
	msg_in.data.player_update.progression = 65535;
	msg_in.data.player_update.money = 0;
	msg_in.data.player_update.selected_skin = 5;

	int ret = emap_encode_msg(EMAP_MSG_PLAYER_DATA_UPDATE, &msg_in, &encoded,
							  &encoded_len);
	TEST_ASSERT_EQ(ret, 0, "emap_encode_msg should succeed");

	memset(&msg_out, 0, sizeof(msg_out));
	ret = emap_decode_msg(EMAP_MSG_PLAYER_DATA_UPDATE, encoded, encoded_len,
						  &msg_out);
	TEST_ASSERT_EQ(ret, 0, "emap_decode_msg should succeed");
	TEST_ASSERT_EQ(msg_out.type, EMAP_MSG_PLAYER_DATA_UPDATE,
				   "Message type mismatch");
	TEST_ASSERT_EQ(msg_out.data.player_update.level, 99, "level mismatch");
	TEST_ASSERT_EQ(msg_out.data.player_update.progression, 65535,
				   "progression max value mismatch");
	TEST_ASSERT_EQ(msg_out.data.player_update.money, 0, "money mismatch");

	free(encoded);
	TEST_PASS();
}

/**
 * @brief Test EMAP_MSG_GAME_START encoding and decoding.
 */
void test_game_start_message(void) {
	EMAP_Message msg_in, msg_out;
	void* encoded = NULL;
	uint16_t encoded_len = 0;

	memset(&msg_in, 0, sizeof(msg_in));
	msg_in.type = EMAP_MSG_GAME_START;
	msg_in.data.game_start.is_starting = EMAP_PLAY_YOUR_TURN;

	int ret =
		emap_encode_msg(EMAP_MSG_GAME_START, &msg_in, &encoded, &encoded_len);
	TEST_ASSERT_EQ(ret, 0, "emap_encode_msg should succeed");
	TEST_ASSERT_EQ(encoded_len, sizeof(EMAP_PayloadGameStart),
				   "Encoded length mismatch");

	memset(&msg_out, 0, sizeof(msg_out));
	ret = emap_decode_msg(EMAP_MSG_GAME_START, encoded, encoded_len, &msg_out);
	TEST_ASSERT_EQ(ret, 0, "emap_decode_msg should succeed");
	TEST_ASSERT_EQ(msg_out.type, EMAP_MSG_GAME_START, "Message type mismatch");
	TEST_ASSERT_EQ(msg_out.data.game_start.is_starting, EMAP_PLAY_YOUR_TURN,
				   "is_starting mismatch");

	free(encoded);
	TEST_PASS();
}

/**
 * @brief Test EMAP_MSG_GAME_END with win result.
 */
void test_game_end_win(void) {
	EMAP_Message msg_in, msg_out;
	void* encoded = NULL;
	uint16_t encoded_len = 0;

	memset(&msg_in, 0, sizeof(msg_in));
	msg_in.type = EMAP_MSG_GAME_END;
	msg_in.data.game_end.has_won = EMAP_END_GAME_WIN;

	int ret =
		emap_encode_msg(EMAP_MSG_GAME_END, &msg_in, &encoded, &encoded_len);
	TEST_ASSERT_EQ(ret, 0, "emap_encode_msg should succeed");
	TEST_ASSERT_EQ(encoded_len, sizeof(EMAP_PayloadGameEnd),
				   "Encoded length mismatch");

	memset(&msg_out, 0, sizeof(msg_out));
	ret = emap_decode_msg(EMAP_MSG_GAME_END, encoded, encoded_len, &msg_out);
	TEST_ASSERT_EQ(ret, 0, "emap_decode_msg should succeed");
	TEST_ASSERT_EQ(msg_out.type, EMAP_MSG_GAME_END, "Message type mismatch");
	TEST_ASSERT_EQ(msg_out.data.game_end.has_won, EMAP_END_GAME_WIN,
				   "has_won mismatch");

	free(encoded);
	TEST_PASS();
}

/**
 * @brief Test EMAP_MSG_GAME_END with lose result.
 */
void test_game_end_lose(void) {
	EMAP_Message msg_in, msg_out;
	void* encoded = NULL;
	uint16_t encoded_len = 0;

	memset(&msg_in, 0, sizeof(msg_in));
	msg_in.type = EMAP_MSG_GAME_END;
	msg_in.data.game_end.has_won = EMAP_END_GAME_LOSE;

	int ret =
		emap_encode_msg(EMAP_MSG_GAME_END, &msg_in, &encoded, &encoded_len);
	TEST_ASSERT_EQ(ret, 0, "emap_encode_msg should succeed");

	memset(&msg_out, 0, sizeof(msg_out));
	ret = emap_decode_msg(EMAP_MSG_GAME_END, encoded, encoded_len, &msg_out);
	TEST_ASSERT_EQ(ret, 0, "emap_decode_msg should succeed");
	TEST_ASSERT_EQ(msg_out.data.game_end.has_won, EMAP_END_GAME_LOSE,
				   "has_won mismatch");

	free(encoded);
	TEST_PASS();
}

/**
 * @brief Test EMAP_MSG_GAME_WAIT (empty payload).
 */
void test_game_wait_message(void) {
	EMAP_Message msg_in, msg_out;
	void* encoded = NULL;
	uint16_t encoded_len = 0;

	memset(&msg_in, 0, sizeof(msg_in));
	msg_in.type = EMAP_MSG_GAME_WAIT;

	int ret =
		emap_encode_msg(EMAP_MSG_GAME_WAIT, &msg_in, &encoded, &encoded_len);
	TEST_ASSERT_EQ(ret, 0, "emap_encode_msg should succeed");
	TEST_ASSERT_EQ(encoded_len, 0, "GAME_WAIT should have no payload");
	TEST_ASSERT(encoded == NULL, "GAME_WAIT encoded buffer should be NULL");

	memset(&msg_out, 0, sizeof(msg_out));
	ret = emap_decode_msg(EMAP_MSG_GAME_WAIT, NULL, 0, &msg_out);
	TEST_ASSERT_EQ(ret, 0, "emap_decode_msg should succeed");
	TEST_ASSERT_EQ(msg_out.type, EMAP_MSG_GAME_WAIT, "Message type mismatch");

	TEST_PASS();
}

/**
 * @brief Test EMAP_MSG_GAME_JOIN (empty payload).
 */
void test_game_join_message(void) {
	EMAP_Message msg_in, msg_out;
	void* encoded = NULL;
	uint16_t encoded_len = 0;

	memset(&msg_in, 0, sizeof(msg_in));
	msg_in.type = EMAP_MSG_GAME_JOIN;

	int ret =
		emap_encode_msg(EMAP_MSG_GAME_JOIN, &msg_in, &encoded, &encoded_len);
	TEST_ASSERT_EQ(ret, 0, "emap_encode_msg should succeed");
	TEST_ASSERT_EQ(encoded_len, 0, "GAME_JOIN should have no payload");

	memset(&msg_out, 0, sizeof(msg_out));
	ret = emap_decode_msg(EMAP_MSG_GAME_JOIN, NULL, 0, &msg_out);
	TEST_ASSERT_EQ(ret, 0, "emap_decode_msg should succeed");
	TEST_ASSERT_EQ(msg_out.type, EMAP_MSG_GAME_JOIN, "Message type mismatch");

	TEST_PASS();
}

/**
 * @brief Test EMAP_MSG_GAME_STATE (empty payload).
 */
void test_game_state_message(void) {
	EMAP_Message msg_in, msg_out;
	void* encoded = NULL;
	uint16_t encoded_len = 0;

	memset(&msg_in, 0, sizeof(msg_in));
	msg_in.type = EMAP_MSG_GAME_STATE;

	int ret =
		emap_encode_msg(EMAP_MSG_GAME_STATE, &msg_in, &encoded, &encoded_len);
	TEST_ASSERT_EQ(ret, 0, "emap_encode_msg should succeed");
	TEST_ASSERT_EQ(encoded_len, 0, "GAME_STATE should have no payload");

	memset(&msg_out, 0, sizeof(msg_out));
	ret = emap_decode_msg(EMAP_MSG_GAME_STATE, NULL, 0, &msg_out);
	TEST_ASSERT_EQ(ret, 0, "emap_decode_msg should succeed");
	TEST_ASSERT_EQ(msg_out.type, EMAP_MSG_GAME_STATE, "Message type mismatch");

	TEST_PASS();
}

/**
 * @brief Test EMAP_MSG_ILLEGAL_PLAY (empty payload).
 */
void test_illegal_play_message(void) {
	EMAP_Message msg_in, msg_out;
	void* encoded = NULL;
	uint16_t encoded_len = 0;

	memset(&msg_in, 0, sizeof(msg_in));
	msg_in.type = EMAP_MSG_ILLEGAL_PLAY;

	int ret =
		emap_encode_msg(EMAP_MSG_ILLEGAL_PLAY, &msg_in, &encoded, &encoded_len);
	TEST_ASSERT_EQ(ret, 0, "emap_encode_msg should succeed");
	TEST_ASSERT_EQ(encoded_len, 0, "ILLEGAL_PLAY should have no payload");

	memset(&msg_out, 0, sizeof(msg_out));
	ret = emap_decode_msg(EMAP_MSG_ILLEGAL_PLAY, NULL, 0, &msg_out);
	TEST_ASSERT_EQ(ret, 0, "emap_decode_msg should succeed");
	TEST_ASSERT_EQ(msg_out.type, EMAP_MSG_ILLEGAL_PLAY,
				   "Message type mismatch");

	TEST_PASS();
}

/**
 * @brief Test EMAP_MSG_SHOP_ACTION encoding and decoding (purchase).
 */
void test_shop_action_purchase(void) {
	EMAP_Message msg_in;
	void* encoded = NULL;
	uint16_t encoded_len = 0;

	memset(&msg_in, 0, sizeof(msg_in));
	msg_in.type = EMAP_MSG_SHOP_ACTION;
	msg_in.data.shop_action.action = EMAP_SHOP_ACTION_PURCHASE;
	msg_in.data.shop_action.skin_index = 3;

	int ret =
		emap_encode_msg(EMAP_MSG_SHOP_ACTION, &msg_in, &encoded, &encoded_len);
	TEST_ASSERT_EQ(ret, 0, "emap_encode_msg should succeed");
	TEST_ASSERT_EQ(encoded_len, sizeof(EMAP_PayloadShopAction),
				   "Encoded length mismatch");

	/* Note: SHOP_ACTION decode not implemented in emap_decode_msg, test encode
	 * only */
	TEST_ASSERT(encoded != NULL, "Encoded buffer should not be NULL");
	uint8_t* buf = (uint8_t*)encoded;
	TEST_ASSERT_EQ(buf[0], EMAP_SHOP_ACTION_PURCHASE, "action mismatch");
	TEST_ASSERT_EQ(buf[1], 3, "skin_index mismatch");

	free(encoded);
	TEST_PASS();
}

/**
 * @brief Test EMAP_MSG_SHOP_ACTION encoding (equip).
 */
void test_shop_action_equip(void) {
	EMAP_Message msg_in;
	void* encoded = NULL;
	uint16_t encoded_len = 0;

	memset(&msg_in, 0, sizeof(msg_in));
	msg_in.type = EMAP_MSG_SHOP_ACTION;
	msg_in.data.shop_action.action = EMAP_SHOP_ACTION_EQUIP;
	msg_in.data.shop_action.skin_index = 5;

	int ret =
		emap_encode_msg(EMAP_MSG_SHOP_ACTION, &msg_in, &encoded, &encoded_len);
	TEST_ASSERT_EQ(ret, 0, "emap_encode_msg should succeed");

	uint8_t* buf = (uint8_t*)encoded;
	TEST_ASSERT_EQ(buf[0], EMAP_SHOP_ACTION_EQUIP, "action mismatch");
	TEST_ASSERT_EQ(buf[1], 5, "skin_index mismatch");

	free(encoded);
	TEST_PASS();
}

/**
 * @brief Run all encode/decode tests.
 */
void run_encode_decode_tests(void) {
	printf("[Encode/Decode Round-Trip Tests]\n");
	RUN_TEST(test_general_ack_ok);
	RUN_TEST(test_general_ack_nok);
	RUN_TEST(test_connect_message);
	RUN_TEST(test_connect_max_username);
	RUN_TEST(test_move_message);
	RUN_TEST(test_move_boundary_values);
	RUN_TEST(test_play_your_turn);
	RUN_TEST(test_play_not_your_turn);
	RUN_TEST(test_player_info_message);
	RUN_TEST(test_player_data_update);
	RUN_TEST(test_game_start_message);
	RUN_TEST(test_game_end_win);
	RUN_TEST(test_game_end_lose);
	RUN_TEST(test_game_wait_message);
	RUN_TEST(test_game_join_message);
	RUN_TEST(test_game_state_message);
	RUN_TEST(test_illegal_play_message);
	RUN_TEST(test_shop_action_purchase);
	RUN_TEST(test_shop_action_equip);
}
