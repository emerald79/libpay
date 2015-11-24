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

#include <stdio.h>
#include <stdlib.h>
#include <check.h>

#include <tlv.h>
#include <emv.h>

/* 2EA.001.00 Entry of Amount Authorzed					      */
START_TEST(test_case_2EA_001_00)
{
	
}

Suite *emv_ep_test_suite(void)
{
	Suite *suite = NULL;
	TCase *tc_tlv_malformed_input = NULL, *tc_tlv_primitive_encoding = NULL;
	TCase *tc_tlv_constructed_encoding = NULL, *tc_tlv_verisign_x509 = NULL;
	TCase *tc_tlv_construct = NULL;

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

	return suite;
}

int main(int argc, char **argv)
{
	Suite *suite;
	SRunner *srunner;
	int failed;

	suite = emv_ep_test_suite();
	srunner = srunner_create(suite);
	srunner_run_all(srunner, CK_VERBOSE);
	failed = srunner_ntests_failed(srunner);
	srunner_free(srunner);

	return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
