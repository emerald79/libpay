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

#include <arpa/inet.h>
#include <log4c.h>
#include <assert.h>
#include <string.h>

#include "emvco_ep_ta.h"

struct tk {
	const struct emv_kernel_ops *ops;
	log4c_category_t *log_cat;
};

static int tk_configure(struct emv_kernel *kernel, const void *config,
								     size_t len)
{
	struct tk *tk = (struct tk *)kernel;

	log4c_category_log(tk->log_cat,
			       LOG4C_PRIORITY_TRACE, "%s(): success", __func__);
	return EMV_RC_OK;
}

static struct tlv *tlv_kernel_parms(struct tk *tk,
						 struct emv_kernel_parms *parms)
{
	const struct emv_ep_preproc_indicators *ind = parms->preproc_indicators;
	struct tlv *tlv_kernel_parms = NULL, *tlv = NULL, *tlv_term_data = NULL;
	uint8_t amount_authorized[6], amount_other[6], txn_type, start;
	uint8_t test_flags[2] = { 0, 0 };
	uint32_t un = ntohl(parms->unpredictable_number);
	int rc = TLV_RC_OK;

	switch (parms->start) {
	case start_a:
		start = 0xA;
		break;
	case start_b:
		start = 0xB;
		break;
	case start_c:
		start = 0xC;
		break;
	case start_d:
		start = 0xD;
		break;
	default:
		rc = TLV_RC_INVALID_ARG;
		goto done;
	}

	if (ind->floor_limit_exceeded)
		test_flags[0] |= 0x80u;

	if (ind->cvm_reqd_limit_exceeded)
		test_flags[0] |= 0x20u;

	if (ind->status_check_requested)
		test_flags[0] |= 0x10u;

	if (ind->zero_amount)
		test_flags[0] |= 0x08u;

	if (ind->ctls_app_not_allowed)
		test_flags[0] |= 0x40u;

	if (parms->restart)
		test_flags[1] |= 0x80u;

	switch (parms->txn->type) {
	case txn_purchase:
		if (parms->txn->amount_other)
			txn_type = 0x09;
		else
			txn_type = 0x00;
		break;
	case txn_purchase_with_cashback:
		txn_type = 0x09;
		break;
	case txn_cash_advance:
		txn_type = 0x01;
		break;
	case txn_refund:
		txn_type = 0x20;
		break;
	default:
		rc = TLV_RC_INVALID_ARG;
		goto done;
	}

	rc = libtlv_u64_to_bcd(parms->txn->amount_authorized, amount_authorized,
						     sizeof(amount_authorized));
	if (rc != TLV_RC_OK)
		goto done;

	rc = libtlv_u64_to_bcd(parms->txn->amount_other, amount_other,
							  sizeof(amount_other));
	if (rc != TLV_RC_OK)
		goto done;

	tlv = tlv_kernel_parms =
		     tlv_new(EMV_ID_TEST_FLAGS, sizeof(test_flags), test_flags);
	tlv = tlv_insert_after(tlv, tlv_new(
					 EMV_ID_TERMINAL_TRANSACTION_QUALIFIERS,
						   sizeof(ind->ttq), ind->ttq));
	tlv = tlv_insert_after(tlv, tlv_new(EMV_ID_KERNEL_IDENTIFIER,
				       parms->kernel_id_len, parms->kernel_id));
	tlv = tlv_insert_after(tlv, tlv_new(EMV_ID_AMOUNT_AUTHORIZED,
				 sizeof(amount_authorized), amount_authorized));
	tlv = tlv_insert_after(tlv, tlv_new(EMV_ID_AMOUNT_OTHER,
					   sizeof(amount_other), amount_other));
	tlv = tlv_insert_after(tlv, tlv_new(EMV_ID_TRANSACTION_TYPE,
						  sizeof(txn_type), &txn_type));
	tlv = tlv_insert_after(tlv, tlv_new(EMV_ID_UNPREDICTABLE_NUMBER,
							      sizeof(un), &un));
	tlv = tlv_insert_after(tlv, tlv_new(EMV_ID_START_POINT,
							sizeof(start), &start));
	tlv = tlv_insert_after(tlv, tlv_new(EMV_ID_SELECT_RESPONSE_SW,
						 sizeof(parms->sw), parms->sw));
	if (parms->fci_len) {
		struct tlv *tlv_fci = NULL;

		rc = tlv_parse(parms->fci, parms->fci_len, &tlv_fci);
		if (rc != TLV_RC_OK)
			goto done;

		tlv = tlv_insert_after(tlv, tlv_fci);
	}

	rc = tlv_parse(parms->terminal_data, parms->terminal_data_len,
								&tlv_term_data);
	if (rc != TLV_RC_OK)
		goto done;

	tlv = tlv_insert_after(tlv, tlv_term_data);

