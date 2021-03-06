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

#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <log4c.h>

#include <libpay_core.h>
#include <libpay/tlv.h>

static log4c_category_t *log_cat;

struct tlv {
	struct tlv	*next;
	struct tlv	*prev;
	struct tlv	*parent;
	struct tlv	*child;

	uint8_t		tag[TLV_MAX_TAG_LENGTH];
	size_t		length;
	uint8_t		value[0];
};

struct tlv_parse_error_info {
	int	    rc;
	const void *pos;
};

static struct tlv_parse_error_info tlv_parse_error_info;

bool tlv_is_constructed(const struct tlv *tlv)
{
	return !!tlv->child;
}

struct tlv *tlv_get_next(const struct tlv *tlv)
{
	return tlv ? tlv->next : NULL;
}

struct tlv *tlv_get_prev(const struct tlv *tlv)
{
	return tlv ? tlv->prev : NULL;
}

struct tlv *tlv_get_parent(const struct tlv *tlv)
{
	return tlv ? tlv->parent : NULL;
}

struct tlv *tlv_get_child(const struct tlv *tlv)
{
	return tlv ? tlv->child : NULL;
}

static int tlv_parse_identifier(const void **buf, size_t len, struct tlv *tlv)
{
	const uint8_t *p = NULL;
	int i;

	if (!buf || !(*buf) || !tlv || !len)
		return TLV_RC_INVALID_ARG;

	p = (uint8_t *)*buf;

	tlv->tag[0] = *p++;

	if ((tlv->tag[0] & TLV_TAG_NUMBER_MASK) != 0x1Fu) {
		*buf = (const void *)p;
		return TLV_RC_OK;
	}

	for (i = 1; i < MIN(sizeof(tlv->tag), len); i++) {
		tlv->tag[i] = *p++;

		if (!(tlv->tag[i] & 0x80u))
			break;
	}

	if (i == sizeof(tlv->tag)) {
		tlv_parse_error_info.rc = TLV_RC_TAG_NUMBER_TOO_LARGE;
		tlv_parse_error_info.pos = *buf;
		return TLV_RC_TAG_NUMBER_TOO_LARGE;
	}

	if (i == len) {
		tlv_parse_error_info.rc = TLV_RC_UNEXPECTED_END_OF_STREAM;
		tlv_parse_error_info.pos = *buf;
		return TLV_RC_UNEXPECTED_END_OF_STREAM;
	}

	*buf = (const void *)p;

	return TLV_RC_OK;
}

static int tlv_parse_dol_length(const void **buffer, size_t length,
								struct tlv *tlv)
{
	const uint8_t *p = NULL;

	if (!buffer || !(*buffer) || !tlv)
		return TLV_RC_INVALID_ARG;

	if (!length) {
		tlv_parse_error_info.rc = TLV_RC_UNEXPECTED_END_OF_STREAM;
		tlv_parse_error_info.pos = *buffer;
		return TLV_RC_UNEXPECTED_END_OF_STREAM;
	}

	/* Per EMV v4.3 Book 3, Section 5.4 "Rules for Using a Data Object List
	 * (DOL)", the a data object lenght field is a "a one-byte length which
	 * represents the number of bytes the field". Note that this is
	 * different from a TLV length field and restricts the maximum length to
	 * 255. */
	p = (const uint8_t *)*buffer;
	tlv->length = (size_t)p[0];
	*buffer = (const void *)&p[1];

	return TLV_RC_OK;
}

static int tlv_parse_length(const void **buffer, size_t length, struct tlv *tlv)
{
	const uint8_t *p = NULL;
	int i, value_length;

	if (!buffer || !(*buffer) || !tlv)
		return TLV_RC_INVALID_ARG;

	if (!length) {
		tlv_parse_error_info.rc = TLV_RC_UNEXPECTED_END_OF_STREAM;
		tlv_parse_error_info.pos = *buffer;
		return TLV_RC_UNEXPECTED_END_OF_STREAM;
	}

	p = (const uint8_t *)*buffer;

	if (p[0] == 0x80u) {
		tlv_parse_error_info.rc =
					 TLV_RC_INDEFINITE_LENGTH_NOT_SUPPORTED;
		tlv_parse_error_info.pos = *buffer;
		return TLV_RC_INDEFINITE_LENGTH_NOT_SUPPORTED;
	}

	if (!(p[0] & 0x80u)) {
		tlv->length = (size_t)p[0];
		*buffer = (const void *)&p[1];
		return TLV_RC_OK;
	}

	value_length = (int)(p[0] & 0x7fu);

	if (value_length > (int)sizeof(size_t)) {
		tlv_parse_error_info.rc = TLV_RC_VALUE_LENGTH_TOO_LARGE;
		tlv_parse_error_info.pos = *buffer;
		return TLV_RC_VALUE_LENGTH_TOO_LARGE;
	}

	for (i = 0, tlv->length = 0; i < value_length; i++) {
		tlv->length <<= 8;
		tlv->length |= (size_t)p[1 + i];
	}

	*buffer = (const void *)&p[1 + value_length];
	return TLV_RC_OK;
}

