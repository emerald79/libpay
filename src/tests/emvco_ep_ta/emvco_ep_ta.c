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

static const char log4c_category[] = "emvco_ep_ta";
static log4c_category_t *log_cat;
static const struct tlv_id_to_fmt id_fmts[] = {
	{ .id = EMV_ID_TEST_FLAGS, .fmt = fmt_b },
	{ .id = NULL				}
};

void gpo_outcome_to_outcome(const struct outcome_gpo_resp *in,
						  struct emv_outcome_parms *out)
{
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
	out->msg_id	     = (enum emv_message_identifier)in->msg_id;
	out->status	     = (uint8_t)out->status;
	out->value_qualifier = (enum emv_value_qualifier)in->value_qual;
	memcpy(out->lang_pref, in->lang_pref, sizeof(in->lang_pref));
	memcpy(out->value, in->value, sizeof(in->value));
	memcpy(out->currency_code, in->currency_code,
						     sizeof(in->currency_code));
}

void ui_req_to_gpo_ui_req(const struct emv_ui_request *in,
						    struct ui_req_gpo_resp *out)
{
	out->status     = (uint8_t)in->status;
	out->hold_time  = htons(in->hold_time);
	out->value_qual = (uint8_t)in->value_qualifier;
	memcpy(out->lang_pref, in->lang_pref, sizeof(in->lang_pref));
	memcpy(out->value, in->value, sizeof(in->value));
	memcpy(out->currency_code, in->currency_code,
						     sizeof(in->currency_code));
}

static int emvco_ep_ta_tc(const char *name, enum termsetting termsetting,
				 enum ltsetting ltsetting, enum emv_start start,
				 const struct emv_txn *txn, struct chk *checker)
{
	uint8_t cfg[4096];
	size_t cfg_sz = sizeof(cfg);
	struct emv_ep *ep = NULL;
	struct emv_hal *lt = NULL;
	struct emv_kernel *tk[13];
	struct tk_id tk_id[13] = {
		{ KERNEL_ID_TK1 },
		{ KERNEL_ID_TK2 },
		{ KERNEL_ID_TK3 },
		{ KERNEL_ID_TK4 },
		{ KERNEL_ID_TK5 },
		{ KERNEL_ID_TK6 },
		{ KERNEL_ID_TK7 },
		{ KERNEL_ID_21  },
		{ KERNEL_ID_22  },
		{ KERNEL_ID_23  },
		{ KERNEL_ID_24  },
		{ KERNEL_ID_25  },
		{ KERNEL_ID_2B  }
	};
	size_t i_tk = 0;
	int rc = EMV_RC_OK;

	log4c_category_log(log_cat, LOG4C_PRIORITY_TRACE, "STARTING '%s'",
									  name);

	memset(tk, 0, sizeof(tk));

	rc = term_get_setting(termsetting, cfg, &cfg_sz);
	if (rc != EMV_RC_OK)
		goto done;

	ep = emv_ep_new(log4c_category);
	if (!ep) {
		rc = EMV_RC_OUT_OF_MEMORY;
		goto done;
	}

	lt = lt_new(ltsetting, checker, log4c_category);
	if (!lt) {
		rc = EMV_RC_OUT_OF_MEMORY;
		goto done;
	}

	rc = emv_ep_register_hal(ep, lt);
	if (rc != EMV_RC_OK)
		goto done;

	for (i_tk = 0; i_tk < ARRAY_SIZE(tk); i_tk++) {

		tk[i_tk] = tk_new(log4c_category);

		if (!tk[i_tk]) {
			rc = EMV_RC_OUT_OF_MEMORY;
			goto done;
		}

		rc = emv_ep_register_kernel(ep, tk[i_tk], tk_id[i_tk].kernel_id,
						     tk_id[i_tk].kernel_id_len);
		if (rc != EMV_RC_OK)
			goto done;
	}

	rc = emv_ep_configure(ep, cfg, cfg_sz);
	if (rc != EMV_RC_OK)
		goto done;

	rc = emv_ep_activate(ep, start, txn);
	if (rc != EMV_RC_OK)
		goto done;

	if (!checker->ops->pass_criteria_met(checker))
		rc = EMV_RC_FAIL;

done:
	for (i_tk = 0; i_tk < ARRAY_SIZE(tk); i_tk++)
		tk_free(tk[i_tk]);
	lt_free(lt);
	emv_ep_free(ep);

	if (rc == EMV_RC_OK) {
		log4c_category_log(log_cat, LOG4C_PRIORITY_INFO, "SUCCESS '%s'",
									  name);
	} else {
		log4c_category_log(log_cat, LOG4C_PRIORITY_NOTICE,
						 "FAIL '%s', rc: %d", name, rc);
	}

	return rc;
}

