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
#include <arpa/inet.h>

#include <tlv.h>
#include <emv.h>

#include "emvco_ep_ta.h"

uint32_t transaction_sequence_counter;

static const char log4c_category[] = "emvco_ep_ta";
static log4c_category_t *log_cat;
static const struct tlv_id_to_fmt id_fmts[] = {
	{ .id = EMV_ID_TEST_FLAGS, .fmt = fmt_b },
	{ .id = NULL				}
};

void gpo_outcome_to_outcome(const struct outcome_gpo_resp *in,
						  struct emv_outcome_parms *out)
{
	memset(out, 0, sizeof(*out));
	out->outcome		      = (enum emv_outcome)in->outcome;
	out->start		      = (enum emv_start)in->start;
	out->cvm		      = (enum emv_cvm)in->cvm;
	out->present.receipt	      = (bool)in->receipt;
	out->removal_timeout	      = (int)ntohs(in->removal_timeout);
	out->online_response.type     = (enum emv_online_response_type)
								in->online_resp;
	out->alternate_interface_pref = (enum emv_alternate_interface_pref)
							     in->alt_iface_pref;
	if (in->field_off_request == 0xffffu) {
		out->present.field_off_request	= false;
		out->field_off_hold_time	= 0;
	} else {
		out->present.field_off_request	= true;
		out->field_off_hold_time = (int)(ntohs(in->field_off_request));
	}
}

void outcome_to_gpo_outcome(const struct emv_outcome_parms *in,
						   struct outcome_gpo_resp *out)
{
	memset(out, 0, sizeof(*out));
	out->outcome		= (uint8_t)in->outcome;
	out->start		= (uint8_t)in->start;
	out->online_resp	= (uint8_t)in->online_response.type;
	out->cvm		= (uint8_t)in->cvm;
	out->alt_iface_pref	= (uint8_t)in->alternate_interface_pref;
	out->receipt		= (uint8_t)in->present.receipt;
	out->field_off_request	= in->present.field_off_request ?
			     htons((uint16_t)in->field_off_hold_time) : 0xffffu;
	out->removal_timeout	= htons((uint16_t)in->removal_timeout);
}


void gpo_ui_req_to_ui_req(const struct ui_req_gpo_resp *in,
						     struct emv_ui_request *out)
{
	memset(out, 0, sizeof(*out));
	out->msg_id	     = (enum emv_message_identifier)in->msg_id;
	out->status	     = (enum emv_status)in->status;
	out->hold_time	     = ntohs(in->hold_time);
	out->value_qualifier = (enum emv_value_qualifier)in->value_qual;
	memcpy(out->lang_pref, in->lang_pref, sizeof(in->lang_pref));
	memcpy(out->value, in->value, sizeof(in->value));
	memcpy(out->currency_code, in->currency_code,
						     sizeof(in->currency_code));
}

void ui_req_to_gpo_ui_req(const struct emv_ui_request *in,
						    struct ui_req_gpo_resp *out)
{
	memset(out, 0, sizeof(*out));
	out->msg_id	= (uint8_t)in->msg_id;
	out->status     = (uint8_t)in->status;
	out->hold_time  = htons(in->hold_time);
	out->value_qual = (uint8_t)in->value_qualifier;
	memcpy(out->lang_pref, in->lang_pref, sizeof(in->lang_pref));
	memcpy(out->value, in->value, sizeof(in->value));
	memcpy(out->currency_code, in->currency_code,
						     sizeof(in->currency_code));
}

struct emvco_ep_ta_tc_fixture {
	struct emv_ep	  *ep;
	struct emv_hal	  *lt;
	struct emv_kernel *tk[18];
};

static void emvco_ep_ta_tc_fixture_teardown(
					 struct emvco_ep_ta_tc_fixture *fixture)
{
	size_t i_tk = 0;

	for (i_tk = 0; i_tk < ARRAY_SIZE(fixture->tk); i_tk++)
		if (fixture->tk[i_tk])
			tk_free(fixture->tk[i_tk]);

