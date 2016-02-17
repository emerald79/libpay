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

static const char *get_mimetype(const char *file)
{
	int n = strlen(file);

	if (n < 5)
		return NULL;

	if (!strcmp(&file[n - 4], ".ico"))
		return "image/x-icon";

	if (!strcmp(&file[n - 4], ".png"))
		return "image/png";

	if (!strcmp(&file[n - 5], ".html"))
		return "text/html";

	if (!strcmp(&file[n - 4], ".css"))
		return "text/css";

	if (!strcmp(&file[n - 3], ".js"))
		return "text/javascript";

	return "";
}

int callback_http(struct lws *wsi, enum lws_callback_reasons reason,
					       void *user, void *in, size_t len)
{
	char filename[256];
	int rc = 0;

	switch (reason) {

	case LWS_CALLBACK_HTTP:

		strcpy(filename, "./www");

		if (!strcmp((char *)in, "/"))
			strcat(filename, "/index.html");
		else
			strcat(filename, (char *)in);

		rc = lws_serve_http_file(wsi, filename, get_mimetype(filename),
								       NULL, 0);
		if (rc < 0)
			goto done;
		break;

	default:
		break;
	}

done:
	return rc;
}
