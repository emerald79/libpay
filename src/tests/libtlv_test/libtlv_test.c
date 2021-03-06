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

#include <stdio.h>
#include <stdlib.h>
#include <check.h>
#include <log4c.h>

#include <libpay/tlv.h>

START_TEST(test_tlv_malformed_input)
{
	int rc = TLV_RC_OK;

	struct tlv *tlv = NULL;
	const uint8_t tag_number_too_large[] = {
		0x1Fu, 0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu, 0x7Fu
	};
	const uint8_t buffer_overflow_1[] = { 0x1Fu };
	const uint8_t buffer_overflow_2[] = { 0x1Fu, 0xFFu, 0xFFu, 0xFFu };

	rc = tlv_parse(NULL, sizeof(tag_number_too_large), &tlv);
	ck_assert(rc == TLV_RC_INVALID_ARG);

	rc = tlv_parse(tag_number_too_large, 0, &tlv);
	ck_assert((rc == TLV_RC_OK) && (!tlv));

	rc = tlv_parse(tag_number_too_large, sizeof(tag_number_too_large),
									  NULL);
	ck_assert(rc == TLV_RC_INVALID_ARG);

	rc = tlv_parse(tag_number_too_large, sizeof(tag_number_too_large),
									  &tlv);
	ck_assert(rc == TLV_RC_TAG_NUMBER_TOO_LARGE);

	rc = tlv_parse(buffer_overflow_1, sizeof(buffer_overflow_1), &tlv);
	ck_assert(rc == TLV_RC_UNEXPECTED_END_OF_STREAM);

	rc = tlv_parse(buffer_overflow_2, sizeof(buffer_overflow_2), &tlv);
	ck_assert(rc == TLV_RC_UNEXPECTED_END_OF_STREAM);
}
END_TEST

START_TEST(test_tlv_primitive_encoding)
{
	int rc = TLV_RC_OK;
	struct tlv *tlv = NULL;
	const uint8_t mueller[] = {
		0x0C, 0x06, 0x4D, 0x7E, 0x6C, 0x6C, 0x65, 0x72
	};
	const uint8_t four_encodings[] = {
		0x0C, 0x06, 0x4D, 0x7E, 0x6C, 0x6C, 0x65, 0x72,
		0x02, 0x01, 0x1E,
		0x01, 0x01, 0x00,
		0x80, 0x01, 0x00
	};

	rc = tlv_parse(mueller, sizeof(mueller), &tlv);
	ck_assert_msg(rc == TLV_RC_OK, "rc %d\n", rc);
	tlv_free(tlv);

	rc = tlv_parse(four_encodings, sizeof(four_encodings), &tlv);
	ck_assert(rc == TLV_RC_OK);
	tlv_free(tlv);
}
END_TEST

START_TEST(test_tlv_constructed_encoding)
{
	int rc = TLV_RC_OK;
	struct tlv *tlv = NULL;
	const uint8_t constructed[] = {
		0x30, 0x11,
			0x0C, 0x06, 0x4D, 0x7E, 0x6C, 0x6C, 0x65, 0x72,
			0x02, 0x01, 0x1E,
			0x01, 0x01, 0x00,
			0x80, 0x01, 0x00
	};

	rc = tlv_parse(constructed, sizeof(constructed), &tlv);
	ck_assert_msg(rc == TLV_RC_OK, "rc %d\n", rc);
	tlv_free(tlv);
}
END_TEST