	if (fixture->lt)
		lt_free(fixture->lt);

	if (fixture->ep)
		emv_ep_free(fixture->ep);

	memset(fixture, 0, sizeof(*fixture));
}

static int emvco_ep_ta_tc_fixture_setup(struct emvco_ep_ta_tc_fixture *fixture,
	struct chk *chk, enum termsetting termsetting, enum ltsetting ltsetting)
{
	uint8_t cfg[4096];
	size_t cfg_sz = sizeof(cfg);
	uint8_t app_ver_num[2] = TK_APPLICATION_VERSION_NUMBER;
	struct tk_id tk_id[18] = {
		{ KERNEL_ID_TK1	   },
		{ KERNEL_ID_TK2	   },
		{ KERNEL_ID_TK3	   },
		{ KERNEL_ID_TK4	   },
		{ KERNEL_ID_TK5	   },
		{ KERNEL_ID_TK6	   },
		{ KERNEL_ID_TK7	   },
		{ KERNEL_ID_21	   },
		{ KERNEL_ID_22	   },
		{ KERNEL_ID_23	   },
		{ KERNEL_ID_24	   },
		{ KERNEL_ID_25	   },
		{ KERNEL_ID_2B	   },
		{ KERNEL_ID_32	   },
		{ KERNEL_ID_810978 },
		{ KERNEL_ID_BF0840 },
		{ KERNEL_ID_C11111 },
		{ KERNEL_ID_FF2222 }
	};
	size_t i_tk = 0;
	int rc = EMV_RC_OK;

	memset(fixture, 0, sizeof(*fixture));

	rc = term_get_setting(termsetting, cfg, &cfg_sz);
	if (rc != EMV_RC_OK)
		goto done;

	fixture->ep = emv_ep_new(log4c_category);
	if (!fixture->ep) {
		rc = EMV_RC_OUT_OF_MEMORY;
		goto done;
	}

	fixture->lt = lt_new(ltsetting, chk, log4c_category);
	if (!fixture->lt) {
		rc = EMV_RC_OUT_OF_MEMORY;
		goto done;
	}

	rc = emv_ep_register_hal(fixture->ep, fixture->lt);
	if (rc != EMV_RC_OK)
		goto done;

	for (i_tk = 0; i_tk < ARRAY_SIZE(fixture->tk); i_tk++) {

		fixture->tk[i_tk] = tk_new(log4c_category);

		if (!fixture->tk[i_tk]) {
			rc = EMV_RC_OUT_OF_MEMORY;
			goto done;
		}

		rc = emv_ep_register_kernel(fixture->ep, fixture->tk[i_tk],
			       tk_id[i_tk].kernel_id, tk_id[i_tk].kernel_id_len,
								   app_ver_num);
		if (rc != EMV_RC_OK)
			goto done;
	}

	rc = emv_ep_configure(fixture->ep, cfg, cfg_sz);
	if (rc != EMV_RC_OK)
		goto done;

done:
	if (rc != EMV_RC_OK)
		emvco_ep_ta_tc_fixture_teardown(fixture);

	return rc;
}

