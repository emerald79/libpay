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

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <argp.h>
#include <assert.h>
#include <json-c/json.h>
#include <log4c.h>

#include <libpay/tlv.h>
#include <libpay/emv.h>

const char *argp_program_version = "tlvdump 0.1";
const char *argp_program_bug_address = "mijung@gmx.net";
static const char doc[] = "BER-TLV conversion tool";

static const char args_doc[] = "";
static struct argp_option options[] = {
	{ "input",	   'i', "FILE",	  0,
		 "Input from FILE instead of standard input" },
	{ "output",	   'o', "FILE",	  0,
		 "Output to FILE instead of standard output" },
	{ "input-format",  'f', "FORMAT", 0,
		 "Format of input (hex or binary, default: hex)" },
	{ "output-format", 'g', "FORMAT", 0,
		 "Format of output (hex, binary, text or c11, default: text)" },
	{ "tags",	   't', "FILE", 0,
		 "Definition of BER-TLV tags in JSON format" },
	{ 0 }
};

enum file_format {
	hex	= 0,
	binary	= 1,
	text	= 2,
	c11	= 3,
	invalid = 4
};

static enum file_format str2ff(const char *string)
{
	if (!strcmp("hex", string))
		return hex;
	if (!strcmp("binary", string))
		return binary;
	if (!strcmp("text", string))
		return text;
	if (!strcmp("c11", string))
		return c11;
	return invalid;
}

struct arguments {
	char *input;
	enum file_format input_format;
	int fd_input;
	char *output;
	enum file_format output_format;
	int fd_output;
	char *tags;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
	struct arguments *arguments = (struct arguments *)state->input;

	switch (key) {
	case 'i':
		arguments->input = arg;
		break;
	case 'o':
		arguments->output = arg;
		break;
	case 't':
		arguments->tags = arg;
		break;
	case 'f':
		arguments->input_format = str2ff(arg);
		if ((arguments->input_format != binary) &&
					     (arguments->input_format != hex)) {
			fprintf(stderr, "Invalid input format '%s' ", arg);
			fprintf(stderr, "Choose 'binary' or 'hex'\n");
			argp_usage(state);
		}
		break;
	case 'g':
		arguments->output_format = str2ff(arg);
		if (arguments->output_format == invalid) {
			fprintf(stderr, "Invalid output format '%s'. ", arg);
			fprintf(stderr,
				   "Choose 'binary', 'hex', 'text' or 'c11'\n");
			argp_usage(state);
		}
		break;
	case ARGP_KEY_ARG:
		argp_usage(state);
		break;
	default:
		return ARGP_ERR_UNKNOWN;
	}

	return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc };

static int binary_to_hex(const uint8_t *binary, size_t binary_len,
						 uint8_t **hex, size_t *hex_len)
{
	int i = 0;

	assert(binary);
	assert(hex);
	assert(hex_len);

	*hex_len = binary_len * 2;
	*hex = malloc(*hex_len);
	if (!*hex)
		return TLV_RC_OUT_OF_MEMORY;

	for (i = 0; i < binary_len; i++) {
		static const uint8_t nibble_to_hex[16] = {
			'0', '1', '2', '3', '4', '5', '6', '7',
			'8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
		};

		(*hex)[2 * i]	  = nibble_to_hex[binary[i] >> 4];
		(*hex)[2 * i + 1] = nibble_to_hex[binary[i] & 0xF];
	}

	return TLV_RC_OK;
}

static int hex_to_binary(const uint8_t *hex, size_t hex_len, uint8_t **binary,
							     size_t *binary_len)
{
	uint8_t *result = NULL;
	size_t buffer_len = 256, i_hex = 0, nibbles = 0;
	bool comment = false;

	assert(hex);
	assert(binary);
	assert(binary_len);

	*binary = NULL;
	*binary_len = 0;

	result = malloc(buffer_len);
	if (!result)
		return TLV_RC_OUT_OF_MEMORY;

	for (i_hex = 0, nibbles = 0, comment = false; i_hex < hex_len;
								      i_hex++) {

		if ((nibbles / 2) >= buffer_len) {
			uint8_t *new_buffer = NULL;

			buffer_len *= 2;
			new_buffer = realloc(result, buffer_len);
			if (!new_buffer) {
				free(result);
				return TLV_RC_OUT_OF_MEMORY;
			}
			result = new_buffer;
		}

		if (!comment) {
			if ((hex[i_hex] >= '0') && (hex[i_hex] <= '9')) {
				result[nibbles >> 1] <<= 4;
				result[nibbles >> 1] |= hex[i_hex] - '0';
				nibbles++;
			}
			if ((tolower(hex[i_hex]) >= 'a') &&
						 (tolower(hex[i_hex]) <= 'f')) {
				result[nibbles >> 1] <<= 4;
				result[nibbles >> 1] |= tolower(hex[i_hex]) -
								       'a' + 10;
				nibbles++;
			}
			if (hex[i_hex] == '#')
				comment = true;
		} else {
			if (hex[i_hex] == '\n')
				comment = false;
		}
	}

	if (nibbles % 2) {
		free(result);
		return TLV_RC_INVALID_ARG;
	}

	*binary = result;
	*binary_len = nibbles / 2;
	return TLV_RC_OK;
}

