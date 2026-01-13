#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>

#include "../include/constants.h"
#include "../include/emap_low_level.h"
#include "../include/emap_message.h"
#include "../include/types.h"

int emap_send_packet(int sock, uint8_t msg_type, const void* payload,
					 uint16_t payload_len) {
	if (payload_len > 0 && payload == NULL) {
		return -1;
	}

	/* Craft packet header */
	EMAP_PacketHeader packet_header;
	packet_header.version = EMAP_PROTO_VERSION;
	packet_header.magic_byte[0] = EMAP_MAGIC_BYTE_0;
	packet_header.magic_byte[1] = EMAP_MAGIC_BYTE_1;
	packet_header.code = msg_type;
	packet_header.size = htons(payload_len); /* htonl if uint32_t */

	/* Send header */
	if (emap_send_all(sock, &packet_header, sizeof(packet_header)) <= 0) {
		return -1;
	}

	/* Send payload */
	if (payload_len > 0 && payload != NULL) {
		if (emap_send_all(sock, payload, payload_len) <= 0) {
			return -1;
		}
	}

	return 0;
}

int emap_recv_packet(int sock, uint8_t* type_out, void** payload_out,
					 uint16_t* payload_len_out) {

	/* Packet header struct to store incoming header data */
	EMAP_PacketHeader in_packet_header;

	/* Receive all from the socket, blocking call ! */
	ssize_t con_state =
		emap_recv_all(sock, &in_packet_header, sizeof(in_packet_header));
	if (con_state <= 0) {
		return -1;
	}

	/* Magic bytes matching test */
	if (in_packet_header.magic_byte[0] != EMAP_MAGIC_BYTE_0 ||
		in_packet_header.magic_byte[1] != EMAP_MAGIC_BYTE_1) {
		return -1;
	}
	/* Version test */
	if (in_packet_header.version != EMAP_PROTO_VERSION) {
		return -1;
	}

	/* Big/Little-endian shenanigans */
	uint16_t payload_len = ntohs(in_packet_header.size); /* ntohl if uint32_t */

	if (payload_len > EMAP_MAX_PAYLOAD_SIZE) {
		return -1;
	}

	*type_out = in_packet_header.code;
	*payload_len_out = payload_len;
	*payload_out = NULL;

	/* By default no payload */
	uint8_t* payload = NULL;
	if (payload_len > 0) {
		payload = malloc(payload_len);
		if (!payload) {
			return -1;
		}

		con_state = emap_recv_all(sock, payload, payload_len);
		if (con_state <= 0) {
			free(payload);
			return -1;
		}

		*payload_out = payload;
	}
	return 0;
}

