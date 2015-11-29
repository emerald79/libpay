#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <json-c/json.h>
#include <emv.h>
#include <tlv.h>

static int parse_emv_tag(const char *hex, size_t len, struct emv_tag *tag)
{
	uint8_t *result = NULL;
	int i = 0;

	assert(hex);
	assert(tag);

	if (len % 2)
		return EMV_RC_SYNTAX_ERROR;

	result = malloc(len >> 1);
	memset(result, 0, len >> 1);

	for (i = 0; i < len; i++) {
		char digit = tolower(hex[i]);

		result[i >> 1] <<= 4;
		if ((digit >= '0') && (digit <= '9'))
			result[i >> 1] |= digit - '0';
		else if ((digit >= 'a') && (digit <= 'f'))
			result[i >> 1] |= digit - 'a' + 10;
		else {
			free(result);
			return EMV_RC_SYNTAX_ERROR;
		}
	}

	tag->value = (void *)result;
	tag->len = len >> 1;
	return EMV_RC_OK;
}

int emv_tag_parse_descriptors(const char *json_string,
	       struct emv_tag_descriptor **descriptors, size_t *num_descriptors)
{
	struct json_tokener *json_tokener = NULL;
	enum json_tokener_error json_error = json_tokener_success;
	struct json_object *json_array = NULL;
	struct emv_tag_descriptor *desc = NULL;
	int rc = EMV_RC_OK, num_desc = 0, i = 0;

	assert(json_string);
	assert(descriptors);
	assert(num_descriptors);

	*descriptors = NULL;
	*num_descriptors = 0;

	json_tokener = json_tokener_new();
	if (!json_tokener) {
		rc = EMV_RC_OUT_OF_MEMORY;
		goto done;
	}

	do {
		json_array = json_tokener_parse_ex(json_tokener, json_string,
									    -1);
		json_error = json_tokener_get_error(json_tokener);
	} while (json_error == json_tokener_continue);

	if ((json_error != json_tokener_success) || !json_array ||
	    !json_object_is_type(json_array, json_type_array)) {
		rc = EMV_RC_SYNTAX_ERROR;
		goto done;
	}

	num_desc = json_object_array_length(json_array);
	if (!num_desc)
		goto done;

	desc = calloc(num_desc, sizeof(struct emv_tag_descriptor));
	if (!desc) {
		rc = EMV_RC_OUT_OF_MEMORY;
		goto done;
	}

	for (i = 0; i < num_desc; i++) {
		struct json_object *json_object = NULL;
		struct json_object *json_string = NULL;

		json_object = json_object_array_get_idx(json_array, i);
		assert(json_object);

		if (!json_object_is_type(json_object, json_type_object)) {
			json_object_put(json_object);
			rc = EMV_RC_SYNTAX_ERROR;
			goto done;
		}

		if (!json_object_object_get_ex(json_object, "tag",
								&json_string) ||
		    !json_object_is_type(json_string, json_type_string)) {
			json_object_put(json_object);
			rc = EMV_RC_SYNTAX_ERROR;
			goto done;
		}

		rc = parse_emv_tag(json_object_get_string(json_string),
					json_object_get_string_len(json_string),
								  &desc[i].tag);
		if (rc != EMV_RC_OK) {
			json_object_put(json_object);
			json_object_put(json_string);
			rc = EMV_RC_SYNTAX_ERROR;
			goto done;
		}

		json_object_put(json_string);

		if (!json_object_object_get_ex(json_object, "name",
								&json_string) ||
		    !json_object_is_type(json_string, json_type_string)) {
			json_object_put(json_object);
			rc = EMV_RC_SYNTAX_ERROR;
			goto done;
		}

		desc[i].name = strdup(json_object_get_string(json_string));
		json_object_put(json_string);

		json_object_put(json_object);
	}

	*descriptors = desc;
	*num_descriptors = num_desc;

done:
	if ((rc != EMV_RC_OK) && desc)
		free(desc);
	if (json_array)
		json_object_put(json_array);
	if (json_tokener)
		json_tokener_free(json_tokener);
	return rc;
}

int emv_bcd_to_u64(const void *buffer, size_t len, uint64_t *u64)
{
	const uint8_t *bcd = (const uint8_t *)buffer;
	size_t i = 0, j = 0;

	if (!bcd || !u64)
		return EMV_RC_INVALID_ARG;

	for (i = len, *u64 = 0; i > 0; i--) {
		for (j = 0; j < 2; j++) {
			uint8_t digit = ((bcd[i - 1] >> (j * 4)) & 0xf);

			if (digit > 9)
				return EMV_RC_INVALID_ARG;

			if (*u64 > (UINT64_MAX - digit) / 10)
				return EMV_RC_OVERFLOW;

			*u64 = *u64 * 10 + digit;
		}
	}

	return EMV_RC_OK;
}

int emv_u64_to_bcd(uint64_t u64, void *buffer, size_t len)
{
	uint8_t *bcd = (uint8_t *)buffer;
	size_t i = 0, j = 0;

	if (!bcd)
		return EMV_RC_INVALID_ARG;

	memset(bcd, 0, len);

	for (i = len; i > 0; i--) {
		for (j = 0; j < 2; j++) {
			bcd[i - 1] |= (u64 % 10) << (j * 4);
			u64 /= 10;
		}
	}

	if (u64)
		return EMV_RC_OVERFLOW;

	return EMV_RC_OK;
}

const char *emv_blob_to_hex(const void *blob, size_t blob_sz)
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