static int tlv_parse_recursive(const void **buffer, size_t length,
			 struct tlv **tlv, struct tlv *prev, struct tlv *parent,
							     bool parse_shallow)
{
	struct tlv temp_tlv;
	const void *start = *buffer;
	ssize_t remaining = 0;
	int rc = TLV_RC_OK;

	if (!length)
		goto done;

	if (!buffer || !(*buffer) || !tlv) {
		log4c_category_log(log_cat, LOG4C_PRIORITY_NOTICE,
			    "%s(buffer: %p, *buffer: %p, length: %d, tlv: %p): "
					  "Invalid arguments", __func__, buffer,
				     buffer ? *buffer : NULL, (int)length, tlv);
		rc = TLV_RC_INVALID_ARG;
		goto done;
	}

	*tlv = NULL;

	/* EMV v4.3 Book 3: 'Before, between, or after TLV-coded data
	 * objects, '00' bytes without any meaning may occur (for example, due
	 * to erased or modified TLV-coded data objects).'		      */
	remaining = length;
	while ((remaining > 0) && (*((uint8_t *)*buffer) == '\x00')) {
		(*buffer)++;
		remaining--;
	}

	if (!remaining)
		goto done;

	memset(&temp_tlv, 0, sizeof(temp_tlv));
	temp_tlv.parent = parent;
	temp_tlv.prev   = prev;

	rc = tlv_parse_identifier(buffer, remaining, &temp_tlv);
	if (rc)
		goto done;

	remaining = length - (*buffer - start);
	rc = tlv_parse_length(buffer, remaining, &temp_tlv);
	if (rc)
		goto done;

	remaining = length - (*buffer - start);
	if (remaining < temp_tlv.length) {
		tlv_parse_error_info.rc = TLV_RC_UNEXPECTED_END_OF_STREAM;
		tlv_parse_error_info.pos = *buffer;
		rc = TLV_RC_OUT_OF_MEMORY;
		goto done;
	}

	*tlv = (struct tlv *)malloc(sizeof(struct tlv) + temp_tlv.length);
	if (!*tlv) {
		tlv_parse_error_info.rc = TLV_RC_OUT_OF_MEMORY;
		tlv_parse_error_info.pos = start;
		rc = TLV_RC_OUT_OF_MEMORY;
		goto done;
	}

	memcpy(*tlv, &temp_tlv, sizeof(struct tlv));

	if (!parse_shallow && ((*tlv)->tag[0] & TLV_TAG_P_C_MASK)) {
		rc = tlv_parse_recursive(buffer, (*tlv)->length, &(*tlv)->child,
						     NULL, *tlv, parse_shallow);
		if (rc != TLV_RC_OK) {
			free(*tlv);
			*tlv = NULL;
			goto done;
		}
	} else {
		remaining = length - (*buffer - start);
		if (remaining < (*tlv)->length) {
			tlv_parse_error_info.rc =
						TLV_RC_UNEXPECTED_END_OF_STREAM;
			tlv_parse_error_info.pos = start;
			rc = TLV_RC_UNEXPECTED_END_OF_STREAM;
			goto done;
		}

		memcpy((*tlv)->value, *buffer, (*tlv)->length);
		*buffer += (*tlv)->length;
	}


	/* EMV v4.3 Book 3: 'Before, between, or after TLV-coded data
	 * objects, '00' bytes without any meaning may occur (for example, due
	 * to erased or modified TLV-coded data objects).'		      */
	remaining = length - (*buffer - start);
	while ((remaining > 0) && (*((uint8_t *)*buffer) == '\x00')) {
		*buffer += 1;
		remaining--;
	}

	if (remaining > 0) {
		rc = tlv_parse_recursive(buffer, remaining,
				    &(*tlv)->next, *tlv, parent, parse_shallow);
		if (rc != TLV_RC_OK) {
			free(*tlv);
			*tlv = NULL;
			goto done;
		}
	}

done:
	if (rc != TLV_RC_OK) {
		if (start) {
			char hex[length * 2 + 1];

			log4c_category_log(log_cat, LOG4C_PRIORITY_NOTICE,
				"%s() failed! rc %d, length: %d, ber-tlv: '%s'",
						      __func__, rc, (int)length,
					 libtlv_bin_to_hex(start, length, hex));
		}
	}

	return rc;
}

struct tlv *tlv_set_identifier(struct tlv *tlv, const void *tag)
{
	int rc = TLV_RC_OK;

	rc = tlv_parse_identifier(&tag, libtlv_get_tag_length(tag), tlv);
	if (rc != TLV_RC_OK)
		goto error;

	return tlv;
error:
	return NULL;
}

struct tlv *tlv_set_value(struct tlv *tlv, size_t length, const void *value)
{
	struct tlv *tlv_old = tlv;

	if (!tlv)
		return NULL;

	if (tlv->tag[0] & TLV_TAG_P_C_MASK)	/* Constructed not supported. */
		return NULL;

	assert(!tlv->child);

	if (!length) {
		tlv->length = 0;
		return tlv;
	}

	if (!value)
		return NULL;

	tlv = realloc(tlv, sizeof(*tlv) + length);
	tlv->length = length;
	memcpy(tlv->value, value, length);