static int write_file(int fd, uint8_t *contents, size_t len)
{
	ssize_t rc = 0;
	size_t written_bytes = 0;

	assert(contents);

	do {
		rc = write(fd, &contents[written_bytes], len - written_bytes);
		if (rc < 0)
			return TLV_RC_IO_ERROR;

		written_bytes += (size_t)rc;
	} while ((rc > 0) && (written_bytes < len));

	if (written_bytes < len)
		return TLV_RC_IO_ERROR;

	return TLV_RC_OK;
}

static int read_file(int fd, uint8_t **contents, size_t *len)
{
	uint8_t *result = NULL;
	size_t buffer_len = 256;
	size_t read_bytes = 0;
	ssize_t rc = 0;


	assert(contents);
	assert(len);

	*contents = NULL;
	*len = 0;

	result = malloc(buffer_len);
	if (!result)
		return TLV_RC_OUT_OF_MEMORY;

	do {
		rc = read(fd, &(result[read_bytes]), buffer_len - read_bytes);
		if (rc < 0) {
			free(result);
			return TLV_RC_IO_ERROR;
		}

		read_bytes += (size_t)rc;

		if (read_bytes >= buffer_len) {
			uint8_t *temp = NULL;

			buffer_len *= 2;
			temp = realloc(result, buffer_len);

			if (!temp) {
				free(result);
				return TLV_RC_OUT_OF_MEMORY;
			}

			result = temp;
		}
	} while (rc > 0);

	*contents = result;
	*len = read_bytes;

	return TLV_RC_OK;
}

static int encode_to_text_file(int fd, struct tlv *tlv,
			       struct emv_tag_descriptor *tags, size_t num_tags)
{
	do {
		struct emv_tag_descriptor *desc = NULL;
		uint8_t buffer[4096];
		char line[4096];
		size_t size;
		ssize_t rc = 0;
		int i, j, len = 0;
		int depth = tlv_get_depth(tlv);

		for (i = 0, j = 0; i < depth; i++, j += 2)
			len += snprintf(&line[len], sizeof(line) - len, "  ");
		size = sizeof(buffer);
		tlv_encode_identifier(tlv, buffer, &size);
		for (i = 0; i < (int)size; i++, j += 2)
			len += snprintf(&line[len], sizeof(line) - len, "%02X",
								     buffer[i]);
		len += snprintf(&line[len], sizeof(line) - len, " ");
		for (i = 0; i < num_tags; i++)
			if ((tags[i].tag.len == size) &&
				       !memcmp(tags[i].tag.value, buffer, size))
				desc = &tags[i];
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

		tlv = tlv_iterate(tlv);
	} while (tlv);

	return TLV_RC_OK;

}

static int encode_to_c11_file(int fd, struct tlv *tlv,
			       struct emv_tag_descriptor *tags, size_t num_tags)
{
	FILE *out = fdopen(fd, "w");

	fprintf(out, "const unsigned char ber_tlv[] = {\n");

	do {
		struct emv_tag_descriptor *desc = NULL;
		uint8_t buffer[256];
		size_t size;
		int i, depth = tlv_get_depth(tlv);

		size = sizeof(buffer);
		tlv_encode_identifier(tlv, buffer, &size);

		for (i = 0; i < num_tags; i++)
			if ((tags[i].tag.len == size) &&
				       !memcmp(tags[i].tag.value, buffer, size))
				desc = &tags[i];

		if (desc) {
			for (i = 0; i <= depth; i++)
				fprintf(out, "\t");
			fprintf(out, "/* %s */\n", desc->name);
		}

		for (i = 0; i <= depth; i++)
			fprintf(out, "\t");

		for (i = 0; i < size; i++)
			fprintf(out, "0x%02X, ", buffer[i]);

		size = sizeof(buffer);
		tlv_encode_length(tlv, buffer, &size);

		for (i = 0; i < size; i++)
			fprintf(out, "0x%02X,%s", buffer[i],
						       i < size - 1 ? " " : "");

		size = sizeof(buffer);
		tlv_encode_value(tlv, buffer, &size);

		if (size) {
			fprintf(out, " ");
			for (i = 0; i < size; i++)
				fprintf(out, "0x%02X,%s", buffer[i],
						       i < size - 1 ? " " : "");
		}

		fprintf(out, "\n");

		tlv = tlv_iterate(tlv);
	} while (tlv);

	fprintf(out, "};\n");

	return TLV_RC_OK;
}

