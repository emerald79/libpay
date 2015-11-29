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
#include <unistd.h>
#include <assert.h>

#include <tlv.h>

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(*(x)))

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

struct tag_desc {
	const char	*tag;
	size_t		length;
	const char	*name;
};

struct tag_desc tag_desc[] = {
	{ "\x4F",	1,	"Application Identifier (AID) - card"         },
	{ "\x50",	1,	"Application Label"			      },
	{ "\x56",	1,	"Track 1 Data"				      },
	{ "\x5A",	1,	"Application Primary Account Number (PAN)"    },
	{ "\x5F\x24",	2,	"Application Expiration Date"		      },
	{ "\x5F\x25",	2,	"Application Effective Date"		      },
	{ "\x5F\x34",	2,	"Application Primary Account Number (PAN) "
						      "Sequence Number (PSN)" },
	{ "\x61",	1,	"Application Template"			      },
	{ "\x6F",	1,	"File Control Information (FCI) Template"     },
	{ "\x70",	1,	"READ RECORD Response Message Template"	      },
	{ "\x77",	1,	"Response Message Template Format 2"	      },
	{ "\x82",	1,	"Application Interchange Profile"	      },
	{ "\x84",	1,	"Dedicated File (DF) Name"		      },
	{ "\x87",	1,	"Application Priority Indicator"	      },
	{ "\x94",	1,	"Application File Locator (AFL)"	      },
	{ "\x9F\x07",	2,	"Application Usage Control (AUC)"	      },
	{ "\x9F\x0D",	2,	"Issuer Action Code - Default"		      },
	{ "\x9F\x0E",	2,	"Issuer Action Code - Denial"		      },
	{ "\x9F\x0F",	2,	"Issuer Action Code - Online"		      },
	{ "\x9F\x36",	2,	"Application Transaction Counter (ATC)"	      },
	{ "\x9F\x60",	2,	"CVC3 (Track1)"				      },
	{ "\x9F\x61",	2,	"CVC3 (Track2)"				      },
	{ "\x9F\x62",	2,	"PCVC3 (Track1)"			      },
	{ "\x9F\x63",	2,	"PUNATC (Track1)"			      },
	{ "\x9F\x64",	2,	"NATC (Track1)"				      },
	{ "\x9F\x65",	2,	"PCVC3 (Track2)"			      },
	{ "\x9F\x66",	2,	"PUNATC (Track2)"			      },
	{ "\x9F\x67",	2,	"NATC (Track2)"				      },
	{ "\x9F\x68",	2,	"Card Additional Processes"		      },
	{ "\x9F\x6B",	2,	"Track 2 Data"				      },
	{ "\x9F\x6C",	2,	"Card Transaction Qualifiers (CTQ)"	      },
	{ "\xA5",	1,	"File Control Information (FCI) "
						    "Proprietary Template"    },
	{ "\xBF\x0C",	2,	"File Control Information (FCI) Issuer "
						      "Discretionary Data"    },
};

int tlv_is_constructed(struct tlv *tlv)
{
	return !!(tlv->leading_octet & TLV_TAG_P_C_MASK);
}

struct tlv *tlv_get_next(struct tlv *tlv)
{
	return tlv ? tlv->next : NULL;
}

struct tlv *tlv_get_prev(struct tlv *tlv)
{
	return tlv ? tlv->prev : NULL;
}

struct tlv *tlv_get_parent(struct tlv *tlv)
{
	return tlv ? tlv->parent : NULL;
}

struct tlv *tlv_get_child(struct tlv *tlv)
{
	return tlv ? tlv->child : NULL;
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
		return buffer ? TLV_RC_BUFFER_OVERFLOW : TLV_RC_OK;
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

struct tlv *tlv_insert_after(struct tlv *tlv1, struct tlv *tlv2)
{
	struct tlv *tail_of_tlv2 = NULL;

	if (!tlv1 || !tlv2 || tlv2->prev)
		return NULL;

	for (tail_of_tlv2 = tlv2; tail_of_tlv2->next; )
		tail_of_tlv2 = tail_of_tlv2->next;

	tail_of_tlv2->next = tlv1->next;
	if (tail_of_tlv2->next)
		tail_of_tlv2->next->prev = tail_of_tlv2;

	tlv1->next = tlv2;
	tlv2->prev = tlv1;

	return tlv2;
}

struct tlv *tlv_insert_below(struct tlv *parent, struct tlv *child)
{
	if (!parent || !child || child->parent)
		return NULL;

	if (parent->child) {
		child->next = parent->child;
		parent->child->prev = child;
	}

	parent->child = child;
	child->parent = parent;