	if (tlv->next)
		tlv->next->prev = tlv;
	if (tlv->prev)
		tlv->prev->next = tlv;
	if (tlv->parent && tlv->parent->child == tlv_old)
		tlv->parent->child = tlv;

	return tlv;
}

struct tlv *tlv_unlink(struct tlv *tlv)
{
	if (!tlv)
		return tlv;

	if (tlv->parent && tlv->parent->child == tlv)
		tlv->parent->child = tlv->next;

	if (tlv->prev)
		tlv->prev->next = tlv->next;

	if (tlv->next)
		tlv->next->prev = tlv->prev;

	tlv->parent = NULL;
	tlv->prev = NULL;
	tlv->next = NULL;

	return tlv;
}

void tlv_free(struct tlv *tlv)
{
	struct tlv *current, *next;

	if (!tlv)
		return;

	/* Unlink from parent, if applicable.				      */
	if (tlv->parent && tlv->parent->child == tlv)
		tlv->parent->child = NULL;

	/* Unlink from siblings 'to-the-left', if applicable.		      */
	if (tlv->prev)
		tlv->prev->next = NULL;

	for (current = tlv; current; current = next) {
		if (current->child)
			tlv_free(current->child);

		next = current->next;
		free(current);
	}
}

int tlv_parse(const void *buffer, size_t length, struct tlv **tlv)
{
	const void *start = buffer;
	int rc = TLV_RC_OK;

	tlv_parse_error_info.rc = TLV_RC_OK;

	if (!length) {
		*tlv = NULL;
		goto done;
	}

	rc = tlv_parse_recursive(&buffer, length, tlv, NULL, NULL, false);
done:
	if (tlv_parse_error_info.rc != TLV_RC_OK) {
		char hex[length * 2 + 1];

		log4c_category_log(log_cat, LOG4C_PRIORITY_NOTICE,
			    "%s('%s') failed at offset %d with rc %d", __func__,
					  libtlv_bin_to_hex(start, length, hex),
					(int)(tlv_parse_error_info.pos - start),
						       tlv_parse_error_info.rc);
	}

	return rc;
}

int tlv_shallow_parse(const void *buffer, size_t length, struct tlv **tlv)
{
	const void *start = buffer;
	int rc = TLV_RC_OK;

	if (!length) {
		*tlv = NULL;
		goto done;
	}

	tlv_parse_error_info.rc = TLV_RC_OK;

	rc = tlv_parse_recursive(&buffer, length, tlv, NULL, NULL, true);
done:
	if (tlv_parse_error_info.rc != TLV_RC_OK) {
		char hex[length * 2 + 1];

		log4c_category_log(log_cat, LOG4C_PRIORITY_NOTICE,
			    "%s('%s') failed at offset %d with rc %d", __func__,
					  libtlv_bin_to_hex(start, length, hex),
					(int)(tlv_parse_error_info.pos - start),
						       tlv_parse_error_info.rc);
	}

	return rc;
}

static size_t tlv_get_encoded_identifier_size(const struct tlv *tlv)
{
	size_t i;

	if (!tlv) {
		log4c_category_log(log_cat, LOG4C_PRIORITY_ERROR,
				"%s(tlv: %p): Invalid argument", __func__, tlv);
		return -1;
	}

	if ((tlv->tag[0] & TLV_TAG_NUMBER_MASK) != 0x1Fu)
		return 1;

	for (i = 1; i < sizeof(tlv->tag); i++)
		if (!(tlv->tag[i] & 0x80))
			return i + 1;

	return -1;
}

static size_t tlv_get_encoded_length(const struct tlv *tlv);

static size_t tlv_get_encoded_length_size(const struct tlv *tlv)
{
	size_t length;

	if (tlv_is_constructed(tlv))
		length = tlv_get_encoded_length(tlv->child);
	else
		length = tlv->length;

	if (length < 0x80u)
		return 1;
	if (length < 0x100u)
		return 2;
	if (length < 0x10000u)
		return 3;
	if (length < 0x1000000u)
		return 4;
	return 5;
}

static size_t tlv_get_encoded_length(const struct tlv *tlv)
{
	size_t size;

	if (!tlv)
		return 0;

	size = tlv_get_encoded_identifier_size(tlv);
	size += tlv_get_encoded_length_size(tlv);

	if (tlv_is_constructed(tlv))
		size += tlv_get_encoded_length(tlv->child);
	else
		size += tlv->length;

	if (tlv->next)
		size += tlv_get_encoded_length(tlv->next);

	return size;
}

static size_t libtlv_copy_tag(void *buffer, size_t size, const void *tag)
{
	size_t tag_len;
	uint8_t *b = (uint8_t *)buffer;
	const uint8_t *t = (const uint8_t *)tag;

	if (!buffer || !size || !tag)
		return -1;

	b[0] = t[0];

	if ((t[0] & TLV_TAG_NUMBER_MASK) != 0x1Fu)
		return 1;

	for (tag_len = 1; tag_len < size; ) {
		b[tag_len] = t[tag_len];
		if (!(t[tag_len++] & 0x80u))
			break;
	}

	if (tag_len == size)
		return -1;

	return tag_len;
}

