/*
 * libemv - Support Library containing EMVCo L2 Kernels.
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

#include <string.h>
#include <stdio.h>
#include <assert.h>

#include <emv.h>
#include <tlv.h>

#define REQUIREMENT(book, id)
#define ARRAY_SIZE(x) (sizeof(x)/sizeof(*(x)))

#define TTQ_B1_MAG_STRIPE_MODE_SUPPORTED	  0x80u
#define TTQ_B1_EMV_MODE_SUPPORTED		  0x20u
#define TTQ_B1_EMV_CONTACT_CHIP_SUPPORTED	  0x10u
#define TTQ_B1_OFFLINE_ONLY_READER		  0x08u
#define TTQ_B1_ONLINE_PIN_SUPPORTED		  0x04u
#define TTQ_B1_SIGNATURE_SUPPORTED		  0x02u
#define TTQ_B1_ODA_FOR_ONLINE_AUTH_SUPPORTED	  0x01u
#define TTQ_B2_ONLINE_CRYPTOGRAM_REQUIRED	  0x80u
#define TTQ_B2_CVM_REQUIRED			  0x40u
#define TTQ_B2_OFFLINE_PIN_SUPPORTED		  0x20u
#define TTQ_B3_ISSUER_UPDATE_PROCESSING_SUPPORTED 0x80u
#define TTQ_B3_CONSUMER_DEVICE_CVM_SUPPORTED	  0x40u

struct emv_ep_config_flags {
	bool	status_check_support:1;
	bool	zero_amount_allowed:1;
	bool	reader_ctls_txn_limit:1;
	bool	reader_ctls_floor_limit:1;
	bool	terminal_floor_limit:1;
	bool	reader_cvm_reqd_limit:1;
	bool	ttq:1;
	bool	ext_selection_support:1;
};

struct emv_ep_config {
	struct emv_ep_config_flags	present;
	struct emv_ep_config_flags	enabled;
	uint8_t				reader_ctls_txn_limit[6];
	uint8_t				reader_ctls_floor_limit[6];
	uint8_t				terminal_floor_limit[6];
	uint8_t				reader_cvm_reqd_limit[6];
	uint8_t				ttq[4];
};

struct emv_ep_combination {
	uint8_t					aid[16];
	size_t					aid_len;
	uint8_t					kernel_id[8];
	size_t					kernel_id_len;
	struct emv_ep_config			config;
	struct emv_ep_preproc_indicators	indicators;
};

struct emv_ep_combination_set {
	struct emv_ep_combination *combinations;
	size_t			   size;
};

struct emv_ep_candidate {
	uint8_t	adf_name[16];
	size_t	adf_name_len;
	uint8_t	application_priority_indicator;
	uint8_t	extended_selection[16];
	size_t	extended_selection_len;
	uint8_t order;
	struct	emv_ep_combination *combination;
};

struct emv_ep_reg_kernel {
	uint8_t		   kernel_id[8];
	size_t		   kernel_id_len;
	struct emv_kernel *kernel;
};

struct emv_ep_reg_kernel_set {
	struct emv_ep_reg_kernel *kernel;
	size_t			  size;
};

struct emv_txn_data {
	enum emv_txn_type type;
	uint8_t		  amount_authorised[6];
	uint8_t		  amount_other[6];
	uint8_t		  currency_code[2];
	uint32_t	  unpredictable_number;
};

struct emv_ep {
	struct emv_hal			*hal;
	struct emv_txn_data		txn_data;
	bool				restart;
	struct emv_ep_combination_set	combination_set[num_txn_types];
	struct emv_ep_reg_kernel_set	reg_kernel_set;
	struct emv_ep_candidate		*candidates;
	int				num_candidates;
};

int emv_ep_register_kernel(struct emv_ep *ep, struct emv_kernel *kernel)
{
	struct emv_ep_reg_kernel_set *set = &ep->reg_kernel_set;
	int rc = EMV_RC_OK;
	size_t i_kernel = set->size++;

	set->kernel = (struct emv_ep_reg_kernel *)realloc(set->kernel,
				  set->size * sizeof(struct emv_ep_reg_kernel));
	if (!set->kernel) {
		rc = EMV_RC_OUT_OF_MEMORY;
		goto error;
	}

	set->kernel[i_kernel].kernel_id_len = sizeof(set->kernel[0].kernel_id);
	rc = kernel->ops->get_id(kernel, set->kernel[i_kernel].kernel_id,
					  &set->kernel[i_kernel].kernel_id_len);
	set->kernel[i_kernel].kernel = kernel;
	if (rc)
		goto error;

	return EMV_RC_OK;

error:
	if (set->kernel) {
		free(set->kernel);
		set->kernel = NULL;
		set->size = 0;
	}
	return rc;
};

static struct emv_kernel *get_kernel(struct emv_ep *ep,
					   const uint8_t *kernel_id, size_t len)
{
	size_t i_krn;

	for (i_krn = 0; i_krn < ep->reg_kernel_set.size; i_krn++)
		if ((len == ep->reg_kernel_set.kernel[i_krn].kernel_id_len) &&
		    (!memcmp(ep->reg_kernel_set.kernel[i_krn].kernel_id,
							       kernel_id, len)))
			return ep->reg_kernel_set.kernel[i_krn].kernel;

	return 0;
}

static bool is_currency_code_supported(const uint8_t *currency_code)
{
	if (!memcmp(ISO4217_USD, currency_code, sizeof(ISO4217_USD)) ||
	    !memcmp(ISO4217_EUR, currency_code, sizeof(ISO4217_EUR)))
		return true;

	return false;
}

static const uint8_t *single_unit_of_currency(const uint8_t *currency_code)
{
	return (uint8_t [6]) { '\x00', '\x00', '\x00', '\x00', '\x01', '\x00' };
}

static enum emv_txn_type get_emv_txn_type(uint8_t txn_type)
{
	switch (txn_type) {
	case 0x00:
		return txn_purchase;
	case 0x01:
		return txn_cash_advance;
	case 0x09:
		return txn_purchase_with_cashback;
	case 0x20:
		return txn_refund;
	default:
		return num_txn_types;
	}
}

static uint64_t bcd_to_u64(const uint8_t *bcd, size_t len)
{
	uint64_t u64 = 0;
	unsigned i = 0;

	for (i = 0, u64 = 0; i < len; i++) {
		u64 *= 10;
		u64 += (bcd[i] >> 4);
		u64 *= 10;
		u64 += bcd[i] & 0xfu;
	}

	return u64;
}

#if 0
static void u64_to_bcd(uint64_t u64, uint8_t *bcd, size_t len)
{
	unsigned i = 0;

	for (i = len; i > 0; i--) {
		bcd[i] = (uint8_t)(u64 % 10);
		u64 /= 10;
		bcd[i] |= (uint8_t)((u64 % 10) << 4);
		u64 /= 10;
	}
}
#endif

int emv_ep_preprocessing(struct emv_ep *ep, struct emv_outcome_parms *outcome)
{
	struct emv_ep_combination_set *combination_set = NULL;
	uint64_t amount_authorised = 0, unit_of_currency = 0;
	bool ctls_app_allowed = 0;
	int i = 0;

	amount_authorised = bcd_to_u64(ep->txn_data.amount_authorised, 6);
	unit_of_currency  = bcd_to_u64(
			single_unit_of_currency(ep->txn_data.currency_code), 6);

	assert(ep->txn_data.type < num_txn_types);
	combination_set = &ep->combination_set[ep->txn_data.type];

	if (!is_currency_code_supported(ep->txn_data.currency_code))
		return EMV_RC_UNSUPPORTED_CURRENCY_CODE;

	for (i = 0; i < combination_set->size; i++) {
		struct emv_ep_combination *combination = NULL;
		struct emv_ep_preproc_indicators *indicators = NULL;
		struct emv_ep_config *cfg = NULL;
		uint64_t ctls_tx_limit = 0, ctls_floor_limit = 0;
		uint64_t floor_limit = 0, cvm_reqd_limit = 0;

		combination = &combination_set->combinations[i];
		cfg	    = &combination->config;
		indicators  = &combination->indicators;

		ctls_tx_limit	 = bcd_to_u64(cfg->reader_ctls_floor_limit, 6);
		ctls_floor_limit = bcd_to_u64(cfg->reader_ctls_floor_limit, 6);
		floor_limit	 = bcd_to_u64(cfg->terminal_floor_limit, 6);
		cvm_reqd_limit	 = bcd_to_u64(cfg->reader_cvm_reqd_limit, 6);


		REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.1.1.1");

		memset(&combination->indicators, 0,
					       sizeof(combination->indicators));


		REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.1.1.2");

		if (cfg->present.ttq) {
			memcpy(indicators->ttq, cfg->ttq, sizeof(cfg->ttq));
			indicators->ttq[1] = indicators->ttq[1] &
					   ~(TTQ_B2_ONLINE_CRYPTOGRAM_REQUIRED |
							   TTQ_B2_CVM_REQUIRED);
		}


		REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.1.1.3");

		if (cfg->present.status_check_support &&
		    cfg->enabled.status_check_support &&
		    (amount_authorised == unit_of_currency))
			indicators->status_check_requested = true;


		REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.1.1.4");

		if (!amount_authorised) {
			if (cfg->present.zero_amount_allowed &&
			    !cfg->enabled.zero_amount_allowed)
				indicators->ctls_app_not_allowed = true;
			else
				indicators->zero_amount = true;
		}


		REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.1.1.5");

		if (cfg->present.reader_ctls_txn_limit &&
		    (amount_authorised >= ctls_tx_limit))
			indicators->ctls_app_not_allowed = true;


		REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.1.1.6");

		if (cfg->present.reader_ctls_floor_limit &&
		    (amount_authorised > ctls_floor_limit))
			indicators->floor_limit_exceeded = true;


		REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.1.1.7");

		if (!cfg->present.reader_ctls_floor_limit &&
		    cfg->present.terminal_floor_limit &&
		    (amount_authorised > floor_limit))
			indicators->floor_limit_exceeded = true;


		REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.1.1.8");

		if (cfg->present.reader_cvm_reqd_limit &&
		    (amount_authorised >= cvm_reqd_limit))
			indicators->cvm_reqd_limit_exceeded = true;


		if (cfg->present.ttq) {


			REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.1.1.9");

			if (indicators->floor_limit_exceeded)
				indicators->ttq[1] |=
					      TTQ_B2_ONLINE_CRYPTOGRAM_REQUIRED;


			REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.1.1.10");

			if (indicators->status_check_requested)
				indicators->ttq[1] |=
					      TTQ_B2_ONLINE_CRYPTOGRAM_REQUIRED;

			REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.1.1.11");

			if (indicators->zero_amount) {
				if (indicators->ttq[0] &
						     TTQ_B1_OFFLINE_ONLY_READER)
					indicators->ctls_app_not_allowed = true;
				else
					indicators->ttq[1] |=
					      TTQ_B2_ONLINE_CRYPTOGRAM_REQUIRED;
			}


			REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.1.1.12");

			if (indicators->cvm_reqd_limit_exceeded)
				indicators->ttq[1] |= TTQ_B2_CVM_REQUIRED;
		}

		if (!indicators->ctls_app_not_allowed)
			ctls_app_allowed = true;
	}


	REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.1.1.13");

	if (!ctls_app_allowed) {
		struct emv_ui_request *ui_req = &outcome->ui_request_on_outcome;

		memset(outcome, 0, sizeof(*outcome));
		outcome->outcome = out_try_another_interface;
		ui_req->present	 = true;
		ui_req->msg_id	 = msg_insert_or_swipe_card;
		ui_req->status	 = sts_processing_error;
	}

	return EMV_RC_OK;
}

int emv_ep_protocol_activation(struct emv_ep *ep, bool started_by_reader)
{
	REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.2.1.1");

	if (!ep->restart) {
		if (started_by_reader) {
			struct emv_ep_combination_set *combination_set = NULL;
			int i = 0;

			combination_set =
					&ep->combination_set[ep->txn_data.type];

			for (i = 0; i < combination_set->size; i++) {
				struct emv_ep_combination *combination = NULL;
				struct emv_ep_config *config = NULL;
				struct emv_ep_preproc_indicators *indicators;

				combination = &combination_set->combinations[i];
				config	    = &combination->config;
				indicators  = &combination->indicators;

				memset(indicators, 0, sizeof(*indicators));

				if (config->present.ttq)
					memcpy(indicators->ttq, config->ttq,
						       sizeof(indicators->ttq));
			}
		}

		/* FIXME: Entry Point shall clear the Candidate list */
	}


	/* FIXME: The following requirements should be done before calling the
	 * entry point.
	 */
	REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.2.1.2"); /* FIXME */


	REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.2.1.3"); /* FIXME */


	REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.2.1.4"); /* FIXME */


	REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.2.1.5"); /* FIXME */


	REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.2.1.6"); /* FIXME */

	return EMV_RC_OK;
}

