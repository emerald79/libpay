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

#include <malloc.h>
#include <string.h>
#include <stdio.h>

#include <tlv.h>

#define TLV_TAG_CLASS_MASK		0xC0u
#define TLV_TAG_P_C_MASK		0x20u
#define TLV_TAG_NUMBER_MASK		0x1Fu

struct tlv {
	uint8_t		leading_octet;
	uint32_t	tag_number;
	size_t		length;
	uint8_t		*value;

	struct tlv	*next;
	struct tlv	*prev;
	struct tlv	*parent;
	struct tlv	*child;
};

int tlv_is_constructed(struct tlv *tlv)
{
	return !!(tlv->leading_octet & TLV_TAG_P_C_MASK);
}

struct tlv *tlv_get_next(struct tlv *tlv)
{
	return tlv->next;
}

struct tlv *tlv_get_prev(struct tlv *tlv)
{
	return tlv->prev;
}

struct tlv *tlv_get_parent(struct tlv *tlv)
{
	return tlv->parent;
}

struct tlv *tlv_get_child(struct tlv *tlv)
{
	return tlv->child;
}

static int tlv_parse_identifier(const void **buffer, size_t length,
								struct tlv *tlv)
{
	const uint8_t *p = NULL;
	int i;

	if (!buffer || !(*buffer) || !tlv)
		return TLV_RC_INVALID_ARG;

	if (!length)
		return TLV_RC_BUFFER_OVERFLOW;

	p = (uint8_t *)*buffer;

	tlv->leading_octet	= p[0];
	tlv->tag_number		= 0;

	if ((tlv->leading_octet & TLV_TAG_NUMBER_MASK) != 0x1Fu) {
		*buffer = (const void *)&p[1];
		return TLV_RC_OK;
	}

	for (i = 1; (i < 6) & (i < length); i++) {
		tlv->tag_number <<= 7;
		tlv->tag_number |= p[i] & 0x7Fu;

		if (!(p[i] & 0x80u))
			break;
	}

	if (i == length) {
		*buffer = (const void *)&p[length];
		return TLV_RC_BUFFER_OVERFLOW;
	}

	if ((i == 5) && (p[1] & 0x70u)) {
		*buffer = (const void *)&p[6];
		return TLV_RC_TAG_NUMBER_TOO_LARGE;
	}

	*buffer = (const void *)&p[i + 1];
	return TLV_RC_OK;
}

