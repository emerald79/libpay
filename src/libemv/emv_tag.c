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

const struct tlv_id_to_fmt *libemv_get_id_fmts(void)
{
	static const struct tlv_id_to_fmt formats[] = {
		{ .id = EMV_ID_ADF_NAME,		       .fmt = fmt_b   },
		{ .id = EMV_ID_APPLICATION_LABEL,	       .fmt = fmt_ans },
		{ .id = EMV_ID_DIRECTORY_ENTRY,		       .fmt = fmt_var },
		{ .id = EMV_ID_FCI_TEMPLATE,		       .fmt = fmt_b   },
		{ .id = EMV_ID_DF_NAME,			       .fmt = fmt_b   },
		{ .id = EMV_ID_APPLICATION_PRIORITY_INDICATOR, .fmt = fmt_b   },
		{ .id = EMV_ID_KERNEL_IDENTIFIER,	       .fmt = fmt_b   },
		{ .id = EMV_ID_EXTENDED_SELECTION,	       .fmt = fmt_b   },
		{ .id = EMV_ID_PDOL,			       .fmt = fmt_b   },
		{ .id = EMV_ID_FCI_PROPRIETARY_TEMPLATE,       .fmt = fmt_var },
		{ .id = EMV_ID_FCI_ISSUER_DISCRETIONARY_DATA,  .fmt = fmt_var },
		{ .id = NULL						      }
	};

	return formats;
}