int emv_ep_register_hal(struct emv_ep *ep, struct emv_hal *hal)
{
	ep->hal = hal;
	return EMV_RC_OK;
}

static int emv_transceive_apdu(struct emv_hal *hal, uint8_t cla, uint8_t ins,
		      uint8_t p1, uint8_t p2, const void *data, size_t data_len,
			 void *response, size_t *response_length, uint8_t sw[2])
{
	uint8_t capdu[262], rapdu[258];
	size_t capdu_len = 0, rapdu_len = sizeof(rapdu);
	int rc = EMV_RC_OK;

	assert(hal && hal->ops && hal->ops->transceive);
	assert(!response || response_length);
	assert(!data_len || data);
	assert(!response_length || (*response_length <= 256));
	assert(data_len <= 256);
	assert(sw);

	capdu[capdu_len++] = cla;
	capdu[capdu_len++] = ins;
	capdu[capdu_len++] = p1;
	capdu[capdu_len++] = p2;

	if (data_len) {
		capdu[capdu_len++] = (uint8_t)data_len;
		memcpy(&capdu[capdu_len], data, data_len);
		capdu_len += data_len;
	}

	if (response_length && *response_length)
		capdu[capdu_len++] = (uint8_t)*response_length;

	rc = hal->ops->transceive(hal, capdu, capdu_len, rapdu, &rapdu_len);
	if (rc != EMV_RC_OK)
		return rc;

	memcpy(response, rapdu, rapdu_len - 2);
	*response_length = rapdu_len - 2;
	sw[0] = rapdu[rapdu_len - 2];
	sw[1] = rapdu[rapdu_len - 1];

	return EMV_RC_OK;
}