int emap_decode_msg(uint8_t msg_type_raw, const void* payload,
					uint16_t payload_len, EMAP_Message* out) {

	const uint8_t* p = payload;
	EMAP_MessageType type = (EMAP_MessageType)msg_type_raw;

	out->type = type;

	/* Match header code to the payload struct  */
	switch (type) {
	case EMAP_MSG_GENERAL_ACK: {
		/* Payload: 1 byte -> ack_type (0/1) */
		if (payload_len != sizeof(EMAP_PayloadGeneralACK)) {
			return -1;
		}
		out->data.general_ack.ack_type = p[0]; /* 0 or 1 */
		break;
	}

	case EMAP_MSG_CONNECT: {
		/* Payload: fixed-size username buffer */
		if (payload_len != sizeof(EMAP_PayloadConnect)) {
			return -1;
		}
		memcpy(out->data.connect.username, p, MAX_PLAYER_USERNAME_SIZE);
		break;
	}

	case EMAP_MSG_MOVE: {
		/* Payload: 2 bytes: coin_index, coin_pos */
		if (payload_len != sizeof(EMAP_PayloadMove)) {
			return -1;
		}
		out->data.move.coin_index = p[0];
		out->data.move.coin_pos = p[1];
		break;
	}

	case EMAP_MSG_PLAY: {
		/* Payload: 1 byte: EMAP_PlayType (0/1) */
		if (payload_len != sizeof(EMAP_PayloadPlay)) {
			return -1;
		}
		/* On the wire we store 0/1. Interpret as EMAP_PlayType if you want. */
		out->data.play.play_type = (EMAP_PlayType)p[0];
		break;
	}

	case EMAP_MSG_PLAYER_INFO:
	case EMAP_MSG_PLAYER_DATA_UPDATE: {
		/* On wire:
		 *  byte 0: level (uint8_t)
		 *  byte 1-2: progression (uint16_t, network order)
		 *  byte 3-4: money       (uint16_t, network order)
		 *  byte 5: selected_skin (uint8_t)
		 *  remaining bytes: possessed_skins[]
		 */
		const size_t expected_len = 1	/* level */
									+ 2 /* progression */
									+ 2 /* money */
									+ 1 /* selected_skin */
									+ TOTAL_SKIN_AMOUNT;

		if (payload_len != expected_len) {
			return -1;
		}

		out->data.player_info.level = p[0];

		uint16_t progression_net;
		uint16_t money_net;

		memcpy(&progression_net, &p[1], sizeof(progression_net));
		memcpy(&money_net, &p[3], sizeof(money_net));
		out->data.player_info.progression = ntohs(progression_net);
		out->data.player_info.money = ntohs(money_net);

		out->data.player_info.selected_skin = p[5];

		memcpy(out->data.player_info.possessed_skins, &p[6], TOTAL_SKIN_AMOUNT);

		break;
	}

	case EMAP_MSG_GAME_START: {
		/* Payload: 1 byte is_starting (0/1) */
		if (payload_len != sizeof(EMAP_PayloadGameStart)) {
			return -1;
		}
		out->data.game_start.is_starting = p[0]; /* EMAP_PlayType logically */
		break;
	}

	case EMAP_MSG_GAME_END: {
		/* Payload: 1 byte has_won (0/1) */
		if (payload_len != sizeof(EMAP_PayloadGameEnd)) {
			return -1;
		}
		out->data.game_end.has_won = p[0]; /* EMAP_EndGameType logically */
		break;
	}

	case EMAP_MSG_GAME_WAIT: {
		/* No payload expected */
		if (payload_len != 0) {
			return -1;
		}
		/* Nothing to fill */
		break;
	}

	case EMAP_MSG_GAME_STATE: {
		/* For now, empty; later you’ll define a proper layout */
		if (payload_len != sizeof(EMAP_PayloadGameState)) {
			/* If GameState has 0 bytes, this check is payload_len != 0 */
			return -1;
		}
		break;
	}

	case EMAP_MSG_GAME_JOIN: {
		/* No payload */
		if (payload_len != sizeof(EMAP_PayloadGameJoin)) {
			return -1;
		}
		break;
	}

	case EMAP_MSG_ILLEGAL_PLAY: {

		/* Define when you have a payload format */
		if (payload_len != sizeof(EMAP_PayloadIllegalPlay)) {
			return -1; /* or handle properly */
		}
		break;
	}

	default:
		/* Unknown type */
		return -1;
	}

	return 0;
}

