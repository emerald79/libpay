#include <malloc.h>
#include <string.h>
#include <stdio.h>

#include <tlv.h>

#define TLV_TAG_CLASS_MASK		0xC0u
#define TLV_TAG_P_C_MASK		0x20u
#define TLV_TAG_NUMBER_MASK		0x1Fu

struct tlv {
	uint8_t		class_of_tag;
	uint8_t		constructed;
	uint32_t	tag_number;
	size_t		length;
	uint8_t		*value;

	struct tlv	*next;
	struct tlv	*prev;
	struct tlv	*parent;
	struct tlv	*child;
};

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

	tlv->class_of_tag	= *p & TLV_TAG_CLASS_MASK;
	tlv->constructed	= *p & TLV_TAG_P_C_MASK;
	tlv->tag_number		= (uint32_t)(*p & TLV_TAG_NUMBER_MASK);

	if (tlv->tag_number != 0x1Fu) {
		*buffer = (const void *)&p[1];
		return TLV_RC_OK;
	}

	tlv->tag_number = 0;
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
		printf("p[0]: 0x%02x\n", (unsigned)p[0]);
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

	if ((*tlv)->constructed) {
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

static size_t tlv_get_encoded_identifier_size(uint32_t tag_number)
{
	if (tag_number < 31)
		return 1;
	if (tag_number < 0x80u)
		return 2;
	if (tag_number < 0x4000u)
		return 3;
	if (tag_number < 0x200000u)
		return 4;
	if (tag_number < 0x10000000u)
		return 5;
	return 6;
}

static size_t tlv_get_encoded_length_size(size_t length)
{
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

static size_t tlv_get_encoded_size(struct tlv *tlv)
{
	size_t size;

	size = tlv_get_encoded_identifier_size(tlv->tag_number);
	size += tlv_get_encoded_length_size(tlv->length);

	if (tlv->constructed)
		size += tlv_get_encoded_size(tlv->child);
	else
		size += tlv->length;

	if (tlv->next)
		size += tlv_get_encoded_size(tlv->next);

	return size;
}

static void tlv_encode_identifier(uint8_t class, uint8_t p_c,
					     uint32_t tag_number, void **buffer)
{
	uint8_t *p = (uint8_t *)*buffer;

	p[0] = class | p_c;

	if (tag_number < 0x1fu) {
		p[0] |= (uint8_t)tag_number;
		*buffer = (void *)&p[1];
		return;
	}

	p[0] |= 0x1fu;

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

static void tlv_encode_length(size_t length, void **buffer)
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
	tlv_encode_identifier(tlv->class_of_tag, tlv->constructed,
						       tlv->tag_number, buffer);

	if (tlv->constructed) {
		tlv_encode_length(tlv_get_encoded_size(tlv->child), buffer);
		tlv_encode_recursive(tlv->child, buffer);
	} else {
		tlv_encode_length(tlv->length, buffer);
		memcpy(*buffer, tlv->value, tlv->length);
		*buffer = (void *)(((uint8_t *)*buffer) + tlv->length);
	}

	if (tlv->next)
		tlv_encode_recursive(tlv->next, buffer);
}

int tlv_encode(struct tlv *tlv, void *buffer, size_t *size)
{
	size_t encoded_size = tlv_get_encoded_size(tlv);

	if (encoded_size > *size) {
		*size = encoded_size;
		return TLV_RC_BUFFER_OVERFLOW;
	}

	*size = encoded_size;

	tlv_encode_recursive(tlv, &buffer);

	return TLV_RC_OK;
}