static int tlv_parse_length(const void **buffer, size_t length, struct tlv *tlv)
{
	const uint8_t *p = NULL;
	int i, value_length;

	if (!buffer || !(*buffer) || !tlv)
		return TLV_RC_INVALID_ARG;

	if (!length)
		return TLV_RC_BUFFER_OVERFLOW;

	p = (uint8_t *)*buffer;

	if (p[0] == 0x80u) {
		*buffer = (const void *)&p[1];
		return TLV_RC_INDEFINITE_LENGTH_NOT_SUPPORTED;
	}

	if (!(p[0] & 0x80u)) {
		tlv->length = (size_t)p[0];
		*buffer = (const void *)&p[1];
		return TLV_RC_OK;
	}

	value_length = (int)(p[0] & 0x7fu);

	if (value_length > (int)sizeof(size_t)) {
		*buffer = (const void *)&p[1];
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
			 struct tlv **tlv, struct tlv *prev, struct tlv *parent)
{
	const void *start = *buffer;
	int rc;

	if (!buffer || !length || !tlv)
		return TLV_RC_INVALID_ARG;

	*tlv = (struct tlv *)malloc(sizeof(struct tlv));
	if (!(*tlv))
		return TLV_RC_OUT_OF_MEMORY;

	memset(*tlv, 0, sizeof(struct tlv));
	(*tlv)->parent	= parent;
	(*tlv)->prev	= prev;

	rc = tlv_parse_identifier(buffer, length - (*buffer - start), *tlv);
	if (rc != TLV_RC_OK) {
		free(*tlv);
		*tlv = NULL;
		return rc;
	}

	rc = tlv_parse_length(buffer, length - (*buffer - start), *tlv);
	if (rc != TLV_RC_OK) {
		free(*tlv);
		*tlv = NULL;
		return rc;
	}

	if ((*tlv)->leading_octet & TLV_TAG_P_C_MASK) {
		rc = tlv_parse_recursive(buffer, (*tlv)->length, &(*tlv)->child,
								    NULL, *tlv);
		if (rc != TLV_RC_OK) {
			free(*tlv);
			*tlv = NULL;
			return rc;
		}
	} else {
		(*tlv)->value = (uint8_t *)malloc((*tlv)->length);
		if (!(*tlv)->value) {
			free(*tlv);
			*tlv = NULL;
			return TLV_RC_OUT_OF_MEMORY;
		}
		memcpy((*tlv)->value, *buffer, (*tlv)->length);
		*buffer += (*tlv)->length;
	}

	if (length - (*buffer - start) > 0) {
		rc = tlv_parse_recursive(buffer, length - (*buffer - start),
						   &(*tlv)->next, *tlv, parent);
		if (rc != TLV_RC_OK) {
			free(*tlv);
			*tlv = NULL;
			return rc;
		}
	}

	return TLV_RC_OK;
}

void tlv_free(struct tlv *tlv)
{
	struct tlv *current, *next;

	for (current = tlv; current; current = next) {
		if (current->value)
			free(current->value);

		if (current->child)
			tlv_free(current->child);

		next = current->next;
		free(current);
	}
}

int tlv_parse(const void *buffer, size_t length, struct tlv **tlv)
{
	return tlv_parse_recursive(&buffer, length, tlv, NULL, NULL);
}

static void tlv_print_recursive(struct tlv *tlv, int depth)
{
	int i;

	while (tlv) {
		for (i = 0; i < depth; i++)
			printf("  ");

		printf("Tag %u, Length %u", tlv->tag_number,
							 (unsigned)tlv->length);
		if (tlv->value) {
			printf(", Value ");

			for (i = 0; i < (int)tlv->length; i++)
				printf("%02X", tlv->value[i]);
		}
		printf("\n");

		if (tlv->child)
			tlv_print_recursive(tlv->child, depth + 1);

		tlv = tlv->next;
	}
}

void tlv_print(struct tlv *tlv)
{
	tlv_print_recursive(tlv, 0);
}

static size_t tlv_get_encoded_identifier_size(struct tlv *tlv)
{
	if ((tlv->leading_octet & TLV_TAG_NUMBER_MASK) != 0x1fu)
		return 1;
	if (tlv->tag_number < 0x80u)
		return 2;
	if (tlv->tag_number < 0x4000u)
		return 3;
	if (tlv->tag_number < 0x200000u)
		return 4;
	if (tlv->tag_number < 0x10000000u)
		return 5;
	return 6;
}

static size_t tlv_get_encoded_length(struct tlv *tlv);

static size_t tlv_get_encoded_length_size(struct tlv *tlv)
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

static size_t tlv_get_encoded_length(struct tlv *tlv)
{
	size_t size;

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

static void __tlv_encode_identifier(uint8_t leading_octet,
					     uint32_t tag_number, void **buffer)
{
	uint8_t *p = (uint8_t *)*buffer;

	p[0] = leading_octet;

	if ((leading_octet & TLV_TAG_NUMBER_MASK) != 0x1fu) {
		*buffer = (void *)&p[1];
		return;
	}

	if (tag_number & 0xf0000000u) {
		p[1] = 0x80 | (uint8_t)(tag_number >> 28);
		p[2] = 0x80 | (uint8_t)(tag_number >> 21);
		p[3] = 0x80 | (uint8_t)(tag_number >> 14);
		p[4] = 0x80 | (uint8_t)(tag_number >>  7);
		p[5] = tag_number & 0x7fu;
		*buffer = (void *)&p[6];
		return;
	}

	if (tag_number & 0x0fe00000u) {
		p[1] = 0x80 | (uint8_t)(tag_number >> 21);
		p[2] = 0x80 | (uint8_t)(tag_number >> 14);
		p[3] = 0x80 | (uint8_t)(tag_number >>  7);
		p[4] = tag_number & 0x7fu;
		*buffer = (void *)&p[5];
		return;
	}

	if (tag_number & 0x001fc000u) {
		p[1] = 0x80 | (uint8_t)(tag_number >> 14);
		p[2] = 0x80 | (uint8_t)(tag_number >>  7);
		p[3] = tag_number & 0x7fu;
		*buffer = (void *)&p[4];
		return;
	}

	if (tag_number & 0x00003f80u) {
		p[1] = 0x80 | (uint8_t)(tag_number >>  7);
		p[2] = tag_number & 0x7fu;
		*buffer = (void *)&p[3];
		return;
	}

	p[1] = tag_number & 0x7fu;
	*buffer = (void *)&p[2];
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

static void tlv_encode_recursive(struct tlv *tlv, void **buffer)
{
	__tlv_encode_identifier(tlv->leading_octet, tlv->tag_number, buffer);

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

int tlv_encode(struct tlv *tlv, void *buffer, size_t *size)
{
	size_t encoded_size = 0;

	if (!tlv || !size)
		return TLV_RC_INVALID_ARG;

	encoded_size = tlv_get_encoded_length(tlv);

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

int tlv_encode_identifier(struct tlv *tlv, void *buffer, size_t *size)
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

	__tlv_encode_identifier(tlv->leading_octet, tlv->tag_number, &buffer);

	return TLV_RC_OK;
}

int tlv_encode_length(struct tlv *tlv, void *buffer, size_t *size)
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

int tlv_encode_value(struct tlv *tlv, void *buffer, size_t *size)
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

int tlv_insert_after(struct tlv *tlv1, struct tlv *tlv2)
{
	struct tlv *tail_of_tlv2 = NULL;

	if (!tlv1 || !tlv2 || tlv2->prev)
		return TLV_RC_INVALID_ARG;

	for (tail_of_tlv2 = tlv2; tail_of_tlv2->next; )
		tail_of_tlv2 = tail_of_tlv2->next;

	tail_of_tlv2->next = tlv1->next;
	if (tail_of_tlv2->next)
		tail_of_tlv2->next->prev = tail_of_tlv2;

	tlv1->next = tlv2;
	tlv2->prev = tlv1;

	return TLV_RC_OK;
}
