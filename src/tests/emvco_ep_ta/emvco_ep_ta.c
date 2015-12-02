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
#include <stdint.h>
#include <check.h>
#include <log4c.h>

#include <tlv.h>
#include <emv.h>

#include "emvco_ep_ta.h"

static const char log4c_category[] = "emvco_ep_ta";
static log4c_category_t *log_cat;
static const struct tlv_id_to_fmt id_fmts[] = {
	{ .id = EMV_ID_TEST_FLAGS, .fmt = fmt_b },
	{ .id = NULL				}
};

/* 2EA.001.00 Entry of Amount Authorzed					      */
START_TEST(test_2EA_001_00)
{
	uint8_t cfg[4096];
	size_t cfg_sz = sizeof(cfg);
	struct emv_ep *ep = NULL;
	struct emv_hal *lt = NULL;
	struct emv_kernel *tk = NULL;
	struct tk_id tk_id[] = {
		{ KERNEL_ID_23 },
	};
	size_t i_tk = 0;
	int rc = EMV_RC_OK;

	log4c_category_log(log_cat, LOG4C_PRIORITY_INFO, "%s(): start",
								      __func__);

	rc = term_get_setting(termsetting2, cfg, &cfg_sz);
	ck_assert(rc == TLV_RC_OK);

	ep = emv_ep_new(log4c_category);
	ck_assert(ep != NULL);

	lt = lt_new(ltsetting1_1, log4c_category);
	ck_assert(lt != NULL);

	rc = emv_ep_register_hal(ep, lt);
	ck_assert(rc == EMV_RC_OK);

	tk = tk_new(log4c_category);
	ck_assert(tk != NULL);

	for (i_tk = 0; i_tk < ARRAY_SIZE(tk_id); i_tk++) {
		rc = emv_ep_register_kernel(ep, tk, tk_id[i_tk].kernel_id,
						     tk_id[i_tk].kernel_id_len);
		ck_assert(rc == EMV_RC_OK);
	}

	rc = emv_ep_configure(ep, cfg, cfg_sz);
	ck_assert(rc == EMV_RC_OK);

	rc = emv_ep_activate(ep, start_a, txn_purchase, 10, 0, ISO4217_EUR,
								   0x12345678u);
	ck_assert(rc == EMV_RC_OK);

	tk_free(tk);
	lt_free(lt);
	emv_ep_free(ep);

	log4c_category_log(log_cat, LOG4C_PRIORITY_INFO, "%s(): done",
								      __func__);
}
END_TEST

Suite *emvco_ep_ta_test_suite(void)
{
	Suite *suite = NULL;
	TCase *tc_general_reqs = NULL;

	suite = suite_create("emvco_ep_ta");

	tc_general_reqs = tcase_create("emvco-ep-ta-general-requirements");
	tcase_add_test(tc_general_reqs, test_2EA_001_00);
	suite_add_tcase(suite, tc_general_reqs);

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

	log_cat = log4c_category_get(log4c_category);

	libtlv_register_fmts(id_fmts);
	libtlv_register_fmts(libemv_get_id_fmts());

	suite = emvco_ep_ta_test_suite();
	srunner = srunner_create(suite);
	srunner_run_all(srunner, CK_VERBOSE);
	failed = srunner_ntests_failed(srunner);
	srunner_free(srunner);
	log4c_fini();

	return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