static int emvco_ep_ta_tc(enum termsetting termsetting,
				enum ltsetting ltsetting, enum pass_criteria pc,
						      const struct emv_txn *txn)
{
	struct emv_outcome_parms outcome;
	struct emvco_ep_ta_tc_fixture fixture;
	struct chk *chk = NULL;
	int rc = EMV_RC_OK;

	chk = chk_pass_criteria_new(pc, log4c_category);
	if (!chk) {
		rc = EMV_RC_OUT_OF_MEMORY;
		goto done;
	}

	rc = emvco_ep_ta_tc_fixture_setup(&fixture, chk, termsetting,
								     ltsetting);
	if (rc != EMV_RC_OK)
		goto done;

	if (emv_ep_get_autorun(fixture.ep)->enabled) {
		/* REQUIREMENT(EMV_CTLS_BOOK_A_V2_5, "8.1.1.6"); */
		/* If the value of the POS System configuration parameter
		 * Autorun is 'Yes', then the reader shall do all of the
		 * following:
		 *  o Ensure the field is on.
		 *  o Request message '15' (Present Card), status Ready to Read.
		 *  o Activate Entry Point at Start B and make the following
		 *    available to Entry Point:
		 *     o for the selected type of transaction (as configured),
		 *	 the corresponding set of supported Combinations and
		 *	 Entry Point Configuration Data/fixed TTQ value.      */
		struct emv_ui_request present_card = {
			.msg_id = msg_present_card,
			.status = sts_ready_to_read
		};

		rc = emv_ep_field_on(fixture.ep);
		if (rc != EMV_RC_OK)
			goto done;

		rc = emv_ep_ui_request(fixture.ep, &present_card);
		if (rc != EMV_RC_OK)
			goto done;

		chk->ops->txn_start(chk);

		rc = emv_ep_activate(fixture.ep, start_b,
					   &emv_ep_get_autorun(fixture.ep)->txn,
				      ++transaction_sequence_counter, &outcome);
		if (rc != EMV_RC_OK)
			goto done;
	} else {
		/* REQUIREMENT(EMV_CTLS_BOOK_A_V2_5, "8.1.1.5"); */
		/* If the value of the POS System configuration parameter
		 * Autorun is 'No',then the reader shall do all of the
		 * following:
		 *  o Ensure the field is off.
		 *  o Request message '14' (Welcome), status Idle.
		 *  o Wait for instruction from the terminal and then activate
		 *    Entry Point at Start A and make the following available to
		 *    Entry Point:
		 *     o Transaction Type
		 *     o the corresponding set of supported Combinations and
		 *	 Entry Point Configuration Data
		 *     o Amount, Authorised
		 *     o Amount, Other					      */
		struct emv_ui_request welcome = {
			.msg_id = msg_welcome,
			.status = sts_idle
		};

		rc = emv_ep_field_off(fixture.ep);
		if (rc != EMV_RC_OK)
			goto done;

		rc = emv_ep_ui_request(fixture.ep, &welcome);
		if (rc != EMV_RC_OK)
			goto done;

		chk->ops->txn_start(chk);

		rc = emv_ep_activate(fixture.ep, start_a, txn,
				      ++transaction_sequence_counter, &outcome);
		if (rc != EMV_RC_OK)
			goto done;

		if (outcome.start != start_na) {
			rc = emv_ep_activate(fixture.ep, outcome.start, txn,
					transaction_sequence_counter, &outcome);
			if (rc != EMV_RC_OK)
				goto done;
		}
	}

	if (!chk->ops->pass_criteria_met(chk))
		rc = EMV_RC_FAIL;

done:
	emvco_ep_ta_tc_fixture_teardown(&fixture);

	if (chk && chk->ops && chk->ops->free)
		chk->ops->free(chk);

	return rc;
}

/* 2EA.001.00 Entry of Amount Authorized				      */
START_TEST(test_2EA_001_00)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;

	txn.amount_authorized = 10;
	rc = emvco_ep_ta_tc(termsetting2, ltsetting1_1, pc_2ea_001_00_case01,
									  &txn);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 75;
	rc = emvco_ep_ta_tc(termsetting2, ltsetting1_2, pc_2ea_001_00_case02,
									  &txn);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 45;
	rc = emvco_ep_ta_tc(termsetting2, ltsetting1_97, pc_2ea_001_00_case03,
									  &txn);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EA.002.00 Entry of Amount Authorized and Transaction Type 'Purchase'      */
START_TEST(test_2EA_002_00)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;
	txn.amount_authorized = 10;

	rc = emvco_ep_ta_tc(termsetting4, ltsetting1_3, pc_2ea_002_00, &txn);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EA.002.01 Entry of Amount Authorzed	and Transaction Type 'Cash Advance'   */
START_TEST(test_2EA_002_01)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_cash_advance;
	txn.amount_authorized = 20;

	rc = emvco_ep_ta_tc(termsetting4, ltsetting1_1, pc_2ea_002_01, &txn);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EA.002.02 Entry of Amount Authorized and Transaction Type 'Refund'	      */