	return child;
}

int tlv_parse_file(int fd, struct tlv **tlv)
{
	ssize_t rc = 0;
	size_t der_size;
	uint8_t buffer[1024], *der = NULL;
	int i, nibbles = 0, comment = 0, tlv_rc = TLV_RC_OK;

	rc = (size_t)lseek(fd, SEEK_SET, 0);
	if (rc < 0) {
		tlv_rc = TLV_RC_IO_ERROR;
		goto done;
	}

	do {
		rc = read(fd, buffer, sizeof(buffer));
		if (rc < 0) {
			tlv_rc = TLV_RC_IO_ERROR;
			goto done;
		}

		for (i = 0; i < (int)rc; i++) {
			if (!comment &&
				     ((buffer[i] >= '0' && buffer[i] <= '9') ||
				      (buffer[i] >= 'a' && buffer[i] <= 'f') ||
				      (buffer[i] >= 'A' && buffer[i] <= 'F')))
				nibbles++;
			if (!comment && (buffer[i] == '#'))
				comment = 1;
			if (comment && buffer[i] == '\n')
				comment = 0;
		}
	} while (rc > 0);

	rc = (size_t)lseek(fd, SEEK_SET, 0);
	if (rc < 0) {
		tlv_rc = TLV_RC_IO_ERROR;
		goto done;
	}

	if (nibbles & 0x1) {
		tlv_rc = TLV_RC_IO_ERROR;
		goto done;
	}

	der = (uint8_t *)malloc(nibbles >> 1);
	if (!der) {
		tlv_rc = TLV_RC_OUT_OF_MEMORY;
		goto done;
	}

	memset(der, 0, nibbles >> 1);
	nibbles = 0;

	do {
		rc = read(fd, buffer, sizeof(buffer));
		if (rc < 0) {
			tlv_rc = TLV_RC_IO_ERROR;
			goto done;
		}

		for (i = 0; i < (int)rc; i++) {
			if (!comment) {
				if (buffer[i] >= '0' && buffer[i] <= '9') {
					der[nibbles >> 1] <<= 4;
					der[nibbles >> 1] |= buffer[i] - '0';
					nibbles++;
				}
				if (buffer[i] >= 'a' && buffer[i] <= 'f') {
					der[nibbles >> 1] <<= 4;
					der[nibbles >> 1] |= buffer[i] - 'a'
									   + 10;
					nibbles++;
				}
				if (buffer[i] >= 'A' && buffer[i] <= 'F') {
					der[nibbles >> 1] <<= 4;
					der[nibbles >> 1] |= buffer[i] - 'A'
									   + 10;
					nibbles++;
				}
				if (buffer[i] == '#')
					comment = 1;
			} else
				if (buffer[i] == '\n')
					comment = 0;
		}
	} while (rc > 0);

	der_size = (size_t)(nibbles >> 1);

	tlv_rc = tlv_parse(der, der_size, tlv);

done:
	if (der)
		free(der);
	return tlv_rc;
}

static struct tlv *tlv_iterate(struct tlv *tlv, int *depth)
{
	if (!tlv)
		return NULL;

	if (tlv_is_constructed(tlv)) {
		(*depth)++;
		return tlv_get_child(tlv);
	}

	if (tlv_get_next(tlv))
		return tlv_get_next(tlv);

	while (tlv_get_parent(tlv)) {
		(*depth)--;
		if (tlv_get_next(tlv_get_parent(tlv)))
			return tlv_get_next(tlv_get_parent(tlv));
		tlv = tlv_get_parent(tlv);
	}

	return NULL;
}

int tlv_encode_file(int fd, struct tlv *tlv)
{
	int depth = 0;

	for (depth = 0; tlv; tlv = tlv_iterate(tlv, &depth)) {
		struct tag_desc *desc = NULL;
		uint8_t buffer[256];
		char line[256];
		size_t size;
		ssize_t rc = 0;
		int i, j, len = 0;

		for (i = 0, j = 0; i < depth; i++, j += 2)
			len += snprintf(&line[len], sizeof(line) - len, "  ");
		size = sizeof(buffer);
		tlv_encode_identifier(tlv, buffer, &size);
		for (i = 0; i < (int)size; i++, j += 2)
			len += snprintf(&line[len], sizeof(line) - len, "%02X",
								     buffer[i]);
		len += snprintf(&line[len], sizeof(line) - len, " ");
		for (i = 0; i < ARRAY_SIZE(tag_desc); i++)
			if ((tag_desc[i].length == size) &&
					 !memcmp(tag_desc[i].tag, buffer, size))
				desc = &tag_desc[i];
		size = sizeof(buffer);
		tlv_encode_length(tlv, buffer, &size);
		for (i = 0; i < (int)size; i++, j += 2)
			len += snprintf(&line[len], sizeof(line) - len, "%02X",
								     buffer[i]);
		len += snprintf(&line[len], sizeof(line) - len, " ");
		size = sizeof(buffer);
		tlv_encode_value(tlv, buffer, &size);
		for (i = 0; i < (int)size; i++, j += 2)
			len += snprintf(&line[len], sizeof(line) - len, "%02X",
								     buffer[i]);
		if (desc) {
			for (; j < 46; j++)
				len += snprintf(&line[len], sizeof(line) - len,
									   " ");
			len += snprintf(&line[len], sizeof(line) - len, " # %s",
								    desc->name);
		}
		len += snprintf(&line[len], sizeof(line) - len, "\n");

		size = 0;
		do {
			rc = write(fd, &line[size], len - size);
			if (rc < 0)
				return TLV_RC_IO_ERROR;
			size += rc;
		} while (size < len);
	}

	return TLV_RC_OK;
}

static inline size_t tlv_get_tag_length(const void *tag)
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

