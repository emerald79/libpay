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
#include <errno.h>
#include <fcntl.h>

#include <tlv.h>

struct tag_desc {
	const char	*tag;
	size_t		length;
	const char	*name;
};

struct tag_desc tag_desc[] = {
	{ "\x4F",	1,	"Application Identifier (AID) – card"      },
	{ "\x50",	1,	"Application Label"			   },
	{ "\x5A",	1,	"Application Primary Account Number (PAN)" },
	{ "\x5F\x24",	2,	"Application Expiration Date"		   },
	{ "\x5F\x25",	2,	"Application Effective Date"		   },
	{ "\x5F\x34",	2,	"Application Primary Account Number (PAN) "
						   "Sequence Number (PSN)" },
	{ "\x61",	1,	"Application Template"			   },
	{ "\x6F",	1,	"File Control Information (FCI) Template"  },
	{ "\x70",	1,	"READ RECORD Response Message Template"	   },
	{ "\x84",	1,	"Dedicated File (DF) Name"		   },
	{ "\x87",	1,	"Application Priority Indicator"	   },
	{ "\x9F\x07",	2,	"Application Usage Control (AUC)"	   },
	{ "\x9F\x0D",	2,	"Issuer Action Code - Default"		   },
	{ "\x9F\x0E",	2,	"Issuer Action Code - Denial"		   },
	{ "\x9F\x0F",	2,	"Issuer Action Code - Online"		   },
	{ "\xA5",	1,	"File Control Information (FCI) "
						    "Proprietary Template" },
	{ "\xBF\x0C",	2,	"File Control Information (FCI) Issuer"
						      "Discretionary Data" },
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

static uint8_t *read_ascii_coded_der(const char *filename, size_t *size)
{
	ssize_t rc = 0;
	uint8_t buffer[1024], *result = NULL;
	int fd = -1, i, nibbles = 0, comment = 0;

	fd = open(filename, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "open('%s') failed: %s\n", filename,
							       strerror(errno));
		goto error;
	}

	do {
		rc = read(fd, buffer, sizeof(buffer));
		if (rc < 0) {
			fprintf(stderr, "read failed: %s\n", strerror(errno));
			goto error;
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
		fprintf(stderr, "lseek failed: %s\n", strerror(errno));
		goto error;
	}

	if (nibbles & 0x1) {
		fprintf(stderr, "Format error: Uneven number of nibbles\n");
		goto error;
	}

	result = (uint8_t *)malloc(nibbles >> 1);
	if (!result) {
		fprintf(stderr, "out of memory");
		goto error;
	}

	memset(result, 0, nibbles >> 1);
	nibbles = 0;

	do {
		rc = read(fd, buffer, sizeof(buffer));
		if (rc < 0) {
			fprintf(stderr, "read failed: %s\n", strerror(errno));
			goto error;
		}

		for (i = 0; i < (int)rc; i++) {
			if (!comment) {
				if (buffer[i] >= '0' && buffer[i] <= '9') {
					result[nibbles >> 1] <<= 4;
					result[nibbles >> 1] |= buffer[i] - '0';
					nibbles++;
				}
				if (buffer[i] >= 'a' && buffer[i] <= 'f') {
					result[nibbles >> 1] <<= 4;
					result[nibbles >> 1] |= buffer[i] - 'a'
									   + 10;
					nibbles++;
				}
				if (buffer[i] >= 'A' && buffer[i] <= 'F') {
					result[nibbles >> 1] <<= 4;
					result[nibbles >> 1] |= buffer[i] - 'A'
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

	*size = (size_t)(nibbles >> 1);
	return result;

error:
	if (result)
		free(result);
	if (fd >= 0)
		close(fd);
	return NULL;
}

int main(int argc, char **argv)
{
	struct tlv *tlv = NULL;
	uint8_t *der_tlv = NULL;
	size_t der_tlv_size = 0;
	int rc = 0, depth = 0;

	der_tlv = read_ascii_coded_der(argv[1], &der_tlv_size);
	if (!der_tlv)
		return EXIT_FAILURE;

	rc = tlv_parse(der_tlv, der_tlv_size, &tlv);
	if (rc != TLV_RC_OK)
		return EXIT_FAILURE;

	for (depth = 0; tlv; tlv = tlv_iterate(tlv, &depth)) {
		struct tag_desc *desc = NULL;
		uint8_t buffer[256];
		size_t size;
		int i, j;
		for (i = 0, j = 0; i < depth; i++, j += 2)
			printf("  ");
		size = sizeof(buffer);
		tlv_encode_identifier(tlv, buffer, &size);
		for (i = 0; i < (int)size; i++, j += 2)
			printf("%02X", buffer[i]);
		printf(" ");
		for (i = 0; i < ARRAY_SIZE(tag_desc); i++)
			if ((tag_desc[i].length == size) &&
					 !memcmp(tag_desc[i].tag, buffer, size))
				desc = &tag_desc[i];
		size = sizeof(buffer);
		tlv_encode_length(tlv, buffer, &size);
		for (i = 0; i < (int)size; i++, j += 2)
			printf("%02X", buffer[i]);
		printf(" ");
		size = sizeof(buffer);
		tlv_encode_value(tlv, buffer, &size);
		for (i = 0; i < (int)size; i++, j += 2)
			printf("%02X", buffer[i]);
		if (desc) {
			for (; j < 40; j++)
				printf(" ");
			printf("# %s", desc->name);
		}
		printf("\n");
	}

	tlv_free(tlv);

	return EXIT_SUCCESS;
}