START_TEST(test_2EA_002_02)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_refund;
	txn.amount_authorized = 30;

	rc = emvco_ep_ta_tc(termsetting4, ltsetting1_4, pc_2ea_002_02, &txn);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EA.003.00 Entry of Amount Authorized, Amount Other			      */
START_TEST(test_2EA_003_00)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase_with_cashback;

	txn.amount_authorized = 30;
	txn.amount_other      = 10;
	rc = emvco_ep_ta_tc(termsetting4, ltsetting1_3, pc_2ea_003_00_case01,
									  &txn);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 70;
	txn.amount_other      = 20;
	rc = emvco_ep_ta_tc(termsetting4, ltsetting1_3, pc_2ea_003_00_case02,
									  &txn);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EA.004.00 Restart flag cleared at new transaction			      */
START_TEST(test_2EA_004_00)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;
	txn.amount_authorized = 10;

	rc = emvco_ep_ta_tc(termsetting2, ltsetting1_1, pc_2ea_004_00_case01,
									  &txn);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting3, ltsetting1_1, pc_2ea_004_00_case02,
									  &txn);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EA.005.00 Field off at new transaction (Autorun = 'No')		      */
START_TEST(test_2EA_005_00)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;
	txn.amount_authorized = 10;

	rc = emvco_ep_ta_tc(termsetting2, ltsetting1_2, pc_2ea_005_00, &txn);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EA.005.01 Reader status Idle at new transaction (Autorun = 'No')	      */
START_TEST(test_2EA_005_01)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;
	txn.amount_authorized = 10;

	rc = emvco_ep_ta_tc(termsetting2, ltsetting1_2, pc_2ea_005_01, &txn);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EA.006.00 Field on at new transaction (Autorun = 'Yes')		      */
START_TEST(test_2EA_006_00)
{
	int rc;

	rc = emvco_ep_ta_tc(termsetting3, ltsetting1_2, pc_2ea_006_00, NULL);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EA.006.01 Reader status Ready to Read at new transaction (Autorun = 'Yes')*/
START_TEST(test_2EA_006_01)
{
	int rc;

	rc = emvco_ep_ta_tc(termsetting3, ltsetting1_2, pc_2ea_006_01, NULL);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EA.006.02 Terminal EMV Data available for Kernel for a Purchase
 * transaction.								      */
START_TEST(test_2EA_006_02)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;

	txn.amount_authorized = 2;
	rc = emvco_ep_ta_tc(termsetting4, ltsetting1_90, pc_2ea_006_02_case01,
									  &txn);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 5;
	rc = emvco_ep_ta_tc(termsetting4, ltsetting1_98, pc_2ea_006_02_case02,
									  &txn);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EA.006.03 Terminal EMV Data available for Kernel for a Purchase with
 * Cashback transaction.						      */
START_TEST(test_2EA_006_03)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase_with_cashback;
	txn.amount_authorized = 2;
	txn.amount_other = 1;

	rc = emvco_ep_ta_tc(termsetting4, ltsetting1_91, pc_2ea_006_03, &txn);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EA.006.04 Terminal EMV Data available for Kernel for a Cash Advance
 * transaction.								      */
START_TEST(test_2EA_006_04)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_cash_advance;
	txn.amount_authorized = 2;

	rc = emvco_ep_ta_tc(termsetting4, ltsetting1_90, pc_2ea_006_04, &txn);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EA.006.05 Terminal EMV Data available for Kernel for a Refund transaction.*/
START_TEST(test_2EA_006_05)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_refund;
	txn.amount_authorized = 2;

	rc = emvco_ep_ta_tc(termsetting4, ltsetting1_91, pc_2ea_006_05, &txn);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EA.007.00 Unpredictable Number different at each transaction.	      */
START_TEST(test_2EA_007_00)
{
	enum ltsetting ltset[3] = { ltsetting1_1, ltsetting1_2, ltsetting2_40 };
	struct emv_outcome_parms outcome;
	struct emvco_ep_ta_tc_fixture fixture;
	struct chk *chk = NULL;
	struct emv_txn txn;
	int rc, i;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;
	txn.amount_authorized = 2;

	chk = chk_pass_criteria_new(pc_2ea_007_00, log4c_category);
	ck_assert(chk != NULL);

	for (i = 0; i < 500; i++) {
		rc = emvco_ep_ta_tc_fixture_setup(&fixture, chk,
						    termsetting2, ltset[i % 3]);
		ck_assert(rc == EMV_RC_OK);

		rc = emv_ep_activate(fixture.ep, start_a, &txn,
				      ++transaction_sequence_counter, &outcome);
		ck_assert(rc == EMV_RC_OK);

		emvco_ep_ta_tc_fixture_teardown(&fixture);
	}

	for (i = 0; i < 500; i++) {
		rc = emvco_ep_ta_tc_fixture_setup(&fixture, chk,
						    termsetting3, ltset[i % 3]);
		ck_assert(rc == EMV_RC_OK);

		rc = emv_ep_activate(fixture.ep, start_a, &txn,
				      ++transaction_sequence_counter, &outcome);
		ck_assert(rc == EMV_RC_OK);

		emvco_ep_ta_tc_fixture_teardown(&fixture);
	}

	ck_assert(chk->ops->pass_criteria_met(chk));

	if (chk && chk->ops && chk->ops->free)
		chk->ops->free(chk);
}
END_TEST

/* 2EA.011.00 User Interface Request Hold Time */
START_TEST(test_2EA_011_00)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;
	txn.amount_authorized = 2;

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_60, pc_2ea_011_00_case01,
									  &txn);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_61, pc_2ea_011_00_case02,
									  &txn);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_62, pc_2ea_011_00_case03,
									  &txn);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EA.012.00 User Interface Request using Language Preference */
