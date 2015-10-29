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

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <tlv.h>

struct tag_desc {
	const char	*tag;
	size_t		length;
	const char	*name;
};

struct tag_desc tag_desc[] = {
	{ "\x70",	1,	"READ RECORD Response Message Template"	   },
	{ "\x5A",	1,	"Application Primary Account Number (PAN)" },
	{ "\x5F\x24",	2,	"Application Expiration Date"		   },
	{ "\x5F\x25",	2,	"Application Effective Date"		   },
	{ "\x5F\x34",	2,	"Application Primary Account Number (PAN) "
						   "Sequence Number (PSN)" },
	{ "\x9F\x07",	2,	"Application Usage Control (AUC)"	   },
	{ "\x9F\x0D",	2,	"Issuer Action Code - Default"		   },
	{ "\x9F\x0E",	2,	"Issuer Action Code - Denial"		   },
	{ "\x9F\x0F",	2,	"Issuer Action Code - Online"		   }
};

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(*(x)))

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

int main(void)
{
	struct tlv *tlv, *tlv2;
	uint8_t read_record_rsp[] = {
		0x70, 0x39, 0x5A, 0x0A, 0x67, 0x99, 0x99, 0x89, 0x00, 0x00,
		0x02, 0x00, 0x06, 0x9F, 0x5F, 0x24, 0x03, 0x25, 0x12, 0x31,
		0x5F, 0x25, 0x03, 0x04, 0x01, 0x01, 0x5F, 0x34, 0x01, 0x25,
		0x9F, 0x07, 0x02, 0xFF, 0x00, 0x9F, 0x0D, 0x05, 0xFC, 0x50,
		0xA0, 0x00, 0x00, 0x9F, 0x0E, 0x05, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x9F, 0x0F, 0x05, 0xF8, 0x70, 0xA4, 0x98, 0x00
	};
	int rc, depth;

	rc = tlv_parse(read_record_rsp, sizeof(read_record_rsp), &tlv);
	if (rc != TLV_RC_OK)
		return EXIT_FAILURE;

	rc = tlv_parse(read_record_rsp, sizeof(read_record_rsp), &tlv2);
	if (rc != TLV_RC_OK)
		return EXIT_FAILURE;

	rc = tlv_insert_after(tlv_get_child(tlv), tlv2);
	if (rc != TLV_RC_OK)
		return EXIT_FAILURE;

	for (depth = 0; tlv; tlv = tlv_iterate(tlv, &depth)) {
		struct tag_desc *desc = NULL;
		uint8_t buffer[256];
		size_t size;
		int i;
		for (i = 0; i < depth; i++)
			printf("  ");
		size = sizeof(buffer);
		tlv_encode_identifier(tlv, buffer, &size);
		for (i = 0; i < (int)size; i++)
			printf("%02X", buffer[i]);
		printf(" ");
		for (i = 0; i < ARRAY_SIZE(tag_desc); i++)
			if ((tag_desc[i].length == size) &&
					 !memcmp(tag_desc[i].tag, buffer, size))
				desc = &tag_desc[i];
		size = sizeof(buffer);
		tlv_encode_length(tlv, buffer, &size);
		for (i = 0; i < (int)size; i++)
			printf("%02X", buffer[i]);
		printf(" ");
		size = sizeof(buffer);
		tlv_encode_value(tlv, buffer, &size);
		for (i = 0; i < (int)size; i++)
			printf("%02X", buffer[i]);
		if (desc)
			printf(" '%s'", desc->name);
		printf("\n");
	}

	tlv_free(tlv);

	return EXIT_SUCCESS;
}
