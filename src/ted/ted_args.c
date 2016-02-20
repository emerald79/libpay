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
#include <argp.h>

#include "ted.h"

const char *argp_program_version = "ted " PACKAGE_VERSION;
const char *argp_program_bug_address = "mijung@gmx.net";
static const char doc[] = "TED - The TLV Editor";
static const char args_doc[] = "";

static struct argp_option options[] = {
	{ "port",	'p',	"<tcpip-port>", 0,
					     "TCP/IP port TED shall serve on" },
	{ "iface",	'i',	"<interface>",	   0,
			 "Network interface TED shall serve on (e.g. 'eth0')" },
	{ 0 }
};

static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
	struct ted_args *args = (struct ted_args *)state->input;
	long int long_int = 0;

	switch (key) {

	case 'i':
		args->iface = arg;
		break;

	case 'p':
		long_int = strtol(arg, NULL, 0);
		if (long_int < 1 || long_int > UINT16_MAX) {
			fprintf(stderr, "Invalid port number: '%s'\n", arg);
			argp_usage(state);
		}
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
