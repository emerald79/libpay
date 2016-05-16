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

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <libpay/tlv.h>
#include <libpay/emv.h>
#include <libpay/test.h>

static const char log4c_category[] = "libemv_ep_wrapper";
static uint32_t transaction_sequence_counter;

/*-----------------------------------------------------------------------------+
| Helpers to convert Termsettings to the corresponding Entry Point Configs     |
+-----------------------------------------------------------------------------*/

static uint8_t get_txn_type(enum emv_txn_type txn_type)
{
	switch (txn_type) {
	case txn_purchase:
		return 0x00;
	case txn_purchase_with_cashback:
		return 0x09;
	case txn_cash_advance:
		return 0x01;
	case txn_refund:
		return 0x20;
	default:
		assert(false);
		return 0xff;
	}
}

static struct tlv *get_txn_types(enum emv_txn_type *txn_types)
{
	size_t len, i;
	uint8_t value[4];

	for (len = 0; len < num_txn_types; len++)
		if (txn_types[len] == num_txn_types)
			break;

	for (i = 0; i < len; i++)
		value[i] = get_txn_type(txn_types[i]);

	return tlv_new(EMV_ID_LIBEMV_TRANSACTION_TYPES, len, value);
}

static struct tlv *get_combinations(struct emv_ep_aid_kernel *aid_kernel)
{
	struct tlv *tlv_combinations = NULL;

	for (; aid_kernel->aid_len; aid_kernel++) {
		struct tlv *tlv = NULL, *tail = NULL;

		tlv = tlv_new(EMV_ID_LIBEMV_COMBINATION, 0, NULL);
		tail = tlv_insert_below(tlv, tlv_new(EMV_ID_LIBEMV_AID,
					 aid_kernel->aid_len, aid_kernel->aid));
		tail = tlv_insert_after(tail, tlv_new(EMV_ID_LIBEMV_KERNEL_ID,
			     aid_kernel->kernel_id_len, aid_kernel->kernel_id));

		if (!tlv_combinations)
			tlv_combinations = tlv;
		else
			tlv_insert_after(tlv_combinations, tlv);
	}

	return tlv_combinations;
}

static struct tlv *get_combination_set(struct emv_ep_combination *comb)
{
	struct tlv *tlv = NULL, *tail = NULL;

	tlv = tlv_new(EMV_ID_LIBEMV_COMBINATION_SET, 0, NULL);

	tail = tlv_insert_below(tlv, get_txn_types(comb->txn_types));

	tail = tlv_insert_after(tail, get_combinations(comb->combinations));

	while (tlv_get_next(tail))
		tail = tlv_get_next(tail);

	if (comb->config.present.status_check_support) {
		uint8_t enabled = comb->config.enabled.status_check_support;

		tail = tlv_insert_after(tail, tlv_new(
			    EMV_ID_LIBEMV_STATUS_CHECK_SUPPORTED, 1, &enabled));
	}

	if (comb->config.present.zero_amount_allowed) {
		uint8_t enabled = comb->config.enabled.zero_amount_allowed;

		tail = tlv_insert_after(tail, tlv_new(
			       EMV_ID_LIBEMV_ZERO_AMOUNT_ALLOWED, 1, &enabled));
	}

	if (comb->config.present.ext_selection_support) {
		uint8_t enabled = comb->config.enabled.ext_selection_support;

		tail = tlv_insert_after(tail, tlv_new(
			   EMV_ID_LIBEMV_EXT_SELECTION_SUPPORTED, 1, &enabled));
	}

	if (comb->config.present.reader_ctls_txn_limit) {
		uint8_t amount[6];
		int rc;

		rc = libtlv_u64_to_bcd(comb->config.reader_ctls_txn_limit,
							amount, sizeof(amount));
		if (rc != EMV_RC_OK)
			goto error;

		tail = tlv_insert_after(tail, tlv_new(
					       EMV_ID_LIBEMV_RDR_CTLS_TXN_LIMIT,
						       sizeof(amount), amount));
	}

	if (comb->config.present.reader_ctls_floor_limit) {
		uint8_t amount[6];
		int rc;

		rc = libtlv_u64_to_bcd(comb->config.reader_ctls_floor_limit,
							amount, sizeof(amount));
		if (rc != EMV_RC_OK)
			goto error;

		tail = tlv_insert_after(tail, tlv_new(
					     EMV_ID_LIBEMV_RDR_CTLS_FLOOR_LIMIT,
						       sizeof(amount), amount));
	}

	if (comb->config.present.terminal_floor_limit) {
		uint8_t amount[6];
		int rc;

		rc = libtlv_u64_to_bcd(comb->config.terminal_floor_limit,
							amount, sizeof(amount));
		if (rc != EMV_RC_OK)
			goto error;

		tail = tlv_insert_after(tail, tlv_new(
					     EMV_ID_LIBEMV_TERMINAL_FLOOR_LIMIT,
						       sizeof(amount), amount));
	}