#define EMV_CMD_SELECT_CLA		0x00u
#define EMV_CMD_SELECT_INS		0xA4u
#define EMV_CMD_SELECT_P1_BY_NAME	0x04u
#define EMV_CMD_SELECT_P2_FIRST		0x00u
#define EMV_CMD_SELECT_P2_NEXT		0x02u

struct ppse_dir_entry {
	uint8_t adf_name[16];
	size_t	adf_name_len;
	char	application_label[16];
	size_t	application_label_len;
	uint8_t	application_priority_indicator;
	uint8_t kernel_identifier[8];
	size_t	kernel_identifier_len;
	uint8_t extended_selection[16];
	size_t	extended_selection_len;
};

int emv_ep_parse_ppse(struct tlv *ppse, struct ppse_dir_entry *entries,
							    size_t *num_entries)
{
	struct tlv *i_tlv;
	size_t num;

	assert(ppse);
	assert(entries);
	assert(num_entries);

	i_tlv = tlv_find(tlv_get_child(tlv_find(tlv_get_child(tlv_find(
			     tlv_get_child(tlv_find(ppse, TLV_ID_FCI_TEMPLATE)),
					      TLV_ID_FCI_PROPRIETARY_TEMPLATE)),
					 TLV_ID_FCI_ISSUER_DISCRETIONARY_DATA)),
							TLV_ID_DIRECTORY_ENTRY);

	for (num = 0;
	     i_tlv && (num < *num_entries);
	     i_tlv = tlv_find(tlv_get_next(i_tlv), TLV_ID_DIRECTORY_ENTRY)) {
		struct tlv *adf_name, *label, *prio, *kernel_id, *ext_sel;
		struct tlv *entry;
		struct ppse_dir_entry *dir_entry;
		int rc = TLV_RC_OK;

		entry	  = tlv_get_child(i_tlv);
		adf_name  = tlv_find(entry, TLV_ID_ADF_NAME);
		label	  = tlv_find(entry, TLV_ID_APPLICATION_LABEL);
		kernel_id = tlv_find(entry, TLV_ID_KERNEL_IDENTIFIER);
		ext_sel	  = tlv_find(entry, TLV_ID_EXTENDED_SELECTION);
		prio      = tlv_find(entry,
					 TLV_ID_APPLICATION_PRIORITY_INDICATOR);

		dir_entry = &entries[num];
		memset(dir_entry, 0, sizeof(*dir_entry));

		if (adf_name) {
			dir_entry->adf_name_len = sizeof(dir_entry->adf_name);
			rc = tlv_encode_value(adf_name, dir_entry->adf_name,
						      &dir_entry->adf_name_len);
			assert(rc == TLV_RC_OK);
		}

		if (label) {
			dir_entry->application_label_len =
					   sizeof(dir_entry->application_label);
			rc = tlv_encode_value(label,
						   dir_entry->application_label,
					     &dir_entry->application_label_len);
			assert(rc == TLV_RC_OK);
		}

		if (kernel_id) {
			dir_entry->kernel_identifier_len =
					   sizeof(dir_entry->kernel_identifier);
			rc = tlv_encode_value(kernel_id,
						   dir_entry->kernel_identifier,
					     &dir_entry->kernel_identifier_len);
			assert(rc == TLV_RC_OK);
		}

		if (ext_sel) {
			dir_entry->extended_selection_len =
					  sizeof(dir_entry->extended_selection);
			rc = tlv_encode_value(ext_sel,
						  dir_entry->extended_selection,
					    &dir_entry->extended_selection_len);
			assert(rc == TLV_RC_OK);
		}

		if (prio) {
			size_t len = 1;

			rc = tlv_encode_value(prio,
				     &dir_entry->application_priority_indicator,
									  &len);
			assert(rc == TLV_RC_OK);
		}

		num++;
	}

	if ((num == *num_entries) && (i_tlv))
		return EMV_RC_BUFFER_OVERFLOW;

	*num_entries = num;
	return EMV_RC_OK;
}