int emap_encode_msg(EMAP_MessageType msg_type, EMAP_Message* message,
					void** out, uint16_t* out_len) {
	if (!message || !out || !out_len) {
		return -1;
	}

	uint8_t* buffer = NULL;
	uint16_t buffer_len = 0;

	/* Match message type to encode the payload struct */
	switch (msg_type) {
	case EMAP_MSG_GENERAL_ACK: {
		/* Payload: 1 byte -> ack_type (0/1) */
		buffer_len = sizeof(EMAP_PayloadGeneralACK);
		buffer = malloc(buffer_len);
		if (!buffer) {
			return -1;
		}
		buffer[0] = message->data.general_ack.ack_type;
		break;
	}

	case EMAP_MSG_MOVE: {
		/* Payload: 2 bytes: coin_index, coin_pos */
		buffer_len = sizeof(EMAP_PayloadMove);
		buffer = malloc(buffer_len);
		if (!buffer) {
			return -1;
		}
		buffer[0] = message->data.move.coin_index;
		buffer[1] = message->data.move.coin_pos;
		break;
	}

	case EMAP_MSG_PLAY: {
		/* Payload: 1 byte: EMAP_PlayType (0/1) */
		buffer_len = sizeof(EMAP_PayloadPlay);
		buffer = malloc(buffer_len);
		if (!buffer) {
			return -1;
		}
		buffer[0] = message->data.play.play_type;
		break;
	}

	case EMAP_MSG_PLAYER_INFO:
	case EMAP_MSG_PLAYER_DATA_UPDATE: {
		/* On wire:
		 *  byte 0: level (uint8_t)
		 *  byte 1-2: progression (uint16_t, network order)
		 *  byte 3-4: money       (uint16_t, network order)
		 *  byte 5: selected_skin (uint8_t)
		 *  byte 6..(6+TOTAL_SKIN_AMOUNT-1): possessed_skins[]
		 */
		buffer_len = 1 + 2 + 2 + 1 + TOTAL_SKIN_AMOUNT;
		buffer = malloc(buffer_len);
		if (!buffer) {
			return -1;
		}

		buffer[0] = message->data.player_info.level;

		uint16_t progression_net = htons(message->data.player_info.progression);
		uint16_t money_net = htons(message->data.player_info.money);

		memcpy(&buffer[1], &progression_net, sizeof(progression_net));
		memcpy(&buffer[3], &money_net, sizeof(money_net));
		buffer[5] = message->data.player_info.selected_skin;

		memcpy(&buffer[6], message->data.player_info.possessed_skins,
			   TOTAL_SKIN_AMOUNT);

		break;
	}

	case EMAP_MSG_CONNECT: {
		/* Payload: username is fixed-size MAX_PLAYER_USERNAME_SIZE bytes */
		buffer_len = MAX_PLAYER_USERNAME_SIZE;
		buffer = malloc(buffer_len);
		if (!buffer) {
			return -1;
		}
		memcpy(buffer, message->data.connect.username,
			   MAX_PLAYER_USERNAME_SIZE);
		break;
	}

	case EMAP_MSG_GAME_START: {
		/* Payload: 1 byte is_starting (0/1) */
		buffer_len = sizeof(EMAP_PayloadGameStart);
		buffer = malloc(buffer_len);
		if (!buffer) {
			return -1;
		}
		buffer[0] = message->data.game_start.is_starting;
		break;
	}

	case EMAP_MSG_GAME_END: {
		/* Payload: 1 byte has_won (0/1) */
		buffer_len = sizeof(EMAP_PayloadGameEnd);
		buffer = malloc(buffer_len);
		if (!buffer) {
			return -1;
		}
		buffer[0] = message->data.game_end.has_won;
		break;
	}

	case EMAP_MSG_GAME_WAIT: {
		/* No payload */
		buffer_len = 0;
		buffer = NULL;
		break;
	}

	case EMAP_MSG_GAME_STATE: {
		/* No payload expected */
		buffer_len = 0;
		buffer = NULL;
		break;
	}

	case EMAP_MSG_GAME_JOIN: {
		/* No payload */
		buffer_len = 0;
		buffer = NULL;
		break;
	}

	case EMAP_MSG_ILLEGAL_PLAY: {
		/* No payload for now */
		buffer_len = 0;
		buffer = NULL;
		break;
	}

	case EMAP_MSG_SHOP_ACTION: {
		buffer_len = sizeof(EMAP_PayloadShopAction);
		buffer = malloc(buffer_len);
		if (!buffer) {
			return -1;
		}
		buffer[0] = message->data.shop_action.action;
		buffer[1] = message->data.shop_action.skin_index;
		break;
	}

	default:
		/* Unknown type */
		return -1;
	}

	*out = buffer;
	*out_len = buffer_len;
	return 0;
}
