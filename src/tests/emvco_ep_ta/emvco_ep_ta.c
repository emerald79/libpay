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
	out->receipt		      = (bool)in->receipt;
	out->removal_timeout	      = (int)ntohs(in->removal_timeout);
	out->online_response_type     = (enum emv_online_response_type)
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
	out->online_resp	= (uint8_t)in->online_response_type;
	out->cvm		= (uint8_t)in->cvm;
	out->alt_iface_pref	= (uint8_t)in->alternate_interface_pref;
	out->receipt		= (uint8_t)in->receipt;
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
	struct chk	  *chk;
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
	struct chk *chk, enum termsetting termsetting, enum ltsetting ltsetting,
								    int lt_mode)
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
	fixture->chk = chk;

	rc = term_get_setting(termsetting, cfg, &cfg_sz);
	if (rc != EMV_RC_OK)
		goto done;

	fixture->ep = emv_ep_new(log4c_category);
	if (!fixture->ep) {
		rc = EMV_RC_OUT_OF_MEMORY;
		goto done;
	}

	fixture->lt = lt_new(ltsetting, chk, log4c_category, lt_mode);
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

static int emvco_ep_ta_tc_perform_transaction(
	      struct emvco_ep_ta_tc_fixture *fixture, const struct emv_txn *txn)
{
	bool autorun = emv_ep_get_autorun(fixture->ep)->enabled;
	enum emv_start start_at = autorun ? start_b : start_a;
	struct emv_outcome_parms outcome;
	int rc = EMV_RC_OK;
	struct chk *chk = fixture->chk;

	if (autorun) {
		/* REQUIREMENT(EMV_CTLS_BOOK_A_V2_5, "8.1.1.6"); */
		/* If the value of the POS System configuration parameter
		 * Autorun is 'Yes', then the reader shall do all of the
		 * following:
		 *  o Ensure the field is on.
		 *  o Request message '15' (Present Card), status Read to Read.
		 *  o Activate Entry Point at Start B and make the following
		 *    available to Entry Point:
		 *     o for the selected type of transaction (as configured),
		 *	 the corresponding set of supported Combinations and
		 *	 Entry Point Configuration Data/fixed TTQ value.      */
		struct emv_ui_request present_card = {
			.msg_id = msg_present_card,
			.status = sts_ready_to_read
		};

		rc = emv_ep_field_on(fixture->ep);
		if (rc != EMV_RC_OK)
			goto done;

		rc = emv_ep_ui_request(fixture->ep, &present_card);
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

		rc = emv_ep_field_off(fixture->ep);
		if (rc != EMV_RC_OK)
			goto done;

		rc = emv_ep_ui_request(fixture->ep, &welcome);
		if (rc != EMV_RC_OK)
			goto done;
	}

	chk->ops->ep_start(chk);

	rc = emv_ep_activate(fixture->ep, start_at, txn,
					++transaction_sequence_counter, NULL, 0,
								      &outcome);
	if (rc != EMV_RC_OK)
		goto done;

	chk->ops->outcome(chk, &outcome);
#if 0
	if (outcome.data_record.len) {
		char hex[outcome.data_record.len * 2 + 1];
		printf("DATA RECORD: '%s'\n", libtlv_bin_to_hex(
		       outcome.data_record.data, outcome.data_record.len, hex));
	}
#endif

	if (outcome.start != start_na) {
		if ((outcome.online_response_type != ort_na) &&
		    (outcome.data_record.len == 0)) {

			/* No online response data -> timeout */
			/* should sleep for		      */
			/* outcome.removal_timeout	      */
			struct emv_ui_request rm_card = {
				.msg_id = msg_card_read_ok,
				.status = sts_card_read_successfully
			};

			rc = emv_ep_ui_request(fixture->ep, &rm_card);
			if (rc != EMV_RC_OK)
				goto done;
		}

		if ((outcome.online_response_type != ort_emv_data) ||
		    (outcome.data_record.len)) {

			chk->ops->ep_restart(chk);

			rc = emv_ep_activate(fixture->ep, outcome.start, txn,
						   transaction_sequence_counter,
						       outcome.data_record.data,
					     outcome.data_record.len, &outcome);
			if (rc != EMV_RC_OK)
				goto done;

			chk->ops->outcome(chk, &outcome);
		}
	}

done:
	return rc;
}