bool emv_ep_is_combination_candidate(struct emv_ep_combination *combination,
					       struct ppse_dir_entry *dir_entry)

{
	uint8_t requested_kernel_id[8];
	size_t requested_kernel_id_len;


	REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.3.2.5 A");

	if (dir_entry->adf_name_len < 5)
		return false;


	REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.3.2.5 B");

	if (memcmp(combination->aid, dir_entry->adf_name, combination->aid_len))
		return false;


	REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.3.2.5 C");

	if (dir_entry->kernel_identifier_len == 0) {
		/* FIXME: See Table 3-6 in EMV_CTLS_BOOK_B */
		requested_kernel_id[0] = 0;
		requested_kernel_id_len = 1;
	} else if (((dir_entry->kernel_identifier[0] & 0xc0) == 0x00) ||
			   ((dir_entry->kernel_identifier[0] & 0xc0) == 0x40)) {
		requested_kernel_id[0] = dir_entry->kernel_identifier[0];
		requested_kernel_id_len = 1;
	} else if (dir_entry->kernel_identifier[0] & 0x3f) {
		memcpy(requested_kernel_id, dir_entry->kernel_identifier, 3);
		requested_kernel_id_len = 3;
	} else {
		memcpy(requested_kernel_id, dir_entry->kernel_identifier,
					      dir_entry->kernel_identifier_len);
		requested_kernel_id_len = dir_entry->kernel_identifier_len;
	}


	REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.3.2.5 D");

	if ((requested_kernel_id[0] != 0) &&
	    ((requested_kernel_id_len != dir_entry->kernel_identifier_len) ||
	     (memcmp(requested_kernel_id, dir_entry->kernel_identifier,
						     requested_kernel_id_len))))
		return false;

	return true;
}

static int compare_candidates(const void *candidate_a, const void *candidate_b)
{
	const struct emv_ep_candidate *a = NULL, *b = NULL;

	a = (const struct emv_ep_candidate *)candidate_a;
	b = (const struct emv_ep_candidate *)candidate_b;

	if (a->application_priority_indicator ==
					    b->application_priority_indicator) {
		if (a->order == b->order)
			return 0;
		if (a->order < b->order)
			return -1;
		else
			return 1;
	}

	if (!a->application_priority_indicator)
		return -1;

	if (!b->application_priority_indicator)
		return 1;

	if (a->application_priority_indicator <
					      b->application_priority_indicator)
		return -1;

	return 1;
}

int emv_ep_combination_selection(struct emv_ep *ep)
{
	struct emv_ep_combination_set *combination_set = NULL;
	struct ppse_dir_entry dir_entry[32];
	size_t num_dir_entries = ARRAY_SIZE(dir_entry);
	uint8_t fci[256];
	size_t fci_len = sizeof(fci);
	uint8_t sw[2];
	int rc = EMV_RC_OK, i_comb, i_dir;
	struct tlv *tlv_fci = NULL;

	combination_set = &ep->combination_set[ep->txn_data.type];


	REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.3.2.1");

	rc = emv_transceive_apdu(ep->hal, EMV_CMD_SELECT_CLA,
				  EMV_CMD_SELECT_INS, EMV_CMD_SELECT_P1_BY_NAME,
				      EMV_CMD_SELECT_P2_FIRST, "2PAY.SYS.DDF01",
							 14, fci, &fci_len, sw);
	if (rc != EMV_RC_OK)
		goto done;

	if ((sw[0] != 0x90) || (sw[1] != 0x00)) {
		rc = EMV_RC_CARD_PROTOCOL_ERROR;
		goto done;
	}


	rc = tlv_parse(fci, fci_len, &tlv_fci);
	if (rc != TLV_RC_OK) {
		rc = EMV_RC_CARD_PROTOCOL_ERROR;
		goto done;
	}

	rc = emv_ep_parse_ppse(tlv_fci, dir_entry, &num_dir_entries);
	assert(rc == EMV_RC_OK);

	ep->num_candidates = 0;
	ep->candidates = (struct emv_ep_candidate *)
				 calloc(combination_set->size * num_dir_entries,
					       sizeof(struct emv_ep_candidate));
	if (!ep->candidates) {
		rc = EMV_RC_OUT_OF_MEMORY;
		goto done;
	}

	for (i_comb = 0; i_comb < combination_set->size; i_comb++) {
		struct emv_ep_combination *comb =
					 &combination_set->combinations[i_comb];


		REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.3.2.5");

		if (comb->indicators.ctls_app_not_allowed)
			continue;

		for (i_dir = 0; i_dir < num_dir_entries; i_dir++) {
			struct ppse_dir_entry *entry = &dir_entry[i_dir];
			struct emv_ep_candidate *candidate = NULL;

			if (!emv_ep_is_combination_candidate(comb, entry))
				continue;

			REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.3.2.5 E");

			candidate = &ep->candidates[ep->num_candidates++];
			memcpy(candidate->adf_name, entry->adf_name,
							   entry->adf_name_len);
			candidate->adf_name_len = entry->adf_name_len;
			candidate->application_priority_indicator =
					  entry->application_priority_indicator;
			memcpy(candidate->extended_selection,
						      entry->extended_selection,
						 entry->extended_selection_len);
			candidate->extended_selection_len =
						  entry->extended_selection_len;
			candidate->order = (uint8_t)i_dir;
			candidate->combination = comb;
		}
	}

	qsort(ep->candidates, ep->num_candidates,
			   sizeof(struct emv_ep_candidate), compare_candidates);

done:
	if (tlv_fci)
		tlv_free(tlv_fci);

	return rc;
}

