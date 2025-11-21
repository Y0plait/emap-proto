#ifndef EMAP_HIGH_LEVEL_H
#define EMAP_HIGH_LEVEL_H

#include "../include/types.h"
#include "../include/emap_message.h"

int emap_send_packet(int sock, uint8_t msg_type, const void *payload, uint16_t payload_len);
int emap_recv_packet(int sock, uint8_t *type_out, void **payload_out, uint16_t *payload_len_out);
int emap_decode_msg(uint8_t msg_type_raw, const void *payload, uint16_t payload_len, EMAP_Message *out);


#endif