static int emvco_ep_ta_tc(enum termsetting termsetting,
				enum ltsetting ltsetting, enum pass_criteria pc,
				      const struct emv_txn *txn, size_t num_txn)
{
	struct emvco_ep_ta_tc_fixture fixture;
	struct chk *chk = NULL;
	int rc = EMV_RC_OK, i_txn = 0;

	chk = chk_pass_criteria_new(pc, log4c_category);
	if (!chk) {
		rc = EMV_RC_OUT_OF_MEMORY;
		goto done;
	}

	rc = emvco_ep_ta_tc_fixture_setup(&fixture, chk, termsetting,
							  ltsetting, LT_NORMAL);
	if (rc != EMV_RC_OK)
		goto done;

	if (emv_ep_get_autorun(fixture.ep)->enabled) {

		rc = emvco_ep_ta_tc_perform_transaction(&fixture,
					  &emv_ep_get_autorun(fixture.ep)->txn);
		if (rc != EMV_RC_OK)
			goto done;

	} else {

		for (i_txn = 0; i_txn < num_txn; i_txn++) {

			rc = emvco_ep_ta_tc_perform_transaction(&fixture,
								   &txn[i_txn]);
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
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 75;
	rc = emvco_ep_ta_tc(termsetting2, ltsetting1_2, pc_2ea_001_00_case02,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 45;
	rc = emvco_ep_ta_tc(termsetting2, ltsetting1_97, pc_2ea_001_00_case03,
								       &txn, 1);
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

	rc = emvco_ep_ta_tc(termsetting4, ltsetting1_3, pc_2ea_002_00, &txn, 1);
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

	rc = emvco_ep_ta_tc(termsetting4, ltsetting1_1, pc_2ea_002_01, &txn, 1);
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

	rc = emvco_ep_ta_tc(termsetting4, ltsetting1_4, pc_2ea_002_02, &txn, 1);
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
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 70;
	txn.amount_other      = 20;
	rc = emvco_ep_ta_tc(termsetting4, ltsetting1_3, pc_2ea_003_00_case02,
								       &txn, 1);
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
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting3, ltsetting1_1, pc_2ea_004_00_case02,
								       &txn, 1);
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

	rc = emvco_ep_ta_tc(termsetting2, ltsetting1_2, pc_2ea_005_00, &txn, 1);
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

	rc = emvco_ep_ta_tc(termsetting2, ltsetting1_2, pc_2ea_005_01, &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EA.006.00 Field on at new transaction (Autorun = 'Yes')		      */
START_TEST(test_2EA_006_00)
{
	int rc;

	rc = emvco_ep_ta_tc(termsetting3, ltsetting1_2, pc_2ea_006_00, NULL, 0);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EA.006.01 Reader status Ready to Read at new transaction (Autorun = 'Yes')*/
START_TEST(test_2EA_006_01)
{
	int rc;

	rc = emvco_ep_ta_tc(termsetting3, ltsetting1_2, pc_2ea_006_01, NULL, 0);
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
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 5;
	rc = emvco_ep_ta_tc(termsetting4, ltsetting1_98, pc_2ea_006_02_case02,
								       &txn, 1);
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

	rc = emvco_ep_ta_tc(termsetting4, ltsetting1_91, pc_2ea_006_03,
								       &txn, 1);
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

	rc = emvco_ep_ta_tc(termsetting4, ltsetting1_90, pc_2ea_006_04,
								       &txn, 1);
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

	rc = emvco_ep_ta_tc(termsetting4, ltsetting1_91, pc_2ea_006_05,
								       &txn, 1);
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
					 termsetting2, ltset[i % 3], LT_NORMAL);
		ck_assert(rc == EMV_RC_OK);

		rc = emv_ep_activate(fixture.ep, start_a, &txn,
			     ++transaction_sequence_counter, NULL, 0, &outcome);
		ck_assert(rc == EMV_RC_OK);

		emvco_ep_ta_tc_fixture_teardown(&fixture);
	}

	for (i = 0; i < 500; i++) {
		rc = emvco_ep_ta_tc_fixture_setup(&fixture, chk,
					 termsetting3, ltset[i % 3], LT_NORMAL);
		ck_assert(rc == EMV_RC_OK);

		rc = emv_ep_activate(fixture.ep, start_a, &txn,
			     ++transaction_sequence_counter, NULL, 0, &outcome);
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
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_61, pc_2ea_011_00_case02,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_62, pc_2ea_011_00_case03,
								       &txn, 1);
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
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_71, pc_2ea_012_00_case02,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_72, pc_2ea_012_00_case03,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_73, pc_2ea_012_00_case04,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EA.013.00 User Interface Request Amount or Balance displayed */
START_TEST(test_2EA_013_00)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;
	txn.amount_authorized = 2;

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_80, pc_2ea_013_00_case01,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_81, pc_2ea_013_00_case02,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_82, pc_2ea_013_00_case03,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EA.013.01 User Interface Request Amount or Balance displayed (2) */
START_TEST(test_2EA_013_01)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;
	txn.amount_authorized = 2;

	rc = emvco_ep_ta_tc(termsetting1, ltsetting8_0, pc_2ea_013_01_case01,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting8_1, pc_2ea_013_01_case02,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EA.014.00 Restart after an Outcome (Response with EMV data) */
START_TEST(test_2EA_014_00)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;
	txn.amount_authorized = 2;

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_20, pc_2ea_014_00_case01,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_11, pc_2ea_014_00_case02,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_22, pc_2ea_014_00_case03,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_13, pc_2ea_014_00_case04,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_27, pc_2ea_014_00_case05,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_99, pc_2ea_014_00_case06,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EA.014.01 Restart after an Outcome (card removed) */
START_TEST(test_2EA_014_01)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;
	txn.amount_authorized = 2;

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_34, pc_2ea_014_01_case01,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_35, pc_2ea_014_01_case02,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EA.015.00 Removal Timeout set and timeout occurs */
START_TEST(test_2EA_015_00)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;
	txn.amount_authorized = 2;

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_15, pc_2ea_015_00_case01,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_18, pc_2ea_015_00_case02,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_28, pc_2ea_015_00_case03,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_24, pc_2ea_015_00_case04,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EA.016.00 Restart after an Outcome (Online Response Data parameter set to
 * ANY)									      */
START_TEST(test_2EA_016_00)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;
	txn.amount_authorized = 2;

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_10, pc_2ea_016_00_case01,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_21, pc_2ea_016_00_case02,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_7, pc_2ea_016_00_case03,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_23, pc_2ea_016_00_case04,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_16, pc_2ea_016_00_case05,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_25, pc_2ea_016_00_case06,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_8, pc_2ea_016_00_case07,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_29, pc_2ea_016_00_case08,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EA.017.00 RFU bytes and bits					      */
START_TEST(test_2EA_017_00)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;
	txn.amount_authorized = 2;

	rc = emvco_ep_ta_tc(termsetting3, ltsetting6_10, pc_2ea_017_00_case01,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 121;
	rc = emvco_ep_ta_tc(termsetting2, ltsetting6_10, pc_2ea_017_00_case02,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 2;
	rc = emvco_ep_ta_tc(termsetting8, ltsetting6_10, pc_2ea_017_00_case03,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EA.017.01 RFU bytes and bits					      */
START_TEST(test_2EA_017_01)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;
	txn.amount_authorized = 2;

	rc = emvco_ep_ta_tc(termsetting1, ltsetting2_10, pc_2ea_017_01,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EA.018.00 Presence of Data ignored					      */
START_TEST(test_2EA_018_00)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;
	txn.amount_authorized = 2;

	rc = emvco_ep_ta_tc(termsetting1, ltsetting6_11, pc_2ea_018_00,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EA.019.00 Receipt							      */
START_TEST(test_2EA_019_00)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;
	txn.amount_authorized = 2;

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_5, pc_2ea_019_00_case01,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_6, pc_2ea_019_00_case02,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_14, pc_2ea_019_00_case03,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_13, pc_2ea_019_00_case04,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EA.020.00 Padding							      */
START_TEST(test_2EA_020_00)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;
	txn.amount_authorized = 2;

	rc = emvco_ep_ta_tc(termsetting1, ltsetting6_12, pc_2ea_020_00_case01,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting6_13, pc_2ea_020_00_case02,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EA.021.00 Order of Data Elements					      */
START_TEST(test_2EA_021_00)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;
	txn.amount_authorized = 2;

	rc = emvco_ep_ta_tc(termsetting1, ltsetting6_14, pc_2ea_021_00,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EB.001.00 All Pre-Processing indicators set to zero at beginning of
 * Pre-Processing							      */
START_TEST(test_2EB_001_00)
{
	struct emv_txn txn[][2] = {
		{
			{ .type = txn_purchase, .amount_authorized =   1 },
			{ .type = txn_purchase, .amount_authorized =   2 }
		}, {
			{ .type = txn_purchase, .amount_authorized = 121 },
			{ .type = txn_purchase, .amount_authorized =   2 }
		}, {
			{ .type = txn_purchase, .amount_authorized =   0 },
			{ .type = txn_purchase, .amount_authorized =   2 }
		}, {
			{ .type = txn_purchase, .amount_authorized =  16 },
			{ .type = txn_purchase, .amount_authorized =   2 }
		}, {
			{ .type = txn_purchase, .amount_authorized =  13 },
			{ .type = txn_purchase, .amount_authorized =   2 }
		}
	};
	int rc;

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_1, pc_2eb_001_00_case01,
								     txn[0], 2);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_2, pc_2eb_001_00_case02,
								     txn[1], 2);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_3, pc_2eb_001_00_case03,
								     txn[2], 2);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_4, pc_2eb_001_00_case04,
								     txn[3], 2);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_97, pc_2eb_001_00_case05,
								     txn[4], 2);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EB.002.00 Terminal Transaction Qualifier set in Entry Point configuration
 * copied during Pre-Processing						      */
START_TEST(test_2EB_002_00)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;
	txn.amount_authorized = 2;

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_2, pc_2eb_002_00_case01,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting2, ltsetting1_1, pc_2eb_002_00_case02,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting2, ltsetting1_3, pc_2eb_002_00_case03,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EB.003.00 Status Check present and set, Amount = 1			      */
START_TEST(test_2EB_003_00)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;
	txn.amount_authorized = 1;

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_2, pc_2eb_003_00_case01,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting2, ltsetting1_1, pc_2eb_003_00_case02,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting2, ltsetting2_1, pc_2eb_003_00_case03,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting2, ltsetting2_3, pc_2eb_003_00_case04,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_97, pc_2eb_003_00_case05,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EB.003.01 Status Check present and set, Amount = 1 and Transaction Type
 * 'Purchase'								      */
START_TEST(test_2EB_003_01)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;
	txn.amount_authorized = 1;

	rc = emvco_ep_ta_tc(termsetting4, ltsetting1_1, pc_2eb_003_01, &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EB.003.02 Status Check present and set, Amount = 1 and Transaction Type
 * 'Refund'								      */
START_TEST(test_2EB_003_02)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_refund;
	txn.amount_authorized = 1;

	rc = emvco_ep_ta_tc(termsetting4, ltsetting1_4, pc_2eb_003_02, &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EB.004.00 Status Check present and set				      */
START_TEST(test_2EB_004_00)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;
	txn.amount_authorized = 1;

	rc = emvco_ep_ta_tc(termsetting2, ltsetting1_2, pc_2eb_004_00_case01,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting2, ltsetting1_89, pc_2eb_004_00_case02,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting2, ltsetting2_2, pc_2eb_004_00_case03,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EB.004.01 Status Check present and not set with Transaction Type
 * 'Purchase'								      */
START_TEST(test_2EB_004_01)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;
	txn.amount_authorized = 1;

	rc = emvco_ep_ta_tc(termsetting4, ltsetting1_2, pc_2eb_004_01_case01,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting4, ltsetting1_97, pc_2eb_004_01_case02,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EB.004.02 Status Check present and not set with Transaction Type
 * 'Cash Advance'							      */
START_TEST(test_2EB_004_02)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_cash_advance;
	txn.amount_authorized = 1;

	rc = emvco_ep_ta_tc(termsetting4, ltsetting1_1, pc_2eb_004_02, &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EB.005.00 Status Check not present					      */
START_TEST(test_2EB_005_00)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;
	txn.amount_authorized = 1;

	rc = emvco_ep_ta_tc(termsetting2, ltsetting1_3, pc_2eb_005_00_case01,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting2, ltsetting1_4, pc_2eb_005_00_case02,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting2, ltsetting1_97, pc_2eb_005_00_case03,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EB.005.01 Status Check not present and Transaction Type 'Purchase'	      */
START_TEST(test_2EB_005_01)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;
	txn.amount_authorized = 1;

	rc = emvco_ep_ta_tc(termsetting4, ltsetting1_3, pc_2eb_005_01, &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EB.006.00 Status Check present and set, Amount different from 1	      */
START_TEST(test_2EB_006_00)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;
	txn.amount_authorized = 2;

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_2, pc_2eb_006_00_case01,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting2, ltsetting1_1, pc_2eb_006_00_case02,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting2, ltsetting2_1, pc_2eb_006_00_case03,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting2, ltsetting2_3, pc_2eb_006_00_case04,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_97, pc_2eb_006_00_case05,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EB.006.01 Status Check present and set, Amount different from 1 with
 * Transaction Type 'Purchase'						      */
START_TEST(test_2EB_006_01)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;
	txn.amount_authorized = 2;

	rc = emvco_ep_ta_tc(termsetting4, ltsetting1_1, pc_2eb_006_01, &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EB.007.00 Zero Amount not allowed */
START_TEST(test_2EB_007_00)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;

	txn.amount_authorized = 0;
	rc = emvco_ep_ta_tc(termsetting2, ltsetting2_42, pc_2eb_007_00_case01,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 2;
	rc = emvco_ep_ta_tc(termsetting2, ltsetting2_42, pc_2eb_007_00_case02,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EB.007.01 Zero Amount not allowed with Transaction Type 'Purchase'	      */
START_TEST(test_2EB_007_01)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;

	txn.amount_authorized = 0;
	rc = emvco_ep_ta_tc(termsetting4, ltsetting2_4, pc_2eb_007_01_case01,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 2;
	rc = emvco_ep_ta_tc(termsetting4, ltsetting2_4, pc_2eb_007_01_case02,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EB.008.00 Zero Amount allowed					      */
START_TEST(test_2EB_008_00)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;

	txn.amount_authorized = 0;
	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_1, pc_2eb_008_00_case01,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 2;
	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_1, pc_2eb_008_00_case02,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 0;
	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_97, pc_2eb_008_00_case03,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 2;
	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_97, pc_2eb_008_00_case04,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EB.008.01 Zero Amount allowed with Transaction Type 'Purchase'	      */
START_TEST(test_2EB_008_01)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;

	txn.amount_authorized = 0;
	rc = emvco_ep_ta_tc(termsetting4, ltsetting1_3, pc_2eb_008_01_case01,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 2;
	rc = emvco_ep_ta_tc(termsetting4, ltsetting1_3, pc_2eb_008_01_case02,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EB.008.02 Zero Amount allowed with Transaction Type 'Refund		      */
START_TEST(test_2EB_008_02)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_refund;

	txn.amount_authorized = 0;
	rc = emvco_ep_ta_tc(termsetting4, ltsetting1_4, pc_2eb_008_02_case01,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 2;
	rc = emvco_ep_ta_tc(termsetting4, ltsetting1_4, pc_2eb_008_02_case02,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EB.009.00 Zero Amount allowed flag not present			      */
START_TEST(test_2EB_009_00)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;

	txn.amount_authorized = 0;
	rc = emvco_ep_ta_tc(termsetting2, ltsetting1_2, pc_2eb_009_00_case01,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 2;
	rc = emvco_ep_ta_tc(termsetting2, ltsetting1_2, pc_2eb_009_00_case02,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EB.009.01 Zero Amount allowed flag not present with Transaction Type
 * 'Purchase'								      */
START_TEST(test_2EB_009_01)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;

	txn.amount_authorized = 0;
	rc = emvco_ep_ta_tc(termsetting4, ltsetting1_2, pc_2eb_008_00_case01,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 2;
	rc = emvco_ep_ta_tc(termsetting4, ltsetting1_2, pc_2eb_008_00_case02,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 0;
	rc = emvco_ep_ta_tc(termsetting4, ltsetting1_97, pc_2eb_008_00_case03,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 2;
	rc = emvco_ep_ta_tc(termsetting4, ltsetting1_97, pc_2eb_008_00_case04,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EB.009.02 Zero Amount allowed flag not present with Transaction Type
 * 'Cash Advance'							      */
START_TEST(test_2EB_009_02)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_cash_advance;

	txn.amount_authorized = 0;
	rc = emvco_ep_ta_tc(termsetting4, ltsetting1_1, pc_2eb_009_02_case01,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 2;
	rc = emvco_ep_ta_tc(termsetting4, ltsetting1_1, pc_2eb_009_02_case02,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EB.010.00 Reader Contactless Transaction Limit exceeded		      */
START_TEST(test_2EB_010_00)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;

	txn.amount_authorized = 199;
	rc = emvco_ep_ta_tc(termsetting2, ltsetting2_6, pc_2eb_010_00_case01,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 200;
	rc = emvco_ep_ta_tc(termsetting2, ltsetting2_6, pc_2eb_010_00_case02,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 300;
	rc = emvco_ep_ta_tc(termsetting2, ltsetting2_6, pc_2eb_010_00_case03,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 199;
	rc = emvco_ep_ta_tc(termsetting2, ltsetting2_43, pc_2eb_010_00_case04,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 200;
	rc = emvco_ep_ta_tc(termsetting2, ltsetting2_43, pc_2eb_010_00_case05,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EB.010.01 Reader Contactless Transaction Limit exceeded with Transaction
 * Type 'Purchase'							      */
START_TEST(test_2EB_010_01)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;

	txn.amount_authorized = 199;
	rc = emvco_ep_ta_tc(termsetting4, ltsetting2_6, pc_2eb_010_01_case01,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 200;
	rc = emvco_ep_ta_tc(termsetting4, ltsetting2_6, pc_2eb_010_01_case02,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 300;
	rc = emvco_ep_ta_tc(termsetting4, ltsetting2_6, pc_2eb_010_01_case03,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EB.011.00 Reader Contactless Transaction Limit not exceeded		      */
START_TEST(test_2EB_011_00)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;

	txn.amount_authorized = 119;
	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_3, pc_2eb_011_00_case01,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 99;
	rc = emvco_ep_ta_tc(termsetting2, ltsetting1_3, pc_2eb_011_00_case02,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 199;
	rc = emvco_ep_ta_tc(termsetting2, ltsetting1_1, pc_2eb_011_00_case03,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 99;
	rc = emvco_ep_ta_tc(termsetting2, ltsetting1_89, pc_2eb_011_00_case04,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EB.011.01 Reader Contactless Transaction Limit not exceeded with Transaction
 * Type 'Refund'							      */
START_TEST(test_2EB_011_01)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_refund;
	txn.amount_authorized = 99;

	rc = emvco_ep_ta_tc(termsetting4, ltsetting1_2, pc_2eb_011_01, &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EB.011.02 Reader Contactless Transaction Limit not exceeded with Transaction
 * Type 'Purchase with Cashback'					      */
START_TEST(test_2EB_011_02)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase_with_cashback;
	txn.amount_authorized = 199;
	txn.amount_other = 10;

	rc = emvco_ep_ta_tc(termsetting4, ltsetting1_3, pc_2eb_011_02, &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EB.012.00 Reader Contactless Transaction Limit not present		      */
START_TEST(test_2EB_012_00)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;
	txn.amount_authorized = 999999999999;

	rc = emvco_ep_ta_tc(termsetting2, ltsetting1_2, pc_2eb_012_00_case01,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting2, ltsetting1_97, pc_2eb_012_00_case02,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EB.012.01 Reader Contactless Transaction Limit not present		      */
START_TEST(test_2EB_012_01)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_refund;
	txn.amount_authorized = 999999999999;

	rc = emvco_ep_ta_tc(termsetting4, ltsetting1_4, pc_2eb_012_01, &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EB.013.00 Reader Contactless Floor Limit present, Amount Authorized greater
 * than limit								      */
START_TEST(test_2EB_013_00)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;

	txn.amount_authorized = 13;
	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_2, pc_2eb_013_00_case01,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 21;
	rc = emvco_ep_ta_tc(termsetting2, ltsetting1_3, pc_2eb_013_00_case02,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 51;
	rc = emvco_ep_ta_tc(termsetting2, ltsetting1_4, pc_2eb_013_00_case03,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 19;
	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_97, pc_2eb_013_00_case04,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 99;
	rc = emvco_ep_ta_tc(termsetting2, ltsetting1_3, pc_2eb_013_00_case05,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 123456;
	rc = emvco_ep_ta_tc(termsetting2, ltsetting1_4, pc_2eb_013_00_case06,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EB.013.01 Reader Contactless Floor Limit present, Amount Authorized greater
 * than limit with Transaction Type 'Purchase'				      */
START_TEST(test_2EB_013_01)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;
	txn.amount_authorized = 21;

	rc = emvco_ep_ta_tc(termsetting4, ltsetting1_3, pc_2eb_013_01, &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EB.013.01 Reader Contactless Floor Limit present, Amount Authorized greater
 * than limit with Transaction Type 'Refund'				      */
START_TEST(test_2EB_013_02)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_refund;
	txn.amount_authorized = 51;

	rc = emvco_ep_ta_tc(termsetting4, ltsetting1_4, pc_2eb_013_02, &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EB.014.00 Reader Contactless Floor Limit present, Amount Authorized less
 * than limit								      */
START_TEST(test_2EB_014_00)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;

	txn.amount_authorized = 12;
	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_4, pc_2eb_014_00_case01,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 20;
	rc = emvco_ep_ta_tc(termsetting2, ltsetting1_92, pc_2eb_014_00_case02,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 50;
	rc = emvco_ep_ta_tc(termsetting2, ltsetting1_4, pc_2eb_014_00_case03,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 15;
	rc = emvco_ep_ta_tc(termsetting2, ltsetting1_92, pc_2eb_014_00_case04,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 45;
	rc = emvco_ep_ta_tc(termsetting2, ltsetting1_97, pc_2eb_014_00_case05,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EB.014.01 Reader Contactless Floor Limit present, Amount Authorized less
 * than limit with Transaction Type 'Purchase'				      */
START_TEST(test_2EB_014_01)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;
	txn.amount_authorized = 20;

	rc = emvco_ep_ta_tc(termsetting4, ltsetting1_3, pc_2eb_014_01, &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EB.014.02 Reader Contactless Floor Limit present, Amount Authorized less
 * than limit with Transaction Type 'Refund'				      */
START_TEST(test_2EB_014_02)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_refund;

	txn.amount_authorized = 20;
	rc = emvco_ep_ta_tc(termsetting4, ltsetting1_2, pc_2eb_014_02_case01,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 40;
	rc = emvco_ep_ta_tc(termsetting4, ltsetting1_3, pc_2eb_014_02_case02,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EB.015.00 Reader Contactless Floor Limit not present, Terminal Floor Limit
 * present								      */
START_TEST(test_2EB_015_00)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;

	txn.amount_authorized = 26;
	rc = emvco_ep_ta_tc(termsetting2, ltsetting1_1, pc_2eb_015_00_case01,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 50;
	rc = emvco_ep_ta_tc(termsetting2, ltsetting2_1, pc_2eb_015_00_case02,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 25;
	rc = emvco_ep_ta_tc(termsetting2, ltsetting1_1, pc_2eb_015_00_case03,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 20;
	rc = emvco_ep_ta_tc(termsetting2, ltsetting2_1, pc_2eb_015_00_case04,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 26;
	rc = emvco_ep_ta_tc(termsetting2, ltsetting1_100, pc_2eb_015_00_case05,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 25;
	rc = emvco_ep_ta_tc(termsetting2, ltsetting1_100, pc_2eb_015_00_case06,
								       &txn, 1);
}
END_TEST

/* 2EB.015.01 Reader Contactless Floor Limit not present, Terminal Floor Limit
 * present with Transaction type 'Purchase'				      */
START_TEST(test_2EB_015_01)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;

	txn.amount_authorized = 26;
	rc = emvco_ep_ta_tc(termsetting4, ltsetting1_1, pc_2eb_015_01_case01,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 50;
	rc = emvco_ep_ta_tc(termsetting4, ltsetting1_1, pc_2eb_015_01_case02,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 25;
	rc = emvco_ep_ta_tc(termsetting4, ltsetting1_1, pc_2eb_015_01_case03,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 20;
	rc = emvco_ep_ta_tc(termsetting4, ltsetting1_1, pc_2eb_015_01_case04,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EB.015.02 Reader Contactless Floor Limit not present, Terminal Floor Limit
 * present with Transaction type 'Purchase with Cashback'		      */
START_TEST(test_2EB_015_02)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase_with_cashback;

	txn.amount_authorized = 26;
	txn.amount_other = 10;
	rc = emvco_ep_ta_tc(termsetting4, ltsetting1_3, pc_2eb_015_02_case01,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 50;
	txn.amount_other = 10;
	rc = emvco_ep_ta_tc(termsetting4, ltsetting1_3, pc_2eb_015_02_case02,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 25;
	txn.amount_other = 10;
	rc = emvco_ep_ta_tc(termsetting4, ltsetting1_3, pc_2eb_015_02_case03,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 20;
	txn.amount_other = 10;
	rc = emvco_ep_ta_tc(termsetting4, ltsetting1_3, pc_2eb_015_02_case04,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EB.016.00 Reader Contactless Floor Limit and Terminal Floor Limit not
 * present								      */
START_TEST(test_2EB_016_00)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;
	txn.amount_authorized = 999999999999;

	rc = emvco_ep_ta_tc(termsetting2, ltsetting1_2, pc_2eb_016_00, &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EB.016.01 Reader Contactless Floor Limit and Terminal Floor Limit not
 * present with Transaction Type 'Purchase'				      */
START_TEST(test_2EB_016_01)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;
	txn.amount_authorized = 999999999999;

	rc = emvco_ep_ta_tc(termsetting4, ltsetting1_2, pc_2eb_016_01, &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EB.016.02 Reader Contactless Floor Limit and Terminal Floor Limit not
 * present with Transaction Type 'Cash Advance'				      */
START_TEST(test_2EB_016_02)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_cash_advance;
	txn.amount_authorized = 999999999999;

	rc = emvco_ep_ta_tc(termsetting4, ltsetting1_1, pc_2eb_016_02_case01,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting4, ltsetting1_97, pc_2eb_016_02_case02,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EB.017.00 Reader CVM Required Limit present, Amount Authorized greater or
 * equal to limit							      */
START_TEST(test_2EB_017_00)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;

	txn.amount_authorized = 16;
	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_1, pc_2eb_017_00_case01,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 10;
	rc = emvco_ep_ta_tc(termsetting2, ltsetting1_2, pc_2eb_017_00_case02,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 21;
	rc = emvco_ep_ta_tc(termsetting2, ltsetting1_4, pc_2eb_017_00_case03,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 20;
	rc = emvco_ep_ta_tc(termsetting2, ltsetting1_97, pc_2eb_017_00_case04,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EB.017.01 Reader CVM Required Limit present, Amount Authorized greater than
 * limit with Transaction Type 'Purchase'				      */
START_TEST(test_2EB_017_01)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;
	txn.amount_authorized = 21;

	rc = emvco_ep_ta_tc(termsetting4, ltsetting1_4, pc_2eb_017_01, &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EB.017.02 Reader CVM Required Limit present, Amount Authorized greater than
 * limit with Transaction Type 'Cash Advance'				      */
START_TEST(test_2EB_017_02)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_cash_advance;
	txn.amount_authorized = 11;

	rc = emvco_ep_ta_tc(termsetting4, ltsetting1_1, pc_2eb_017_02, &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EB.018.00 Reader CVM Required Limit present, Amount Authorized less than
 * limit								      */
START_TEST(test_2EB_018_00)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;

	txn.amount_authorized = 14;
	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_1, pc_2eb_018_00_case01,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 5;
	rc = emvco_ep_ta_tc(termsetting2, ltsetting1_2, pc_2eb_018_00_case02,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 19;
	rc = emvco_ep_ta_tc(termsetting2, ltsetting1_4, pc_2eb_018_00_case03,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 19;
	rc = emvco_ep_ta_tc(termsetting2, ltsetting1_97, pc_2eb_018_00_case04,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EB.018.01 Reader CVM Required Limit present, Amount Authorized less than
 * limit with Transaction Type 'Purchase'				      */
START_TEST(test_2EB_018_01)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;
	txn.amount_authorized = 9;

	rc = emvco_ep_ta_tc(termsetting4, ltsetting1_2, pc_2eb_018_01, &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EB.018.02 Reader CVM Required Limit present, Amount Authorized less than
 * limit with Transaction Type 'Refund'					      */
START_TEST(test_2EB_018_02)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_refund;
	txn.amount_authorized = 19;

	rc = emvco_ep_ta_tc(termsetting4, ltsetting1_4, pc_2eb_018_02, &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EB.019.00 Reader CVM Required Limit not present			      */
START_TEST(test_2EB_019_00)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;
	txn.amount_authorized = 999999999999;

	rc = emvco_ep_ta_tc(termsetting2, ltsetting1_92, pc_2eb_019_00,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EB.019.01 Reader CVM Required Limit not present with Transaction Type
 * 'Purchase'								      */
START_TEST(test_2EB_019_01)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;
	txn.amount_authorized = 999999999999;

	rc = emvco_ep_ta_tc(termsetting4, ltsetting1_3, pc_2eb_019_01, &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EB.019.02 Reader CVM Required Limit not present with Transaction Type
 * 'Refund'								      */
START_TEST(test_2EB_019_02)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_refund;
	txn.amount_authorized = 999999999999;

	rc = emvco_ep_ta_tc(termsetting4, ltsetting1_3, pc_2eb_019_02, &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EB.020.00 Zero Amount indicator set with TTQ (online capable reader)      */
START_TEST(test_2EB_020_00)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;

	txn.amount_authorized = 0;
	rc = emvco_ep_ta_tc(termsetting2, ltsetting1_3, pc_2eb_020_00_case01,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 2;
	rc = emvco_ep_ta_tc(termsetting2, ltsetting1_3, pc_2eb_020_00_case02,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EB.020.01 Zero Amount indicator set with TTQ (online capable reader) with
 * Transaction Type 'Purchase'						      */
START_TEST(test_2EB_020_01)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;

	txn.amount_authorized = 0;
	rc = emvco_ep_ta_tc(termsetting4, ltsetting1_3, pc_2eb_020_00_case01,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 2;
	rc = emvco_ep_ta_tc(termsetting4, ltsetting1_3, pc_2eb_020_00_case02,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EB.021.00 Zero Amount indicator set with TTQ (offline only reader)	      */
START_TEST(test_2EB_021_00)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;

	txn.amount_authorized = 0;
	rc = emvco_ep_ta_tc(termsetting2, ltsetting2_5, pc_2eb_021_00_case01,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 2;
	rc = emvco_ep_ta_tc(termsetting2, ltsetting2_5, pc_2eb_021_00_case02,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EB.022.00 Contactless Application Not Allowed for all combinations after
 * Pre-Processing							      */
START_TEST(test_2EB_022_00)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;
	txn.amount_authorized = 121;

	rc = emvco_ep_ta_tc(termsetting1, ltsetting2_5, pc_2eb_022_00, &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EC.001.00 All Pre-Processing indicators set to zero when Pre-Processing not
 * performed								      */
START_TEST(test_2EC_001_00)
{
	int rc;

	rc = emvco_ep_ta_tc(termsetting3, ltsetting1_1, pc_2ec_001_00_case01,
								       NULL, 0);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting3, ltsetting1_2, pc_2ec_001_00_case02,
								       NULL, 0);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting3, ltsetting1_3, pc_2ec_001_00_case03,
								       NULL, 0);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting3, ltsetting1_4, pc_2ec_001_00_case04,
								       NULL, 0);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting3, ltsetting1_97, pc_2ec_001_00_case05,
								       NULL, 0);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EC.001.01 All Pre-Processing indicators set to zero when Pre-Processing not
* performed with Transaction Type 'Purchase'				      */
START_TEST(test_2EC_001_01)
{
	int rc;

	rc = emvco_ep_ta_tc(termsetting5_1, ltsetting1_1, pc_2ec_001_01, NULL,
									     0);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EC.001.02 All Pre-Processing indicators set to zero when Pre-Processing not
 * performed with Transaction Type 'Purchase with Cashback'		      */
START_TEST(test_2EC_001_02)
{
	int rc;

	rc = emvco_ep_ta_tc(termsetting5_2, ltsetting1_3, pc_2ec_001_02, NULL,
									     0);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EC.001.03 All Pre-Processing indicators set to zero when Pre-Processing not
 * performed with Transaction Type 'Cash Advance'			      */
START_TEST(test_2EC_001_03)
{
	int rc;

	rc = emvco_ep_ta_tc(termsetting5_3, ltsetting1_1, pc_2ec_001_03, NULL,
									     0);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EC.001.04 All Pre-Processing indicators set to zero when Pre-Processing not
 * performed with Transaction Type ‘Refund’				      */
START_TEST(test_2EC_001_04)
{
	int rc;

	rc = emvco_ep_ta_tc(termsetting5_4, ltsetting1_4, pc_2ec_001_04, NULL,
									     0);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EC.002.00 Restart flag set and UI Request on Restart present	      */
START_TEST(test_2EC_002_00)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;
	txn.amount_authorized = 1;

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_17, pc_2ec_002_00_case01,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_19, pc_2ec_002_00_case02,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_12, pc_2ec_002_00_case03,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_9, pc_2ec_002_00_case04,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EC.003.00 Restart flag set and UI Request on Restart not present	      */
START_TEST(test_2EC_003_00)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;
	txn.amount_authorized = 1;

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_20, pc_2ec_003_00_case01,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_26, pc_2ec_003_00_case02,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_22, pc_2ec_003_00_case03,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_29, pc_2ec_003_00_case04,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EC.004.00 Restart flag not set, Message 'PRESENT CARD'		      */
START_TEST(test_2EC_004_00)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;
	txn.amount_authorized = 1;

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_1, pc_2ec_004_00_case01,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting3, ltsetting1_2, pc_2ec_004_00_case02,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EC.005.00 Field powered and polling					      */
START_TEST(test_2EC_005_00)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;
	txn.amount_authorized = 2;

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_1, pc_2ec_005_00_case01,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_2, pc_2ec_005_00_case02,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting3, ltsetting1_1, pc_2ec_005_00_case03,
								       NULL, 0);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting3, ltsetting1_2, pc_2ec_005_00_case04,
								       NULL, 0);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

static int emvco_ep_ta_tc_collision(enum termsetting termsetting,
		   enum ltsetting ltsetting, int ltmode, enum pass_criteria pc,
						     const struct emv_txn *txn)
{
	struct emv_outcome_parms outcome;
	struct emvco_ep_ta_tc_fixture fixture;
	struct chk *chk = NULL;
	int rc;

	chk = chk_pass_criteria_new(pc, log4c_category);
	if (!chk) {
		rc = EMV_RC_OUT_OF_MEMORY;
		goto done;
	}

	rc = emvco_ep_ta_tc_fixture_setup(&fixture, chk, termsetting, ltsetting,
									ltmode);
	if (rc != EMV_RC_OK)
		goto done;

	rc = emv_ep_activate(fixture.ep, start_a, txn,
			      ++transaction_sequence_counter, txn, 0, &outcome);
	if (rc != EMV_RC_OK)
		goto done;

	emvco_ep_ta_tc_fixture_teardown(&fixture);

	if (!chk->ops->pass_criteria_met(chk))
		rc = EMV_RC_FAIL;
done:
	if (chk && chk->ops && chk->ops->free)
		chk->ops->free(chk);

	return rc;
}

/* 2EC.006.00 Protocol Activation Collision				      */
START_TEST(test_2EC_006_00)
{
	struct emv_txn txn;
	int rc, ltmode = LT_COLLISION_THEN_WITHDRAW_BOTH;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;
	txn.amount_authorized = 2;

	rc = emvco_ep_ta_tc_collision(termsetting1, ltsetting1_1, ltmode,
						    pc_2ec_006_00_case01, &txn);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc_collision(termsetting1, ltsetting1_2, ltmode,
						    pc_2ec_006_00_case02, &txn);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc_collision(termsetting3, ltsetting1_1, ltmode,
						    pc_2ec_006_00_case03, &txn);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc_collision(termsetting3, ltsetting1_2, ltmode,
						    pc_2ec_006_00_case04, &txn);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2EC.007.00 Protocol Activation Collision				      */
START_TEST(test_2EC_007_00)
{
	struct emv_txn txn;
	int rc, ltmode = LT_COLLISION_THEN_WITHDRAW_ONE;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;
	txn.amount_authorized = 2;

	rc = emvco_ep_ta_tc_collision(termsetting1, ltsetting1_1, ltmode,
						    pc_2ec_007_00_case01, &txn);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc_collision(termsetting1, ltsetting1_2, ltmode,
						    pc_2ec_007_00_case02, &txn);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc_collision(termsetting3, ltsetting1_1, ltmode,
						    pc_2ec_006_00_case03, &txn);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc_collision(termsetting3, ltsetting1_2, ltmode,
						    pc_2ec_006_00_case04, &txn);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2ED.001.00 Entry point Activation at Start B with Issuer Authentication Data
 * or Issuer Script present						      */
START_TEST(test_2ED_001_00)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;
	txn.amount_authorized = 2;

	rc = emvco_ep_ta_tc(termsetting1, ltsetting2_10, pc_2ed_001_00_case01,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting3, ltsetting2_7, pc_2ed_001_00_case02,
								       NULL, 0);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting2_17, pc_2ed_001_00_case03,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting2_8, pc_2ed_001_00_case04,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting3, ltsetting2_9, pc_2ed_001_00_case05,
								       NULL, 0);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting2_18, pc_2ed_001_00_case06,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2ED.001.01 Entry point Activation at Start B with Issuer Authentication Data
 * or Issuer Script present with Extended Selection			      */
START_TEST(test_2ED_001_01)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;
	txn.amount_authorized = 2;

	rc = emvco_ep_ta_tc(termsetting1, ltsetting2_11, pc_2ed_001_01_case01,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting3, ltsetting2_12, pc_2ed_001_01_case02,
								       NULL, 0);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting2_13, pc_2ed_001_01_case03,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting2_14, pc_2ed_001_01_case04,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting3, ltsetting2_15, pc_2ed_001_01_case05,
								       NULL, 0);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting2_16, pc_2ed_001_01_case06,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2ED.002.00 Entry point Activation at Start C (Restart)		      */
START_TEST(test_2ED_002_00)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;
	txn.amount_authorized = 2;

	rc = emvco_ep_ta_tc(termsetting1, ltsetting2_19, pc_2ed_002_00, &txn,
									     1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2ED.002.01 Entry point Activation at Start C (Restart) with Extended
 * Selection								      */
START_TEST(test_2ED_002_01)
{
	int rc;

	rc = emvco_ep_ta_tc(termsetting3, ltsetting2_25, pc_2ed_002_01, NULL,
									     0);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2ED.003.00 PPSE selection						     */
START_TEST(test_2ED_003_00)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;
	txn.amount_authorized = 2;

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_1, pc_2ed_003_00, &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting3, ltsetting1_1, pc_2ed_003_00, NULL, 0);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2ED.003.01 PPSE with additional data					      */
START_TEST(test_2ED_003_01)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;
	txn.amount_authorized = 2;

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_50, pc_2ed_003_01_case01,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_51, pc_2ed_003_01_case02,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_52, pc_2ed_003_01_case03,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2ED.004.00 PPSE selection rejected					      */
START_TEST(test_2ED_004_00)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;
	txn.amount_authorized = 2;

	rc = emvco_ep_ta_tc(termsetting1, ltsetting6_1, pc_2ed_004_00, &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting3, ltsetting6_2, pc_2ed_004_00, NULL, 0);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2ED.005.00 PPSE select response without Directory Entry		      */
START_TEST(test_2ED_005_00)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;
	txn.amount_authorized = 2;

	rc = emvco_ep_ta_tc(termsetting1, ltsetting6_3, pc_2ed_005_00, &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting3, ltsetting6_3, pc_2ed_005_00, NULL, 0);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2ED.007.00 PPSE select response with a Directory Entry wrongly coded on ADF
 * name and another correctly coded					      */
START_TEST(test_2ED_007_00)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;
	txn.amount_authorized = 2;

	rc = emvco_ep_ta_tc(termsetting1, ltsetting6_5, pc_2ed_007_00, &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting3, ltsetting6_5, pc_2ed_007_00, NULL, 0);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2ED.007.01 PPSE select response with a Directory Entry with ADF name missing
 * and another correctly coded						      */
START_TEST(test_2ED_007_01)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;
	txn.amount_authorized = 2;

	rc = emvco_ep_ta_tc(termsetting1, ltsetting6_16, pc_2ed_007_01, &txn,
									     1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting3, ltsetting6_16, pc_2ed_007_01, NULL,
									     0);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2ED.008.00 PPSE select response having a Directory Entry with AID not
 * matching								      */
START_TEST(test_2ED_008_00)
{
	struct emv_txn txn;
	int rc;

	memset(&txn, 0, sizeof(txn));
	txn.type = txn_purchase;
	txn.amount_authorized = 2;

	rc = emvco_ep_ta_tc(termsetting1, ltsetting6_6, pc_2ed_008_00, &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting3, ltsetting6_6, pc_2ed_008_00, NULL, 0);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2ED.009.00 PPSE select response having a Directory Entry with Kernel ID not
 * present (Matching AID = Visa AID, TK3 Kernel ID = ’03’)		      */
START_TEST(test_2ED_009_00)
{
	struct emv_txn txn = { .type = txn_purchase, .amount_authorized = 2 };
	int rc;

	rc = emvco_ep_ta_tc(termsetting1, ltsetting5_1, pc_2ed_009_00, &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2ED.009.02 PPSE select response having a Directory Entry with Kernel ID not
 * present (Matching AID = MasterCard AID, TK2 Kernel ID = '02')	      */
START_TEST(test_2ED_009_02)
{
	struct emv_txn txn = { .type = txn_purchase, .amount_authorized = 2 };
	int rc;

	rc = emvco_ep_ta_tc(termsetting3, ltsetting5_2, pc_2ed_009_02, NULL, 0);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting5_11, pc_2ed_009_02,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2ED.009.04 PPSE select response having a Directory Entry with Kernel ID not
 * present (Matching AID = JCB AID, TK1 Kernel ID = '05'		      */
START_TEST(test_2ED_009_04)
{
	int rc;

	rc = emvco_ep_ta_tc(termsetting3, ltsetting5_3, pc_2ed_009_04, NULL, 0);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2ED.009.06 PPSE select response having a Directory Entry with Kernel ID not
 * present (Matching AID = AMEX AID, TK4 Kernel ID = '04')		      */
START_TEST(test_2ED_009_06)
{
	struct emv_txn txn = { .type = txn_purchase, .amount_authorized = 2 };
	int rc;

	rc = emvco_ep_ta_tc(termsetting1, ltsetting5_4, pc_2ed_009_06_case01,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting5_10, pc_2ed_009_06_case02,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2ED.009.10 PPSE select response having a Directory Entry with Kernel ID with
 * null length (Matching AID = Visa AID, TK3 Kernel ID = '03')		      */
START_TEST(test_2ED_009_10)
{
	struct emv_txn txn = { .type = txn_purchase, .amount_authorized = 2 };
	int rc;

	rc = emvco_ep_ta_tc(termsetting1, ltsetting5_13, pc_2ed_009_10, &txn,
									     1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2ED.009.12 PPSE select response having a Directory Entry with Kernel ID with
 * null length (Matching AID = MasterCard AID, TK2 Kernel ID = '02')	      */
START_TEST(test_2ED_009_12)
{
	int rc;

	rc = emvco_ep_ta_tc(termsetting3, ltsetting5_14, pc_2ed_009_12, NULL,
									     0);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2ED.009.14 PPSE select response having a Directory Entry with Kernel ID with
* null length (Matching AID = JCB AID, TK1 Kernel ID = '05')		      */
START_TEST(test_2ED_009_14)
{
	int rc;

	rc = emvco_ep_ta_tc(termsetting3, ltsetting5_15, pc_2ed_009_14, NULL,
									     0);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2ED.009.16 PPSE select response having a Directory Entry with Kernel ID with
 * null length (Matching AID = AMEX AID, TK4 Kernel ID = '04')		      */
START_TEST(test_2ED_009_16)
{
	struct emv_txn txn = { .type = txn_purchase, .amount_authorized = 2 };
	int rc;

	rc = emvco_ep_ta_tc(termsetting1, ltsetting5_16, pc_2ed_009_16, &txn,
									     1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2ED.009.18 PPSE select response having a Directory Entry with Kernel ID not
 * present (Matching AID = DISCOVER AID, TK6 Kernel ID = '06')		      */
START_TEST(test_2ED_009_18)
{
	struct emv_txn txn = { .type = txn_purchase, .amount_authorized = 2 };
	int rc;

	rc = emvco_ep_ta_tc(termsetting1, ltsetting5_17, pc_2ed_009_18, &txn,
									     1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2ED.009.20 PPSE select response having a Directory Entry with Kernel ID with
 * null length (Matching AID = DISCOVER AID, TK6 Kernel ID = '06')	      */
START_TEST(test_2ED_009_20)
{
	struct emv_txn txn = { .type = txn_purchase, .amount_authorized = 2 };
	int rc;

	rc = emvco_ep_ta_tc(termsetting1, ltsetting5_18, pc_2ed_009_20, &txn,
									     1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2ED.009.22 PPSE select response having a Directory Entry with Kernel ID not
 * present (Matching AID = CUP AID, TK7 Kernel ID = '07')		      */
START_TEST(test_2ED_009_22)
{
	struct emv_txn txn = { .type = txn_purchase, .amount_authorized = 2 };
	int rc;

	rc = emvco_ep_ta_tc(termsetting1, ltsetting5_19, pc_2ed_009_22, &txn,
									     1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2ED.009.24 PPSE select response having a Directory Entry with Kernel ID with
 * null length (Matching AID = CUP AID, TK7 Kernel ID = '07')		      */
START_TEST(test_2ED_009_24)
{
	struct emv_txn txn = { .type = txn_purchase, .amount_authorized = 2 };
	int rc;

	rc = emvco_ep_ta_tc(termsetting1, ltsetting5_20, pc_2ed_009_24, &txn,
									     1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2ED.009.26 PPSE select response having a Directory Entry with Kernel ID not
 * present (Matching AID = DISCOVER AID, TK6 Kernel ID = '06')		      */
START_TEST(test_2ED_009_26)
{
	struct emv_txn txn = { .type = txn_purchase, .amount_authorized = 2 };
	int rc;

	rc = emvco_ep_ta_tc(termsetting1, ltsetting5_23, pc_2ed_009_26, &txn,
									     1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2ED.009.28 PPSE select response having a Directory Entry with Kernel ID with
 * null length (Matching AID = DISCOVER AID, TK6 Kernel ID = '06')	      */
START_TEST(test_2ED_009_28)
{
	struct emv_txn txn = { .type = txn_purchase, .amount_authorized = 2 };
	int rc;

	rc = emvco_ep_ta_tc(termsetting1, ltsetting5_24, pc_2ed_009_28, &txn,
									     1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2ED.010.00 PPSE select response having a Directory Entry with Kernel ID wrong
 * length								      */
START_TEST(test_2ED_010_00)
{
	struct emv_txn txn = { .type = txn_purchase, .amount_authorized = 2 };
	int rc;

	rc = emvco_ep_ta_tc(termsetting1, ltsetting6_7, pc_2ed_010_00, &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting3, ltsetting6_7, pc_2ed_010_00, NULL, 0);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2ED.011.00 PPSE select response having a Directory Entry with Kernel ID not
 * matching								      */
START_TEST(test_2ED_011_00)
{
	struct emv_txn txn = { .type = txn_purchase, .amount_authorized = 2 };
	int rc;

	rc = emvco_ep_ta_tc(termsetting1, ltsetting4_1, pc_2ed_011_00, &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting2, ltsetting4_2, pc_2ed_011_00, &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting3, ltsetting4_2, pc_2ed_011_00, NULL, 0);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2ED.012.00 PPSE select response having a Directory Entry with null Kernel ID
 * (Matching AID = Visa AID, TK3 Kernel ID = '03')			      */
START_TEST(test_2ED_012_00)
{
	struct emv_txn txn = { .type = txn_purchase, .amount_authorized = 2 };
	int rc;

	rc = emvco_ep_ta_tc(termsetting2, ltsetting5_5, pc_2ed_012_00, &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2ED.012.02 PPSE select response having a Directory Entry with null Kernel ID
 * (Matching AID = MasterCard AID, TK2 Kernel ID = '02')		      */
START_TEST(test_2ED_012_02)
{
	int rc;

	rc = emvco_ep_ta_tc(termsetting3, ltsetting5_6, pc_2ed_012_02, NULL, 0);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2ED.012.04 PPSE select response having a Directory Entry with null Kernel ID
 * (Matching AID = JCB AID, TK5 Kernel ID = '05')			      */
START_TEST(test_2ED_012_04)
{
	struct emv_txn txn = { .type = txn_purchase, .amount_authorized = 2 };
	int rc;

	rc = emvco_ep_ta_tc(termsetting2, ltsetting5_7, pc_2ed_012_04, &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2ED.012.06 PPSE select response having a Directory Entry with null Kernel ID
 * (Matching AID = AMEX AID, TK4 Kernel ID = '04')			      */
START_TEST(test_2ED_012_06)
{
	struct emv_txn txn = { .type = txn_purchase, .amount_authorized = 2 };
	int rc;

	rc = emvco_ep_ta_tc(termsetting3, ltsetting5_8, pc_2ed_012_06_case01,
								       NULL, 0);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting2, ltsetting5_12, pc_2ed_012_06_case02,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2ED.012.08 PPSE select response having a Directory Entry with null Kernel ID
 * (Matching AID, Kernel ID = domestic Kernel)				      */
START_TEST(test_2ED_012_08)
{
	int rc;

	rc = emvco_ep_ta_tc(termsetting13, ltsetting1_93, pc_2ed_012_08_case01,
								       NULL, 0);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting13, ltsetting1_94, pc_2ed_012_08_case02,
								       NULL, 0);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting13, ltsetting1_95, pc_2ed_012_08_case03,
								       NULL, 0);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting13, ltsetting1_96, pc_2ed_012_08_case04,
								       NULL, 0);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2ED.012.10 Matching combination with International or Domestic Kernel ID   */
START_TEST(test_2ED_012_10)
{
	struct emv_txn txn = { .type = txn_purchase, .amount_authorized = 2 };
	int rc;

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_1, pc_2ed_012_10_case01,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_2, pc_2ed_012_10_case02,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_3, pc_2ed_012_10_case03,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_4, pc_2ed_012_10_case04,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_84, pc_2ed_012_10_case05,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_85, pc_2ed_012_10_case06,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_86, pc_2ed_012_10_case07,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_87, pc_2ed_012_10_case08,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_88, pc_2ed_012_10_case09,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_97, pc_2ed_012_10_case10,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2ED.012.11 PPSE select response having a Directory Entry with null Kernel ID
 * (Matching AID = DISCOVER AID, TK6 Kernel ID = '06')			      */
START_TEST(test_2ED_012_11)
{
	struct emv_txn txn = { .type = txn_purchase, .amount_authorized = 2 };
	int rc;

	rc = emvco_ep_ta_tc(termsetting2, ltsetting5_21, pc_2ed_012_11, &txn,
									     1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2ED.012.13 PPSE select response having a Directory Entry with null Kernel ID
 * (Matching AID = CUP AID, TK7 Kernel ID = '07')			      */
START_TEST(test_2ED_012_13)
{
	struct emv_txn txn = { .type = txn_purchase, .amount_authorized = 2 };
	int rc;

	rc = emvco_ep_ta_tc(termsetting2, ltsetting5_22, pc_2ed_012_13, &txn,
									     1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2ED.012.15 PPSE select response having a Directory Entry with null Kernel ID
 * (Matching AID = DISCOVER AID, TK6 Kernel ID = '06')			      */
START_TEST(test_2ED_012_15)
{
	struct emv_txn txn = { .type = txn_purchase, .amount_authorized = 2 };
	int rc;

	rc = emvco_ep_ta_tc(termsetting2, ltsetting5_25, pc_2ed_012_15, &txn,
									     1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2ED.013.00 PPSE select response having each Directory Entry with different
 * priority set								      */
START_TEST(test_2ED_013_00)
{
	struct emv_txn txn = { .type = txn_purchase };
	int rc;

	rc = emvco_ep_ta_tc(termsetting3, ltsetting2_20, pc_2ed_013_00_case01,
								       NULL, 0);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 2;
	rc = emvco_ep_ta_tc(termsetting2, ltsetting2_20, pc_2ed_013_00_case02,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 121;
	rc = emvco_ep_ta_tc(termsetting2, ltsetting2_20, pc_2ed_013_00_case03,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting3, ltsetting2_21, pc_2ed_013_00_case04,
								       NULL, 0);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 0;
	rc = emvco_ep_ta_tc(termsetting2, ltsetting2_21, pc_2ed_013_00_case05,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting3, ltsetting3_1, pc_2ed_013_00_case06,
								       NULL, 0);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 0;
	rc = emvco_ep_ta_tc(termsetting2, ltsetting3_1, pc_2ed_013_00_case07,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 2;
	rc = emvco_ep_ta_tc(termsetting1, ltsetting2_24, pc_2ed_013_00_case08,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 2;
	rc = emvco_ep_ta_tc(termsetting1, ltsetting3_4, pc_2ed_013_00_case09,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 201;
	rc = emvco_ep_ta_tc(termsetting2, ltsetting2_44, pc_2ed_013_00_case10,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2ED.014.00 PPSE select response having several Directory Entries with same
 * highest priority set.						      */
START_TEST(test_2ED_014_00)
{
	struct emv_txn txn = { .type = txn_purchase };
	int rc;

	rc = emvco_ep_ta_tc(termsetting3, ltsetting2_22, pc_2ed_014_00_case01,
								       NULL, 0);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 121;
	rc = emvco_ep_ta_tc(termsetting2, ltsetting2_22, pc_2ed_014_00_case02,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting3, ltsetting3_2, pc_2ed_014_00_case03,
								       NULL, 0);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 0;
	rc = emvco_ep_ta_tc(termsetting2, ltsetting3_2, pc_2ed_014_00_case04,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 2;
	rc = emvco_ep_ta_tc(termsetting2, ltsetting3_6, pc_2ed_014_00_case05,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2ED.015.00 PPSE select response having Directory Entries with null priority
 * or no priority set.							      */
START_TEST(test_2ED_015_00)
{
	struct emv_txn txn = { .type = txn_purchase };
	int rc;

	rc = emvco_ep_ta_tc(termsetting3, ltsetting2_23, pc_2ed_015_00_case01,
								       NULL, 0);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 0;
	rc = emvco_ep_ta_tc(termsetting2, ltsetting2_23, pc_2ed_015_00_case02,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting3, ltsetting3_3, pc_2ed_015_00_case03,
								       NULL, 0);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 0;
	rc = emvco_ep_ta_tc(termsetting2, ltsetting3_3, pc_2ed_015_00_case04,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 2;
	rc = emvco_ep_ta_tc(termsetting2, ltsetting3_7, pc_2ed_015_00_case05,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2ED.016.00 Candidate List empty					      */
START_TEST(test_2ED_016_00)
{
	struct emv_txn txn = { .type = txn_purchase, .amount_authorized = 0 };
	int rc;

	rc = emvco_ep_ta_tc(termsetting2, ltsetting4_2, pc_2ed_016_00, &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2ED.017.00 Extended selection present in Directory Entry but Extended
 * Selection Support flag not present or not set in Reader Combination.	      */
START_TEST(test_2ED_017_00)
{
	struct emv_txn txn = { .type = txn_purchase, .amount_authorized = 0 };
	int rc;

	rc = emvco_ep_ta_tc(termsetting3, ltsetting3_5, pc_2ed_017_00_case01,
								       NULL, 0);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting2, ltsetting3_5, pc_2ed_017_00_case02,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2ED.018.00 Extended selection present in Directory Entry and Extended
 * Selection Support flag set in Reader Combination.			      */
START_TEST(test_2ED_018_00)
{
	struct emv_txn txn = { .type = txn_purchase, .amount_authorized = 2 };
	int rc;

	rc = emvco_ep_ta_tc(termsetting1, ltsetting3_5, pc_2ed_018_00_case01,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting2, ltsetting1_30, pc_2ed_018_00_case02,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting3, ltsetting1_30, pc_2ed_018_00_case03,
								       NULL, 0);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_31, pc_2ed_018_00_case04,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting1, ltsetting1_101, pc_2ed_018_00_case05,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

/* 2ED.019.00 Final selection rejected (restart flag = 0)		      */
START_TEST(test_2ED_019_00)
{
	struct emv_txn txn = { .type = txn_purchase };
	int rc;

	rc = emvco_ep_ta_tc(termsetting3, ltsetting6_8, pc_2ed_019_00_case01,
								       NULL, 0);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 121;
	rc = emvco_ep_ta_tc(termsetting2, ltsetting6_8, pc_2ed_019_00_case02,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);

	rc = emvco_ep_ta_tc(termsetting3, ltsetting6_17, pc_2ed_019_00_case03,
								       NULL, 0);
	ck_assert(rc == EMV_RC_OK);

	txn.amount_authorized = 2;
	rc = emvco_ep_ta_tc(termsetting2, ltsetting6_18, pc_2ed_019_00_case04,
								       &txn, 1);
	ck_assert(rc == EMV_RC_OK);
}
END_TEST

Suite *emvco_ep_ta_test_suite(void)
{
	Suite *suite = NULL;
	TCase *tc_general_reqs = NULL, *tc_pre_processing = NULL;
	TCase *tc_protocol_activation = NULL;
	TCase *tc_aid_and_kernel_selection = NULL;
	TCase *tc_kernel_activation = NULL, *tc_outcome_processing = NULL;

	suite = suite_create("EMVCo Type Approval - Book A & Book B - Test "
							"Cases - Version 2.4a");

	tc_general_reqs = tcase_create("General Requirements");
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
	tcase_add_test(tc_general_reqs, test_2EA_013_00);
	tcase_add_test(tc_general_reqs, test_2EA_013_01);
	tcase_add_test(tc_general_reqs, test_2EA_014_00);
	tcase_add_test(tc_general_reqs, test_2EA_014_01);
	tcase_add_test(tc_general_reqs, test_2EA_015_00);
	tcase_add_test(tc_general_reqs, test_2EA_016_00);
	tcase_add_test(tc_general_reqs, test_2EA_017_00);
	tcase_add_test(tc_general_reqs, test_2EA_017_01);
	tcase_add_test(tc_general_reqs, test_2EA_018_00);
	tcase_add_test(tc_general_reqs, test_2EA_019_00);
	tcase_add_test(tc_general_reqs, test_2EA_020_00);
	tcase_add_test(tc_general_reqs, test_2EA_021_00);
	suite_add_tcase(suite, tc_general_reqs);

	tc_pre_processing = tcase_create("Pre-processing");
	tcase_add_test(tc_pre_processing, test_2EB_001_00);
	tcase_add_test(tc_pre_processing, test_2EB_002_00);
	tcase_add_test(tc_pre_processing, test_2EB_003_00);
	tcase_add_test(tc_pre_processing, test_2EB_003_01);
	tcase_add_test(tc_pre_processing, test_2EB_003_02);
	tcase_add_test(tc_pre_processing, test_2EB_004_00);
	tcase_add_test(tc_pre_processing, test_2EB_004_01);
	tcase_add_test(tc_pre_processing, test_2EB_004_02);
	tcase_add_test(tc_pre_processing, test_2EB_005_00);
	tcase_add_test(tc_pre_processing, test_2EB_005_01);
	tcase_add_test(tc_pre_processing, test_2EB_006_00);
	tcase_add_test(tc_pre_processing, test_2EB_006_01);
	tcase_add_test(tc_pre_processing, test_2EB_007_00);
	tcase_add_test(tc_pre_processing, test_2EB_007_01);
	tcase_add_test(tc_pre_processing, test_2EB_008_00);
	tcase_add_test(tc_pre_processing, test_2EB_008_01);
	tcase_add_test(tc_pre_processing, test_2EB_008_02);
	tcase_add_test(tc_pre_processing, test_2EB_009_00);
	tcase_add_test(tc_pre_processing, test_2EB_009_01);
	tcase_add_test(tc_pre_processing, test_2EB_009_02);
	tcase_add_test(tc_pre_processing, test_2EB_010_00);
	tcase_add_test(tc_pre_processing, test_2EB_010_01);
	tcase_add_test(tc_pre_processing, test_2EB_011_00);
	tcase_add_test(tc_pre_processing, test_2EB_011_01);
	tcase_add_test(tc_pre_processing, test_2EB_011_02);
	tcase_add_test(tc_pre_processing, test_2EB_012_00);
	tcase_add_test(tc_pre_processing, test_2EB_012_01);
	tcase_add_test(tc_pre_processing, test_2EB_013_00);
	tcase_add_test(tc_pre_processing, test_2EB_013_01);
	tcase_add_test(tc_pre_processing, test_2EB_013_02);
	tcase_add_test(tc_pre_processing, test_2EB_014_00);
	tcase_add_test(tc_pre_processing, test_2EB_014_01);
	tcase_add_test(tc_pre_processing, test_2EB_014_02);
	tcase_add_test(tc_pre_processing, test_2EB_015_00);
	tcase_add_test(tc_pre_processing, test_2EB_015_01);
	tcase_add_test(tc_pre_processing, test_2EB_015_02);
	tcase_add_test(tc_pre_processing, test_2EB_016_00);
	tcase_add_test(tc_pre_processing, test_2EB_016_01);
	tcase_add_test(tc_pre_processing, test_2EB_016_02);
	tcase_add_test(tc_pre_processing, test_2EB_017_00);
	tcase_add_test(tc_pre_processing, test_2EB_017_01);
	tcase_add_test(tc_pre_processing, test_2EB_017_02);
	tcase_add_test(tc_pre_processing, test_2EB_018_00);
	tcase_add_test(tc_pre_processing, test_2EB_018_01);
	tcase_add_test(tc_pre_processing, test_2EB_018_02);
	tcase_add_test(tc_pre_processing, test_2EB_019_00);
	tcase_add_test(tc_pre_processing, test_2EB_019_01);
	tcase_add_test(tc_pre_processing, test_2EB_019_02);
	tcase_add_test(tc_pre_processing, test_2EB_020_00);
	tcase_add_test(tc_pre_processing, test_2EB_020_01);
	tcase_add_test(tc_pre_processing, test_2EB_021_00);
	tcase_add_test(tc_pre_processing, test_2EB_022_00);
	suite_add_tcase(suite, tc_pre_processing);

	tc_protocol_activation = tcase_create("Protocol Activation");
	tcase_add_test(tc_protocol_activation, test_2EC_001_00);
	tcase_add_test(tc_protocol_activation, test_2EC_001_01);
	tcase_add_test(tc_protocol_activation, test_2EC_001_02);
	tcase_add_test(tc_protocol_activation, test_2EC_001_03);
	tcase_add_test(tc_protocol_activation, test_2EC_001_04);
	tcase_add_test(tc_protocol_activation, test_2EC_002_00);
	tcase_add_test(tc_protocol_activation, test_2EC_003_00);
	tcase_add_test(tc_protocol_activation, test_2EC_004_00);
	tcase_add_test(tc_protocol_activation, test_2EC_005_00);
	tcase_add_test(tc_protocol_activation, test_2EC_006_00);
	tcase_add_test(tc_protocol_activation, test_2EC_007_00);
	suite_add_tcase(suite, tc_protocol_activation);

	tc_aid_and_kernel_selection = tcase_create("AID and Kernel Selection");
	tcase_add_test(tc_aid_and_kernel_selection, test_2ED_001_00);
	tcase_add_test(tc_aid_and_kernel_selection, test_2ED_001_01);
	tcase_add_test(tc_aid_and_kernel_selection, test_2ED_002_00);
	tcase_add_test(tc_aid_and_kernel_selection, test_2ED_002_01);
	tcase_add_test(tc_aid_and_kernel_selection, test_2ED_003_00);
	tcase_add_test(tc_aid_and_kernel_selection, test_2ED_003_01);
	tcase_add_test(tc_aid_and_kernel_selection, test_2ED_004_00);
	tcase_add_test(tc_aid_and_kernel_selection, test_2ED_005_00);
	tcase_add_test(tc_aid_and_kernel_selection, test_2ED_007_00);
	tcase_add_test(tc_aid_and_kernel_selection, test_2ED_007_01);
	tcase_add_test(tc_aid_and_kernel_selection, test_2ED_008_00);
	tcase_add_test(tc_aid_and_kernel_selection, test_2ED_009_00);
	tcase_add_test(tc_aid_and_kernel_selection, test_2ED_009_02);
	tcase_add_test(tc_aid_and_kernel_selection, test_2ED_009_04);
	tcase_add_test(tc_aid_and_kernel_selection, test_2ED_009_06);
	tcase_add_test(tc_aid_and_kernel_selection, test_2ED_009_10);
	tcase_add_test(tc_aid_and_kernel_selection, test_2ED_009_12);
	tcase_add_test(tc_aid_and_kernel_selection, test_2ED_009_14);
	tcase_add_test(tc_aid_and_kernel_selection, test_2ED_009_16);
	tcase_add_test(tc_aid_and_kernel_selection, test_2ED_009_18);
	tcase_add_test(tc_aid_and_kernel_selection, test_2ED_009_20);
	tcase_add_test(tc_aid_and_kernel_selection, test_2ED_009_22);
	tcase_add_test(tc_aid_and_kernel_selection, test_2ED_009_24);
	tcase_add_test(tc_aid_and_kernel_selection, test_2ED_009_26);
	tcase_add_test(tc_aid_and_kernel_selection, test_2ED_009_28);
	tcase_add_test(tc_aid_and_kernel_selection, test_2ED_010_00);
	tcase_add_test(tc_aid_and_kernel_selection, test_2ED_011_00);
	tcase_add_test(tc_aid_and_kernel_selection, test_2ED_012_00);
	tcase_add_test(tc_aid_and_kernel_selection, test_2ED_012_02);
	tcase_add_test(tc_aid_and_kernel_selection, test_2ED_012_04);
	tcase_add_test(tc_aid_and_kernel_selection, test_2ED_012_06);
	tcase_add_test(tc_aid_and_kernel_selection, test_2ED_012_08);
	tcase_add_test(tc_aid_and_kernel_selection, test_2ED_012_10);
	tcase_add_test(tc_aid_and_kernel_selection, test_2ED_012_11);
	tcase_add_test(tc_aid_and_kernel_selection, test_2ED_012_13);
	tcase_add_test(tc_aid_and_kernel_selection, test_2ED_012_15);
	tcase_add_test(tc_aid_and_kernel_selection, test_2ED_013_00);
	tcase_add_test(tc_aid_and_kernel_selection, test_2ED_014_00);
	tcase_add_test(tc_aid_and_kernel_selection, test_2ED_015_00);
	tcase_add_test(tc_aid_and_kernel_selection, test_2ED_016_00);
	tcase_add_test(tc_aid_and_kernel_selection, test_2ED_017_00);
	tcase_add_test(tc_aid_and_kernel_selection, test_2ED_018_00);
	tcase_add_test(tc_aid_and_kernel_selection, test_2ED_019_00);
	suite_add_tcase(suite, tc_aid_and_kernel_selection);

	tc_kernel_activation = tcase_create("Kernel Activation");
	suite_add_tcase(suite, tc_kernel_activation);

	tc_outcome_processing = tcase_create("Outcome Processing");
	suite_add_tcase(suite, tc_outcome_processing);

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

	libtlv_init(log4c_category);
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