int emv_ep_final_combination_selection(struct emv_ep *ep)
{
	struct emv_ep_candidate *candidate = NULL;
	struct emv_ep_config *config = NULL;
	struct emv_kernel *kernel = NULL;
	uint8_t adf[32];
	size_t adf_len = 0;
	uint8_t fci[256];
	size_t fci_len = sizeof(fci);
	uint8_t sw[2];
	int rc = EMV_RC_OK;

	candidate = &ep->candidates[ep->num_candidates - 1];
	config = &candidate->combination->config;

	adf_len = candidate->adf_name_len;
	memcpy(adf, candidate->adf_name, adf_len);

	if (candidate->extended_selection_len &&
					config->present.ext_selection_support &&
					config->enabled.ext_selection_support) {
		memcpy(&adf[adf_len], candidate->extended_selection,
					     candidate->extended_selection_len);
		adf_len += candidate->extended_selection_len;
	}

	rc = emv_transceive_apdu(ep->hal, EMV_CMD_SELECT_CLA,
				  EMV_CMD_SELECT_INS, EMV_CMD_SELECT_P1_BY_NAME,
					  EMV_CMD_SELECT_P2_FIRST, adf, adf_len,
							     fci, &fci_len, sw);

	kernel = get_kernel(ep, candidate->combination->kernel_id,
					 candidate->combination->kernel_id_len);
	if (!kernel)
		return EMV_RC_NO_KERNEL;

	rc = kernel->ops->activate(kernel, ep->hal,
			  &candidate->combination->indicators, fci, fci_len, sw,
							      NULL, NULL, NULL);

	return rc;
}

int emv_ep_activate(struct emv_ep *ep, enum emv_start start,
			enum emv_txn_type txn_type, uint8_t amount_authorise[6],
			      uint8_t amount_other[6], uint8_t currency_code[2],
						  uint32_t unpredictable_number)
{
	int rc = EMV_RC_OK;

	rc = emv_ep_combination_selection(ep);
	if (rc != EMV_RC_OK)
		return rc;

	rc = emv_ep_final_combination_selection(ep);
	if (rc != EMV_RC_OK)
		return rc;

	return rc;
}

static int parse_combination(struct tlv *tlv_combination,
						struct emv_ep_combination *comb)
{
	int rc = EMV_RC_OK;
	struct tlv *tlv_attr = NULL;

