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
#include <argp.h>

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
