/**
 * @file test_error_handling.h
 * @brief Error handling test declarations.
 */

#ifndef TEST_ERROR_HANDLING_H
#define TEST_ERROR_HANDLING_H

void test_encode_null_message(void);
void test_encode_null_out(void);
void test_encode_null_out_len(void);
void test_encode_unknown_type(void);
void test_decode_wrong_payload_len_ack(void);
void test_decode_wrong_payload_len_move(void);
void test_decode_wrong_payload_len_connect(void);
void test_decode_wrong_payload_len_player_info(void);
void test_decode_unknown_type(void);
void test_decode_game_wait_with_payload(void);

void run_error_handling_tests(void);

#endif /* TEST_ERROR_HANDLING_H */
