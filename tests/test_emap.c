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

    uint8_t payload[1 + 2 + 2 + TOTAL_SKIN_AMOUNT];
    payload[0] = level;
    payload[1] = (uint8_t)(progression >> 8);
    payload[2] = (uint8_t)(progression & 0xFF);
    payload[3] = (uint8_t)(money >> 8);
    payload[4] = (uint8_t)(money & 0xFF);
    for (size_t i = 0; i < TOTAL_SKIN_AMOUNT; ++i)
    {
        payload[5 + i] = (uint8_t)i;
    }

    EMAP_Message message;
    EXPECT_EQ(0, emap_decode_msg(EMAP_MSG_PLAYER_INFO, payload, sizeof(payload), &message));

    EXPECT_EQ(EMAP_MSG_PLAYER_INFO, message.type);
    EXPECT_EQ(level, message.data.player_info.level);
    EXPECT_EQ(progression, message.data.player_info.progression);
    EXPECT_EQ(money, message.data.player_info.money);
    EXPECT_MEMEQ(&payload[5], message.data.player_info.possessed_skins, TOTAL_SKIN_AMOUNT);

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