START_TEST(test_tlv_verisign_x509)
{
	int rc = TLV_RC_OK;
	struct tlv *tlv = NULL;
	uint8_t encoded[1024];
	const uint8_t verisign_x509[576] = {
		0x30, 0x82, 0x02, 0x3c, 0x30, 0x82, 0x01, 0xa5, 0x02, 0x10,
		0x3f, 0x69, 0x1e, 0x81, 0x9c, 0xf0, 0x9a, 0x4a, 0xf3, 0x73,
		0xff, 0xb9, 0x48, 0xa2, 0xe4, 0xdd, 0x30, 0x0d, 0x06, 0x09,
		0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x05, 0x05,
		0x00, 0x30, 0x5f, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55,
		0x04, 0x06, 0x13, 0x02, 0x55, 0x53, 0x31, 0x17, 0x30, 0x15,
		0x06, 0x03, 0x55, 0x04, 0x0a, 0x13, 0x0e, 0x56, 0x65, 0x72,
		0x69, 0x53, 0x69, 0x67, 0x6e, 0x2c, 0x20, 0x49, 0x6e, 0x63,
		0x2e, 0x31, 0x37, 0x30, 0x35, 0x06, 0x03, 0x55, 0x04, 0x0b,
		0x13, 0x2e, 0x43, 0x6c, 0x61, 0x73, 0x73, 0x20, 0x31, 0x20,
		0x50, 0x75, 0x62, 0x6c, 0x69, 0x63, 0x20, 0x50, 0x72, 0x69,
		0x6d, 0x61, 0x72, 0x79, 0x20, 0x43, 0x65, 0x72, 0x74, 0x69,
		0x66, 0x69, 0x63, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x20, 0x41,
		0x75, 0x74, 0x68, 0x6f, 0x72, 0x69, 0x74, 0x79, 0x30, 0x1e,
		0x17, 0x0d, 0x39, 0x36, 0x30, 0x31, 0x32, 0x39, 0x30, 0x30,
		0x30, 0x30, 0x30, 0x30, 0x5a, 0x17, 0x0d, 0x32, 0x38, 0x30,
		0x38, 0x30, 0x32, 0x32, 0x33, 0x35, 0x39, 0x35, 0x39, 0x5a,
		0x30, 0x5f, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04,
		0x06, 0x13, 0x02, 0x55, 0x53, 0x31, 0x17, 0x30, 0x15, 0x06,
		0x03, 0x55, 0x04, 0x0a, 0x13, 0x0e, 0x56, 0x65, 0x72, 0x69,
		0x53, 0x69, 0x67, 0x6e, 0x2c, 0x20, 0x49, 0x6e, 0x63, 0x2e,
		0x31, 0x37, 0x30, 0x35, 0x06, 0x03, 0x55, 0x04, 0x0b, 0x13,
		0x2e, 0x43, 0x6c, 0x61, 0x73, 0x73, 0x20, 0x31, 0x20, 0x50,
		0x75, 0x62, 0x6c, 0x69, 0x63, 0x20, 0x50, 0x72, 0x69, 0x6d,
		0x61, 0x72, 0x79, 0x20, 0x43, 0x65, 0x72, 0x74, 0x69, 0x66,
		0x69, 0x63, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x20, 0x41, 0x75,
		0x74, 0x68, 0x6f, 0x72, 0x69, 0x74, 0x79, 0x30, 0x81, 0x9f,
		0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d,
		0x01, 0x01, 0x01, 0x05, 0x00, 0x03, 0x81, 0x8d, 0x00, 0x30,
		0x81, 0x89, 0x02, 0x81, 0x81, 0x00, 0xe5, 0x19, 0xbf, 0x6d,
		0xa3, 0x56, 0x61, 0x2d, 0x99, 0x48, 0x71, 0xf6, 0x67, 0xde,
		0xb9, 0x8d, 0xeb, 0xb7, 0x9e, 0x86, 0x80, 0x0a, 0x91, 0x0e,
		0xfa, 0x38, 0x25, 0xaf, 0x46, 0x88, 0x82, 0xe5, 0x73, 0xa8,
		0xa0, 0x9b, 0x24, 0x5d, 0x0d, 0x1f, 0xcc, 0x65, 0x6e, 0x0c,
		0xb0, 0xd0, 0x56, 0x84, 0x18, 0x87, 0x9a, 0x06, 0x9b, 0x10,
		0xa1, 0x73, 0xdf, 0xb4, 0x58, 0x39, 0x6b, 0x6e, 0xc1, 0xf6,
		0x15, 0xd5, 0xa8, 0xa8, 0x3f, 0xaa, 0x12, 0x06, 0x8d, 0x31,
		0xac, 0x7f, 0xb0, 0x34, 0xd7, 0x8f, 0x34, 0x67, 0x88, 0x09,
		0xcd, 0x14, 0x11, 0xe2, 0x4e, 0x45, 0x56, 0x69, 0x1f, 0x78,
		0x02, 0x80, 0xda, 0xdc, 0x47, 0x91, 0x29, 0xbb, 0x36, 0xc9,
		0x63, 0x5c, 0xc5, 0xe0, 0xd7, 0x2d, 0x87, 0x7b, 0xa1, 0xb7,
		0x32, 0xb0, 0x7b, 0x30, 0xba, 0x2a, 0x2f, 0x31, 0xaa, 0xee,
		0xa3, 0x67, 0xda, 0xdb, 0x02, 0x03, 0x01, 0x00, 0x01, 0x30,
		0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01,
		0x01, 0x05, 0x05, 0x00, 0x03, 0x81, 0x81, 0x00, 0x58, 0x15,
		0x29, 0x39, 0x3c, 0x77, 0xa3, 0xda, 0x5c, 0x25, 0x03, 0x7c,
		0x60, 0xfa, 0xee, 0x09, 0x99, 0x3c, 0x27, 0x10, 0x70, 0xc8,
		0x0c, 0x09, 0xe6, 0xb3, 0x87, 0xcf, 0x0a, 0xe2, 0x18, 0x96,
		0x35, 0x62, 0xcc, 0xbf, 0x9b, 0x27, 0x79, 0x89, 0x5f, 0xc9,
		0xc4, 0x09, 0xf4, 0xce, 0xb5, 0x1d, 0xdf, 0x2a, 0xbd, 0xe5,
		0xdb, 0x86, 0x9c, 0x68, 0x25, 0xe5, 0x30, 0x7c, 0xb6, 0x89,
		0x15, 0xfe, 0x67, 0xd1, 0xad, 0xe1, 0x50, 0xac, 0x3c, 0x7c,
		0x62, 0x4b, 0x8f, 0xba, 0x84, 0xd7, 0x12, 0x15, 0x1b, 0x1f,
		0xca, 0x5d, 0x0f, 0xc1, 0x52, 0x94, 0x2a, 0x11, 0x99, 0xda,
		0x7b, 0xcf, 0x0c, 0x36, 0x13, 0xd5, 0x35, 0xdc, 0x10, 0x19,
		0x59, 0xea, 0x94, 0xc1, 0x00, 0xbf, 0x75, 0x8f, 0xd9, 0xfa,
		0xfd, 0x76, 0x04, 0xdb, 0x62, 0xbb, 0x90, 0x6a, 0x03, 0xd9,
		0x46, 0x35, 0xd9, 0xf8, 0x7c, 0x5b
	};
	size_t size = sizeof(encoded);

	rc = tlv_parse(verisign_x509, sizeof(verisign_x509), &tlv);
	ck_assert_msg(rc == TLV_RC_OK, "rc %d\n", rc);
	tlv_encode(tlv, encoded, &size);
	tlv_free(tlv);
	ck_assert(size == sizeof(verisign_x509));
	ck_assert(!memcmp(encoded, verisign_x509, size));
}
END_TEST

