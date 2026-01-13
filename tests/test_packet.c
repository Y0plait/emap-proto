/**
 * @file test_packet.c
 * @brief Packet header and protocol constant tests for EMAP protocol.
 */

#include <string.h>

#include "../include/libemap.h"
#include "test_framework.h"
#include "test_packet.h"

/**
 * @brief Test EMAP_PacketHeader structure size and alignment.
 */
void test_packet_header_size(void) {
	TEST_ASSERT_EQ(sizeof(EMAP_PacketHeader), 6,
				   "EMAP_PacketHeader should be 6 bytes packed");
	TEST_PASS();
}

/**
 * @brief Test protocol constants.
 */
void test_protocol_constants(void) {
	TEST_ASSERT_EQ(EMAP_MAGIC_BYTE_0, 'E', "Magic byte 0 should be 'E'");
	TEST_ASSERT_EQ(EMAP_MAGIC_BYTE_1, 'M', "Magic byte 1 should be 'M'");
	TEST_ASSERT_EQ(EMAP_PROTO_VERSION, 1, "Protocol version should be 1");
	TEST_ASSERT_EQ(EMAP_MAX_PAYLOAD_SIZE, 1024,
				   "Max payload size should be 1024");
	TEST_ASSERT_EQ(MAX_PLAYER_USERNAME_SIZE, 16,
				   "Max username size should be 16");
	TEST_ASSERT_EQ(TOTAL_SKIN_AMOUNT, 6, "Total skin amount should be 6");
	TEST_PASS();
}

/**
 * @brief Run all packet tests.
 */
void run_packet_tests(void) {
	printf("\n[Packet Header Tests]\n");
	RUN_TEST(test_packet_header_size);
	RUN_TEST(test_protocol_constants);
}
