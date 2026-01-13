/**
 * @file test_emap.c
 * @brief Main entry point for the EMAP protocol test suite.
 *
 * This file orchestrates running all test modules:
 * - Encode/Decode round-trip tests
 * - Error handling tests
 * - Network byte order tests
 * - Packet header tests
 * - Socket integration tests
 *
 * @authors Mattéo BEZET-TORRES, Elias GARACH-MALULY, Anton MOULIN
 */

#include <stdio.h>

#include "test_byte_order.h"
#include "test_encode_decode.h"
#include "test_error_handling.h"
#include "test_framework.h"
#include "test_packet.h"
#include "test_socket.h"

/* Global test counters */
int tests_passed = 0;
int tests_failed = 0;

/* ========================================================================== */
/*                              Main Entry Point */
/* ========================================================================== */

int main(void) {
	printf("========================================\n");
	printf("       EMAP Protocol Test Suite        \n");
	printf("========================================\n\n");

	/* Run all test modules */
	run_encode_decode_tests();
	run_error_handling_tests();
	run_byte_order_tests();
	run_packet_tests();
	run_socket_tests();

	printf("\n========================================\n");
	printf("        Test Results Summary           \n");
	printf("========================================\n");
	printf("  Passed: %d\n", tests_passed);
	printf("  Failed: %d\n", tests_failed);
	printf("  Total:  %d\n", tests_passed + tests_failed);
	printf("========================================\n");

	return tests_failed > 0 ? 1 : 0;
}