START_TEST(test_tlv_construct)
{
	struct tlv *tlv, *tlv2, *tlv3;
	uint8_t read_record_rsp[] = {
		0x70, 0x39, 0x5A, 0x0A, 0x67, 0x99, 0x99, 0x89, 0x00, 0x00,
		0x02, 0x00, 0x06, 0x9F, 0x5F, 0x24, 0x03, 0x25, 0x12, 0x31,
		0x5F, 0x25, 0x03, 0x04, 0x01, 0x01, 0x5F, 0x34, 0x01, 0x25,
		0x9F, 0x07, 0x02, 0xFF, 0x00, 0x9F, 0x0D, 0x05, 0xFC, 0x50,
		0xA0, 0x00, 0x00, 0x9F, 0x0E, 0x05, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x9F, 0x0F, 0x05, 0xF8, 0x70, 0xA4, 0x98, 0x00
	};
	uint8_t combined[] = {
		0x70, 0x74, 0x5A, 0x0A, 0x67, 0x99, 0x99, 0x89, 0x00, 0x00,
		0x02, 0x00, 0x06, 0x9F, 0x70, 0x39, 0x5A, 0x0A, 0x67, 0x99,
		0x99, 0x89, 0x00, 0x00, 0x02, 0x00, 0x06, 0x9F, 0x5F, 0x24,
		0x03, 0x25, 0x12, 0x31, 0x5F, 0x25, 0x03, 0x04, 0x01, 0x01,
		0x5F, 0x34, 0x01, 0x25, 0x9F, 0x07, 0x02, 0xFF, 0x00, 0x9F,
		0x0D, 0x05, 0xFC, 0x50, 0xA0, 0x00, 0x00, 0x9F, 0x0E, 0x05,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x9F, 0x0F, 0x05, 0xF8, 0x70,
		0xA4, 0x98, 0x00, 0x5F, 0x24, 0x03, 0x25, 0x12, 0x31, 0x5F,
		0x25, 0x03, 0x04, 0x01, 0x01, 0x5F, 0x34, 0x01, 0x25, 0x9F,
		0x07, 0x02, 0xFF, 0x00, 0x9F, 0x0D, 0x05, 0xFC, 0x50, 0xA0,
		0x00, 0x00, 0x9F, 0x0E, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x9F, 0x0F, 0x05, 0xF8, 0x70, 0xA4, 0x98, 0x00
	};
	uint8_t encoded[sizeof(combined)];
	size_t size;
	int rc;

	rc = tlv_parse(read_record_rsp, sizeof(read_record_rsp), &tlv);
	ck_assert(rc == TLV_RC_OK);

	rc = tlv_parse(read_record_rsp, sizeof(read_record_rsp), &tlv2);
	ck_assert(rc == TLV_RC_OK);

	tlv3 = tlv_insert_after(tlv_get_child(tlv), tlv2);
	ck_assert(tlv3 == tlv2);

	size = sizeof(encoded);
	rc = tlv_encode(tlv, encoded, &size);
	ck_assert(rc == TLV_RC_OK);

	tlv_free(tlv);

	ck_assert(size == sizeof(encoded));
	ck_assert(!memcmp(combined, encoded, size));
}
END_TEST

