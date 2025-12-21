#ifndef EMAP_HIGH_LEVEL_H
#define EMAP_HIGH_LEVEL_H

/**
 * @file emap_high_level.h
 * @brief High-level helpers responsible for framing and decoding EMAP packets.
 */

#include "emap_message.h"
#include "types.h"

/**
 * @brief Serialize and send an EMAP packet over a stream socket.
 *
 * Builds a @ref EMAP_PacketHeader from the provided parameters and streams both
 * the header and the payload using @ref emap_send_all(). Payload length
 * validation is delegated to the caller.
 *
 * @param sock Connected stream socket descriptor.
 * @param msg_type EMAP message type identifier to encode in the packet header.
 * @param payload Pointer to the payload bytes, or @c NULL when @p payload_len
 * is zero.
 * @param payload_len Number of payload bytes to send.
 * @return 0 on success, -1 on error (errno is preserved by @ref
 * emap_send_all()).
 */
int emap_send_packet(int sock, uint8_t msg_type, const void* payload,
					 uint16_t payload_len);

/**
 * @brief Receive and validate an EMAP packet from a stream socket.
 *
 * The function blocks until a full header is gathered, validates the magic
 * bytes and version, and reads the payload if one is present. On success,
 * ownership of the returned payload buffer is transferred to the caller, who
 * must free it with
 * @c free().
 *
 * @param sock Connected stream socket descriptor.
 * @param type_out Output pointer updated with the decoded message type.
 * @param payload_out Output pointer filled with a heap-allocated buffer or set
 * to @c NULL when empty.
 * @param payload_len_out Output pointer receiving the payload length in bytes.
 * @return 0 on success, -1 on validation or transport error.
 *
 * @post @ref emap_recv_packet must be called after on payload_out to get a
 * usable EMAP struct.
 *
 */
int emap_recv_packet(int sock, uint8_t* type_out, void** payload_out,
					 uint16_t* payload_len_out);

/**
 * @brief Decode a raw payload according to its EMAP message type.
 *
 * Converts raw network-ordered fields to host endianness and populates an
 * @ref EMAP_Message structure that can be consumed by the gameplay layer.
 * Validation is type-specific: any mismatch in payload length returns -1.
 *
 * @param msg_type_raw Message type identifier read from the packet header.
 * @param payload Pointer to the raw payload bytes (may be @c NULL when @p
 * payload_len is zero).
 * @param payload_len Payload size in bytes.
 * @param out Destination structure populated on success.
 * @return 0 on success, -1 when the payload is malformed or the type is
 * unknown.
 */
int emap_decode_msg(uint8_t msg_type_raw, const void* payload,
					uint16_t payload_len, EMAP_Message* out);

/**
 * @brief Encode an EMAP message into a contiguous binary buffer.
 *
 * This function serializes an EMAP_Message according to the EMAP protocol
 * and returns a newly allocated buffer containing the encoded representation.
 * The buffer is suitable for transmission or storage and its length is
 * provided in bytes.
 *
 * @param[in] msg_type  Type of the message; determines the encoding rules
 *                      applied to the @p message parameter.
 * @param[in] message   Pointer to the message structure to encode. Must be
 *                      a valid, fully-initialized EMAP_Message appropriate
 *                      for @p msg_type. The function does not take ownership
 *                      of this pointer.
 * @param[out] out      Pointer to a variable that will receive the address
 *                      of the newly allocated encoded buffer on success.
 *                      On success, *out points to heap memory that the caller
 *                      is responsible for freeing. The allocation failure or
 *                      an encoding error leaves *out unchanged (caller must
 *                      check the return value before using or freeing it).
 * @param[out] out_len  Pointer to a uint16_t that will be set to the length
 *                      in bytes of the encoded buffer on success. Must not be
 * NULL.
 *
 * @pre  @p out and @p out_len must be non-NULL. @p message must be non-NULL
 *       and valid for the specified @p msg_type.
 * @post On success, *out points to an allocated buffer of length *out_len
 *       containing the encoded message. The caller is responsible for freeing
 *       this buffer.
 *
 * @return 0 on success; non-zero error code on failure.
 *         Error codes indicate invalid parameters, allocation failures, or
 *         encoding errors. The exact error values are implementation-defined;
 *         callers should check for non-zero returns and handle errors
 *         appropriately.
 */
int emap_encode_msg(EMAP_MessageType msg_type, EMAP_Message* message,
					void** out, uint16_t* out_len);

#endif