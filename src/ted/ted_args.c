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

#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <argp.h>
#include <assert.h>

#include "ted.h"

const char *argp_program_version = "ted " PACKAGE_VERSION;
const char *argp_program_bug_address = "mijung@gmx.net";
static const char doc[] = "TED - TLV Editor";
static const char args_doc[] = "";

static struct argp_option options[] = {
	{ "port",	  'p',	"<tcpip-port>",	0,
			"TCP/IP port TED shall serve on"		      },
	{ "iface",	  'c',	"<interface>",	0,
			"Network interface TED shall serve on (e.g. 'eth0')"  },
	{ "input-format", 'f',  "<format>",	0,
			"'hex' or 'binary'. Default: 'hex'"		      },
	{ "input",	  'i',	"<filename>",	0,
			"Input from <filename> instead of standard input"     },
	{ 0 }
};

static enum ted_file_format str2ff(const char *string)
{
	if (!strcmp("hex", string))
		return ted_hex;
	if (!strcmp("binary", string))
		return ted_binary;
	return ted_num_file_formats;
}

static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
	struct ted_args *args = (struct ted_args *)state->input;
	long int long_int = 0;

	switch (key) {

	case 'i':
		if (access(arg, R_OK))
			argp_failure(state, EXIT_FAILURE, errno,
					       "Invalid input file: '%s'", arg);
		args->input = arg;
		break;

	case 'f':
		args->input_format = str2ff(arg);
		if (args->input_format == ted_num_file_formats)
			argp_error(state, "Invalid input file format: '%s'",
									   arg);
		break;

	case 'c':
		args->iface = arg;
		break;

	case 'p':
		long_int = strtol(arg, NULL, 0);
		if (long_int < 1 || long_int > UINT16_MAX)
			argp_error(state, "Invalid TCP/IP port: '%s'", arg);
		args->port = (int)long_int;
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

int ted_parse_args(int argc, char **argv, struct ted_args *ted_args)
{
	errno = argp_parse(&argp, argc, argv, 0, 0, ted_args);

	return errno ? -1 : 0;
}

/**
 * Parses an ASCII-HEX encoded string into its binary representation.
 *
 * Non hex characters (i.e. characters other than [0-9A-Fa-f] are ignored.  A
 * hash mark (#) starts a comment, which continues until the end of the line
 * ('\n').
 *
 * @params [in]   hex	      The ASCII-HEX string to be converted.
 * @params [in]   hex_len     Length of said ASCII-HEX string.
 * @params [out]  binary      The resulting binary string.  The caller is
 *				responsible to eventually 'free' the respective
 *				buffer.
 * @params [out]  binary_len  Length of the resulting binary string.
 *
 * Returns TLV_RC_OK if successful, other TLC_RC_* codes in case of failure.
 */
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

struct tlv *ted_parse_input(struct ted_args *args)
{
	uint8_t *input = NULL;
	size_t input_sz;
	int fd = -1, rc;
	struct tlv *result = NULL;

	if (args->input) {
		fd = open(args->input, O_RDONLY);
		if (fd < 0) {
			fprintf(stderr, "open('%s') failed: %s", args->input,
							       strerror(errno));
			goto done;
		}
	} else {
		fd = STDIN_FILENO;
	}

	rc = read_file(fd, &input, &input_sz);
	if (rc != TLV_RC_OK) {
		fprintf(stderr, "read_file('%s') failed. rc %d.\n", args->input,
									    rc);
		goto done;
	}

	if (args->input_format == ted_hex) {
		uint8_t *temp = NULL;
		size_t temp_sz = 0;

		rc = hex_to_binary(input, input_sz, &temp, &temp_sz);
		if (rc != TLV_RC_OK) {
			fprintf(stderr, "%s() hex_to_binary failed. rc %d\n",
								  __func__, rc);
			goto done;
		}

		free(input);
		input = temp;
		input_sz = temp_sz;
	}

	rc = tlv_parse(input, input_sz, &result);
	if (rc != TLV_RC_OK) {
		fprintf(stderr, "%s() tlv_parse failed. rc %d\n", __func__, rc);
		goto done;
	}

done:
	if (input)
		free(input);

	if ((fd >= 0) && (fd != STDIN_FILENO))
		close(fd);

	return result;
}
