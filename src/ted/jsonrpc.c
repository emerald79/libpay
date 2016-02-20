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
#include <string.h>

static int jsonrpc_session_init(struct jsonrpc_session *session,
								struct lws *wsi)
{
	session->wsi = wsi;
	session->tok = json_tokener_new();
	if (!session->tok)
		return -1;

	return 0;
}

static void jsonrpc_session_free(struct jsonrpc_session *session)
{
	json_tokener_free(session->tok);
	session->tok = NULL;
}

static struct json_object *jsonrpc_error_response(int jsonrpc_rc,
							 struct json_object *id)
{
	struct json_object *response = json_object_new_object();
	struct json_object *error = json_object_new_object();
	char msg[32];

	switch (jsonrpc_rc) {
	case JSONRPC_RC_INVALID_REQUEST:
		strncpy(msg, "Invalid request", sizeof(msg));
		break;
	case JSONRPC_RC_INVALID_PARAMS:
		strncpy(msg, "Invalid params", sizeof(msg));
		break;
	case JSONRPC_RC_METHOD_NOT_FOUND:
		strncpy(msg, "Method not found", sizeof(msg));
		break;
	default:
		snprintf(msg, sizeof(msg), "Error code: %d", jsonrpc_rc);
		break;
	}

	json_object_object_add(error, "code", json_object_new_int(jsonrpc_rc));
	json_object_object_add(error, "message", json_object_new_string(msg));

	json_object_object_add(response, "jsonrpc",
						 json_object_new_string("2.0"));
	json_object_object_add(response, "error", error);
	json_object_object_add(response, "id", id);

	return response;
}

static struct json_object *jsonrpc_result_response(struct json_object *result,
							 struct json_object *id)
{
	struct json_object *response = json_object_new_object();

	json_object_object_add(response, "jsonrpc",
						 json_object_new_string("2.0"));
	json_object_object_add(response, "result", result);
	json_object_object_add(response, "id", id);

	return response;
}

static void jsonrpc_call(struct jsonrpc_session *session,
		     struct json_object *request, struct json_object **response)
{
	const char *method_name = NULL;
	struct json_object *obj = NULL, *id = NULL, *result = NULL;
	struct json_object *params = NULL;
	struct jsonrpc_method *method = NULL, *methods = NULL;
	int jsonrpc_rc = JSONRPC_RC_OK;

	if (json_object_get_type(request) != json_type_object) {
		*response = jsonrpc_error_response(JSONRPC_RC_INVALID_REQUEST,
									    id);
		return;
	}

	json_object_object_get_ex(request, "id", &id);
	json_object_object_get_ex(request, "params", &params);

	if ((!json_object_object_get_ex(request, "jsonrpc", &obj)) ||
	    (json_object_get_type(obj) != json_type_string) ||
	    (strcmp(json_object_get_string(obj), "2.0")) ||
	    (!json_object_object_get_ex(request, "method", &obj))) {
		*response = jsonrpc_error_response(JSONRPC_RC_INVALID_REQUEST,
									    id);
		return;
	}

	method_name = json_object_get_string(obj);

	methods = (struct jsonrpc_method *)
					 (lws_get_protocol(session->wsi)->user);

	for (method = methods; method->name; method++)
		if (!strcmp(method->name, method_name))
			break;

	if (!method->name) {
		*response = jsonrpc_error_response(JSONRPC_RC_METHOD_NOT_FOUND,
									    id);
		return;
	}


	jsonrpc_rc = method->stub(params, &result);

	if (jsonrpc_rc != JSONRPC_RC_OK) {
		*response = jsonrpc_error_response(jsonrpc_rc, id);
		return;
	}

	*response = jsonrpc_result_response(result, id);
	return;
}

static int jsonrpc_session_receive(struct jsonrpc_session *session, void *in,
								     size_t len)
{
	struct json_tokener *tok = session->tok;
	struct json_object *request = NULL, *response = NULL;
	unsigned char *reply = NULL;
	size_t reply_len = 0;
	int rc = 0;

	request = json_tokener_parse_ex(tok, (const char *)in, (int)len);
	if (!request) {
		enum json_tokener_error err = json_tokener_get_error(tok);

		if (err == json_tokener_continue)
			return 0;

		return -1;
	}

	jsonrpc_call(session, request, &response);

	reply_len = LWS_PRE + strlen(json_object_get_string(response)) + 1;
	reply = (unsigned char *)calloc(1, reply_len);
	strcpy((char *)(&reply[LWS_PRE]), json_object_get_string(response));
	lws_write(session->wsi, &reply[LWS_PRE], reply_len - LWS_PRE,
								LWS_WRITE_TEXT);
	free(reply);

	json_object_put(request);
	json_object_put(response);

	return 0;
}

int callback_jsonrpc(struct lws *wsi, enum lws_callback_reasons reason,
					       void *user, void *in, size_t len)
{
	struct jsonrpc_session *session = (struct jsonrpc_session *)user;

	switch (reason) {

	case LWS_CALLBACK_PROTOCOL_INIT:
	case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
	case LWS_CALLBACK_WS_PEER_INITIATED_CLOSE:
	case LWS_CALLBACK_PROTOCOL_DESTROY:
		break;

	case LWS_CALLBACK_ESTABLISHED:
		return jsonrpc_session_init(session, wsi);

	case LWS_CALLBACK_CLOSED:
		jsonrpc_session_free(session);
		break;

	case LWS_CALLBACK_RECEIVE:
		return jsonrpc_session_receive(session, in, len);
		break;

	default:
		fprintf(stderr, "%s(reason: %d, '%s')\n", __func__, (int)reason,
								    (char *)in);
		break;
	}

	return 0;
}
