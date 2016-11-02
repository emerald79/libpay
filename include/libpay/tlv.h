/*
 * LibPAY - The Toolkit for Smart Payment Applications
 *
 * Copyright (C) 2015, 2016  Michael Jung <mijung@gmx.net>
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
 * @defgroup LIBTLV libTLV
 *
 * @brief TLV Parser library

 * @code
 * #include <libpay/tlv.h>
 * @endcode
 */

/**
 * @addtogroup LIBTLV
 * @{
 */

/**
 * @file
 */

#ifndef __LIBPAY__TLV_H__
#define __LIBPAY__TLV_H__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

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
 * @brief Parse DER-TLV encoded data into a TLV data structure
 *
 * @param[in]  buffer  The DER-TLV encoded data to parse.
 * @param[in]  size    Length of the DER-TLV encoded data.
 * @param[out] tlv     The corresponding TLV data structure.
 *
 * @return TLV_RC_OK on success. Other TLV_RC_* codes on failure.
 */
int tlv_parse(const void *buffer, size_t size, struct tlv **tlv);

/**
 * @brief Parse DER-TLV encoded data into a TLV data structure, treating constructed
 * tags as primitive.
 *
 * Think twice before you use this function.  It violates the EMV TLV parsing
 * rules, but is required in some very special cases such as implementing test
 * suites or card emulators which test broken cards.
 *
 * @param[in]  buffer  The DER-TLV encoded data to parse.
 * @param[in]  size    Length of the DER-TLV encoded data.
 * @param[out] tlv     The corresponding TLV data structure.
 *
 * @return TLV_RC_OK on success. Other TLV_RC_* codes on failure.
 */
int tlv_shallow_parse(const void *buffer, size_t size, struct tlv **tlv);

/**
 * @brief Encode a TLV data structure into a DER-TLV byte stream
 *
 * @param[in]    tlv     The corresponding TLV data structure to encode.
 * @param[out]   buffer  The buffer to store the encoded byte stream in.
 * @param[inout] size    Input: Size of buffer, Output Length of byte stream.
 *
 * If buffer is NULL the output length of the byte stream will be put into size
 * and tlv_encode will return with TLV_RC_OK. I.e for dynamically allocated
 * buffers you would call tlv_encode twice: Once to determine the required size
 * of the buffer and a second time to actually encode the TLV data structure.
 *
 * @return TLV_RC_OK on success. TLV_RC_BUFFER_OVERFLOW is buffer is too small.
 *         size will hold the required buffer size in this case. Other TLV_RC_*
 *         codes on failure.
 */
int tlv_encode(const struct tlv *tlv, void *buffer, size_t *size);

/**
 * @brief Create a new TLV node.
 *
 * @param[in]  tag     The tag of the new TLV node.
 * @param[in]  length  The length of the value of the new TLV node.
 * @param[in]  value   The value of the new TLV node.
 *
 * @return A pointer to the new TLV node if successful. NULL otherwise.
 */
struct tlv *tlv_new(const void *tag, size_t length, const void *value);

/**
 * @brief Create a deep copy of a TLV data structure.
 *
 * @param[in]  tlv  The TLV data structure to create a deep copy from.
 *
 * @return A deep copy of the provided TLV data structure.
 */
struct tlv *tlv_copy(const struct tlv *tlv);

/**
 * @brief Overwrite the identifier (aka tag) of a TLV node.
 *
 * @param[in]  tlv  The TLV node whose identifier shall be set to a new value.
 * @param[in]  tag  The value to set the TLV node's identifier to.
 *
 * @return The TLV node that was provided is passed through.
 */
struct tlv *tlv_set_identifier(struct tlv *tlv, const void *tag);

/**
 * @brief Overwrite the value of a primitive type TLV node.
 *
 * @attention This function might free the TLV node that is passed to it and
 *		return a newly created TLV node.  Take care to not use dangling
 *		pointers to the old TLV node after having called this function.
 *		The pointers in all the neighbor, child and parent nodes will be
 *		updated automatically in this case, though.
 *
 * @note This function only works for primitive type TLV nodes. If called on a
 *	   constructed TLV node, this function will fail and return a NULL
 *	   pointer.
 *
 * @param[in]  tlv     The TLV node whose value shall be set.  Note that his TLV
 *			 node might get de-allocated.
 * @param[in]  length  The length of the new value of the TLV node.
 * @param[in]  value   The new value of the TLV node.
 *
 * @return Success: A pointer to a TLV node with the new value. This might
 *	     either be the TLV node that was passed, or it might be a newly
 *	     created TLV node.
 * @return Failure: NULL (if called on a constructed TLV node).
 */
struct tlv *tlv_set_value(struct tlv *tlv, size_t length, const void *value);

/**
 * @brief Unlink a TLV data structure from its surrounding.
 *
 * If the provided TLV data stucture is a pure root node (I.e. it has neither a
 * parent nor siblings) this is a no-op.  Otherwise the provided TLV data
 * structure is unhinged from the larger TLV data structure it is embedded
 * in and thus made a new pure root node.
 *
 * Do not forget to eventually call tlv_free on an unlink'ed TLV data structure,
 * as naturally it wont be free'd when tlv_free is called on the previously
 * embedding TLV data structure any more.
 *
 * @param[in]  tlv  The TLV data structure to segragate.
 *
 * @return The TLV node that was provided is passed through.
 */
struct tlv *tlv_unlink(struct tlv *tlv);