struct txn_checker {
	const struct chk_ops *ops;
	const struct emv_txn *txn;
	bool		      pass_criteria_met;
};

void txn_checker_check_gpo_data(struct chk *chk, struct tlv *gpo_data)
{
	struct txn_checker *txn_checker = (struct txn_checker *)chk;
	uint8_t bcd_amount[6], txn_type;
	size_t bcd_amount_sz = sizeof(bcd_amount);
	size_t txn_type_sz = sizeof(txn_type);
	struct tlv *tlv = NULL;
	uint64_t amount = 0;
	int rc = EMV_RC_OK;

	tlv = tlv_find(gpo_data, EMV_ID_AMOUNT_AUTHORIZED);
	if (!tlv) {
		log4c_category_log(log_cat, LOG4C_PRIORITY_NOTICE,
			      "EMV_ID_AMOUNT_AUTHORIZED not found in GPO data");
		txn_checker->pass_criteria_met = false;
	}

	bcd_amount_sz = sizeof(bcd_amount);
	rc = tlv_encode_value(tlv, bcd_amount, &bcd_amount_sz);
	if (rc != EMV_RC_OK) {
		log4c_category_log(log_cat, LOG4C_PRIORITY_NOTICE,
				  "Can not get EMV_ID_AMOUNT_AUTHORIZED value");
		txn_checker->pass_criteria_met = false;
	}

	libtlv_bcd_to_u64(bcd_amount, bcd_amount_sz, &amount);
	if (amount != txn_checker->txn->amount_authorized) {
		log4c_category_log(log_cat, LOG4C_PRIORITY_NOTICE,
			      "Incorrect 'Amount Authorized' passed to kernel! "
							"TXN: %lu, KERNEL: %lu",
				   txn_checker->txn->amount_authorized, amount);
		txn_checker->pass_criteria_met = false;
	}

	tlv = tlv_find(gpo_data, EMV_ID_AMOUNT_OTHER);
	if (!tlv) {
		log4c_category_log(log_cat, LOG4C_PRIORITY_NOTICE,
				   "EMV_ID_AMOUNT_OTHER not found in GPO data");
		txn_checker->pass_criteria_met = false;
	}

	bcd_amount_sz = sizeof(bcd_amount);
	rc = tlv_encode_value(tlv, bcd_amount, &bcd_amount_sz);
	if (rc != EMV_RC_OK) {
		log4c_category_log(log_cat, LOG4C_PRIORITY_NOTICE,
				       "Can not get EMV_ID_AMOUNT_OTHER value");
		txn_checker->pass_criteria_met = false;
	}

	libtlv_bcd_to_u64(bcd_amount, bcd_amount_sz, &amount);
	if (amount != txn_checker->txn->amount_other) {
		log4c_category_log(log_cat, LOG4C_PRIORITY_NOTICE,
				  "Incorrect 'Amount Other' passed to kernel! "
							"TXN: %lu, KERNEL: %lu",
					txn_checker->txn->amount_other, amount);
		txn_checker->pass_criteria_met = false;
	}

	tlv = tlv_find(gpo_data, EMV_ID_TRANSACTION_TYPE);
	if (!tlv) {
		log4c_category_log(log_cat, LOG4C_PRIORITY_NOTICE,
			       "EMV_ID_TRANSACTION_TYPE not found in GPO data");
		txn_checker->pass_criteria_met = false;
	}

	txn_type_sz = sizeof(txn_type);
	rc = tlv_encode_value(tlv, &txn_type, &txn_type_sz);
	if (rc != EMV_RC_OK) {
		log4c_category_log(log_cat, LOG4C_PRIORITY_NOTICE,
				   "Can not get EMV_ID_TRANSACTION_TYPE value");
		txn_checker->pass_criteria_met = false;
	}

	switch (txn_checker->txn->type) {
	case txn_purchase:
		if (txn_type != 0x00)
			txn_checker->pass_criteria_met = false;
		break;
	case txn_purchase_with_cashback:
		if (txn_type != 0x09)
			txn_checker->pass_criteria_met = false;
		break;
	case txn_cash_advance:
		if (txn_type != 0x01)
			txn_checker->pass_criteria_met = false;
		break;
	case txn_refund:
		if (txn_type != 0x20)
			txn_checker->pass_criteria_met = false;
		break;
	default:
		txn_checker->pass_criteria_met = false;
	}
}

