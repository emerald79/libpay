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

#include <libpay/tlv.h>
#include "ted.h"

struct json_object *tlv_to_json(const struct tlv *tlv)
{
	struct json_object *result = NULL;
	char *hex = NULL;
	void *enc = NULL;
	size_t enc_sz = 0;
	int rc = TLV_RC_OK;

	rc = tlv_encode(tlv, NULL, &enc_sz);
	if (rc != TLV_RC_OK)
		goto done;

	enc = malloc(enc_sz);
	if (!enc)
		goto done;

	rc = tlv_encode(tlv, enc, &enc_sz);
	if (rc != TLV_RC_OK)
		goto done;

	hex = malloc(enc_sz * 2 + 1);
	if (!hex)
		goto done;

	result = json_object_new_string(libtlv_bin_to_hex(enc, enc_sz, hex));

done:
	if (hex)
		free(hex);

	if (enc)
		free(enc);

	return result;
}