/**
 * @brief Free resources allocated by a TLV data structure
 *
 * This is a deep free.  I.e. it recursively frees all next siblings (I.e. those
 * returned by tlv_get_next) and all children (I.e. those returned by
 * tlv_get_child).
 *
 * @param[in]  tlv  The corresponding TLV data structure to free.
 */
void tlv_free(struct tlv *tlv);

/**
 * @brief Returns the DER encoded identifier (tag) octets of the TLV node.
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
int tlv_encode_identifier(const struct tlv *tlv, void *buffer, size_t *size);

/**
 * @brief Returns the DER encoded length octets of the TLV node.
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
 * @brief Returns the value octets of the TLV node.
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
int tlv_encode_value(const struct tlv *tlv, void *buffer, size_t *size);

/**
 * @brief Returns whether a TLV node is constructed or primitive.
 *
 * @param[in]  tlv  TLV node to determine whether its constructed or primitive.
 *
 * @returns true if TLV node is contructed, false if TLV node is primitive.
 */
bool tlv_is_constructed(const struct tlv *tlv);

/**
 * @brief Get the constructed TLV node which the current TLV node is an element of.
 *
 * @param[in]  tlv  The current TLV node.
 *
 * @returns The constructed TLV node or NULL if there is none.
 */
struct tlv *tlv_get_parent(const struct tlv *tlv);

/**
 * @brief Get the first element of a constructed TLV node.
 *
 * @param[in]  tlv  The constructed TLV node.
 *
 * @returns The first element of the constructed TLV node or NULL if there is
 *            none.
 */
struct tlv *tlv_get_child(const struct tlv *tlv);

/**
 * @brief Get the TLV node after the current TLV node. Elements of constructed TLV
 * nodes skipped.
 *
 * @param[in]  tlv  The current TLV node.
 *
 * @returns The TLV node after the current TLV node or NULL if there is none.
 */
struct tlv *tlv_get_next(const struct tlv *tlv);

/**
 * @brief Get the TLV node before the current TLV node. Elements of constructed TLV
 * nodes skipped.
 *
 * @param[in]  tlv  The current TLV node.
 *
 * @returns The TLV node before the current TLV node or NULL if there is none.
 */
struct tlv *tlv_get_prev(const struct tlv *tlv);

/**
 * @brief Shallow search for a TLV node with a given specific tag.
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
 * @brief Iterate through all TLV nodes in depth first order.
 *
 * @param[in]  tlv  The current TLV node.
 *
 * @returns The next TLV node in depth first order.
 */
struct tlv *tlv_iterate(struct tlv *tlv);

/**
 * @brief Deep search for a TLV node with a given specific tag.
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
 * @brief Get the number of ancestors a given TLV node has. I.e. the number of parent
 * nodes until the root node is found.
 *
 * @param[in]  tlv  The TLV node whose depth is to be queried.
 *
 * @returns The depth of the given TLV node.
 */
int tlv_get_depth(struct tlv *tlv);

/**
 * @brief Insert one TLV structure after a TLV node
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
 * @brief Insert a TLV structure as a child of another constructed TLV node.
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
 * @brief Construct a Data Element List (DEL) from a Data Object List (DOL) and a
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
 * @brief Construct a list of TLV node from a Data Object List (DOL) and a Data Element
 * list.
 *
 * See section 'Rules for Using a Data Object List (DOL)' in EMV v4.3 Book 3.
 */
int dol_and_del_to_tlv(const void *dol, size_t dol_sz,
			      const void *del, size_t del_sz, struct tlv **tlv);

void libtlv_get_dol_field(const void *tag, const void *in, size_t in_sz,
						      void *out, size_t out_sz);

size_t libtlv_get_tag_length(const void *tag);

const void *dol_tok(const void **dol, size_t *dol_sz);
const void *dol_find_tag(const void *dol, size_t dol_sz, const void *tag);

/**
 * @brief Convert a BCD encoded value into a 64 bit wide unsigned integer.
 */
int libtlv_bcd_to_u64(const void *bcd, size_t len, uint64_t *u64);

/**
 * @brief Convert a 64 bit wide unsigned integer into a BCD encoded value.
 */
int libtlv_u64_to_bcd(uint64_t u64, void *bcd, size_t len);

/**
 * @brief Format a binary string into a hexdecimal (ASCII coded) string.
 *
 * @param[in]  bin     Pointer to the binary string to encode in hex.
 * @param[in]  bin_sz  Size of the binary string in bytes.
 * @param[out] hex     Buffer to hold the hex encoded ASCII string. NOTE: This
 *			 buffer must be at least 2 * bin_sz + 1 bytes large.
 *
 * @returns The ASCII encoded hex string.
 */
char *libtlv_bin_to_hex(const void *bin, size_t bin_sz, char *hex);

/**
 * @brief Convert a hexadecimal (ASCII coded) string into its binary
 *	representation.
 *
 * @param[in]    hex	Pointer to the hex (ASCII) string to decode in binary.
 * @param[out]   bin	Buffer to hold the binary representation of the decoded
 *			 ASCII string. NOTE: This buffer must be at least
 * 			 0.5 times the length of the ascii string large.
 * @param[inout] bin_sz	Pointer to the size of the binary buffer, contains the
 *			 size of the binary representation on return.
 *
 * @returns The binary representation of the ASCII encoded hex string
 * 			or NULL on conversion error.
 */
void *libtlv_hex_to_bin(const char *hex, void *bin, size_t *bin_sz);

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

/**
 * @}
 */


#ifdef __cplusplus
}
#endif

#endif						    /* ndef __LIBPAY__TLV_H__ */
