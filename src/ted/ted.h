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

#ifndef __LIBPAY_TED_H__
#define __LIBPAY_TED_H__

#include <libwebsockets.h>
#include <json-c/json.h>

/*-----------------------------------------------------------------------------+
| TED command line argument parsing					       |
+-----------------------------------------------------------------------------*/

struct ted_args {
	int port;
	char *iface;
};

int ted_parse_args(int argc, char **argv, struct ted_args *ted_args);

/*-----------------------------------------------------------------------------+
| HTTP libwebsockets protocol implementation				       |
+-----------------------------------------------------------------------------*/

int callback_http(struct lws *wsi, enum lws_callback_reasons reason, void *user,
							  void *in, size_t len);

/*-----------------------------------------------------------------------------+
| JSON-RPC 2.0 libwebsockets protocol implementation			       |
+-----------------------------------------------------------------------------*/

#define JSONRPC_RC_OK			 0
#define JSONRPC_RC_INVALID_REQUEST	-32600
#define JSONRPC_RC_METHOD_NOT_FOUND	-32601
#define JSONRPC_RC_INVALID_PARAMS	-32602

struct jsonrpc_method {
	const char *name;
	int (*stub)(json_object *params, json_object **result);
};

struct jsonrpc_session {
	struct lws		*wsi;
	struct json_tokener	*tok;
};

int callback_jsonrpc(struct lws *wsi, enum lws_callback_reasons reason,
					      void *user, void *in, size_t len);

#endif						     /* ndef __LIBPAY_TED_H__ */