	if (parms->online_response && parms->online_response_len) {
		struct tlv *tlv_online_response = NULL;
		char hex[parms->online_response_len * 2 + 1];

		log4c_category_log(tk->log_cat,
			       LOG4C_PRIORITY_TRACE, "%s() online resp '%s'",
			  __func__, libtlv_bin_to_hex(parms->online_response,
					   parms->online_response_len, hex));

		rc = tlv_shallow_parse(parms->online_response,
			      parms->online_response_len, &tlv_online_response);
		if (rc != TLV_RC_OK)
			goto done;

		tlv = tlv_insert_after(tlv, tlv_online_response);
	}

	if (!tlv) {
		rc = TLV_RC_OUT_OF_MEMORY;
		goto done;
	}

done:
	if (rc != TLV_RC_OK) {
		if (tlv_kernel_parms) {
			tlv_free(tlv_kernel_parms);
			tlv_kernel_parms = NULL;
		}
	}

	return tlv_kernel_parms;
}

static void gpo_outcome_to_outcome(const struct outcome_gpo_resp *in,
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

static void gpo_ui_req_to_ui_req(const struct ui_req_gpo_resp *in,
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

static int tk_activate(struct emv_kernel *kernel, struct emv_hal *hal,
	      struct emv_kernel_parms *parms, struct emv_outcome_parms *outcome)
{
	struct tk *tk = (struct tk *)kernel;
	struct tlv *tlv_fci = NULL, *tlv = NULL, *tlv_parms = NULL;
	struct tlv *tlv_resp = NULL, *tlv_data_record = NULL;
	struct tlv *tlv_resp_msg = NULL;
	uint8_t pdol[256], gpo_data[256], gpo_resp[256], sw[2];
	uint8_t resp_msg[256];
	char hex[513];
	size_t pdol_sz = sizeof(pdol), gpo_data_sz = sizeof(gpo_data);
	size_t gpo_resp_sz = sizeof(gpo_resp);
	size_t resp_msg_sz = sizeof(resp_msg);
	int rc = EMV_RC_OK;

	rc = tlv_parse(parms->fci, parms->fci_len, &tlv_fci);
	if (rc != TLV_RC_OK) {
		rc = EMV_RC_CARD_PROTOCOL_ERROR;
		goto done;
	}

	tlv = tlv_find(tlv_get_child(tlv_find(tlv_get_child(tlv_find(tlv_fci,
		       EMV_ID_FCI_TEMPLATE)), EMV_ID_FCI_PROPRIETARY_TEMPLATE)),
								   EMV_ID_PDOL);
	if (tlv) {
		rc = tlv_encode_value(tlv, pdol, &pdol_sz);
		if (rc != TLV_RC_OK) {
			rc = EMV_RC_CARD_PROTOCOL_ERROR;
			goto done;
		}

		log4c_category_log(tk->log_cat, LOG4C_PRIORITY_TRACE,
						    "%s(): PDOL='%s'", __func__,
					 libtlv_bin_to_hex(pdol, pdol_sz, hex));
	} else {
		pdol_sz = 0;
	}

	tlv_parms = tlv_kernel_parms(tk, parms);
	if (!tlv_parms) {
		rc = EMV_RC_SYNTAX_ERROR;
		goto done;
	}

	rc = tlv_and_dol_to_del(tlv_parms, pdol, pdol_sz, gpo_data,
								  &gpo_data_sz);
	if (rc != TLV_RC_OK) {
		rc = EMV_RC_CARD_PROTOCOL_ERROR;
		goto done;
	}

	log4c_category_log(tk->log_cat, LOG4C_PRIORITY_TRACE,
					       "%s(): GPO DATA ='%s'", __func__,
				 libtlv_bin_to_hex(gpo_data, gpo_data_sz, hex));

	rc = emv_transceive_apdu(hal, EMV_CMD_GPO_CLA, EMV_CMD_GPO_INS,
			EMV_CMD_P1_NONE, EMV_CMD_P2_NONE, gpo_data, gpo_data_sz,
						    gpo_resp, &gpo_resp_sz, sw);
	if (rc != TLV_RC_OK) {
		rc = EMV_RC_CARD_PROTOCOL_ERROR;
		goto done;
	}

	log4c_category_log(tk->log_cat, LOG4C_PRIORITY_TRACE,
			    "%s(): GPO RESP = '%s' SW: %02hhX%02hhX", __func__,
		  libtlv_bin_to_hex(gpo_resp, gpo_resp_sz, hex), sw[0], sw[1]);

	rc = tlv_shallow_parse(gpo_resp, gpo_resp_sz, &tlv_resp);
	if (rc != TLV_RC_OK) {
		log4c_category_log(tk->log_cat, LOG4C_PRIORITY_ERROR,
				"%s(): Failed to parse GPO response", __func__);
		rc = EMV_RC_CARD_PROTOCOL_ERROR;
		goto done;
	}

	rc = tlv_encode_value(tlv_find(tlv_resp,
		       EMV_ID_RESP_MSG_TEMPLATE_FMT_2), resp_msg, &resp_msg_sz);
	if (rc != TLV_RC_OK) {
		log4c_category_log(tk->log_cat, LOG4C_PRIORITY_ERROR,
				       "%s() no response message found. rc: %d",
							     __func__, (int)rc);
		rc = EMV_RC_CARD_PROTOCOL_ERROR;
		goto done;
	}

	rc = tlv_shallow_parse(resp_msg, resp_msg_sz, &tlv_resp_msg);
	if (rc != TLV_RC_OK) {
		log4c_category_log(tk->log_cat, LOG4C_PRIORITY_ERROR,
				"%s() failed to parse response message. rc: %d",
							     __func__, (int)rc);
		rc = EMV_RC_CARD_PROTOCOL_ERROR;
		goto done;
	}

	tlv = tlv_find(tlv_resp_msg, EMV_ID_OUTCOME_DATA);
	if (tlv) {
		struct outcome_gpo_resp gpo_outcome;
		size_t gpo_outcome_sz = sizeof(gpo_outcome);

		rc = tlv_encode_value(tlv, &gpo_outcome, &gpo_outcome_sz);
		if (rc != EMV_RC_OK)
			goto done;

		gpo_outcome_to_outcome(&gpo_outcome, outcome);
	}

	tlv = tlv_find(tlv_resp_msg, EMV_ID_UI_REQ_ON_OUTCOME);
	if (tlv) {
		struct ui_req_gpo_resp gpo_ui_req;
		size_t gpo_ui_req_sz = sizeof(gpo_ui_req);

		log4c_category_log(tk->log_cat, LOG4C_PRIORITY_TRACE,
			       "%s(): UI Request on Outcome present", __func__);

		rc = tlv_encode_value(tlv, &gpo_ui_req, &gpo_ui_req_sz);
		if (rc != EMV_RC_OK)
			goto done;

		gpo_ui_req_to_ui_req(&gpo_ui_req,
					       &outcome->ui_request_on_outcome);
		outcome->present.ui_request_on_outcome = true;
	}

	tlv = tlv_find(tlv_resp_msg, EMV_ID_UI_REQ_ON_RESTART);
	if (tlv) {
		struct ui_req_gpo_resp gpo_ui_req;
		size_t gpo_ui_req_sz = sizeof(gpo_ui_req);

		log4c_category_log(tk->log_cat, LOG4C_PRIORITY_TRACE,
			       "%s(): UI Request on Restart present", __func__);

		rc = tlv_encode_value(tlv, &gpo_ui_req, &gpo_ui_req_sz);
		if (rc != EMV_RC_OK)
			goto done;

		gpo_ui_req_to_ui_req(&gpo_ui_req,
					       &outcome->ui_request_on_restart);
		outcome->present.ui_request_on_restart = true;
	}

	tlv = tlv_copy(tlv_find(tlv_resp_msg,
					    EMV_ID_ISSUER_AUTHENTICATION_DATA));
	if (!tlv_data_record)
		tlv_data_record = tlv;

	tlv = tlv_insert_after(tlv, tlv_copy(tlv_find(tlv_resp_msg,
					     EMV_ID_ISSUER_SCRIPT_TEMPLATE_1)));
	if (!tlv_data_record)
		tlv_data_record = tlv;

	tlv = tlv_insert_after(tlv, tlv_copy(tlv_find(tlv_resp_msg,
					     EMV_ID_ISSUER_SCRIPT_TEMPLATE_2)));
	if (!tlv_data_record)
		tlv_data_record = tlv;

	if (tlv_data_record) {
		outcome->data_record.len = sizeof(outcome->data_record.data);
		rc = tlv_encode(tlv_data_record, outcome->data_record.data,
						     &outcome->data_record.len);
		if (rc != EMV_RC_OK)
			goto done;
	}

done:
	tlv_free(tlv_resp);
	tlv_free(tlv_resp_msg);
	tlv_free(tlv_parms);
	tlv_free(tlv_fci);
	tlv_free(tlv_data_record);

	if (rc == EMV_RC_OK) {
		log4c_category_log(tk->log_cat, LOG4C_PRIORITY_TRACE,
						     "%s(): success", __func__);
	} else {
		log4c_category_log(tk->log_cat, LOG4C_PRIORITY_NOTICE,
					     "%s(): fail. rc %d", __func__, rc);
	}

	return rc;
}

const struct emv_kernel_ops tk_ops = {
	.configure = tk_configure,
	.activate  = tk_activate
};

struct emv_kernel *tk_new(const char *log4c_category)
{
	struct tk *tk = NULL;
	char cat[64];

	tk = malloc(sizeof(struct tk));
	if (!tk)
		return NULL;

	tk->ops = &tk_ops;

	snprintf(cat, sizeof(cat), "%s.tk", log4c_category);
	tk->log_cat = log4c_category_get(cat);

	return (struct emv_kernel *)tk;
}

void tk_free(struct emv_kernel *tk)
{
	if (tk)
		free(tk);
}
