/**
 * @file test_socket.h
 * @brief Socket integration test declarations.
 */

#ifndef TEST_SOCKET_H
#define TEST_SOCKET_H

void test_send_recv_packet_roundtrip(void);
void test_send_recv_empty_payload(void);
void test_send_recv_player_info(void);
void test_send_packet_invalid_params(void);
void test_send_packet_null_payload_with_len(void);

void run_socket_tests(void);

#endif /* TEST_SOCKET_H */
