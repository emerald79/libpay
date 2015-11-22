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
#define TLV_RC_IO_ERROR				7

#define TLV_TAG_CLASS_UNIVERSAL		0x00u
#define TLV_TAG_CLASS_APPLICATION	0x40u
#define TLV_TAG_CLASS_CONTEXT_SPECIFIC	0x80u
#define TLV_TAG_CLASS_PRIVATE		0xC0u

#define TLV_ID_1(a)		   ((const uint8_t [1]) { a })
#define TLV_ID_2(a, b)		   ((const uint8_t [2]) { a, b })
#define TLV_ID_3(a, b, c)	   ((const uint8_t [3]) { a, b, c })
#define TLV_ID_4(a, b, c, d)	   ((const uint8_t [4]) { a, b, c, d })
#define TLV_ID_5(a, b, c, d, e)	   ((const uint8_t [5]) { a, b, c, d, e })
#define TLV_ID_6(a, b, c, d, e, f) ((const uint8_t [6]) { a, b, c, d, e, f })

#define TLV_ID_FCI_TEMPLATE			TLV_ID_1(0x6F)
#define TLV_ID_FCI_PROPRIETARY_TEMPLATE		TLV_ID_1(0xA5)
#define TLV_ID_FCI_ISSUER_DISCRETIONARY_DATA	TLV_ID_2(0xBF, 0x0C)
#define TLV_ID_DIRECTORY_ENTRY			TLV_ID_1(0x61)
#define TLV_ID_ADF_NAME				TLV_ID_1(0x4F)
#define TLV_ID_APPLICATION_LABEL		TLV_ID_1(0x50)
#define TLV_ID_APPLICATION_PRIORITY_INDICATOR	TLV_ID_1(0x87)
#define TLV_ID_KERNEL_IDENTIFIER		TLV_ID_2(0x9F, 0x2A)
#define TLV_ID_EXTENDED_SELECTION		TLV_ID_2(0x9F, 0x29)

#define TLV_ID_LIBEMV_CONFIGURATION		TLV_ID_4(0xFF, 0x81, 0xE3, 0x71)
#define TLV_ID_LIBEMV_COMBINATION_SET		TLV_ID_4(0xFF, 0x82, 0xE3, 0x71)
#define TLV_ID_LIBEMV_TRANSACTION_TYPE		TLV_ID_4(0xDF, 0x83, 0xE3, 0x71)
#define TLV_ID_LIBEMV_COMBINATION		TLV_ID_4(0xFF, 0x84, 0xE3, 0x71)
#define TLV_ID_LIBEMV_AID			TLV_ID_4(0xDF, 0x85, 0xE3, 0x71)
#define TLV_ID_LIBEMV_KERNEL_ID			TLV_ID_4(0xDF, 0x86, 0xE3, 0x71)
#define TLV_ID_LIBEMV_STATUS_CHECK_SUPPORTED	TLV_ID_4(0xDF, 0x87, 0xE3, 0x71)
#define TLV_ID_LIBEMV_ZERO_AMOUNT_ALLOWED	TLV_ID_4(0xDF, 0x88, 0xE3, 0x71)
#define TLV_ID_LIBEMV_EXT_SELECTION_SUPPORTED	TLV_ID_4(0xDF, 0x89, 0xE3, 0x71)
#define TLV_ID_LIBEMV_RDR_CTLS_TXN_LIMIT	TLV_ID_4(0xDF, 0x8A, 0xE3, 0x71)
#define TLV_ID_LIBEMV_RDR_CTLS_FLOOR_LIMIT	TLV_ID_4(0xDF, 0x8B, 0xE3, 0x71)
#define TLV_ID_LIBEMV_TERMINAL_FLOOR_LIMIT	TLV_ID_4(0xDF, 0x8C, 0xE3, 0x71)
#define TLV_ID_LIBEMV_RDR_CVM_REQUIRED_LIMIT	TLV_ID_4(0xDF, 0x8D, 0xE3, 0x71)
#define TLV_ID_LIBEMV_TTQ			TLV_ID_4(0xDF, 0x8E, 0xE3, 0x71)

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
 * @param[out]   buffer  The buffer to store the encoded byte stream in.
 * @param[inout] size    Input: Size of buffer, Output Length of byte stream.
 *
 * @return TLV_RC_OK on success. TLV_RC_BUFFER_OVERFLOW is buffer is too small.
 *         size will hold the required buffer size in this case. Other TLV_RC_*
 *         codes on failure.
 */
int tlv_encode(struct tlv *tlv, void *buffer, size_t *size);

struct tlv *tlv_new(const void *tag, size_t length, const void *value);

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
 * @param[out]   buffer  Buffer to put the encoded identifier in.
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
 * @param[out]   buffer  Buffer to put the encoded length in.
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
 * @param[out]   buffer  Buffer to put the value in.
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

/**
 * Search for a TLV node with a given specific tag.
 *
 * Note that the descendant nodes of constructed TLV nodes will not be searched.
 *
 * @param[in]  tlv  The list of TLV nodes to search.
 * @param[in]  tag  The tag to search for.
 *
 * @returns The first occurence of a TLV node with the given tag or NULL if
 *          there is none.
 */
struct tlv *tlv_find(struct tlv *tlv, const void *tag);

/**
 * Insert one TLV structure after a TLV node
 *
 * @param[in]  tlv1  TLV node behind which the TLV structure shall be inserted.
 * @param[in]  tlv2  The TLV structure to be inserted.
 *
 * @return TLV_RC_OK on success. Other TLV_RC_* codes on failure.
 */
int tlv_insert_after(struct tlv *tlv1, struct tlv *tlv2);

int tlv_insert_below(struct tlv *parent, struct tlv *child);

#endif /* ndef __TLV_H__ */
