/**
 * @file test_encode_decode.h
 * @brief Encode/Decode round-trip test declarations.
 */

#ifndef TEST_ENCODE_DECODE_H
#define TEST_ENCODE_DECODE_H

void test_general_ack_ok(void);
void test_general_ack_nok(void);
void test_connect_message(void);
void test_connect_max_username(void);
void test_move_message(void);
void test_move_boundary_values(void);
void test_play_your_turn(void);
void test_play_not_your_turn(void);
void test_player_info_message(void);
void test_player_data_update(void);
void test_game_start_message(void);
void test_game_end_win(void);
void test_game_end_lose(void);
void test_game_wait_message(void);
void test_game_join_message(void);
void test_game_state_message(void);
void test_illegal_play_message(void);
void test_shop_action_purchase(void);
void test_shop_action_equip(void);

void run_encode_decode_tests(void);

#endif /* TEST_ENCODE_DECODE_H */
