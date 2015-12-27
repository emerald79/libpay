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
#include <stdbool.h>

#define TLV_MAX_TAG_LENGTH			8

#define TLV_RC_OK				0
#define TLV_RC_INVALID_ARG			1
#define TLV_RC_BUFFER_OVERFLOW			2
#define TLV_RC_TAG_NUMBER_TOO_LARGE		3
#define TLV_RC_OUT_OF_MEMORY			4
#define TLV_RC_INDEFINITE_LENGTH_NOT_SUPPORTED	5
#define TLV_RC_VALUE_LENGTH_TOO_LARGE		6
#define TLV_RC_IO_ERROR				7
#define TLV_RC_VALUE_OUT_OF_RANGE		8
#define TLV_RC_UNEXPECTED_END_OF_STREAM		9

/**
 * Structure that represents a complex TLV structure. I.e. for example a single
 * TLV node, a list of TLV nodes, or a constructed TLV node with child nodes.
 */
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
int tlv_encode(const struct tlv *tlv, void *buffer, size_t *size);

/**
 * Create a new TLV node.
 *
 * @param[in]  tag     The tag of the new TLV node.
 * @param[in]  length  The length of the value of the new TLV node.
 * @param[in]  value   The value of the new TLV node.
 *
 * @return A pointer to the new TLV node if successful. NULL otherwise.
 */
struct tlv *tlv_new(const void *tag, size_t length, const void *value);

struct tlv *tlv_copy(const struct tlv *tlv);

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
int tlv_encode_length(const struct tlv *tlv, void *buffer, size_t *size);

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
 * @returns true if TLV node is contructed, false if TLV node is primitive.
 */
bool tlv_is_constructed(const struct tlv *tlv);

/**
 * Get the constructed TLV node which the current TLV node is an element of.
 *
 * @param[in]  tlv  The current TLV node.
 *
 * @returns The constructed TLV node or NULL if there is none.
 */
struct tlv *tlv_get_parent(const struct tlv *tlv);

/**
 * Get the first element of a constructed TLV node.
 *
 * @param[in]  tlv  The constructed TLV node.
 *
 * @returns The first element of the constructed TLV node or NULL if there is
 *            none.
 */
struct tlv *tlv_get_child(const struct tlv *tlv);

/**
 * Get the TLV node after the current TLV node. Elements of constructed TLV
 * nodes skipped.
 *
 * @param[in]  tlv  The current TLV node.
 *
 * @returns The TLV node after the current TLV node or NULL if there is none.
 */
struct tlv *tlv_get_next(const struct tlv *tlv);

struct tlv *tlv_get_prev(const struct tlv *tlv);

/**
 * Shallow search for a TLV node with a given specific tag.
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
 * Iterate through all TLV nodes in depth first order.
 *
 * @param[in]  tlv  The current TLV node.
 *
 * @returns The next TLV node in depth first order.
 */
struct tlv *tlv_iterate(struct tlv *tlv);

/**
 * Deep search for a TLV node with a given specific tag.
 *
 * Note that descendant nodes will be searched order.
 *
 * @param[in]  tlv  The list of TLV nodes to search.
 * @param[in]  tag  The tag to search for.
 *
 * @returns The first occurence of a TLV node with the given tag or NULL if
 *          there is none.
 */
struct tlv *tlv_deep_find(struct tlv *tlv, const void *tag);

/**
 * Get the number of ancestors a given TLV node has. I.e. the number of parent
 * nodes until the root node is found.
 *
 * @param[in]  tlv  The TLV node whose depth is to be queried.
 *
 * @returns The depth of the given TLV node.
 */
int tlv_get_depth(struct tlv *tlv);

/**
 * Insert one TLV structure after a TLV node
 *
 * Note: If tlv1 is part of a TLV node list, and if tlv1 is not the last element
 * of this list, then tlv2 will be inserted between tlv1 and the remainder of
 * the list.
 *
 * @param[in]  tlv1  TLV node behind which the TLV structure shall be inserted.
 * @param[in]  tlv2  The TLV structure to be inserted.
 *
 * @returns tlv2 on success, NULL on error.
 */
struct tlv *tlv_insert_after(struct tlv *tlv1, struct tlv *tlv2);

/**
 * Insert a TLV structure as a child of another constructed TLV node.
 *
 * Note: If parent already has childs, then the new child will be added as the
 * head of the list of childs.
 *
 * @param[in]  parent  The constructed TLV node to which a child TLV node shall
 *			 be added.
 * @param[in]  child   The TLV node to add as a child TLV node to parent.
 *
 * @returns child on success, NULL on error.
 */
struct tlv *tlv_insert_below(struct tlv *parent, struct tlv *child);

/**
 * Construct a Data Element List (DEL) from a Data Object List (DOL) and a
 * corresponding TLV list.
 *
 * See section 'Rules for Using a Data Object List (DOL)' in EMV v4.3 Book 3.
 *
 * @param[in]	 tlv	 A TLV encoded list of data elements to fetch the values
 *			   from.
 * @param[in]	 dol	 The Data Object List that identifies the order and size
 *			   of data object values to concatenate.
 * @param[in]	 dol_sz	 Size of the Data Object List in bytes.
 * @param[out]	 del	 The concatenated value fields (Data Element List).
 * @param[inout] del_sz	 On input: The size of the output buffer. On output:
 *			       The length of the DEL in bytes.
 */
int tlv_and_dol_to_del(struct tlv *tlv, const void *dol,
				      size_t dol_sz, void *del, size_t *del_sz);

/**
 * Construct a list of TLV node from a Data Object List (DOL) and a Data Element
 * list.
 *
 * See section 'Rules for Using a Data Object List (DOL)' in EMV v4.3 Book 3.
 */
int dol_and_del_to_tlv(const void *dol, size_t dol_sz,
			      const void *del, size_t del_sz, struct tlv **tlv);

void libtlv_get_dol_field(const void *tag, const void *in, size_t in_sz,
						      void *out, size_t out_sz);

size_t libtlv_get_tag_length(const void *tag);

/**
 * Convert a BCD encoded value into a 64 bit wide unsigned integer.
 */
int libtlv_bcd_to_u64(const void *bcd, size_t len, uint64_t *u64);

/**
 * Convert a 64 bit wide unsigned integer into a BCD encoded value.
 */
int libtlv_u64_to_bcd(uint64_t u64, void *bcd, size_t len);

/**
 * Format a binary string into a hexdecimal (ASCII coded) string.
 *
 * @param[in]  bin     Pointer to the binary string to encode in hex.
 * @param[in]  bin_sz  Size of the binary string in bytes.
 * @param[out] hex     Buffer to hold the hex encoded ASCII string. NOTE: This
 *			 buffer must be at least 2 * bin_sz + 1 bytes large.
 *
 * @returns The ASCII encoded hex string.
 */
char *libtlv_bin_to_hex(const void *bin, size_t bin_sz, char *hex);

enum tlv_fmt {
	fmt_a,
	fmt_an,
	fmt_ans,
	fmt_b,
	fmt_cn,
	fmt_n,
	fmt_var,
	fmt_unknown
};

struct tlv_id_to_fmt {
	const void  *id;
	enum tlv_fmt fmt;
};

int libtlv_register_fmts(const struct tlv_id_to_fmt *fmts);
void libtlv_free_fmts(void);

enum tlv_fmt libtlv_id_to_fmt(const void *id);

void libtlv_init(const char *log4c_category);

#endif /* ndef __TLV_H__ */