static void __tlv_encode_length(size_t length, void **buffer)
{
	uint8_t *p = (uint8_t *)*buffer;

	if (length < (size_t)0x80u) {
		p[0] = (uint8_t)(length & 0x7fu);
		*buffer = (void *)&p[1];
		return;
	}

	if (length < 0x100u) {
		p[0] = 0x81u;
		p[1] = (uint8_t)length;
		*buffer = (void *)&p[2];
		return;
	}

	if (length < 0x10000u) {
		p[0] = 0x82u;
		p[1] = (uint8_t)(length >> 8);
		p[2] = (uint8_t)length;
		*buffer = (void *)&p[3];
		return;
	}

	if (length < 0x1000000u) {
		p[0] = 0x83u;
		p[1] = (uint8_t)(length >> 16);
		p[2] = (uint8_t)(length >> 8);
		p[3] = (uint8_t)length;
		*buffer = (void *)&p[4];
		return;
	}

	p[0] = 0x84u;
	p[1] = (uint8_t)(length >> 24);
	p[2] = (uint8_t)(length >> 16);
	p[3] = (uint8_t)(length >> 8);
	p[4] = (uint8_t)length;
	*buffer = (void *)&p[5];
}

static void tlv_encode_recursive(const struct tlv *tlv, void **buffer)
{
	size_t tag_len = 0;

	if (!tlv)
		return;

	tag_len = libtlv_copy_tag(*buffer, sizeof(tlv->tag), tlv->tag);
	*buffer += tag_len;

	if (tlv_is_constructed(tlv)) {
		__tlv_encode_length(tlv_get_encoded_length(tlv->child), buffer);
		tlv_encode_recursive(tlv->child, buffer);
	} else {
		__tlv_encode_length(tlv->length, buffer);
		memcpy(*buffer, tlv->value, tlv->length);
		*buffer = (void *)(((uint8_t *)*buffer) + tlv->length);
	}

	if (tlv->next)
		tlv_encode_recursive(tlv->next, buffer);
}

int tlv_encode(const struct tlv *tlv, void *buffer, size_t *size)
{
	size_t encoded_size = 0;

	if (!size)
		return TLV_RC_INVALID_ARG;

	encoded_size = tlv_get_encoded_length(tlv);

	if (!buffer) {
		*size = encoded_size;
		return TLV_RC_OK;
	}

	if (encoded_size > *size) {
		*size = encoded_size;
		return TLV_RC_BUFFER_OVERFLOW;
	}

	*size = encoded_size;

	if (!buffer)
		return TLV_RC_INVALID_ARG;

	tlv_encode_recursive(tlv, &buffer);

	return TLV_RC_OK;
}

int tlv_encode_identifier(const struct tlv *tlv, void *buffer, size_t *size)
{
	size_t encoded_size = 0;

	if (!tlv || !size)
		return TLV_RC_INVALID_ARG;

	encoded_size = tlv_get_encoded_identifier_size(tlv);

	if (!buffer) {
		*size = encoded_size;
		return TLV_RC_OK;
	}

	if (encoded_size > *size) {
		*size = encoded_size;
		return TLV_RC_BUFFER_OVERFLOW;
	}

	*size = encoded_size;

	if (!buffer)
		return TLV_RC_INVALID_ARG;

	libtlv_copy_tag(buffer, encoded_size, tlv->tag);

	return TLV_RC_OK;
}

int tlv_encode_length(const struct tlv *tlv, void *buffer, size_t *size)
{
	size_t encoded_size = 0, length = 0;

	if (!tlv || !size)
		return TLV_RC_INVALID_ARG;

	encoded_size = tlv_get_encoded_length_size(tlv);

	if (!buffer) {
		*size = encoded_size;
		return TLV_RC_OK;
	}

	if (encoded_size > *size) {
		*size = encoded_size;
		return TLV_RC_BUFFER_OVERFLOW;
	}

	*size = encoded_size;

	if (!buffer)
		return TLV_RC_INVALID_ARG;

	if (tlv_is_constructed(tlv))
		length = tlv_get_encoded_length(tlv->child);
	else
		length = tlv->length;

	__tlv_encode_length(length, &buffer);

	return TLV_RC_OK;
}

int tlv_encode_value(const struct tlv *tlv, void *buffer, size_t *size)
{
	if (!tlv || !size)
		return TLV_RC_INVALID_ARG;

	if (tlv_is_constructed(tlv)) {
		*size = 0;
		return TLV_RC_OK;
	}

	if (!buffer) {
		*size = tlv->length;
		return TLV_RC_OK;
	}

	if (tlv->length > *size) {
		*size = tlv->length;
		return TLV_RC_BUFFER_OVERFLOW;
	}

	*size = tlv->length;
	memcpy(buffer, tlv->value, tlv->length);

	return TLV_RC_OK;
}

struct tlv *tlv_insert_after(struct tlv *tlv1, struct tlv *tlv2)
{
	struct tlv *tail_of_tlv2 = NULL;

	if (!tlv1)
		return tlv2;

	if (!tlv2)
		return tlv1;

	assert(!tlv2->prev);

	for (tail_of_tlv2 = tlv2; tail_of_tlv2->next; ) {
		assert(!tail_of_tlv2->parent);
		tail_of_tlv2->parent = tlv1->parent;
		tail_of_tlv2 = tail_of_tlv2->next;
	}

