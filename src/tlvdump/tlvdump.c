/*
 * libtlv - Support Library for EMV TLV handling.
 * Copyright (C) 2015 Michael Jung <mijung@gmx.net>, All rights reserved.
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
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include <tlv.h>

int main(int argc, char **argv)
{
	struct tlv *tlv = NULL;
	int rc = 0, fd_in = -1, fd_out = -1;

	if (argc != 3) {
		fprintf(stderr, "Usage: %s input-file output-file\n", argv[0]);
		return EXIT_FAILURE;
	}

	fd_in = open(argv[1], O_RDONLY);
	if (fd_in < 0) {
		fprintf(stderr, "open('%s') failed: %s\n", argv[1],
							       strerror(errno));
		return EXIT_FAILURE;
	}

	rc = tlv_parse_file(fd_in, &tlv);
	if (rc != TLV_RC_OK) {
		close(fd_in);
		return EXIT_FAILURE;
	}
	close(fd_in);

	fd_out = open(argv[2], O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP |
						   S_IWGRP | S_IROTH | S_IWOTH);
	if (fd_out < 0) {
		fprintf(stderr, "open('%s') failed: %s\n", argv[2],
							       strerror(errno));
		tlv_free(tlv);
		return EXIT_FAILURE;
	}

	rc = tlv_encode_file(fd_out, tlv);
	if (rc != TLV_RC_OK) {
		close(fd_out);
		tlv_free(tlv);
		return EXIT_FAILURE;
	}
	close(fd_out);

	return EXIT_SUCCESS;
}