int main(int argc, char **argv)
{
	struct emv_tag_descriptor *tags = NULL;
	size_t num_tags = 0;
	struct tlv *tlv = NULL;
	int rc = 0;
	struct arguments arguments;
	uint8_t *der = NULL;
	size_t der_len = 0;

	if (log4c_init()) {
		fprintf(stderr, "log4c_init() failed!\n");
		return EXIT_FAILURE;
	}

	libtlv_init("tlvdump");

	memset(&arguments, 0, sizeof(arguments));
	arguments.input_format = hex;
	arguments.output_format = text;

	argp_parse(&argp, argc, argv, 0, 0, &arguments);

	if (arguments.input) {
		arguments.fd_input = open(arguments.input, O_RDONLY);
		if (arguments.fd_input < 0) {
			fprintf(stderr, "open('%s') failed: %s\n",
					      arguments.input, strerror(errno));
			return EXIT_FAILURE;
		}
	} else {
		arguments.fd_input = STDIN_FILENO;
	}

	if (arguments.output) {
		arguments.fd_output = open(arguments.output, O_WRONLY | O_CREAT,
			       S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH |
								       S_IWOTH);
		if (arguments.fd_output < 0) {
			fprintf(stderr, "open('%s') failed: %s\n",
					     arguments.output, strerror(errno));
		}
	} else {
		arguments.fd_output = STDOUT_FILENO;
	}

	if (arguments.tags) {
		int fd = open(arguments.tags, O_RDONLY);

		if (fd < 0) {
			fprintf(stderr, "open('%s') failed: %s\n",
					       arguments.tags, strerror(errno));
			return EXIT_FAILURE;
		}

		rc = read_file(fd, &der, &der_len);
		if (rc != TLV_RC_OK) {
			fprintf(stderr, "Failed to read file '%s'\n",
								arguments.tags);
			return EXIT_FAILURE;
		}

		rc = emv_tag_parse_descriptors((char *)der, &tags, &num_tags);
		if (rc != EMV_RC_OK) {
			fprintf(stderr, "Failed to parse file '%s'\n",
								arguments.tags);
			return EXIT_FAILURE;
		}

		free(der);
	}

	rc = read_file(arguments.fd_input, &der, &der_len);
	if (rc != TLV_RC_OK) {
		fprintf(stderr, "Failed to read file '%s'\n",
				   arguments.input ? arguments.input : "stdin");
		return EXIT_FAILURE;
	}

	if (arguments.input_format == hex) {
		uint8_t *temp = NULL;
		size_t temp_len = 0;

		rc = hex_to_binary(der, der_len, &temp, &temp_len);
		if (rc != TLV_RC_OK) {
			free(der);
			fprintf(stderr, "Failed to parse input file '%s'.\n",
				   arguments.input ? arguments.input : "stdin");
			return EXIT_FAILURE;
		}

		free(der);
		der = temp;
		der_len = temp_len;
	}

	rc = tlv_parse(der, der_len, &tlv);
	if (rc != TLV_RC_OK) {
		free(der);
		fprintf(stderr, "Failed to parse input file '%s'.\n",
				   arguments.input ? arguments.input : "stdin");
		return EXIT_FAILURE;
	}

	free(der);
	der = NULL;
	der_len = 0;

	if ((arguments.output_format == binary) ||
					     (arguments.output_format == hex)) {
		rc = tlv_encode(tlv, NULL, &der_len);
		if (rc != TLV_RC_OK) {
			fprintf(stderr, "Failed to encode output.\n");
			return EXIT_FAILURE;
		}

		der = malloc(der_len);
		if (!der) {
			fprintf(stderr, "Out of memory error.\n");
			return EXIT_FAILURE;
		}

		rc = tlv_encode(tlv, der, &der_len);
		if (rc != TLV_RC_OK) {
			fprintf(stderr, "Failed to encode output.\n");
			return EXIT_FAILURE;
		}

		if (arguments.output_format == hex) {
			uint8_t *temp = NULL;
			size_t temp_len = 0;

			rc = binary_to_hex(der, der_len, &temp, &temp_len);
			if (rc != TLV_RC_OK) {
				free(der);
				fprintf(stderr, "Failed to encode output.\n");
				return EXIT_FAILURE;
			}

			free(der);
			der = temp;
			der_len = temp_len;
		}

		rc = write_file(arguments.fd_output, der, der_len);
		if (rc != TLV_RC_OK) {
			fprintf(stderr, "Failed to write file '%s'\n",
				arguments.output ? arguments.output : "stdout");
			return EXIT_FAILURE;
		}
	} else if (arguments.output_format == text) {
		rc = encode_to_text_file(arguments.fd_output, tlv, tags,
								      num_tags);
		if (rc != TLV_RC_OK) {
			fprintf(stderr, "Failed to encode output file '%s'.\n",
							      arguments.output);
			tlv_free(tlv);
			return EXIT_FAILURE;
		}
	} else if (arguments.output_format == c11) {
		rc = encode_to_c11_file(arguments.fd_output, tlv, tags,
								      num_tags);
		if (rc != TLV_RC_OK) {
			fprintf(stderr, "Failed to encode output file '%s'.\n",
							      arguments.output);
			tlv_free(tlv);
			return EXIT_FAILURE;
		}
	}

	tlv_free(tlv);
	return EXIT_SUCCESS;
}
