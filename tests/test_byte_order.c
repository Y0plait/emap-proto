/**
 * @file test_byte_order.c
 * @brief Network byte order tests for EMAP protocol.
 */

#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>

#include "../include/libemap.h"
#include "test_byte_order.h"
#include "test_framework.h"

/**
 * @brief Test that PLAYER_INFO correctly handles network byte order.
 */
void test_player_info_byte_order(void) {
	EMAP_Message msg_in, msg_out;
	void* encoded = NULL;
	uint16_t encoded_len = 0;

	memset(&msg_in, 0, sizeof(msg_in));
	msg_in.type = EMAP_MSG_PLAYER_INFO;
	msg_in.data.player_info.level = 1;
	msg_in.data.player_info.progression = 0x1234; /* Test endianness */
	msg_in.data.player_info.money = 0xABCD;		  /* Test endianness */
	msg_in.data.player_info.selected_skin = 0;

	int ret =
		emap_encode_msg(EMAP_MSG_PLAYER_INFO, &msg_in, &encoded, &encoded_len);
	TEST_ASSERT_EQ(ret, 0, "emap_encode_msg should succeed");

	/* Verify raw bytes are in network order */
	uint8_t* buf = (uint8_t*)encoded;
	TEST_ASSERT_EQ(buf[0], 1, "level byte mismatch");
	/* progression at bytes 1-2 should be big-endian 0x1234 */
	TEST_ASSERT_EQ(buf[1], 0x12, "progression high byte mismatch");
	TEST_ASSERT_EQ(buf[2], 0x34, "progression low byte mismatch");
	/* money at bytes 3-4 should be big-endian 0xABCD */
	TEST_ASSERT_EQ(buf[3], 0xAB, "money high byte mismatch");
	TEST_ASSERT_EQ(buf[4], 0xCD, "money low byte mismatch");

	memset(&msg_out, 0, sizeof(msg_out));
	ret = emap_decode_msg(EMAP_MSG_PLAYER_INFO, encoded, encoded_len, &msg_out);
	TEST_ASSERT_EQ(ret, 0, "emap_decode_msg should succeed");
	TEST_ASSERT_EQ(msg_out.data.player_info.progression, 0x1234,
				   "progression decode mismatch");
	TEST_ASSERT_EQ(msg_out.data.player_info.money, 0xABCD,
				   "money decode mismatch");

	free(encoded);
	TEST_PASS();
}

/**
 * @brief Run all byte order tests.
 */
void run_byte_order_tests(void) {
	printf("\n[Network Byte Order Tests]\n");
	RUN_TEST(test_player_info_byte_order);
}