bool txn_checker_pass_criteria_met(struct chk *chk)
{
	struct txn_checker *txn_checker = (struct txn_checker *)chk;

	if (txn_checker->pass_criteria_met) {
		log4c_category_log(log_cat, LOG4C_PRIORITY_TRACE,
							  "Pass Criteria met.");
	} else {
		log4c_category_log(log_cat, LOG4C_PRIORITY_NOTICE,
						      "Pass Criteria not met!");
	}

	return txn_checker->pass_criteria_met;
}

static const struct chk_ops txn_checker_ops = {
	.check_gpo_data	   = txn_checker_check_gpo_data,
	.pass_criteria_met = txn_checker_pass_criteria_met
};

struct chk *txn_checker_new(const struct emv_txn *txn)
{
	struct txn_checker *txn_checker = NULL;

	txn_checker = malloc(sizeof(struct txn_checker));
	if (!txn_checker)
		return (struct chk *)txn_checker;

	txn_checker->ops = &txn_checker_ops;
	txn_checker->txn = txn;
	txn_checker->pass_criteria_met = true;

	return (struct chk *)txn_checker;
};

void txn_checker_free(struct chk *chk)
{
	struct txn_checker *txn_checker = (struct txn_checker *)chk;

	free(txn_checker);
}

/* 2EA.001.00 Entry of Amount Authorzed					      */
START_TEST(test_2EA_001_00)
{
	struct emv_txn txn = {
		.amount_authorized = 10,
		.amount_other	   = 0,
		.currency	   = { 0, 0 }
	};
	struct chk *checker = NULL;
	int rc;

	txn.type = txn_purchase;
	checker = txn_checker_new(&txn);
	rc = emvco_ep_ta_tc("2EA.001.00 - Entry of Amount Authorized - Case01",
			    termsetting2, ltsetting1_1, start_a, &txn, checker);
	txn_checker_free(checker);
	ck_assert(rc == EMV_RC_OK);

	txn.type = txn_purchase_with_cashback;
	checker = txn_checker_new(&txn);
	rc = emvco_ep_ta_tc("2EA.001.00 - Entry of Amount Authorized - Case01",
			    termsetting2, ltsetting1_1, start_a, &txn, checker);
	txn_checker_free(checker);
	ck_assert(rc == EMV_RC_OK);

	txn.type = txn_cash_advance;
	checker = txn_checker_new(&txn);
	rc = emvco_ep_ta_tc("2EA.001.00 - Entry of Amount Authorized - Case01",
			    termsetting2, ltsetting1_1, start_a, &txn, checker);
	txn_checker_free(checker);
	ck_assert(rc == EMV_RC_OK);

	txn.type = txn_refund;
	checker = txn_checker_new(&txn);
	rc = emvco_ep_ta_tc("2EA.001.00 - Entry of Amount Authorized - Case01",
			    termsetting2, ltsetting1_1, start_a, &txn, checker);
	txn_checker_free(checker);
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