	tag_len = tlv_get_tag_length(tag);

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

struct tlv *tlv_new(const void *tag, size_t length, const void *value)
{
	struct tlv *tlv = NULL;
	int rc = TLV_RC_OK;

	tlv = (struct tlv *)calloc(1, sizeof(struct tlv));
	if (!tlv)
		goto error;

	rc = tlv_parse_identifier(&tag, tlv_get_tag_length(tag), tlv);
	if (rc != TLV_RC_OK)
		goto error;

	tlv->length = length;
	tlv->value = malloc(length);
	if (!tlv->value)
		goto error;
	memcpy(tlv->value, value, length);

	return tlv;

error:
	if (tlv) {
		if (tlv->value)
			free(tlv->value);
		free(tlv);
	}
	return NULL;
}

int tlv_process_dol(struct tlv *data_elements, const void *dol, size_t dol_sz,
						    void *data, size_t *data_sz)
{
	const void *i_dol = dol;
	uint8_t *out_data = (uint8_t *)data;
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
			printf("%s(): tlv_parse_identifier_fail!\n", __func__);
			goto done;
		}

		dol_sz_left = dol_sz - (i_dol - dol);
		rc = tlv_parse_length(&i_dol, dol_sz_left, &tlv_do);
		if (rc != TLV_RC_OK) {
			printf("%s(): tlv_parse_length_fail!\n", __func__);
			goto done;
		}

		rc = tlv_encode_identifier(&tlv_do, tag, &tag_sz);
		if (rc != TLV_RC_OK) {
			printf("%s(): tlv_encode_identifier_fail!\n", __func__);
			goto done;
		}

		if (out_data_sz + tlv_do.length > *data_sz) {
			rc = TLV_RC_BUFFER_OVERFLOW;
			goto done;
		}

		tlv_de = tlv_find(data_elements, tag);
		if (!tlv_de || tlv_is_constructed(tlv_de)) {
			memset(&out_data[out_data_sz], 0, tlv_do.length);
			out_data_sz += tlv_do.length;
		} else {
			/* FIXME */
			memset(&out_data[out_data_sz], 0, tlv_do.length);
			out_data_sz += tlv_do.length;
		}
	}

done:
	return rc;
}

int tlv_bcd_to_u64(const void *buffer, size_t len, uint64_t *u64)
{
	const uint8_t *bcd = (const uint8_t *)buffer;
	size_t i = 0, j = 0;

	if (!bcd || !u64)
		return TLV_RC_INVALID_ARG;

	for (i = len, *u64 = 0; i > 0; i--) {
		for (j = 0; j < 2; j++) {
			uint8_t digit = ((bcd[i - 1] >> (j * 4)) & 0xf);

			if (digit > 9)
				return TLV_RC_INVALID_ARG;

			if (*u64 > (UINT64_MAX - digit) / 10)
				return TLV_RC_VALUE_OUT_OF_RANGE;

			*u64 = *u64 * 10 + digit;
		}
	}

	return TLV_RC_OK;
}

int tlv_u64_to_bcd(uint64_t u64, void *buffer, size_t len)
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

const char *tlv_bin_to_hex(const void *blob, size_t blob_sz)
{
	const uint8_t hex_digit[16] = {
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
	};
	const uint8_t *bin = (const uint8_t *)blob;
	static __thread char hex[68];
	size_t i;

	for (i = 0; (i < blob_sz) && (i < 32); i++) {
		hex[i * 2]     = hex_digit[bin[i] >> 4];
		hex[i * 2 + 1] = hex_digit[bin[i] & 0xf];
	}

	if (i < blob_sz)
		strcpy(&hex[i * 2], "...");
	else
		hex[i * 2] = '\0';

	return hex;
}