START_TEST(test_2EA_012_00)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;
	txn.amount_authorized = 2;

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_70, pc_2ea_012_00_case01,
									  &txn);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_71, pc_2ea_012_00_case02,
									  &txn);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_72, pc_2ea_012_00_case03,
									  &txn);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_73, pc_2ea_012_00_case04,
									  &txn);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

Suite *emvco_ep_ta_test_suite(void)
{
	Suite *suite = NULL;
	TCase *tc_general_reqs = NULL;

	suite = suite_create("emvco_ep_ta");

	tc_general_reqs = tcase_create("emvco-ep-ta-general-requirements");
	tcase_add_test(tc_general_reqs, test_2EA_001_00);
	tcase_add_test(tc_general_reqs, test_2EA_002_00);
	tcase_add_test(tc_general_reqs, test_2EA_002_01);
	tcase_add_test(tc_general_reqs, test_2EA_002_02);
	tcase_add_test(tc_general_reqs, test_2EA_003_00);
	tcase_add_test(tc_general_reqs, test_2EA_004_00);
	tcase_add_test(tc_general_reqs, test_2EA_005_00);
	tcase_add_test(tc_general_reqs, test_2EA_005_01);
	tcase_add_test(tc_general_reqs, test_2EA_006_00);
	tcase_add_test(tc_general_reqs, test_2EA_006_01);
	tcase_add_test(tc_general_reqs, test_2EA_006_02);
	tcase_add_test(tc_general_reqs, test_2EA_006_03);
	tcase_add_test(tc_general_reqs, test_2EA_006_04);
	tcase_add_test(tc_general_reqs, test_2EA_006_05);
	tcase_add_test(tc_general_reqs, test_2EA_007_00);
	tcase_add_test(tc_general_reqs, test_2EA_011_00);
	tcase_add_test(tc_general_reqs, test_2EA_012_00);
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
	srunner_set_fork_status(srunner, CK_NOFORK);
	srunner_run_all(srunner, CK_VERBOSE);
	failed = srunner_ntests_failed(srunner);
	srunner_free(srunner);

	libtlv_free_fmts();
	log4c_fini();

	return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