	tail_of_tlv2->next = tlv1->next;
	if (tail_of_tlv2->next)
		tail_of_tlv2->next->prev = tail_of_tlv2;

	tlv1->next = tlv2;
	tlv2->prev = tlv1;

	return tlv2;
}

struct tlv *tlv_insert_below(struct tlv *parent, struct tlv *child)
{
	struct tlv *tail_of_child = NULL;

	if (!parent || !child)
		return NULL;

	assert(!child->prev);

	for (tail_of_child = child; tail_of_child->next; ) {
		assert(!tail_of_child->parent);
		tail_of_child->parent = parent;
		tail_of_child = tail_of_child->next;
	}

	if (parent->child) {
		tail_of_child->next = parent->child;
		parent->child->prev = tail_of_child;
	}

	parent->child = child;

	return child;
}

size_t libtlv_get_tag_length(const void *tag)
{
	uint8_t *p = (uint8_t *)tag;
	size_t i;

	if ((p[0] & TLV_TAG_NUMBER_MASK) != 0x1Fu)
		return 1;

	for (i = 1; i < 6; i++)
		if (!(p[i] & 0x80u))
			break;

	return i + 1;
}

struct tlv *tlv_find(struct tlv *tlv, const void *tag)
{
	struct tlv *tlv_i = NULL;
	size_t tag_len;

	assert(tag);

	if (!tlv)
		return NULL;

	tag_len = libtlv_get_tag_length(tag);

	for (tlv_i = tlv; tlv_i; tlv_i = tlv_i->next) {
		uint8_t encoded_tag[6];
		size_t encoded_tag_len = sizeof(encoded_tag);

		tlv_encode_identifier(tlv_i, encoded_tag, &encoded_tag_len);
		if ((tag_len == encoded_tag_len) &&
					   (!memcmp(tag, encoded_tag, tag_len)))
			break;
	}

	return tlv_i;
}

int tlv_get_depth(struct tlv *tlv)
{
	int depth = 0;

	while (tlv_get_parent(tlv)) {
		tlv = tlv_get_parent(tlv);
		depth++;
	};

	return depth;
}

struct tlv *tlv_iterate(struct tlv *tlv)
{
	if (!tlv)
		return NULL;

	if (tlv_is_constructed(tlv))
		return tlv_get_child(tlv);

	if (tlv_get_next(tlv))
		return tlv_get_next(tlv);

	while (tlv_get_parent(tlv)) {
		if (tlv_get_next(tlv_get_parent(tlv)))
			return tlv_get_next(tlv_get_parent(tlv));

		tlv = tlv_get_parent(tlv);
	}

	return NULL;
}

struct tlv *tlv_deep_find(struct tlv *tlv, const void *tag)
{
	size_t tag_len;

	assert(tag);

	if (!tlv)
		return NULL;

	tag_len = libtlv_get_tag_length(tag);

	while (tlv) {
		uint8_t encoded_tag[6];
		size_t encoded_tag_len = sizeof(encoded_tag);

		tlv_encode_identifier(tlv, encoded_tag, &encoded_tag_len);
		if ((tag_len == encoded_tag_len) &&
					   (!memcmp(tag, encoded_tag, tag_len)))
			break;

		tlv = tlv_iterate(tlv);
	}

	return tlv;
}

struct tlv *tlv_new(const void *tag, size_t length, const void *value)
{
	struct tlv *tlv = NULL;
	int rc = TLV_RC_OK;

	if (!tag || (length && !value))
		goto error;

	tlv = (struct tlv *)calloc(1, sizeof(struct tlv) + length);

	if (!tlv)
		goto error;

	rc = tlv_parse_identifier(&tag, libtlv_get_tag_length(tag), tlv);
	if (rc != TLV_RC_OK)
		goto error;

	if (length) {
		tlv->length = length;
		memcpy(tlv->value, value, length);
	}

	return tlv;

error:
	free(tlv);
	return NULL;
}

struct tlv *tlv_copy(const struct tlv *tlv)
{
	struct tlv *result = NULL;
	int rc = TLV_RC_OK;

	if (!tlv)
		return NULL;

	if (tlv_is_constructed(tlv)) {
		struct tlv *childs = NULL;
		uint8_t *buffer = NULL;
		size_t size = 0;

		rc = tlv_encode(tlv_get_child(tlv), buffer, &size);
		if ((rc != TLV_RC_OK) || (size == 0))
			return NULL;

		buffer = (uint8_t *)malloc(size);
		if (!buffer)
			return NULL;

		rc = tlv_encode(tlv_get_child(tlv), buffer, &size);
		if (rc != TLV_RC_OK) {
			free(buffer);
			return NULL;
		}

		rc = tlv_parse(buffer, size, &childs);
		if (rc != TLV_RC_OK) {
			free(buffer);
			return NULL;
		}

		result = tlv_new(tlv->tag, 0, NULL);
		if (!result)
			return NULL;

		tlv_insert_below(result, childs);
	} else {
		result = tlv_new(tlv->tag, tlv->length, tlv->value);
	}

	return result;
}

