/*
 * libtlv - Support Library for EMV TLV handling.
 * Copyright (C) 2015 Michael Jung <mijung@gmx.net>, All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3.0 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 */

/**
 * @file
 * TLV Parser Library
 */

#ifndef __TLV_H__
#define __TLV_H__

#include <stdint.h>
#include <stdlib.h>

#define TLV_RC_OK				0
#define TLV_RC_INVALID_ARG			1
#define TLV_RC_BUFFER_OVERFLOW			2
#define TLV_RC_TAG_NUMBER_TOO_LARGE		3
#define TLV_RC_OUT_OF_MEMORY			4
#define TLV_RC_INDEFINITE_LENGTH_NOT_SUPPORTED	5
#define TLV_RC_VALUE_LENGTH_TOO_LARGE		6

#define TLV_TAG_CLASS_UNIVERSAL		0x00u
#define TLV_TAG_CLASS_APPLICATION	0x40u
#define TLV_TAG_CLASS_CONTEXT_SPECIFIC	0x80u
#define TLV_TAG_CLASS_PRIVATE		0xC0u

struct tlv;

/**
 * Parse DER-TLV encoded data into a TLV data structure
 *
 * @param[in]  buffer  The DER-TLV encoded data to parse.
 * @param[in]  size    Length of the DER-TLV encoded data.
 * @param[out] tlv     The corresponding TLV data structure.
 *
 * @return TLV_RC_OK on success. Other TLV_RC_* codes on failure.
 */
int tlv_parse(const void *buffer, size_t size, struct tlv **tlv);

/**
 * Encode a TLV data structure into a DER-TLV byte stream
 *
 * @param[in]    tlv     The corresponding TLV data structure to encode.
 * @param[in]    buffer  The buffer to store the encoded byte stream in.
 * @param[inout] size    Input: Size of buffer, Output Length of byte stream.
 *
 * @return TLV_RC_OK on success. TLV_RC_BUFFER_OVERFLOW is buffer is too small.
 *         size will hold the required buffer size in this case. Other TLV_RC_*
 *         codes on failure.
 */
int tlv_encode(struct tlv *tlv, void *buffer, size_t *size);

/**
 * Free resources allocated by a TLV data structure
 *
 * @param[in]  tlv  The corresponding TLV data structure to free.
 */
void tlv_free(struct tlv *tlv);

/**
 * Returns the DER encoded identifier (tag) octets of the TLV node.
 *
 * If the buffer parameter is NULL then only the size of the encoded tag will
 * be computed and returned in the size output parameter.
 *
 * @param[in]    tlv     TLV node whose identifier (tag) shall be DER encoded.
 * @param[in]    buffer  Buffer to put the encoded identifier in.
 * @param[inout] size    Size of buffer on input, size of encoded tag on output.
 *
 * @return TLV_RC_OK on success. Other TLV_RC_* codes on failure.
 */
int tlv_encode_identifier(struct tlv *tlv, void *buffer, size_t *size);

/**
 * Returns the DER encoded length octets of the TLV node.
 *
 * If the buffer parameter is NULL then only the size of the encoded length will
 * be computed and returned in the size output parameter.
 *
 * @param[in]    tlv     TLV node whose length shall be DER encoded.
 * @param[in]    buffer  Buffer to put the encoded length in.
 * @param[inout] size    Size of buffer on input, size of encoding on output.
 *
 * @return TLV_RC_OK on success. Other TLV_RC_* codes on failure.
 */
int tlv_encode_length(struct tlv *tlv, void *buffer, size_t *size);

/**
 * Returns the value octets of the TLV node.
 *
 * For constructed TLV nodes no value is returned (I.e. the value of the size
 * paramter will be set to zero).
 *
 * If the buffer parameter is NULL then only the size of the value will
 * be computed and returned in the size output parameter.
 *
 * @param[in]    tlv     TLV node whose value shall be returned.
 * @param[in]    buffer  Buffer to put the value in.
 * @param[inout] size    Size of buffer on input, size of value on output.
 *
 * @return TLV_RC_OK on success. Other TLV_RC_* codes on failure.
 */
int tlv_encode_value(struct tlv *tlv, void *buffer, size_t *size);

/**
 * Returns whether a TLV node is constructed or primitive.
 *
 * @param[in]  tlv  TLV node to determine whether its constructed or primitive.
 *
 * @returns 1 if TLV node is contructed, 0 if TLV node is primitive.
 */
int tlv_is_constructed(struct tlv *tlv);

/**
 * Get the TLV node after the current TLV node. Elements of constructed TLV
 * nodes skipped.
 *
 * @param[in]  tlv  The current TLV node.
 *
 * @returns The TLV node after the current TLV node or NULL if there is none.
 */
struct tlv *tlv_get_next(struct tlv *tlv);

/**
 * Get the constructed TLV node which the current TLV node is an element of.
 *
 * @param[in]  tlv  The current TLV node.
 *
 * @returns The constructed TLV node or NULL if there is none.
 */
struct tlv *tlv_get_parent(struct tlv *tlv);

/**
 * Get the first element of a constructed TLV node.
 *
 * @param[in]  tlv  The constructed TLV node.
 *
 * @returns The first element of the constructed TLV node or NULL if there is
 *            none.
 */
struct tlv *tlv_get_child(struct tlv *tlv);

#endif /* ndef __TLV_H__ */