	if (comb->config.present.reader_cvm_reqd_limit) {
		uint8_t amount[6];
		int rc;

		rc = libtlv_u64_to_bcd(comb->config.reader_cvm_reqd_limit,
							amount, sizeof(amount));
		if (rc != EMV_RC_OK)
			goto error;

		tail = tlv_insert_after(tail, tlv_new(
					   EMV_ID_LIBEMV_RDR_CVM_REQUIRED_LIMIT,
						       sizeof(amount), amount));
	}

	if (comb->config.present.ttq)
		tail = tlv_insert_after(tail, tlv_new(EMV_ID_LIBEMV_TTQ,
				   sizeof(comb->config.ttq), comb->config.ttq));

	return tlv;

error:
	if (tlv)
		tlv_free(tlv);

	return NULL;
}

static struct tlv *get_autorun(const struct emv_ep_autorun *autorun)
{
	struct tlv *tlv = NULL, *tail = NULL;
	uint8_t amount[6], txn_type = get_txn_type(autorun->txn_type);
	int rc = EMV_RC_OK;

	tlv = tlv_new(EMV_ID_LIBEMV_AUTORUN, 0, NULL);

	rc = libtlv_u64_to_bcd(autorun->amount_authorized, amount,
								sizeof(amount));
	if (rc != EMV_RC_OK)
		goto error;

	tail = tlv_insert_below(tlv,
				tlv_new(EMV_ID_LIBEMV_AUTORUN_AMOUNT_AUTHORIZED,
						       sizeof(amount), amount));
	tail = tlv_insert_after(tail,
				 tlv_new(EMV_ID_LIBEMV_AUTORUN_TRANSACTION_TYPE,
						  sizeof(txn_type), &txn_type));
	if (!tail)
		goto error;

	return tlv;

error:
	tlv_free(tlv);
	return NULL;
}

static struct tlv *get_terminal_data(struct emv_ep_terminal_data *data)
{
	struct tlv *term_data = NULL, *tail = NULL;

	term_data = tlv_new(EMV_ID_LIBEMV_TERMINAL_DATA, 0, NULL);

	tail = tlv_insert_below(term_data, tlv_new(EMV_ID_ACQUIRER_IDENTIFIER,
		 sizeof(data->acquirer_identifier), data->acquirer_identifier));
	tail = tlv_insert_after(tail, tlv_new(EMV_ID_MERCHANT_CATEGORY_CODE,
					   sizeof(data->merchant_category_code),
						 data->merchant_category_code));
	tail = tlv_insert_after(tail, tlv_new(EMV_ID_MERCHANT_IDENTIFIER,
		 strlen(data->merchant_identifier), data->merchant_identifier));
	tail = tlv_insert_after(tail, tlv_new(EMV_ID_TERMINAL_COUNTRY_CODE,
					    sizeof(data->terminal_country_code),
						  data->terminal_country_code));
	tail = tlv_insert_after(tail, tlv_new(EMV_ID_TERMINAL_IDENTIFICATION,
					  sizeof(data->terminal_identification),
						data->terminal_identification));
	tail = tlv_insert_after(tail, tlv_new(EMV_ID_TERMINAL_TYPE,
			    sizeof(data->terminal_type), &data->terminal_type));
	tail = tlv_insert_after(tail, tlv_new(EMV_ID_POS_ENTRY_MODE,
			  sizeof(data->pos_entry_mode), &data->pos_entry_mode));
	tail = tlv_insert_after(tail,
				tlv_new(EMV_ID_ADDITIONAL_TERMINAL_CAPABILITIES,
				 sizeof(data->additional_terminal_capabilities),
				       data->additional_terminal_capabilities));
	tail = tlv_insert_after(tail, tlv_new(EMV_ID_TERMINAL_CAPABILITIES,
					    sizeof(data->terminal_capabilities),
						  data->terminal_capabilities));
	tail = tlv_insert_after(tail, tlv_new(EMV_ID_MERCHANT_NAME_AND_LOCATION,
				       strlen(data->merchant_name_and_location),
					     data->merchant_name_and_location));
	if (!tail) {
		tlv_free(term_data);
		term_data = NULL;
	}

	return term_data;
};

static int get_termsetting(const struct emv_ep_terminal_settings *settings,
						     void *buffer, size_t *size)
{
	struct tlv *tlv = NULL, *tail = NULL;
	int i = 0, rc = TLV_RC_OK;

	tlv = tlv_new(EMV_ID_LIBEMV_CONFIGURATION, 0, NULL);
	if (!tlv) {
		rc = TLV_RC_OUT_OF_MEMORY;
		goto done;
	}

	tail = tlv_insert_below(tlv,
			   get_combination_set(&settings->combination_sets[0]));

	for (i = 1; i < settings->num_combination_sets; i++)
		tail = tlv_insert_after(tail,
			   get_combination_set(&settings->combination_sets[i]));

	if (settings->autorun.enabled)
		tail = tlv_insert_after(tail, get_autorun(&settings->autorun));

	if (settings->terminal_data)
		tail = tlv_insert_after(tail,
				    get_terminal_data(settings->terminal_data));
	if (!tail) {
		rc = TLV_RC_OUT_OF_MEMORY;
		goto done;
	}

	rc = tlv_encode(tlv, buffer, size);

done:
	tlv_free(tlv);

	return rc;
}