START_TEST(test_tlv_deep_find)
{
	const unsigned char termsetting2[] = {
		0xFF, 0x81, 0xE3, 0x71, 0x82, 0x04, 0x61, 0xFF, 0x82, 0xE3,
		0x71, 0x62, 0xDF, 0x83, 0xE3, 0x71, 0x04, 0x00, 0x09, 0x01,
		0x20, 0xFF, 0x84, 0xE3, 0x71, 0x12, 0xDF, 0x85, 0xE3, 0x71,
		0x07, 0xA0, 0x00, 0x00, 0x00, 0x65, 0x10, 0x10, 0xDF, 0x86,
		0xE3, 0x71, 0x01, 0x01, 0xFF, 0x84, 0xE3, 0x71, 0x12, 0xDF,
		0x85, 0xE3, 0x71, 0x07, 0xA0, 0x00, 0x00, 0x00, 0x03, 0x00,
		0x03, 0xDF, 0x86, 0xE3, 0x71, 0x01, 0x21, 0xDF, 0x88, 0xE3,
		0x71, 0x01, 0x01, 0xDF, 0x89, 0xE3, 0x71, 0x01, 0x01, 0xDF,
		0x8A, 0xE3, 0x71, 0x06, 0x00, 0x00, 0x00, 0x00, 0x01, 0x20,
		0xDF, 0x8B, 0xE3, 0x71, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x20, 0xDF, 0x8E, 0xE3, 0x71, 0x04, 0x84, 0x00, 0x80, 0x00,
		0xFF, 0x82, 0xE3, 0x71, 0x76, 0xDF, 0x83, 0xE3, 0x71, 0x04,
		0x00, 0x09, 0x01, 0x20, 0xFF, 0x84, 0xE3, 0x71, 0x12, 0xDF,
		0x85, 0xE3, 0x71, 0x07, 0xA0, 0x00, 0x00, 0x00, 0x04, 0x10,
		0x10, 0xDF, 0x86, 0xE3, 0x71, 0x01, 0x02, 0xFF, 0x84, 0xE3,
		0x71, 0x12, 0xDF, 0x85, 0xE3, 0x71, 0x07, 0xA0, 0x00, 0x00,
		0x00, 0x02, 0x00, 0x02, 0xDF, 0x86, 0xE3, 0x71, 0x01, 0x26,
		0xFF, 0x84, 0xE3, 0x71, 0x12, 0xDF, 0x85, 0xE3, 0x71, 0x07,
		0xA0, 0x00, 0x00, 0x00, 0x04, 0x10, 0x10, 0xDF, 0x86, 0xE3,
		0x71, 0x01, 0x26, 0xFF, 0x84, 0xE3, 0x71, 0x12, 0xDF, 0x85,
		0xE3, 0x71, 0x07, 0xA0, 0x00, 0x00, 0x00, 0x02, 0x00, 0x02,
		0xDF, 0x86, 0xE3, 0x71, 0x01, 0x22, 0xDF, 0x87, 0xE3, 0x71,
		0x01, 0x00, 0xDF, 0x8D, 0xE3, 0x71, 0x06, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x10, 0xFF, 0x82, 0xE3, 0x71, 0x81, 0x96, 0xDF,
		0x83, 0xE3, 0x71, 0x04, 0x00, 0x09, 0x01, 0x20, 0xFF, 0x84,
		0xE3, 0x71, 0x12, 0xDF, 0x85, 0xE3, 0x71, 0x07, 0xA0, 0x00,
		0x00, 0x00, 0x03, 0x10, 0x10, 0xDF, 0x86, 0xE3, 0x71, 0x01,
		0x03, 0xFF, 0x84, 0xE3, 0x71, 0x12, 0xDF, 0x85, 0xE3, 0x71,
		0x07, 0xA0, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0xDF, 0x86,
		0xE3, 0x71, 0x01, 0x26, 0xFF, 0x84, 0xE3, 0x71, 0x12, 0xDF,
		0x85, 0xE3, 0x71, 0x07, 0xA0, 0x00, 0x00, 0x00, 0x03, 0x10,
		0x10, 0xDF, 0x86, 0xE3, 0x71, 0x01, 0x26, 0xFF, 0x84, 0xE3,
		0x71, 0x12, 0xDF, 0x85, 0xE3, 0x71, 0x07, 0xA0, 0x00, 0x00,
		0x00, 0x01, 0x00, 0x01, 0xDF, 0x86, 0xE3, 0x71, 0x01, 0x23,
		0xDF, 0x87, 0xE3, 0x71, 0x01, 0x01, 0xDF, 0x88, 0xE3, 0x71,
		0x01, 0x00, 0xDF, 0x89, 0xE3, 0x71, 0x01, 0x00, 0xDF, 0x8A,
		0xE3, 0x71, 0x06, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0xDF,
		0x8C, 0xE3, 0x71, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x25,
		0xDF, 0x8E, 0xE3, 0x71, 0x04, 0x28, 0x00, 0x00, 0x00, 0xFF,
		0x82, 0xE3, 0x71, 0x81, 0x90, 0xDF, 0x83, 0xE3, 0x71, 0x04,
		0x00, 0x09, 0x01, 0x20,	0xFF, 0x84, 0xE3, 0x71, 0x12, 0xDF,
		0x85, 0xE3, 0x71, 0x07, 0xA0, 0x00, 0x00, 0x00, 0x25, 0x10,
		0x10, 0xDF, 0x86, 0xE3, 0x71, 0x01, 0x04, 0xFF, 0x84, 0xE3,
		0x71, 0x12, 0xDF, 0x85, 0xE3, 0x71, 0x07, 0xA0, 0x00, 0x00,
		0x00, 0x03, 0x00, 0x03, 0xDF, 0x86, 0xE3, 0x71, 0x01, 0x25,
		0xFF, 0x84, 0xE3, 0x71, 0x12, 0xDF, 0x85, 0xE3, 0x71, 0x07,
		0xA0, 0x00, 0x00, 0x00, 0x04, 0x00, 0x04, 0xDF, 0x86, 0xE3,
		0x71, 0x01, 0x24, 0xFF, 0x84, 0xE3, 0x71, 0x12, 0xDF, 0x85,
		0xE3, 0x71, 0x07, 0xA0, 0x00, 0x00, 0x00, 0x65, 0x10, 0x10,
		0xDF, 0x86, 0xE3, 0x71, 0x01, 0x05, 0xDF, 0x88, 0xE3, 0x71,
		0x01, 0x00, 0xDF, 0x89, 0xE3, 0x71, 0x01, 0x01, 0xDF, 0x8B,
		0xE3, 0x71, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x50, 0xDF,
		0x8D, 0xE3, 0x71, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20,
		0xDF, 0x8E, 0xE3, 0x71, 0x04, 0x84, 0xC0, 0x80, 0x00, 0xFF,
		0x82, 0xE3, 0x71, 0x57, 0xDF, 0x83, 0xE3, 0x71, 0x04, 0x00,
		0x09, 0x01, 0x20, 0xFF, 0x84, 0xE3, 0x71, 0x12, 0xDF, 0x85,
		0xE3, 0x71, 0x07, 0xA0, 0x00, 0x00, 0x00, 0x04, 0x10, 0x10,
		0xDF, 0x86, 0xE3, 0x71, 0x01, 0x04, 0xFF, 0x84, 0xE3, 0x71,
		0x12, 0xDF, 0x85, 0xE3, 0x71, 0x07, 0xA0, 0x00, 0x00, 0x00,
		0x02, 0x00, 0x02, 0xDF, 0x86, 0xE3, 0x71, 0x01, 0x24, 0xDF,
		0x88, 0xE3, 0x71, 0x01, 0x01, 0xDF, 0x89, 0xE3, 0x71, 0x01,
		0x01, 0xDF, 0x8B, 0xE3, 0x71, 0x06, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x20, 0xDF, 0x8E, 0xE3, 0x71, 0x04, 0x84, 0x00, 0x80,
		0x00, 0xFF, 0x82, 0xE3, 0x71, 0x4E, 0xDF, 0x83, 0xE3, 0x71,
		0x04, 0x00, 0x09, 0x01, 0x20, 0xFF, 0x84, 0xE3, 0x71, 0x12,
		0xDF, 0x85, 0xE3, 0x71, 0x07, 0xA0, 0x00, 0x00, 0x00, 0x03,
		0x10, 0x10, 0xDF, 0x86, 0xE3, 0x71, 0x01, 0x01, 0xFF, 0x84,
		0xE3, 0x71, 0x12, 0xDF, 0x85, 0xE3, 0x71, 0x07, 0xA0, 0x00,
		0x00, 0x00, 0x01, 0x00, 0x01, 0xDF, 0x86, 0xE3, 0x71, 0x01,
		0x21, 0xDF, 0x87, 0xE3, 0x71, 0x01, 0x00, 0xDF, 0x89, 0xE3,
		0x71, 0x01, 0x00, 0xDF, 0x8D, 0xE3, 0x71, 0x06, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x10, 0xFF, 0x82, 0xE3, 0x71, 0x81, 0x90,
		0xDF, 0x83, 0xE3, 0x71, 0x04, 0x00, 0x09, 0x01, 0x20, 0xFF,
		0x84, 0xE3, 0x71, 0x12, 0xDF, 0x85, 0xE3, 0x71, 0x07, 0xA0,
		0x00, 0x00, 0x00, 0x65, 0x10, 0x10, 0xDF, 0x86, 0xE3, 0x71,
		0x01, 0x02, 0xFF, 0x84, 0xE3, 0x71, 0x12, 0xDF, 0x85, 0xE3,
		0x71, 0x07, 0xA0, 0x00, 0x00, 0x00, 0x04, 0x00, 0x04, 0xDF,
		0x86, 0xE3, 0x71, 0x01, 0x25, 0xFF, 0x84, 0xE3, 0x71, 0x12,
		0xDF, 0x85, 0xE3, 0x71, 0x07, 0xA0, 0x00, 0x00, 0x00, 0x03,
		0x00, 0x03, 0xDF, 0x86, 0xE3, 0x71, 0x01, 0x22, 0xFF, 0x84,
		0xE3, 0x71, 0x12, 0xDF, 0x85, 0xE3, 0x71, 0x07, 0xA0, 0x00,
		0x00, 0x00, 0x25, 0x10, 0x10, 0xDF, 0x86, 0xE3, 0x71, 0x01,
		0x05, 0xDF, 0x87, 0xE3, 0x71, 0x01, 0x01, 0xDF, 0x88, 0xE3,
		0x71, 0x01, 0x01, 0xDF, 0x8A, 0xE3, 0x71, 0x06, 0x00, 0x00,
		0x00, 0x00, 0x02, 0x00, 0xDF, 0x8C, 0xE3, 0x71, 0x06, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x25, 0xDF, 0x8E, 0xE3, 0x71, 0x04,
		0x28, 0x00, 0x00, 0x00, 0xFF, 0x82, 0xE3, 0x71, 0x5F, 0xDF,
		0x83, 0xE3, 0x71, 0x04, 0x00, 0x09, 0x01, 0x20, 0xFF, 0x84,
		0xE3, 0x71, 0x12, 0xDF, 0x85, 0xE3, 0x71, 0x07, 0xA0, 0x00,
		0x00, 0x00, 0x25, 0x10, 0x10, 0xDF, 0x86, 0xE3, 0x71, 0x01,
		0x03, 0xFF, 0x84, 0xE3, 0x71, 0x12, 0xDF, 0x85, 0xE3, 0x71,
		0x07, 0xA0, 0x00, 0x00, 0x00, 0x04, 0x00, 0x04, 0xDF, 0x86,
		0xE3, 0x71, 0x01, 0x23, 0xDF, 0x87, 0xE3, 0x71, 0x01, 0x01,
		0xDF, 0x88, 0xE3, 0x71, 0x01, 0x01, 0xDF, 0x89, 0xE3, 0x71,
		0x01, 0x01, 0xDF, 0x8B, 0xE3, 0x71, 0x06, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x50, 0xDF, 0x8D, 0xE3, 0x71, 0x06, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x20, 0xFF, 0x82, 0xE3, 0x71, 0x81, 0x9E,
		0xDF, 0x83, 0xE3, 0x71, 0x04, 0x00, 0x09, 0x01, 0x20, 0xFF,
		0x84, 0xE3, 0x71, 0x12, 0xDF, 0x85, 0xE3, 0x71, 0x07, 0xA0,
		0x00, 0x00, 0x00, 0x25, 0x10, 0x10, 0xDF, 0x86, 0xE3, 0x71,
		0x01, 0x26, 0xFF, 0x84, 0xE3, 0x71, 0x12, 0xDF, 0x85, 0xE3,
		0x71, 0x07, 0xA0, 0x00, 0x00, 0x03, 0x24, 0x10, 0x10, 0xDF,
		0x86, 0xE3, 0x71, 0x01, 0x06, 0xFF, 0x84, 0xE3, 0x71, 0x12,
		0xDF, 0x85, 0xE3, 0x71, 0x07, 0xA0, 0x00, 0x00, 0x03, 0x33,
		0x01, 0x01, 0xDF, 0x86, 0xE3, 0x71, 0x01, 0x07, 0xFF, 0x84,
		0xE3, 0x71, 0x12, 0xDF, 0x85, 0xE3, 0x71, 0x07, 0xA0, 0x00,
		0x00, 0x01, 0x52, 0x30, 0x10, 0xDF, 0x86, 0xE3, 0x71, 0x01,
		0x06, 0xFF, 0x84, 0xE3, 0x71, 0x12, 0xDF, 0x85, 0xE3, 0x71,
		0x07, 0xA0, 0x00, 0x00, 0x00, 0x04, 0x00, 0x04, 0xDF, 0x86,
		0xE3, 0x71, 0x01, 0x26, 0xDF, 0x87, 0xE3, 0x71, 0x01, 0x00,
		0xDF, 0x89, 0xE3, 0x71, 0x01, 0x00, 0xDF, 0x8A, 0xE3, 0x71,
		0x06, 0x00, 0x00, 0x00, 0x00, 0x01, 0x20, 0xDF, 0x8D, 0xE3,
		0x71, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10
	};
	struct tlv *tlv = NULL, *i_tlv = NULL;
	int rc, num_aids;

	rc = tlv_parse(termsetting2, sizeof(termsetting2), &tlv);
	ck_assert(rc == TLV_RC_OK);

	for (i_tlv = tlv_deep_find(tlv, "\xDF\x85\xE3\x71"), num_aids = 0;
	     i_tlv;
	     i_tlv = tlv_deep_find(tlv_iterate(i_tlv), "\xDF\x85\xE3\x71")) {
		num_aids++;
	}

	tlv_free(tlv);
	ck_assert(num_aids == 29);
}
END_TEST

