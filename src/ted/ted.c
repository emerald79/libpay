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

#include "ted.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

int ted_srv_method_substract(json_object *params, json_object **result)
{
	int minuend = 0, subtrahend = 0;

	*result = NULL;

	if ((!params) ||
	    (json_object_get_type(params) != json_type_array) ||
	    (json_object_array_length(params) != 2) ||
	    (json_object_get_type(json_object_array_get_idx(params, 0)) !=
							       json_type_int) ||
	    (json_object_get_type(json_object_array_get_idx(params, 1)) !=
								 json_type_int))
		return JSONRPC_RC_INVALID_PARAMS;

	minuend = json_object_get_int(json_object_array_get_idx(params, 0));
	subtrahend = json_object_get_int(json_object_array_get_idx(params, 1));

	*result = json_object_new_int(minuend - subtrahend);
	return JSONRPC_RC_OK;
}

struct jsonrpc_method ted_srv_methods[] = {
	{
		.name = "substract",
		.stub = ted_srv_method_substract
	},
	{ NULL }
};

struct ted_srv {
	const struct lws_protocols *protocols;
	struct lws_context	   *lws_context;
	bool			    stop_requested;

	struct tlv		   *tlv;
};

static const struct lws_protocols ted_srv_protocols[] = {
	{
		.name			= "http",
		.callback		= callback_http,
	},
	{
		.name			= "jsonrpc",
		.callback		= callback_jsonrpc,
		.per_session_data_size	= sizeof(struct jsonrpc_session),
		.user			= ted_srv_methods
	},
	{ NULL }
};

static struct ted_srv ted_srv = {
	.lws_context	= NULL,
	.protocols	= ted_srv_protocols,
	.stop_requested	= false,
	.tlv		= NULL
};

static void sigint_handler(int sig)
{
	ted_srv.stop_requested = true;
	lws_cancel_service(ted_srv.lws_context);
}

int ted_srv_init(int argc, char **argv)
{
	struct ted_args args;
	struct lws_context_creation_info info;

	memset(&args, 0, sizeof(args));
	args.port = 7681;

	if (ted_parse_args(argc, argv, &args)) {
		fprintf(stderr, "Failed to parse command line arguments: %s\n",
							       strerror(errno));
		return -1;
	}

	memset(&info, 0, sizeof(info));
	info.port = args.port;
	info.iface = args.iface;
	info.protocols = ted_srv.protocols;
	info.gid = -1;
	info.uid = -1;

	ted_srv.lws_context = lws_create_context(&info);
	if (!ted_srv.lws_context) {
		fprintf(stderr, "%s(): lws_create_context failed!\n", __func__);
		return -1;
	}

	ted_srv.tlv = ted_parse_input(&args);

	signal(SIGINT, sigint_handler);

	return 0;
}

void ted_srv_term(void)
{
	tlv_free(ted_srv.tlv);
	lws_context_destroy(ted_srv.lws_context);
}

void ted_srv_serve(void)
{
	while (!ted_srv.stop_requested)
		lws_service(ted_srv.lws_context, 100);
}

int main(int argc, char **argv)
{
	if (ted_srv_init(argc, argv))
		return EXIT_FAILURE;

	ted_srv_serve();

	ted_srv_term();

	return EXIT_SUCCESS;
}