/*-----------------------------------------------------------------------------+
| libemv_ep_wrapper - Test suite wrapper for LibPAY's entry point	       |
+-----------------------------------------------------------------------------*/

struct libemv_ep_wrapper {
	struct emv_ep_wrapper	 base;
	struct emv_ep		*ep;
	struct emv_chk		*chk;
	struct emv_hal		*lt;
};

static int libemv_ep_wrapper_setup(struct libemv_ep_wrapper *self,
					struct emv_hal *lt, struct emv_chk *chk,
				 const struct emv_ep_terminal_settings *termset)
{
	uint8_t cfg[8192];
	size_t cfg_sz = sizeof(cfg);
	int rc = EMV_RC_OK;

	rc = get_termsetting(termset, (void *)cfg, &cfg_sz);
	if (EMV_RC_OK != rc)
		goto done;

	rc = emv_ep_configure(self->ep, cfg, cfg_sz);
	if (EMV_RC_OK != rc)
		goto done;

	rc = emv_ep_register_hal(self->ep, lt);
	if (EMV_RC_OK != rc)
		goto done;

	self->lt  = lt;
	self->chk = chk;

done:
	return rc;
}

static int libemv_ep_wrapper_register_kernel(struct libemv_ep_wrapper *self,
			    struct emv_kernel *kernel, const uint8_t *kernel_id,
			     size_t kernel_id_len, const uint8_t app_ver_num[2])
{
	return emv_ep_register_kernel(self->ep, kernel, kernel_id,
						    kernel_id_len, app_ver_num);
}

static int libemv_ep_wrapper_activate(struct libemv_ep_wrapper *self,
						      const struct emv_txn *txn)
{
	const struct emv_autorun *autorun = emv_ep_get_autorun(self->ep);
	enum emv_start start_at = autorun->enabled ? start_b : start_a;
	struct emv_outcome_parms outcome;
	int rc = EMV_RC_OK;

	if (autorun->enabled) {
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

		rc = emv_ep_field_on(self->ep);
		if (rc != EMV_RC_OK)
			goto done;

		rc = emv_ep_ui_request(self->ep, &present_card);
		if (rc != EMV_RC_OK)
			goto done;

		txn = &autorun->txn;
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

		rc = emv_ep_field_off(self->ep, 0);
		if (rc != EMV_RC_OK)
			goto done;

		rc = emv_ep_ui_request(self->ep, &welcome);
		if (rc != EMV_RC_OK)
			goto done;
	}

	emv_chk_start(self->chk);

	rc = emv_ep_activate(self->ep, start_at, txn,
					++transaction_sequence_counter, NULL, 0,
								      &outcome);
	if (rc != EMV_RC_OK)
		goto done;

	emv_chk_outcome(self->chk, &outcome);

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

			rc = emv_ep_ui_request(self->ep, &rm_card);
			if (rc != EMV_RC_OK)
				goto done;
		}

		if ((outcome.online_response_type != ort_emv_data) ||
		    (outcome.data_record.len)) {

			emv_chk_restart(self->chk);

			rc = emv_ep_activate(self->ep, outcome.start, txn,
						   transaction_sequence_counter,
						       outcome.data_record.data,
					     outcome.data_record.len, &outcome);
			if (rc != EMV_RC_OK)
				goto done;

			emv_chk_outcome(self->chk, &outcome);
		}
	}

	rc = emv_ep_field_off(self->ep, 0);
	if (rc != EMV_RC_OK)
		goto done;

	emv_chk_txn_end(self->chk);

done:
	return rc;
}

static void libemv_ep_wrapper_free(struct libemv_ep_wrapper *self)
{
	if (self->ep)
		emv_ep_free(self->ep);

	memset(self, 0, sizeof(*self));
	free(self);
}

static const struct emv_ep_wrapper_ops libemv_ep_wrapper_ops = {
	.register_kernel = (emv_ep_wrapper_register_kernel_t)
					      libemv_ep_wrapper_register_kernel,
	.setup		 = (emv_ep_wrapper_setup_t)libemv_ep_wrapper_setup,
	.activate	 = (emv_ep_wrapper_activate_t)
						     libemv_ep_wrapper_activate,
	.free		 = (emv_ep_wrapper_free_t)libemv_ep_wrapper_free
};

struct emv_ep_wrapper *emv_ep_wrapper_new(void)
{
	struct libemv_ep_wrapper *self = NULL;

	self = (struct libemv_ep_wrapper *)malloc(sizeof(*self));
	memset(self, 0, sizeof(*self));
	self->base.ops = &libemv_ep_wrapper_ops;

	self->ep = emv_ep_new(log4c_category);
	if (!self->ep) {
		emv_ep_wrapper_free(self);
		return NULL;
	}

	return (struct emv_ep_wrapper *)self;
}