	for (tlv_attr = tlv_get_child(tlv_combination);
	     tlv_attr;
	     tlv_attr = tlv_get_next(tlv_attr)) {
		uint8_t tag[4];
		size_t tag_size = sizeof(tag);
		struct emv_ep_config *cfg = &comb->config;

		rc = tlv_encode_identifier(tlv_attr, tag, &tag_size);
		if ((rc != TLV_RC_OK) || (tag_size != sizeof(tag)))
			return EMV_RC_SYNTAX_ERROR;

		if (!memcmp(tag, TLV_ID_LIBEMV_AID, tag_size)) {
			comb->aid_len = sizeof(comb->aid);
			rc = tlv_encode_value(tlv_attr, comb->aid,
								&comb->aid_len);
			if (rc != TLV_RC_OK)
				return EMV_RC_SYNTAX_ERROR;

			continue;
		}

		if (!memcmp(tag, TLV_ID_LIBEMV_KERNEL_ID, tag_size)) {
			comb->kernel_id_len = sizeof(comb->kernel_id);
			rc = tlv_encode_value(tlv_attr, comb->kernel_id,
							  &comb->kernel_id_len);
			if (rc != TLV_RC_OK)
				return EMV_RC_SYNTAX_ERROR;

			continue;
		}

		if (!memcmp(tag, TLV_ID_LIBEMV_STATUS_CHECK_SUPPORTED,
								    tag_size)) {
			uint8_t enabled;
			size_t enabled_sz = sizeof(enabled);

			rc = tlv_encode_value(tlv_attr, &enabled, &enabled_sz);
			if ((rc != TLV_RC_OK) || (!enabled_sz))
				return EMV_RC_SYNTAX_ERROR;

			cfg->present.status_check_support = 1;
			cfg->enabled.status_check_support = !!enabled;

			continue;
		}

		if (!memcmp(tag, TLV_ID_LIBEMV_ZERO_AMOUNT_ALLOWED, tag_size)) {
			uint8_t enabled;
			size_t enabled_sz = sizeof(enabled);

			rc = tlv_encode_value(tlv_attr, &enabled, &enabled_sz);
			if ((rc != TLV_RC_OK) || (!enabled_sz))
				return EMV_RC_SYNTAX_ERROR;

			cfg->present.zero_amount_allowed = 1;
			cfg->enabled.zero_amount_allowed = !!enabled;

			continue;
		}

		if (!memcmp(tag, TLV_ID_LIBEMV_STATUS_CHECK_SUPPORTED,
								    tag_size)) {
			uint8_t enabled;
			size_t enabled_sz = sizeof(enabled);

			rc = tlv_encode_value(tlv_attr, &enabled, &enabled_sz);
			if ((rc != TLV_RC_OK) || (!enabled_sz))
				return EMV_RC_SYNTAX_ERROR;

			cfg->present.ext_selection_support = 1;
			cfg->enabled.ext_selection_support = !!enabled;

			continue;
		}

		if (!memcmp(tag, TLV_ID_LIBEMV_RDR_CTLS_TXN_LIMIT, tag_size)) {
			size_t size = 0;

			cfg->present.reader_ctls_txn_limit = 1;

			size = sizeof(cfg->reader_ctls_txn_limit);
			rc = tlv_encode_value(tlv_attr,
					     cfg->reader_ctls_txn_limit, &size);
			if ((rc != TLV_RC_OK) ||
			    (size != sizeof(cfg->reader_ctls_txn_limit)))
				return EMV_RC_SYNTAX_ERROR;

			continue;
		}

		if (!memcmp(tag, TLV_ID_LIBEMV_RDR_CTLS_FLOOR_LIMIT,
								    tag_size)) {
			size_t size = 0;

			cfg->present.reader_ctls_floor_limit = 1;

			size = sizeof(cfg->reader_ctls_floor_limit);
			rc = tlv_encode_value(tlv_attr,
					   cfg->reader_ctls_floor_limit, &size);
			if ((rc != TLV_RC_OK) ||
			    (size != sizeof(cfg->reader_ctls_floor_limit)))
				return EMV_RC_SYNTAX_ERROR;

			continue;
		}

		if (!memcmp(tag, TLV_ID_LIBEMV_TERMINAL_FLOOR_LIMIT,
								    tag_size)) {
			size_t size = sizeof(cfg->terminal_floor_limit);

			cfg->present.terminal_floor_limit = 1;

			rc = tlv_encode_value(tlv_attr,
					      cfg->terminal_floor_limit, &size);
			if ((rc != TLV_RC_OK) ||
			    (size != sizeof(cfg->terminal_floor_limit)))
				return EMV_RC_SYNTAX_ERROR;

			continue;
		}

		if (!memcmp(tag, TLV_ID_LIBEMV_RDR_CVM_REQUIRED_LIMIT,
								    tag_size)) {
			size_t size = sizeof(cfg->reader_cvm_reqd_limit);

			cfg->present.reader_cvm_reqd_limit = 1;

			rc = tlv_encode_value(tlv_attr,
					     cfg->reader_cvm_reqd_limit, &size);
			if ((rc != TLV_RC_OK) ||
			    (size != sizeof(cfg->reader_cvm_reqd_limit)))
				return EMV_RC_SYNTAX_ERROR;

			continue;
		}

		if (!memcmp(tag, TLV_ID_LIBEMV_TTQ, tag_size)) {
			size_t size = sizeof(cfg->ttq);

			cfg->present.ttq = 1;

			rc = tlv_encode_value(tlv_attr, cfg->ttq, &size);
			if ((rc != TLV_RC_OK) || (size != sizeof(cfg->ttq)))
				return EMV_RC_SYNTAX_ERROR;

			continue;
		}

		if (!memcmp(tag, TLV_ID_LIBEMV_TRANSACTION_TYPE, tag_size))
			return EMV_RC_SYNTAX_ERROR;
	}

