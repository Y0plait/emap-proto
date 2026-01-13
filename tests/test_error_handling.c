/**
 * @file test_error_handling.c
 * @brief Error handling tests for EMAP protocol.
 */

#include <stdlib.h>
#include <string.h>

#include "../include/libemap.h"
#include "test_error_handling.h"
#include "test_framework.h"

/**
 * @brief Test emap_encode_msg with NULL message pointer.
 */
void test_encode_null_message(void) {
	void* encoded = NULL;
	uint16_t encoded_len = 0;

	int ret =
		emap_encode_msg(EMAP_MSG_GENERAL_ACK, NULL, &encoded, &encoded_len);
	TEST_ASSERT_EQ(ret, -1, "emap_encode_msg should fail with NULL message");

	TEST_PASS();
}

/**
 * @brief Test emap_encode_msg with NULL output pointer.
 */
void test_encode_null_out(void) {
	EMAP_Message msg_in;
	uint16_t encoded_len = 0;

	memset(&msg_in, 0, sizeof(msg_in));
	msg_in.type = EMAP_MSG_GENERAL_ACK;

	int ret =
		emap_encode_msg(EMAP_MSG_GENERAL_ACK, &msg_in, NULL, &encoded_len);
	TEST_ASSERT_EQ(ret, -1, "emap_encode_msg should fail with NULL out");

	TEST_PASS();
}

/**
 * @brief Test emap_encode_msg with NULL out_len pointer.
 */
void test_encode_null_out_len(void) {
	EMAP_Message msg_in;
	void* encoded = NULL;

	memset(&msg_in, 0, sizeof(msg_in));
	msg_in.type = EMAP_MSG_GENERAL_ACK;

	int ret = emap_encode_msg(EMAP_MSG_GENERAL_ACK, &msg_in, &encoded, NULL);
	TEST_ASSERT_EQ(ret, -1, "emap_encode_msg should fail with NULL out_len");

	TEST_PASS();
}

/**
 * @brief Test emap_encode_msg with unknown message type.
 */
void test_encode_unknown_type(void) {
	EMAP_Message msg_in;
	void* encoded = NULL;
	uint16_t encoded_len = 0;

	memset(&msg_in, 0, sizeof(msg_in));
	msg_in.type = 0xFF; /* Unknown type */

	int ret = emap_encode_msg(0xFF, &msg_in, &encoded, &encoded_len);
	TEST_ASSERT_EQ(ret, -1, "emap_encode_msg should fail with unknown type");

	TEST_PASS();
}

/**
 * @brief Test emap_decode_msg with wrong payload length for GENERAL_ACK.
 */
void test_decode_wrong_payload_len_ack(void) {
	EMAP_Message msg_out;
	uint8_t payload[10] = {0};

	memset(&msg_out, 0, sizeof(msg_out));
	int ret = emap_decode_msg(EMAP_MSG_GENERAL_ACK, payload, 10, &msg_out);
	TEST_ASSERT_EQ(ret, -1,
				   "emap_decode_msg should fail with wrong payload length");

	TEST_PASS();
}

/**
 * @brief Test emap_decode_msg with wrong payload length for MOVE.
 */
void test_decode_wrong_payload_len_move(void) {
	EMAP_Message msg_out;
	uint8_t payload[1] = {0};

	memset(&msg_out, 0, sizeof(msg_out));
	int ret = emap_decode_msg(EMAP_MSG_MOVE, payload, 1, &msg_out);
	TEST_ASSERT_EQ(
		ret, -1,
		"emap_decode_msg should fail with wrong payload length for MOVE");

	TEST_PASS();
}

/**
 * @brief Test emap_decode_msg with wrong payload length for CONNECT.
 */
void test_decode_wrong_payload_len_connect(void) {
	EMAP_Message msg_out;
	uint8_t payload[5] = {0};

	memset(&msg_out, 0, sizeof(msg_out));
	int ret = emap_decode_msg(EMAP_MSG_CONNECT, payload, 5, &msg_out);
	TEST_ASSERT_EQ(
		ret, -1,
		"emap_decode_msg should fail with wrong payload length for CONNECT");

	TEST_PASS();
}

/**
 * @brief Test emap_decode_msg with wrong payload length for PLAYER_INFO.
 */
void test_decode_wrong_payload_len_player_info(void) {
	EMAP_Message msg_out;
	uint8_t payload[3] = {0};

	memset(&msg_out, 0, sizeof(msg_out));
	int ret = emap_decode_msg(EMAP_MSG_PLAYER_INFO, payload, 3, &msg_out);
	TEST_ASSERT_EQ(ret, -1,
				   "emap_decode_msg should fail with wrong payload length for "
				   "PLAYER_INFO");

	TEST_PASS();
}

/**
 * @brief Test emap_decode_msg with unknown message type.
 */
void test_decode_unknown_type(void) {
	EMAP_Message msg_out;
	uint8_t payload[1] = {0};

	memset(&msg_out, 0, sizeof(msg_out));
	int ret = emap_decode_msg(0xFF, payload, 1, &msg_out);
	TEST_ASSERT_EQ(ret, -1, "emap_decode_msg should fail with unknown type");

	TEST_PASS();
}

/**
 * @brief Test emap_decode_msg with unexpected payload for GAME_WAIT.
 */
void test_decode_game_wait_with_payload(void) {
	EMAP_Message msg_out;
	uint8_t payload[5] = {0};

	memset(&msg_out, 0, sizeof(msg_out));
	int ret = emap_decode_msg(EMAP_MSG_GAME_WAIT, payload, 5, &msg_out);
	TEST_ASSERT_EQ(ret, -1,
				   "emap_decode_msg should fail when GAME_WAIT has payload");

	TEST_PASS();
}

/**
 * @brief Run all error handling tests.
 */
void run_error_handling_tests(void) {
	printf("\n[Error Handling Tests]\n");
	RUN_TEST(test_encode_null_message);
	RUN_TEST(test_encode_null_out);
	RUN_TEST(test_encode_null_out_len);
	RUN_TEST(test_encode_unknown_type);
	RUN_TEST(test_decode_wrong_payload_len_ack);
	RUN_TEST(test_decode_wrong_payload_len_move);
	RUN_TEST(test_decode_wrong_payload_len_connect);
	RUN_TEST(test_decode_wrong_payload_len_player_info);
	RUN_TEST(test_decode_unknown_type);
	RUN_TEST(test_decode_game_wait_with_payload);
}
