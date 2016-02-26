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

static struct json_object *tlv_tree_to_json(const struct tlv *tlv)
{
	uint8_t tag[TLV_MAX_TAG_LENGTH] = { 0 }, *value = NULL;
	char hex_tag[TLV_MAX_TAG_LENGTH * 2 + 1], *hex_value = NULL;
	size_t tag_len = sizeof(tag), value_len = 0;
	struct json_object *result = NULL, *json_tag = NULL, *json_value = NULL;
	int rc = TLV_RC_OK;

	if (!tlv)
		goto done;

	rc = tlv_encode_identifier(tlv, tag, &tag_len);
	if (rc != TLV_RC_OK)
		goto done;

	json_tag = json_object_new_string(
				      libtlv_bin_to_hex(tag, tag_len, hex_tag));
	if (!json_tag)
		goto done;

	if (tlv_is_constructed(tlv)) {
		struct tlv *tlv_child = tlv_get_child(tlv);

		if (tlv_child)
			json_value = tlv_to_json(tlv_child);
	} else {
		rc = tlv_encode_value(tlv, NULL, &value_len);
		if (rc != TLV_RC_OK)
			goto done;

		value = (uint8_t *)malloc(value_len);
		if (!value)
			goto done;

		rc = tlv_encode_value(tlv, value, &value_len);
		if (rc != TLV_RC_OK)
			goto done;

		hex_value = (char *)malloc(value_len * 2 + 1);
		if (!hex_value)
			goto done;

		json_value = json_object_new_string(
				libtlv_bin_to_hex(value, value_len, hex_value));
	}

	result = json_object_new_object();
	if (!result)
		goto done;

	json_object_object_add(result, "tag", json_tag);

	if (json_value)
		json_object_object_add(result, "value", json_value);

done:
	if (hex_value)
		free(hex_value);

	if (value)
		free(value);

	if (!result) {
		if (json_tag)
			json_object_put(json_tag);

		if (json_value)
			json_object_put(json_value);
	}

	return result;
}

struct json_object *tlv_to_json(const struct tlv *tlv)
{
	struct json_object *result = NULL;
	int idx = 0;

	if (!tlv)
		goto done;

	result = json_object_new_array();
	if (!result)
		goto done;

	do {
		struct json_object *json_tree = tlv_tree_to_json(tlv);

		if (!json_tree) {
			json_object_put(result);
			result = NULL;
			goto done;
		}

		json_object_array_put_idx(result, idx++, json_tree);
		tlv = tlv_get_next(tlv);
	} while (tlv);

done:
	return result;
}