	return EMV_RC_OK;
}

static int parse_combination_set(struct tlv *tlv_set,
				 struct emv_ep_combination_set *set)
{
	struct tlv *tlv_comb = NULL;
	int rc = EMV_RC_OK;

	for (tlv_comb = tlv_find(tlv_get_child(tlv_set),
						     TLV_ID_LIBEMV_COMBINATION);
	     tlv_comb;
	     tlv_comb = tlv_find(tlv_get_next(tlv_comb),
						   TLV_ID_LIBEMV_COMBINATION)) {

		set->combinations = (struct emv_ep_combination *)realloc(
							      set->combinations,
			   sizeof(struct emv_ep_combination) * (set->size + 1));

		rc = parse_combination(tlv_comb, &set->combinations[set->size]);
		if (rc != TLV_RC_OK) {
			rc = EMV_RC_SYNTAX_ERROR;
			goto error;
		}

		set->size++;
	}

	return EMV_RC_OK;

error:
	if (set->combinations)
		free(set->combinations);
	set->size = 0;
	return rc;
}

int emv_ep_configure(struct emv_ep *ep, const void *config, size_t len)
{
	struct tlv *tlv_config = NULL;
	struct tlv *tlv_combination_set = NULL;
	int rc = EMV_RC_OK;

	rc = tlv_parse(config, len, &tlv_config);
	if (rc != TLV_RC_OK) {
		rc = EMV_RC_SYNTAX_ERROR;
		goto error;
	}

	for (tlv_combination_set = tlv_find(tlv_get_child(tlv_find(tlv_config,
		  TLV_ID_LIBEMV_CONFIGURATION)), TLV_ID_LIBEMV_COMBINATION_SET);
	     tlv_combination_set;
	     tlv_combination_set = tlv_find(tlv_get_next(tlv_combination_set),
					       TLV_ID_LIBEMV_COMBINATION_SET)) {
		enum emv_txn_type emv_txn_type = num_txn_types;
		uint8_t txn_type = 0;
		size_t txn_type_size = sizeof(txn_type);
		struct tlv *tlv_txn_type = NULL;

		tlv_txn_type = tlv_find(tlv_get_child(tlv_combination_set),
						TLV_ID_LIBEMV_TRANSACTION_TYPE);
		if (!tlv_txn_type) {
			rc = EMV_RC_SYNTAX_ERROR;
			goto error;
		}

		rc = tlv_encode_value(tlv_txn_type, &txn_type, &txn_type_size);
		if ((rc != TLV_RC_OK) || (txn_type_size != 1)) {
			rc = EMV_RC_SYNTAX_ERROR;
			goto error;
		}

		emv_txn_type = get_emv_txn_type(txn_type);
		if (emv_txn_type >= num_txn_types) {
			rc = EMV_RC_SYNTAX_ERROR;
			goto error;
		}

		rc = parse_combination_set(tlv_combination_set,
					    &ep->combination_set[emv_txn_type]);
		if (rc != EMV_RC_OK)
			goto error;
	}

	return EMV_RC_OK;

error:
	if (tlv_config)
		tlv_free(tlv_config);

	return rc;
}

struct emv_ep *emv_ep_new(void)
{
	return (struct emv_ep *)calloc(1, sizeof(struct emv_ep));
}

void emv_ep_free(struct emv_ep *ep)
{
	free(ep);
}