void libtlv_get_dol_field(const void *tag, const void *in, size_t in_sz,
						       void *out, size_t out_sz)
{
	const uint8_t *i = (const uint8_t *)in;
	uint8_t *o = (uint8_t *)out;

	assert(tag && in && out);

	if (out_sz == in_sz) {
		memcpy(o, i, out_sz);
	} else if (out_sz < in_sz) {
		switch (libtlv_id_to_fmt(tag)) {
		case fmt_n:	       /* truncate leftmost bytes if numeric. */
			memcpy(o, &i[in_sz - out_sz], out_sz);
			break;
		default:	       /* truncate rightmost bytes otherwise. */
			memcpy(o, i, out_sz);
			break;
		}
	} else {
		switch (libtlv_id_to_fmt(tag)) {
		case fmt_cn:			/* trailing hexadecimal 'FF's */
			memcpy(o, i, in_sz);
			memset(&o[in_sz], 0xff, out_sz - in_sz);
			break;
		case fmt_n:			/* leading hexadecimal zeroes */
			memset(o, 0x00, out_sz - in_sz);
			memcpy(&o[out_sz - in_sz], i, in_sz);
			break;
		default:		       /* trailing hexadecimal zeroes */
			memcpy(o, i, in_sz);
			memset(&o[in_sz], 0x00, out_sz - in_sz);
			break;
		}
	}
}

int tlv_and_dol_to_del(struct tlv *tlv, const void *dol,
				       size_t dol_sz, void *del, size_t *del_sz)
{
	const void *i_dol = dol;
	uint8_t *out_data = (uint8_t *)del;
	size_t out_data_sz = 0;
	int rc = TLV_RC_OK;

	while (i_dol - dol < dol_sz) {
		size_t dol_sz_left = 0;
		struct tlv tlv_do, *tlv_de;
		uint8_t tag[6];
		size_t tag_sz = sizeof(tag);

		dol_sz_left = dol_sz - (i_dol - dol);
		rc = tlv_parse_identifier(&i_dol, dol_sz_left, &tlv_do);
		if (rc != TLV_RC_OK) {
			log4c_category_log(log_cat, LOG4C_PRIORITY_NOTICE,
					 "%s(): tlv_parse_identifier failed!\n",
								      __func__);
			goto done;
		}

		dol_sz_left = dol_sz - (i_dol - dol);
		rc = tlv_parse_dol_length(&i_dol, dol_sz_left, &tlv_do);
		if (rc != TLV_RC_OK) {
			log4c_category_log(log_cat, LOG4C_PRIORITY_NOTICE,
				    "%s(): tlv_parse_dol_length failed! rc: %d",
								  __func__, rc);
			goto done;
		}

		rc = tlv_encode_identifier(&tlv_do, tag, &tag_sz);
		if (rc != TLV_RC_OK) {
			log4c_category_log(log_cat, LOG4C_PRIORITY_NOTICE,
					"%s(): tlv_encode_identifier failed!\n",
								      __func__);
			goto done;
		}

		if (out_data_sz + tlv_do.length > *del_sz) {
			rc = TLV_RC_BUFFER_OVERFLOW;
			goto done;
		}

		tlv_de = tlv_find(tlv, tag);

		if (!tlv_de) {
			memset(&out_data[out_data_sz], 0, tlv_do.length);
			out_data_sz += tlv_do.length;
		} else if (tlv_is_constructed(tlv_de)) {
			uint8_t *value = NULL;
			size_t length = 0;

			rc = tlv_encode(tlv_get_child(tlv_de), value, &length);
			if (rc != TLV_RC_OK)
				goto done;

			value = (uint8_t *)malloc(length);
			if (!value) {
				rc = TLV_RC_OUT_OF_MEMORY;
				goto done;
			}

			rc = tlv_encode(tlv_get_child(tlv_de), value, &length);
			if (rc != TLV_RC_OK) {
				free(value);
				goto done;
			}

			libtlv_get_dol_field(tlv_do.tag, value, length,
					 &out_data[out_data_sz], tlv_do.length);
			out_data_sz += tlv_do.length;

			free(value);
		} else {
			libtlv_get_dol_field(tlv_do.tag, tlv_de->value,
					 tlv_de->length, &out_data[out_data_sz],
								 tlv_do.length);
			out_data_sz += tlv_do.length;
		}
	}

	*del_sz = out_data_sz;

done:
	return rc;
}

const void *dol_tok(const void **dol, size_t *dol_sz)
{
	const void *tok = *dol;
	struct tlv tlv_do;
	int rc = TLV_RC_OK;

	if (*dol == NULL || *dol_sz == 0) {
		tok = NULL;
		goto done;
	}

	log4c_category_log(log_cat, LOG4C_PRIORITY_TRACE,
		      "%s(dol: %p, *dol: %p, dol_sz: %p, *dol_sz: %d) -> start",
				     __func__, dol, *dol, dol_sz, (int)*dol_sz);

	rc = tlv_parse_identifier(dol, *dol_sz, &tlv_do);
	if (rc != TLV_RC_OK) {
		log4c_category_log(log_cat, LOG4C_PRIORITY_NOTICE,
				   "%s(): tlv_parse_identifier failed! rc %d\n",
								  __func__, rc);
		tok = NULL;
		*dol_sz = 0;
		goto done;
	}

	rc = tlv_parse_dol_length(dol, *dol_sz - (*dol - tok), &tlv_do);
	if (rc != TLV_RC_OK) {
		log4c_category_log(log_cat, LOG4C_PRIORITY_NOTICE,
			 "%s(): tlv_parse_dol_length failed! rc %d\n", __func__,
									    rc);
		tok = NULL;
		*dol_sz = 0;
		goto done;
	}

	*dol_sz -= (*dol - tok);

done:
	log4c_category_log(log_cat, LOG4C_PRIORITY_TRACE,
		       "%s(dol: %p, *dol: %p, dol_sz: %p, *dol_sz: %d) <- done",
				     __func__, dol, *dol, dol_sz, (int)*dol_sz);
	return tok;
}

