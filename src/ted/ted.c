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

struct ted {
	const struct lws_protocols *protocols;
	struct lws_context	   *lws_context;
	bool			    stop_requested;
	struct json_object	   *doc;
};

static struct ted ted;

int ted_method_get_doc(json_object *params, json_object **result)
{
	if (params)
		return JSONRPC_RC_INVALID_PARAMS;

	json_object_get(ted.doc);
	*result = ted.doc;

	return JSONRPC_RC_OK;
}

struct jsonrpc_method ted_methods[] = {
	{
		.name = "get_doc",
		.stub = ted_method_get_doc
	},
	{ NULL }
};

static const struct lws_protocols ted_protocols[] = {
	{
		.name			= "http",
		.callback		= callback_http,
	},
	{
		.name			= "jsonrpc",
		.callback		= callback_jsonrpc,
		.per_session_data_size	= sizeof(struct jsonrpc_session),
		.user			= ted_methods
	},
	{ NULL }
};

static struct ted ted = {
	.lws_context	= NULL,
	.protocols	= ted_protocols,
	.stop_requested	= false,
	.doc		= NULL
};

static void sigint_handler(int sig)
{
	ted.stop_requested = true;
	lws_cancel_service(ted.lws_context);
}

int ted_init(int argc, char **argv)
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
	info.protocols = ted.protocols;
	info.gid = -1;
	info.uid = -1;

	ted.lws_context = lws_create_context(&info);
	if (!ted.lws_context) {
		fprintf(stderr, "%s(): lws_create_context failed!\n", __func__);
		return -1;
	}

	ted.doc = ted_parse_input(&args);

	signal(SIGINT, sigint_handler);

	return 0;
}

void ted_term(void)
{
	if (ted.doc)
		json_object_put(ted.doc);
	lws_context_destroy(ted.lws_context);
}

void ted_serve(void)
{
	while (!ted.stop_requested)
		lws_service(ted.lws_context, 100);
}

int main(int argc, char **argv)
{
	if (ted_init(argc, argv))
		return EXIT_FAILURE;

	ted_serve();

	ted_term();

	return EXIT_SUCCESS;
}
