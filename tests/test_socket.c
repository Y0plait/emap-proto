/**
 * @file test_socket.c
 * @brief Socket integration tests for EMAP protocol.
 */

#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../include/emap_low_level.h"
#include "../include/libemap.h"
#include "test_framework.h"
#include "test_socket.h"

/**
 * @brief Helper to create a connected socket pair for testing.
 */
static int create_socket_pair(int fds[2]) {
	return socketpair(AF_UNIX, SOCK_STREAM, 0, fds);
}

/**
 * @brief Test emap_send_packet and emap_recv_packet round-trip.
 */
void test_send_recv_packet_roundtrip(void) {
	int fds[2];
	if (create_socket_pair(fds) < 0) {
		printf("  [SKIP] %s: socketpair failed\n", __func__);
		return;
	}

	EMAP_Message msg_in;
	memset(&msg_in, 0, sizeof(msg_in));
	msg_in.type = EMAP_MSG_MOVE;
	msg_in.data.move.coin_index = 7;
	msg_in.data.move.coin_pos = 42;

	void* encoded = NULL;
	uint16_t encoded_len = 0;
	int ret = emap_encode_msg(EMAP_MSG_MOVE, &msg_in, &encoded, &encoded_len);
	TEST_ASSERT_EQ(ret, 0, "emap_encode_msg should succeed");

	ret = emap_send_packet(fds[0], EMAP_MSG_MOVE, encoded, encoded_len);
	TEST_ASSERT_EQ(ret, 0, "emap_send_packet should succeed");

	uint8_t type_out;
	void* payload_out = NULL;
	uint16_t payload_len_out = 0;
	ret = emap_recv_packet(fds[1], &type_out, &payload_out, &payload_len_out);
	TEST_ASSERT_EQ(ret, 0, "emap_recv_packet should succeed");
	TEST_ASSERT_EQ(type_out, EMAP_MSG_MOVE, "Received type mismatch");
	TEST_ASSERT_EQ(payload_len_out, encoded_len,
				   "Received payload length mismatch");

	EMAP_Message msg_out;
	memset(&msg_out, 0, sizeof(msg_out));
	ret = emap_decode_msg(type_out, payload_out, payload_len_out, &msg_out);
	TEST_ASSERT_EQ(ret, 0, "emap_decode_msg should succeed");
	TEST_ASSERT_EQ(msg_out.data.move.coin_index, 7, "coin_index mismatch");
	TEST_ASSERT_EQ(msg_out.data.move.coin_pos, 42, "coin_pos mismatch");

	free(encoded);
	free(payload_out);
	close(fds[0]);
	close(fds[1]);
	TEST_PASS();
}

/**
 * @brief Test sending and receiving empty payload message.
 */
void test_send_recv_empty_payload(void) {
	int fds[2];
	if (create_socket_pair(fds) < 0) {
		printf("  [SKIP] %s: socketpair failed\n", __func__);
		return;
	}

	int ret = emap_send_packet(fds[0], EMAP_MSG_GAME_JOIN, NULL, 0);
	TEST_ASSERT_EQ(ret, 0, "emap_send_packet should succeed for empty payload");

	uint8_t type_out;
	void* payload_out = NULL;
	uint16_t payload_len_out = 0;
	ret = emap_recv_packet(fds[1], &type_out, &payload_out, &payload_len_out);
	TEST_ASSERT_EQ(ret, 0, "emap_recv_packet should succeed");
	TEST_ASSERT_EQ(type_out, EMAP_MSG_GAME_JOIN, "Received type mismatch");
	TEST_ASSERT_EQ(payload_len_out, 0, "Payload length should be 0");
	TEST_ASSERT(payload_out == NULL,
				"Payload should be NULL for empty message");

	close(fds[0]);
	close(fds[1]);
	TEST_PASS();
}

/**
 * @brief Test sending PLAYER_INFO through socket.
 */
void test_send_recv_player_info(void) {
	int fds[2];
	if (create_socket_pair(fds) < 0) {
		printf("  [SKIP] %s: socketpair failed\n", __func__);
		return;
	}

	EMAP_Message msg_in;
	memset(&msg_in, 0, sizeof(msg_in));
	msg_in.type = EMAP_MSG_PLAYER_INFO;
	msg_in.data.player_info.level = 50;
	msg_in.data.player_info.progression = 2500;
	msg_in.data.player_info.money = 5000;
	msg_in.data.player_info.selected_skin = 4;
	msg_in.data.player_info.possessed_skins[0] = 1;
	msg_in.data.player_info.possessed_skins[3] = 1;

	void* encoded = NULL;
	uint16_t encoded_len = 0;
	int ret =
		emap_encode_msg(EMAP_MSG_PLAYER_INFO, &msg_in, &encoded, &encoded_len);
	TEST_ASSERT_EQ(ret, 0, "emap_encode_msg should succeed");

	ret = emap_send_packet(fds[0], EMAP_MSG_PLAYER_INFO, encoded, encoded_len);
	TEST_ASSERT_EQ(ret, 0, "emap_send_packet should succeed");

	uint8_t type_out;
	void* payload_out = NULL;
	uint16_t payload_len_out = 0;
	ret = emap_recv_packet(fds[1], &type_out, &payload_out, &payload_len_out);
	TEST_ASSERT_EQ(ret, 0, "emap_recv_packet should succeed");

	EMAP_Message msg_out;
	memset(&msg_out, 0, sizeof(msg_out));
	ret = emap_decode_msg(type_out, payload_out, payload_len_out, &msg_out);
	TEST_ASSERT_EQ(ret, 0, "emap_decode_msg should succeed");
	TEST_ASSERT_EQ(msg_out.data.player_info.level, 50, "level mismatch");
	TEST_ASSERT_EQ(msg_out.data.player_info.progression, 2500,
				   "progression mismatch");
	TEST_ASSERT_EQ(msg_out.data.player_info.money, 5000, "money mismatch");
	TEST_ASSERT_EQ(msg_out.data.player_info.selected_skin, 4,
				   "selected_skin mismatch");

	free(encoded);
	free(payload_out);
	close(fds[0]);
	close(fds[1]);
	TEST_PASS();
}

/**
 * @brief Test emap_send_packet with invalid parameters.
 */
void test_send_packet_invalid_params(void) {
	int ret = emap_send_packet(-1, EMAP_MSG_GENERAL_ACK, NULL, 0);
	/* Should fail with invalid socket */
	TEST_ASSERT_EQ(ret, -1, "emap_send_packet should fail with invalid socket");

	TEST_PASS();
}

/**
 * @brief Test emap_send_packet with payload but NULL pointer.
 */
void test_send_packet_null_payload_with_len(void) {
	int fds[2];
	if (create_socket_pair(fds) < 0) {
		printf("  [SKIP] %s: socketpair failed\n", __func__);
		return;
	}

	int ret = emap_send_packet(fds[0], EMAP_MSG_MOVE, NULL, 5);
	TEST_ASSERT_EQ(
		ret, -1,
		"emap_send_packet should fail with NULL payload but non-zero len");

	close(fds[0]);
	close(fds[1]);
	TEST_PASS();
}

/**
 * @brief Run all socket tests.
 */
void run_socket_tests(void) {
	printf("\n[Socket Integration Tests]\n");
	RUN_TEST(test_send_recv_packet_roundtrip);
	RUN_TEST(test_send_recv_empty_payload);
	RUN_TEST(test_send_recv_player_info);
	RUN_TEST(test_send_packet_invalid_params);
	RUN_TEST(test_send_packet_null_payload_with_len);
}