const void *dol_find_tag(const void *dol, size_t dol_sz, const void *tag)
{
	size_t tag_length = libtlv_get_tag_length(tag);
	const void *tok;

	for (tok = dol_tok(&dol, &dol_sz); tok; tok = dol_tok(&dol, &dol_sz))
		if ((libtlv_get_tag_length(tok) == tag_length) &&
		    (!memcmp(tag, tok, tag_length)))
			break;

	return tok;
}

int dol_and_del_to_tlv(const void *dol, size_t dol_sz,
			       const void *del, size_t del_sz, struct tlv **out)
{
	char hex_dol[2 * dol_sz + 1], hex_del[2 * del_sz + 1];
	const void *i_dol = NULL, *i_del = NULL;
	struct tlv *tlv = NULL;
	int rc = TLV_RC_OK;

	if (!dol || !del || !out) {
		rc = TLV_RC_INVALID_ARG;
		goto done;
	}

	log4c_category_log(log_cat, LOG4C_PRIORITY_TRACE,
				  "%s(dol: '%s', del: '%s') -> start", __func__,
					libtlv_bin_to_hex(dol, dol_sz, hex_dol),
				       libtlv_bin_to_hex(del, del_sz, hex_del));

	*out = NULL;

	i_dol = dol;
	i_del = del;

	while (i_dol - dol < dol_sz) {
		struct tlv tlv_do;
		size_t dol_sz_left = 0, del_sz_left = 0;
		uint8_t tag[6];
		size_t tag_sz = sizeof(tag);

		dol_sz_left = dol_sz - (i_dol - dol);
		rc = tlv_parse_identifier(&i_dol, dol_sz_left, &tlv_do);
		if (rc != TLV_RC_OK) {
			log4c_category_log(log_cat, LOG4C_PRIORITY_NOTICE,
					 "%s(): tlv_parse_identifier failed!\n",
								      __func__);
			goto done;
		}

		dol_sz_left = dol_sz - (i_dol - dol);
		rc = tlv_parse_dol_length(&i_dol, dol_sz_left, &tlv_do);
		if (rc != TLV_RC_OK) {
			log4c_category_log(log_cat, LOG4C_PRIORITY_NOTICE,
					 "%s(): tlv_parse_dol_length failed!\n",
								      __func__);
			goto done;
		}

		rc = tlv_encode_identifier(&tlv_do, tag, &tag_sz);
		if (rc != TLV_RC_OK) {
			log4c_category_log(log_cat, LOG4C_PRIORITY_NOTICE,
					"%s(): tlv_encode_identifier failed!\n",
								      __func__);
			goto done;
		}

		del_sz_left = del_sz - (i_del - del);
		if (tlv_do.length > del_sz_left) {
			rc = TLV_RC_BUFFER_OVERFLOW;
			goto done;
		}

		tlv = tlv_insert_after(tlv, tlv_new(tag, tlv_do.length, i_del));
		i_del += tlv_do.length;


		if (!*out)
			*out = tlv;
	}

	if ((i_del - del != del_sz) || (i_dol - dol != dol_sz)) {
		rc = TLV_RC_INVALID_ARG;
		goto done;
	}

done:
	if (rc != TLV_RC_OK) {
		*out = NULL;
		tlv_free(tlv);
	}

	return rc;
}

int libtlv_bcd_to_u64(const void *buffer, size_t len, uint64_t *u64)
{
	const uint8_t *bcd = (const uint8_t *)buffer;
	size_t i = 0, j = 0;

	if (!bcd || !u64)
		return TLV_RC_INVALID_ARG;

	for (i = 0, *u64 = 0; i < len; i++) {
		for (j = 0; j < 2; j++) {
			uint8_t digit = ((bcd[i] >> ((1 - j) * 4)) & 0xf);

			if (digit > 9)
				return TLV_RC_INVALID_ARG;

			if (*u64 > (UINT64_MAX - digit) / 10)
				return TLV_RC_VALUE_OUT_OF_RANGE;

			*u64 = *u64 * 10 + digit;
		}
	}

	return TLV_RC_OK;
}

int libtlv_u64_to_bcd(uint64_t u64, void *buffer, size_t len)
{
	uint8_t *bcd = (uint8_t *)buffer;
	size_t i = 0, j = 0;

	if (!bcd)
		return TLV_RC_INVALID_ARG;

	memset(bcd, 0, len);

	for (i = len; i > 0; i--) {
		for (j = 0; j < 2; j++) {
			bcd[i - 1] |= (u64 % 10) << (j * 4);
			u64 /= 10;
		}
	}

	if (u64)
		return TLV_RC_VALUE_OUT_OF_RANGE;

	return TLV_RC_OK;
}