START_TEST(test_tlv_set_value)
{
	const char *label = "SomeLongApplicationLabel";
	const unsigned char ppse[] = {
		0x6F, 0x2F,
			0x84, 0x0E, 0x32, 0x50, 0x41, 0x59, 0x2E, 0x53, 0x59,
				    0x53, 0x2E, 0x44, 0x44, 0x46, 0x30, 0x31,
			0xA5, 0x1D,
				0xBF, 0x0C, 0x1A,
					0x61, 0x18,
						0x4F, 0x07, 0xA0, 0x00, 0x00,
						      0x00, 0x04, 0x10, 0x10,
						0x50, 0x0A, 0x4D, 0x61, 0x73,
						      0x74, 0x65, 0x72, 0x43,
						      0x61, 0x72, 0x64,
						0x87, 0x01, 0x01
	};
	const unsigned char modified_ppse[] = {
		0x6F, 0x3D,
			0x84, 0x0E, 0x32, 0x50, 0x41, 0x59, 0x2E, 0x53, 0x59,
				    0x53, 0x2E, 0x44, 0x44, 0x46, 0x30, 0x31,
			0xA5, 0x2B,
				0xBF, 0x0C, 0x28,
					0x61, 0x26,
						0x4F, 0x07, 0xA0, 0x00, 0x00,
						      0x00, 0x04, 0x10, 0x10,
						0x50, 0x18, 0x53, 0x6F, 0x6D,
						      0x65, 0x4C, 0x6F, 0x6E,
						      0x67, 0x41, 0x70, 0x70,
						      0x6C, 0x69, 0x63, 0x61,
						      0x74, 0x69, 0x6F, 0x6E,
						      0x4C, 0x61, 0x62, 0x65,
						      0x6C,
						0x87, 0x01, 0x01
	};
	unsigned char buffer[256];
	size_t size = sizeof(buffer);
	struct tlv *tlv_ppse = NULL, *tlv_app_label = NULL;
	int rc;

	rc = tlv_parse(ppse, sizeof(ppse), &tlv_ppse);
	ck_assert(rc == TLV_RC_OK);

	tlv_app_label = tlv_deep_find(tlv_ppse, "\x50");
	ck_assert(tlv_app_label);

	tlv_app_label = tlv_set_value(tlv_app_label, strlen(label), label);
	ck_assert(tlv_app_label);

	rc = tlv_encode(tlv_ppse, buffer, &size);
	ck_assert(rc == TLV_RC_OK);
	ck_assert(size == sizeof(modified_ppse));
	ck_assert(!memcmp(buffer, modified_ppse, size));
}
END_TEST

