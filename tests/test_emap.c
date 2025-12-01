#include <arpa/inet.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "../include/emap_high_level.h"
#include "../include/emap_low_level.h"
#include "../include/constants.h"

#define EXPECT_TRUE(cond)                                                               \
    do                                                                                  \
    {                                                                                   \
        if (!(cond))                                                                    \
        {                                                                               \
            fprintf(stderr, "%s:%d: EXPECT_TRUE(%s) failed\n", __FILE__, __LINE__, #cond); \
            return 1;                                                                   \
        }                                                                               \
    } while (0)

#define EXPECT_EQ(expected, actual)                                                     \
    do                                                                                  \
    {                                                                                   \
        long long _exp = (long long)(expected);                                         \
        long long _act = (long long)(actual);                                           \
        if (_exp != _act)                                                               \
        {                                                                               \
            fprintf(stderr, "%s:%d: EXPECT_EQ(%s=%lld, %s=%lld) failed\n",             \
                    __FILE__, __LINE__, #expected, _exp, #actual, _act);                \
            return 1;                                                                   \
        }                                                                               \
    } while (0)

#define EXPECT_MEMEQ(expected, actual, len)                                              \
    do                                                                                  \
    {                                                                                   \
        if (memcmp((expected), (actual), (len)) != 0)                                   \
        {                                                                               \
            fprintf(stderr, "%s:%d: EXPECT_MEMEQ failed for %s and %s\n",             \
                    __FILE__, __LINE__, #expected, #actual);                            \
            return 1;                                                                   \
        }                                                                               \
    } while (0)

static int test_send_packet_rejects_null_payload(void)
{
    int fds[2];
    EXPECT_EQ(0, socketpair(AF_UNIX, SOCK_STREAM, 0, fds));

    int rc = emap_send_packet(fds[0], EMAP_MSG_MOVE, NULL, 1);
    EXPECT_EQ(-1, rc);

    uint8_t buffer[8];
    errno = 0;
    ssize_t received = recv(fds[1], buffer, sizeof(buffer), MSG_DONTWAIT);
    EXPECT_EQ(-1, received);
    EXPECT_TRUE(errno == EAGAIN || errno == EWOULDBLOCK);

    close(fds[0]);
    close(fds[1]);
    return 0;
}

static int test_send_receive_roundtrip(void)
{
    int fds[2];
    EXPECT_EQ(0, socketpair(AF_UNIX, SOCK_STREAM, 0, fds));

    const uint8_t payload[] = {0xAA, 0x55};
    EXPECT_EQ(0, emap_send_packet(fds[0], EMAP_MSG_MOVE, payload, sizeof(payload)));

    uint8_t type = 0;
    void *payload_out = NULL;
    uint16_t payload_len = 0;
    EXPECT_EQ(0, emap_recv_packet(fds[1], &type, &payload_out, &payload_len));

    EXPECT_EQ(EMAP_MSG_MOVE, type);
    EXPECT_EQ((int)sizeof(payload), payload_len);
    EXPECT_TRUE(payload_out != NULL);
    EXPECT_MEMEQ(payload, payload_out, payload_len);

    free(payload_out);
    close(fds[0]);
    close(fds[1]);
    return 0;
}

static int test_recv_rejects_oversized_payload(void)
{
    int fds[2];
    EXPECT_EQ(0, socketpair(AF_UNIX, SOCK_STREAM, 0, fds));

    EMAP_PacketHeader header = {
        .magic_byte = {EMAP_MAGIC_BYTE_0, EMAP_MAGIC_BYTE_1},
        .version = EMAP_PROTO_VERSION,
        .code = EMAP_MSG_MOVE,
        .size = htons((uint16_t)(EMAP_MAX_PAYLOAD_SIZE + 1))};

    EXPECT_EQ((long long)sizeof(header), (long long)send(fds[0], &header, sizeof(header), 0));

    uint8_t type = 0;
    void *payload = NULL;
    uint16_t payload_len = 0;
    int rc = emap_recv_packet(fds[1], &type, &payload, &payload_len);
    EXPECT_EQ(-1, rc);

    close(fds[0]);
    close(fds[1]);
    return 0;
}

static int test_recv_zero_length_payload(void)
{
    int fds[2];
    EXPECT_EQ(0, socketpair(AF_UNIX, SOCK_STREAM, 0, fds));

    EMAP_PacketHeader header = {
        .magic_byte = {EMAP_MAGIC_BYTE_0, EMAP_MAGIC_BYTE_1},
        .version = EMAP_PROTO_VERSION,
        .code = EMAP_MSG_GAME_WAIT,
        .size = htons(0)};

    EXPECT_EQ((long long)sizeof(header), (long long)send(fds[0], &header, sizeof(header), 0));

    uint8_t type = 0xFF;
    void *payload = (void *)0x1;
    uint16_t payload_len = 1234;

    EXPECT_EQ(0, emap_recv_packet(fds[1], &type, &payload, &payload_len));

    EXPECT_EQ(EMAP_MSG_GAME_WAIT, type);
    EXPECT_EQ(0, payload_len);
    EXPECT_TRUE(payload == NULL);

    close(fds[0]);
    close(fds[1]);
    return 0;
}

static int test_decode_player_info_endianness(void)
{
    const uint8_t level = 42;
    const uint16_t progression = 4321;
    const uint16_t money = 999;
    const uint8_t selected_skin = 7;

    uint8_t payload[1 + 2 + 2 + 1 + TOTAL_SKIN_AMOUNT];
    payload[0] = level;
    payload[1] = (uint8_t)(progression >> 8);
    payload[2] = (uint8_t)(progression & 0xFF);
    payload[3] = (uint8_t)(money >> 8);
    payload[4] = (uint8_t)(money & 0xFF);
    payload[5] = selected_skin;
    for (size_t i = 0; i < TOTAL_SKIN_AMOUNT; ++i)
    {
        payload[6 + i] = (uint8_t)i;
    }

    EMAP_Message message;
    EXPECT_EQ(0, emap_decode_msg(EMAP_MSG_PLAYER_INFO, payload, sizeof(payload), &message));

    EXPECT_EQ(EMAP_MSG_PLAYER_INFO, message.type);
    EXPECT_EQ(level, message.data.player_info.level);
    EXPECT_EQ(progression, message.data.player_info.progression);
    EXPECT_EQ(money, message.data.player_info.money);
    EXPECT_EQ(selected_skin, message.data.player_info.selected_skin);
    EXPECT_MEMEQ(&payload[6], message.data.player_info.possessed_skins, TOTAL_SKIN_AMOUNT);

    return 0;
}

static int test_decode_general_ack(void)
{
    const uint8_t payload[] = {EMAP_MESSAGE_ACK_OK};
    EMAP_Message message;
    EXPECT_EQ(0, emap_decode_msg(EMAP_MSG_GENERAL_ACK, payload, sizeof(payload), &message));

    EXPECT_EQ(EMAP_MSG_GENERAL_ACK, message.type);
    EXPECT_EQ(EMAP_MESSAGE_ACK_OK, message.data.general_ack.ack_type);
    return 0;
}

static int test_send_all_basic(void)
{
    int fds[2];
    EXPECT_EQ(0, socketpair(AF_UNIX, SOCK_STREAM, 0, fds));

    const char payload[] = "Hello EMAP";
    ssize_t sent = emap_send_all(fds[0], payload, strlen(payload));
    EXPECT_EQ((long long)strlen(payload), sent);

    char buffer[32] = {0};
    ssize_t received = recv(fds[1], buffer, strlen(payload), 0);
    EXPECT_EQ((long long)strlen(payload), received);
    EXPECT_MEMEQ(payload, buffer, strlen(payload));

    close(fds[0]);
    close(fds[1]);
    return 0;
}

static int test_recv_all_basic(void)
{
    int fds[2];
    EXPECT_EQ(0, socketpair(AF_UNIX, SOCK_STREAM, 0, fds));

    const char payload[] = "Awaiting data";
    EXPECT_EQ((long long)strlen(payload), (long long)send(fds[0], payload, strlen(payload), 0));

    char buffer[32] = {0};
    ssize_t received = emap_recv_all(fds[1], buffer, strlen(payload));
    EXPECT_EQ((long long)strlen(payload), received);
    EXPECT_MEMEQ(payload, buffer, strlen(payload));

    close(fds[0]);
    close(fds[1]);
    return 0;
}

static int test_encode_general_ack(void)
{
    EMAP_Message message = {
        .type = EMAP_MSG_GENERAL_ACK,
        .data.general_ack.ack_type = EMAP_MESSAGE_ACK_OK};

    void *out = NULL;
    uint16_t out_len = 0;
    EXPECT_EQ(0, emap_encode_msg(EMAP_MSG_GENERAL_ACK, &message, &out, &out_len));

    EXPECT_TRUE(out != NULL);
    EXPECT_EQ(1, out_len);
    EXPECT_EQ(EMAP_MESSAGE_ACK_OK, *(uint8_t *)out);

    free(out);
    return 0;
}

static int test_encode_move(void)
{
    EMAP_Message message = {
        .type = EMAP_MSG_MOVE,
        .data.move.coin_index = 5,
        .data.move.coin_pos = 3};

    void *out = NULL;
    uint16_t out_len = 0;
    EXPECT_EQ(0, emap_encode_msg(EMAP_MSG_MOVE, &message, &out, &out_len));

    EXPECT_TRUE(out != NULL);
    EXPECT_EQ(2, out_len);
    uint8_t *buffer = (uint8_t *)out;
    EXPECT_EQ(5, buffer[0]);
    EXPECT_EQ(3, buffer[1]);

    free(out);
    return 0;
}

static int test_encode_play(void)
{
    EMAP_Message message = {
        .type = EMAP_MSG_PLAY,
        .data.play.play_type = EMAP_PLAY_YOUR_TURN};

    void *out = NULL;
    uint16_t out_len = 0;
    EXPECT_EQ(0, emap_encode_msg(EMAP_MSG_PLAY, &message, &out, &out_len));

    EXPECT_TRUE(out != NULL);
    EXPECT_EQ(1, out_len);
    EXPECT_EQ(EMAP_PLAY_YOUR_TURN, *(uint8_t *)out);

    free(out);
    return 0;
}

static int test_encode_player_info_endianness(void)
{
    EMAP_Message message = {
        .type = EMAP_MSG_PLAYER_INFO,
        .data.player_info.level = 42,
        .data.player_info.progression = 4321,
        .data.player_info.money = 999,
        .data.player_info.selected_skin = 3};

    for (size_t i = 0; i < TOTAL_SKIN_AMOUNT; ++i)
    {
        message.data.player_info.possessed_skins[i] = (uint8_t)i;
    }

    void *out = NULL;
    uint16_t out_len = 0;
    EXPECT_EQ(0, emap_encode_msg(EMAP_MSG_PLAYER_INFO, &message, &out, &out_len));

    EXPECT_TRUE(out != NULL);
    EXPECT_EQ(1 + 2 + 2 + 1 + TOTAL_SKIN_AMOUNT, out_len);

    uint8_t *buffer = (uint8_t *)out;
    EXPECT_EQ(42, buffer[0]);

    uint16_t progression_net;
    uint16_t money_net;
    memcpy(&progression_net, &buffer[1], sizeof(progression_net));
    memcpy(&money_net, &buffer[3], sizeof(money_net));

    EXPECT_EQ(4321, ntohs(progression_net));
    EXPECT_EQ(999, ntohs(money_net));
    EXPECT_EQ(3, buffer[5]);
    EXPECT_MEMEQ(&buffer[6], message.data.player_info.possessed_skins, TOTAL_SKIN_AMOUNT);

    free(out);
    return 0;
}

static int test_encode_connect(void)
{
    EMAP_Message message = {.type = EMAP_MSG_CONNECT};
    const char *username = "TestPlayer";
    memset(message.data.connect.username, 0, MAX_PLAYER_USERNAME_SIZE);
    strncpy((char *)message.data.connect.username, username, MAX_PLAYER_USERNAME_SIZE - 1);

    void *out = NULL;
    uint16_t out_len = 0;
    EXPECT_EQ(0, emap_encode_msg(EMAP_MSG_CONNECT, &message, &out, &out_len));

    EXPECT_TRUE(out != NULL);
    EXPECT_EQ(MAX_PLAYER_USERNAME_SIZE, out_len);
    EXPECT_MEMEQ(message.data.connect.username, out, MAX_PLAYER_USERNAME_SIZE);

    free(out);
    return 0;
}

static int test_encode_game_start(void)
{
    EMAP_Message message = {
        .type = EMAP_MSG_GAME_START,
        .data.game_start.is_starting = 1};

    void *out = NULL;
    uint16_t out_len = 0;
    EXPECT_EQ(0, emap_encode_msg(EMAP_MSG_GAME_START, &message, &out, &out_len));

    EXPECT_TRUE(out != NULL);
    EXPECT_EQ(1, out_len);
    EXPECT_EQ(1, *(uint8_t *)out);

    free(out);
    return 0;
}

static int test_encode_game_end(void)
{
    EMAP_Message message = {
        .type = EMAP_MSG_GAME_END,
        .data.game_end.has_won = 1};

    void *out = NULL;
    uint16_t out_len = 0;
    EXPECT_EQ(0, emap_encode_msg(EMAP_MSG_GAME_END, &message, &out, &out_len));

    EXPECT_TRUE(out != NULL);
    EXPECT_EQ(1, out_len);
    EXPECT_EQ(1, *(uint8_t *)out);

    free(out);
    return 0;
}

static int test_encode_game_wait(void)
{
    EMAP_Message message = {.type = EMAP_MSG_GAME_WAIT};

    void *out = NULL;
    uint16_t out_len = 0;
    EXPECT_EQ(0, emap_encode_msg(EMAP_MSG_GAME_WAIT, &message, &out, &out_len));

    EXPECT_TRUE(out == NULL);
    EXPECT_EQ(0, out_len);

    return 0;
}

static int test_encode_game_state(void)
{
    EMAP_Message message = {.type = EMAP_MSG_GAME_STATE};

    void *out = NULL;
    uint16_t out_len = 0;
    EXPECT_EQ(0, emap_encode_msg(EMAP_MSG_GAME_STATE, &message, &out, &out_len));

    EXPECT_TRUE(out == NULL);
    EXPECT_EQ(0, out_len);

    return 0;
}

static int test_encode_game_join(void)
{
    EMAP_Message message = {.type = EMAP_MSG_GAME_JOIN};

    void *out = NULL;
    uint16_t out_len = 0;
    EXPECT_EQ(0, emap_encode_msg(EMAP_MSG_GAME_JOIN, &message, &out, &out_len));

    EXPECT_TRUE(out == NULL);
    EXPECT_EQ(0, out_len);

    return 0;
}

static int test_encode_shop_action(void)
{
    EMAP_Message message = {
        .type = EMAP_MSG_SHOP_ACTION,
        .data.shop_action.action = EMAP_SHOP_ACTION_EQUIP,
        .data.shop_action.skin_index = 4};

    void *out = NULL;
    uint16_t out_len = 0;
    EXPECT_EQ(0, emap_encode_msg(EMAP_MSG_SHOP_ACTION, &message, &out, &out_len));

    EXPECT_TRUE(out != NULL);
    EXPECT_EQ((int)sizeof(EMAP_PayloadShopAction), out_len);
    uint8_t *buffer = (uint8_t *)out;
    EXPECT_EQ(EMAP_SHOP_ACTION_EQUIP, buffer[0]);
    EXPECT_EQ(4, buffer[1]);

    free(out);
    return 0;
}

static int test_encode_rejects_null_message(void)
{
    void *out = NULL;
    uint16_t out_len = 0;
    int rc = emap_encode_msg(EMAP_MSG_MOVE, NULL, &out, &out_len);
    EXPECT_EQ(-1, rc);

    return 0;
}

static int test_encode_rejects_null_out_ptr(void)
{
    EMAP_Message message = {.type = EMAP_MSG_MOVE};
    uint16_t out_len = 0;
    int rc = emap_encode_msg(EMAP_MSG_MOVE, &message, NULL, &out_len);
    EXPECT_EQ(-1, rc);

    return 0;
}

static int test_encode_rejects_null_out_len_ptr(void)
{
    EMAP_Message message = {.type = EMAP_MSG_MOVE};
    void *out = NULL;
    int rc = emap_encode_msg(EMAP_MSG_MOVE, &message, &out, NULL);
    EXPECT_EQ(-1, rc);

    return 0;
}

static int test_encode_decode_roundtrip_move(void)
{
    EMAP_Message original = {
        .type = EMAP_MSG_MOVE,
        .data.move.coin_index = 7,
        .data.move.coin_pos = 2};

    void *encoded = NULL;
    uint16_t encoded_len = 0;
    EXPECT_EQ(0, emap_encode_msg(EMAP_MSG_MOVE, &original, &encoded, &encoded_len));

    EMAP_Message decoded;
    EXPECT_EQ(0, emap_decode_msg(EMAP_MSG_MOVE, encoded, encoded_len, &decoded));

    EXPECT_EQ(original.data.move.coin_index, decoded.data.move.coin_index);
    EXPECT_EQ(original.data.move.coin_pos, decoded.data.move.coin_pos);

    free(encoded);
    return 0;
}

static int test_encode_decode_roundtrip_player_info(void)
{
    EMAP_Message original = {
        .type = EMAP_MSG_PLAYER_INFO,
        .data.player_info.level = 15,
        .data.player_info.progression = 5000,
        .data.player_info.money = 2500,
        .data.player_info.selected_skin = 4};

    for (size_t i = 0; i < TOTAL_SKIN_AMOUNT; ++i)
    {
        original.data.player_info.possessed_skins[i] = (uint8_t)(i * 2);
    }

    void *encoded = NULL;
    uint16_t encoded_len = 0;
    EXPECT_EQ(0, emap_encode_msg(EMAP_MSG_PLAYER_INFO, &original, &encoded, &encoded_len));

    EMAP_Message decoded;
    EXPECT_EQ(0, emap_decode_msg(EMAP_MSG_PLAYER_INFO, encoded, encoded_len, &decoded));

    EXPECT_EQ(original.data.player_info.level, decoded.data.player_info.level);
    EXPECT_EQ(original.data.player_info.progression, decoded.data.player_info.progression);
    EXPECT_EQ(original.data.player_info.money, decoded.data.player_info.money);
    EXPECT_EQ(original.data.player_info.selected_skin, decoded.data.player_info.selected_skin);
    EXPECT_MEMEQ(original.data.player_info.possessed_skins,
                 decoded.data.player_info.possessed_skins,
                 TOTAL_SKIN_AMOUNT);

    free(encoded);
    return 0;
}

static int test_decode_move_invalid_length(void)
{
    const uint8_t payload[] = {0xAA}; /* Only 1 byte, but MOVE expects 2 */
    EMAP_Message message;
    int rc = emap_decode_msg(EMAP_MSG_MOVE, payload, sizeof(payload), &message);
    EXPECT_EQ(-1, rc);

    return 0;
}

static int test_decode_play_message(void)
{
    const uint8_t payload[] = {EMAP_PLAY_YOUR_TURN};
    EMAP_Message message;
    EXPECT_EQ(0, emap_decode_msg(EMAP_MSG_PLAY, payload, sizeof(payload), &message));

    EXPECT_EQ(EMAP_MSG_PLAY, message.type);
    EXPECT_EQ(EMAP_PLAY_YOUR_TURN, message.data.play.play_type);

    return 0;
}

static int test_decode_move_message(void)
{
    const uint8_t payload[] = {10, 20};
    EMAP_Message message;
    EXPECT_EQ(0, emap_decode_msg(EMAP_MSG_MOVE, payload, sizeof(payload), &message));

    EXPECT_EQ(EMAP_MSG_MOVE, message.type);
    EXPECT_EQ(10, message.data.move.coin_index);
    EXPECT_EQ(20, message.data.move.coin_pos);

    return 0;
}

static int test_decode_connect_message(void)
{
    const char *username = "PlayerOne";
    uint8_t payload[MAX_PLAYER_USERNAME_SIZE];
    memset(payload, 0, MAX_PLAYER_USERNAME_SIZE);
    strncpy((char *)payload, username, MAX_PLAYER_USERNAME_SIZE - 1);

    EMAP_Message message;
    EXPECT_EQ(0, emap_decode_msg(EMAP_MSG_CONNECT, payload, MAX_PLAYER_USERNAME_SIZE, &message));

    EXPECT_EQ(EMAP_MSG_CONNECT, message.type);
    EXPECT_MEMEQ(username, message.data.connect.username, strlen(username));

    return 0;
}

static int test_decode_connect_oversized_payload(void)
{
    uint8_t payload[MAX_PLAYER_USERNAME_SIZE + 1];
    memset(payload, 'A', sizeof(payload));

    EMAP_Message message;
    int rc = emap_decode_msg(EMAP_MSG_CONNECT, payload, sizeof(payload), &message);
    EXPECT_EQ(-1, rc);

    return 0;
}

static int test_decode_game_start_message(void)
{
    const uint8_t payload[] = {1};
    EMAP_Message message;
    EXPECT_EQ(0, emap_decode_msg(EMAP_MSG_GAME_START, payload, sizeof(payload), &message));

    EXPECT_EQ(EMAP_MSG_GAME_START, message.type);
    EXPECT_EQ(1, message.data.game_start.is_starting);

    return 0;
}

static int test_decode_game_end_message(void)
{
    const uint8_t payload[] = {1};
    EMAP_Message message;
    EXPECT_EQ(0, emap_decode_msg(EMAP_MSG_GAME_END, payload, sizeof(payload), &message));

    EXPECT_EQ(EMAP_MSG_GAME_END, message.type);
    EXPECT_EQ(1, message.data.game_end.has_won);

    return 0;
}

static int test_decode_game_wait_message(void)
{
    const uint8_t payload[] = {};
    EMAP_Message message;
    EXPECT_EQ(0, emap_decode_msg(EMAP_MSG_GAME_WAIT, payload, 0, &message));

    EXPECT_EQ(EMAP_MSG_GAME_WAIT, message.type);

    return 0;
}

static int test_decode_game_wait_invalid_payload(void)
{
    const uint8_t payload[] = {0x00};
    EMAP_Message message;
    int rc = emap_decode_msg(EMAP_MSG_GAME_WAIT, payload, sizeof(payload), &message);
    EXPECT_EQ(-1, rc);

    return 0;
}

static int test_decode_game_state_message(void)
{
    const uint8_t payload[] = {};
    EMAP_Message message;
    EXPECT_EQ(0, emap_decode_msg(EMAP_MSG_GAME_STATE, payload, 0, &message));

    EXPECT_EQ(EMAP_MSG_GAME_STATE, message.type);

    return 0;
}

static int test_decode_game_join_message(void)
{
    const uint8_t payload[] = {};
    EMAP_Message message;
    EXPECT_EQ(0, emap_decode_msg(EMAP_MSG_GAME_JOIN, payload, 0, &message));

    EXPECT_EQ(EMAP_MSG_GAME_JOIN, message.type);

    return 0;
}

static int test_decode_shop_action_message(void)
{
    const uint8_t payload[] = {EMAP_SHOP_ACTION_PURCHASE, 5};
    EMAP_Message message;
    EXPECT_EQ(0, emap_decode_msg(EMAP_MSG_SHOP_ACTION, payload, sizeof(payload), &message));

    EXPECT_EQ(EMAP_MSG_SHOP_ACTION, message.type);
    EXPECT_EQ(EMAP_SHOP_ACTION_PURCHASE, message.data.shop_action.action);
    EXPECT_EQ(5, message.data.shop_action.skin_index);

    return 0;
}

static int test_decode_shop_action_invalid_length(void)
{
    const uint8_t payload[] = {EMAP_SHOP_ACTION_EQUIP};
    EMAP_Message message;
    int rc = emap_decode_msg(EMAP_MSG_SHOP_ACTION, payload, sizeof(payload), &message);
    EXPECT_EQ(-1, rc);

    return 0;
}

static int test_decode_general_ack_invalid_length(void)
{
    const uint8_t payload[] = {}; /* Empty, but ACK expects 1 byte */
    EMAP_Message message;
    int rc = emap_decode_msg(EMAP_MSG_GENERAL_ACK, payload, 0, &message);
    EXPECT_EQ(-1, rc);

    return 0;
}

static int test_decode_player_info_invalid_length(void)
{
    const uint8_t payload[] = {0x42, 0x10, 0xE5}; /* Too short */
    EMAP_Message message;
    int rc = emap_decode_msg(EMAP_MSG_PLAYER_INFO, payload, sizeof(payload), &message);
    EXPECT_EQ(-1, rc);

    return 0;
}

static int test_decode_player_info_correct_length(void)
{
    const size_t expected_len = 1 + 2 + 2 + 1 + TOTAL_SKIN_AMOUNT;
    uint8_t payload[expected_len];
    payload[0] = 25;                        /* level */
    payload[1] = 0x12;
    payload[2] = 0x34;                      /* progression in network byte order */
    payload[3] = 0x56;
    payload[4] = 0x78;                      /* money in network byte order */
    payload[5] = 0x02;                      /* selected skin */
    for (size_t i = 0; i < TOTAL_SKIN_AMOUNT; ++i)
    {
        payload[6 + i] = (uint8_t)i;
    }

    EMAP_Message message;
    EXPECT_EQ(0, emap_decode_msg(EMAP_MSG_PLAYER_INFO, payload, sizeof(payload), &message));

    EXPECT_EQ(EMAP_MSG_PLAYER_INFO, message.type);
    EXPECT_EQ(25, message.data.player_info.level);
    EXPECT_EQ(0x1234, message.data.player_info.progression);
    EXPECT_EQ(0x5678, message.data.player_info.money);
    EXPECT_EQ(0x02, message.data.player_info.selected_skin);
    EXPECT_MEMEQ(&payload[6], message.data.player_info.possessed_skins, TOTAL_SKIN_AMOUNT);

    return 0;
}

static int test_decode_play_invalid_length(void)
{
    const uint8_t payload[] = {}; /* Empty, but PLAY expects 1 byte */
    EMAP_Message message;
    int rc = emap_decode_msg(EMAP_MSG_PLAY, payload, 0, &message);
    EXPECT_EQ(-1, rc);

    return 0;
}

static int test_decode_game_start_invalid_length(void)
{
    const uint8_t payload[] = {}; /* Empty, but GAME_START expects 1 byte */
    EMAP_Message message;
    int rc = emap_decode_msg(EMAP_MSG_GAME_START, payload, 0, &message);
    EXPECT_EQ(-1, rc);

    return 0;
}

static int test_decode_game_end_invalid_length(void)
{
    const uint8_t payload[] = {}; /* Empty, but GAME_END expects 1 byte */
    EMAP_Message message;
    int rc = emap_decode_msg(EMAP_MSG_GAME_END, payload, 0, &message);
    EXPECT_EQ(-1, rc);

    return 0;
}

static int test_decode_unknown_message_type(void)
{
    const uint8_t payload[] = {0xAA, 0xBB};
    EMAP_Message message;
    int rc = emap_decode_msg(255, payload, sizeof(payload), &message); /* Invalid type */
    EXPECT_EQ(-1, rc);

    return 0;
}

typedef int (*test_fn)(void);

typedef struct TestCase
{
    const char *name;
    test_fn fn;
} TestCase;

int main(void)
{
    const TestCase tests[] = {
        {"emap_send_packet rejects null payload", test_send_packet_rejects_null_payload},
        {"emap_send_packet/emap_recv_packet roundtrip", test_send_receive_roundtrip},
        {"emap_recv_packet rejects oversized payload", test_recv_rejects_oversized_payload},
        {"emap_recv_packet handles zero-length payload", test_recv_zero_length_payload},
        {"emap_decode_msg handles player info endianness", test_decode_player_info_endianness},
        {"emap_decode_msg handles general ack", test_decode_general_ack},
        {"emap_send_all sends full buffer", test_send_all_basic},
        {"emap_recv_all receives full buffer", test_recv_all_basic},
        {"emap_encode_msg encodes general ack", test_encode_general_ack},
        {"emap_encode_msg encodes move", test_encode_move},
        {"emap_encode_msg encodes play", test_encode_play},
        {"emap_encode_msg encodes player info with endianness", test_encode_player_info_endianness},
        {"emap_encode_msg encodes connect", test_encode_connect},
        {"emap_encode_msg encodes game start", test_encode_game_start},
        {"emap_encode_msg encodes game end", test_encode_game_end},
        {"emap_encode_msg encodes game wait", test_encode_game_wait},
        {"emap_encode_msg encodes game state", test_encode_game_state},
        {"emap_encode_msg encodes game join", test_encode_game_join},
        {"emap_encode_msg encodes shop action", test_encode_shop_action},
        {"emap_encode_msg rejects null message", test_encode_rejects_null_message},
        {"emap_encode_msg rejects null out pointer", test_encode_rejects_null_out_ptr},
        {"emap_encode_msg rejects null out_len pointer", test_encode_rejects_null_out_len_ptr},
        {"emap_encode_msg/emap_decode_msg roundtrip move", test_encode_decode_roundtrip_move},
        {"emap_encode_msg/emap_decode_msg roundtrip player info", test_encode_decode_roundtrip_player_info},
        {"emap_decode_msg rejects invalid move length", test_decode_move_invalid_length},
        {"emap_decode_msg decodes play message", test_decode_play_message},
        {"emap_decode_msg decodes move message", test_decode_move_message},
        {"emap_decode_msg decodes connect message", test_decode_connect_message},
        {"emap_decode_msg rejects oversized connect", test_decode_connect_oversized_payload},
        {"emap_decode_msg decodes game start message", test_decode_game_start_message},
        {"emap_decode_msg decodes game end message", test_decode_game_end_message},
        {"emap_decode_msg decodes game wait message", test_decode_game_wait_message},
        {"emap_decode_msg rejects invalid game wait", test_decode_game_wait_invalid_payload},
        {"emap_decode_msg decodes game state message", test_decode_game_state_message},
        {"emap_decode_msg decodes game join message", test_decode_game_join_message},
        {"emap_decode_msg decodes shop action message", test_decode_shop_action_message},
        {"emap_decode_msg rejects invalid shop action length", test_decode_shop_action_invalid_length},
        {"emap_decode_msg rejects invalid ack length", test_decode_general_ack_invalid_length},
        {"emap_decode_msg rejects invalid player info length", test_decode_player_info_invalid_length},
        {"emap_decode_msg decodes player info with correct length", test_decode_player_info_correct_length},
        {"emap_decode_msg rejects invalid play length", test_decode_play_invalid_length},
        {"emap_decode_msg rejects invalid game start length", test_decode_game_start_invalid_length},
        {"emap_decode_msg rejects invalid game end length", test_decode_game_end_invalid_length},
        {"emap_decode_msg rejects unknown message type", test_decode_unknown_message_type},
    };

    int failures = 0;
    const size_t total = sizeof(tests) / sizeof(tests[0]);

    for (size_t i = 0; i < total; ++i)
    {
        const TestCase *test = &tests[i];
        int result = test->fn();
        if (result == 0)
        {
            printf("[  OK  ] %s\n", test->name);
        }
        else
        {
            printf("[ FAIL ] %s\n", test->name);
            ++failures;
        }
    }

    if (failures != 0)
    {
        printf("%d of %zu tests failed\n", failures, total);
        return 1;
    }

    printf("All %zu tests passed\n", total);
    return 0;
}