char *libtlv_bin_to_hex(const void *blob, size_t blob_sz, char *buffer)
{
	const char hex_digit[16] = {
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
	};
	const uint8_t *bin = (const uint8_t *)blob;
	size_t i;

	for (i = 0; i < blob_sz; i++) {
		buffer[i * 2]     = hex_digit[bin[i] >> 4];
		buffer[i * 2 + 1] = hex_digit[bin[i] & 0xf];
	}

	buffer[i * 2] = '\0';

	return buffer;
}

static uint8_t nibble_from_hex(char hex)
{
	uint8_t byte;
	byte = (hex <= '9') ? (hex - '0') :
		(10 + ((hex <= 'F') ? (hex - 'A') : (hex - 'a')));
	if (byte & 0xf0)
		return 0xff;
	return byte;
}

void * libtlv_hex_to_bin(const char *hex, void * bin_buffer,
							size_t * bin_buffer_len)
{
	const char * ptr;
	uint8_t * bin_ptr = bin_buffer;
	const uint8_t * bin_buffer_end = bin_ptr + *bin_buffer_len;
	int nibble_idx = 1; /* 1 upper, 0 lower */
	uint8_t byte = 0;

	for (ptr = hex; *ptr != 0; ++ptr) {
		uint8_t nibble;

		if (bin_ptr >= bin_buffer_end) /* bin buffer too small */
			return NULL;
		if (*ptr == ' ' || *ptr == '\n' || *ptr == '\r')
			continue;
		nibble = nibble_from_hex(*ptr);
		if (nibble &0xf0)
			return NULL;
		if (nibble_idx) {
			byte = nibble << 4;
			nibble_idx = 0;
		}
		else {
			byte |= nibble;
			nibble_idx++;
			*bin_ptr++ = byte;
		}
	}

	*bin_buffer_len = bin_ptr - (const uint8_t *)bin_buffer;
	return bin_buffer;
}

static struct tlv_id_to_fmt *known_formats;
static size_t num_known_formats;

static size_t id_sz(const uint8_t *id)
{
	size_t sz = 1;

	if ((id[0] & TLV_TAG_NUMBER_MASK) != 0x1Fu)
		return sz;

	while (id[sz++] & 0x80u)
		;

	return sz;
}

static int compare_ids(const uint8_t *id_a, const uint8_t *id_b)
{
	size_t id_a_sz = 0, id_b_sz = 0, min_id_sz = 0;
	int rc = 0;

	id_a_sz = id_sz(id_a);
	id_b_sz = id_sz(id_b);

	min_id_sz = (id_a_sz < id_b_sz) ? id_a_sz : id_b_sz;

	rc = memcmp(id_a, id_b, min_id_sz);
	if (rc)
		return rc;

	if (id_a_sz < id_b_sz)
		return -1;

	if (id_a_sz > id_b_sz)
		return 1;

	return 0;
}

static int compare_formats(const void *a, const void *b)
{
	const struct tlv_id_to_fmt *fmt_a = NULL, *fmt_b = NULL;

	fmt_a = (const struct tlv_id_to_fmt *)a;
	fmt_b = (const struct tlv_id_to_fmt *)b;

	return compare_ids(fmt_a->id, fmt_b->id);
}

static int compare_id_with_format(const void *a, const void *b)
{
	const uint8_t *id = (const uint8_t *)a;
	const struct tlv_id_to_fmt *fmt = (const struct tlv_id_to_fmt *)b;

	return compare_ids(id, fmt->id);
}

int libtlv_register_fmts(const struct tlv_id_to_fmt *fmts)
{
	const struct tlv_id_to_fmt *i_fmt;
	int rc = TLV_RC_OK;
	size_t num_fmts = 0;

	for (i_fmt = fmts, num_fmts = 0; i_fmt->id; i_fmt++)
		num_fmts++;

	known_formats = (struct tlv_id_to_fmt *)realloc(known_formats,
				(num_known_formats + num_fmts) * sizeof(*fmts));
	if (!known_formats) {
		rc = TLV_RC_OUT_OF_MEMORY;
		goto done;
	}

	memcpy(&known_formats[num_known_formats], fmts,
						      num_fmts * sizeof(*fmts));
	num_known_formats += num_fmts;
	qsort(known_formats, num_known_formats, sizeof(*fmts), compare_formats);

done:
	return rc;
}

void libtlv_free_fmts(void)
{
	free(known_formats);
	known_formats = NULL;
}

enum tlv_fmt libtlv_id_to_fmt(const void *id)
{
	const struct tlv_id_to_fmt *fmt = NULL;

	fmt = (const struct tlv_id_to_fmt *)bsearch(id, known_formats,
		       num_known_formats, sizeof(*fmt), compare_id_with_format);

	return fmt ? fmt->fmt : fmt_unknown;
}

void libtlv_init(const char *log4c_category)
{
	char cat[64];

	snprintf(cat, sizeof(cat), "%s.libtlv", log4c_category);
	log_cat = log4c_category_get(cat);
}