Suite *tlv_test_suite(void)
{
	Suite *suite = NULL;
	TCase *tc_tlv_malformed_input = NULL, *tc_tlv_primitive_encoding = NULL;
	TCase *tc_tlv_constructed_encoding = NULL, *tc_tlv_verisign_x509 = NULL;
	TCase *tc_tlv_construct = NULL, *tc_tlv_deep_find = NULL;
	TCase *tc_tlv_set_value = NULL;

	suite = suite_create("tlv_test");

	tc_tlv_malformed_input = tcase_create("tlv-malformed-input");
	tcase_add_test(tc_tlv_malformed_input, test_tlv_malformed_input);
	suite_add_tcase(suite, tc_tlv_malformed_input);

	tc_tlv_primitive_encoding = tcase_create("tlv-primitive-encoding");
	tcase_add_test(tc_tlv_primitive_encoding, test_tlv_primitive_encoding);
	suite_add_tcase(suite, tc_tlv_primitive_encoding);

	tc_tlv_constructed_encoding = tcase_create("tlv-constructed-encoding");
	tcase_add_test(tc_tlv_constructed_encoding,
						test_tlv_constructed_encoding);
	suite_add_tcase(suite, tc_tlv_constructed_encoding);

	tc_tlv_verisign_x509 = tcase_create("tlv-versign-x509");
	tcase_add_test(tc_tlv_verisign_x509, test_tlv_verisign_x509);
	suite_add_tcase(suite, tc_tlv_verisign_x509);

	tc_tlv_construct = tcase_create("tlv-tlv-construct");
	tcase_add_test(tc_tlv_construct, test_tlv_construct);
	suite_add_tcase(suite, tc_tlv_construct);

	tc_tlv_deep_find = tcase_create("tlv-deep-find");
	tcase_add_test(tc_tlv_deep_find, test_tlv_deep_find);
	suite_add_tcase(suite, tc_tlv_deep_find);

	tc_tlv_set_value = tcase_create("tlv-set-value");
	tcase_add_test(tc_tlv_set_value, test_tlv_set_value);
	suite_add_tcase(suite, tc_tlv_set_value);

	return suite;
}

int main(int argc, char **argv)
{
	Suite *suite;
	SRunner *srunner;
	int failed;

	if (log4c_init()) {
		fprintf(stderr, "log4c_init() failed!\n");
		return EXIT_FAILURE;
	}

	libtlv_init("libtlv_test");

	suite = tlv_test_suite();
	srunner = srunner_create(suite);
	srunner_run_all(srunner, CK_VERBOSE);
	failed = srunner_ntests_failed(srunner);
	srunner_free(srunner);

	log4c_fini();

	return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
