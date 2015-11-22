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

static int get_txn_type(struct emv_transaction_data *txn,
						   enum emv_txn_type *txn_type)
{
	/* See EMV Contactless Book A v2.5, Table 5-6: Type of Transaction.   */
	switch (txn->transaction_type) {
	case 0x00:
		if (txn->amount_other)
			*txn_type = txn_purchase_with_cashback;
		else
			*txn_type = txn_purchase;
		break;
	case 0x09:
		*txn_type = txn_purchase_with_cashback;
		break;
	case 0x01:
		*txn_type = txn_cash_advance;
		break;
	case 0x20:
		*txn_type = txn_refund;
		break;
	default:
		return EMV_RC_UNSUPPORTED_TRANSACTION_TYPE;
	}

	return EMV_RC_OK;
}

int emv_ep_preprocessing(struct emv_ep *ep, struct emv_transaction_data *tx,
					      struct emv_outcome_parms *outcome)
{
	struct emv_ep_combination_set *combination_set = NULL;
	uint64_t amount_authorised = 0, unit_of_currency = 0;
	bool ctls_app_allowed = 0;
	int i = 0, rc = EMV_RC_OK;

	amount_authorised = bcd_to_u64(tx->amount_authorised, 6);
	unit_of_currency  = bcd_to_u64(
				 single_unit_of_currency(tx->currency_code), 6);

	rc = get_txn_type(tx, &ep->txn_type);
	if (rc != EMV_RC_OK)
		return rc;

	assert(ep->txn_type < num_txn_types);
	combination_set = &ep->combination_set[ep->txn_type];

	if (!is_currency_code_supported(tx->currency_code))
		return EMV_RC_UNSUPPORTED_CURRENCY_CODE;

	for (i = 0; i < combination_set->size; i++) {
		struct emv_ep_combination *combination = NULL;
		struct emv_ep_preproc_indicators *indicators = NULL;
		struct emv_ep_config *cfg = NULL;
		uint64_t ctls_tx_limit = 0, ctls_floor_limit = 0;
		uint64_t floor_limit = 0, cvm_reqd_limit = 0;
		uint32_t *ttq = NULL;

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
			ttq = &indicators->copy_of_ttq;
			*ttq = cfg->ttq & ~(TTQ_ONLINE_CRYPTOGRAM_REQUIRED |
							      TTQ_CVM_REQUIRED);
		}


		REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.1.1.3");

		if (cfg->present.status_check_support &&
		    cfg->supported.status_check_support &&
		    (amount_authorised == unit_of_currency))
			indicators->status_check_requested = true;


		REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.1.1.4");

		if (!amount_authorised) {
			if (cfg->present.zero_amount_allowed &&
			    !cfg->supported.zero_amount_allowed)
				indicators->ctls_app_not_allowed = true;
			else
				indicators->zero_amount = true;
		}


		REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.1.1.5");

		if (cfg->present.reader_ctls_tx_limit &&
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
				*ttq |= TTQ_ONLINE_CRYPTOGRAM_REQUIRED;


			REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.1.1.10");

			if (indicators->status_check_requested)
				*ttq |= TTQ_ONLINE_CRYPTOGRAM_REQUIRED;


			REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.1.1.11");

			if (indicators->zero_amount) {
				if (*ttq & TTQ_OFFLINE_ONLY_READER)
					indicators->ctls_app_not_allowed = true;
				else
					*ttq |= TTQ_ONLINE_CRYPTOGRAM_REQUIRED;
			}


			REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.1.1.12");

			if (indicators->cvm_reqd_limit_exceeded)
				*ttq |= TTQ_CVM_REQUIRED;
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

int emv_ep_protocol_activation(struct emv_ep *ep,
			struct emv_transaction_data *tx, bool started_by_reader)
{
	REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.2.1.1");

	if (!ep->restart) {
		if (started_by_reader) {
			struct emv_ep_combination_set *combination_set = NULL;
			int i = 0;

			combination_set = &ep->combination_set[ep->txn_type];

			for (i = 0; i < combination_set->size; i++) {
				struct emv_ep_combination *combination = NULL;
				struct emv_ep_config *config = NULL;
				struct emv_ep_preproc_indicators *indicators;

				combination = &combination_set->combinations[i];
				config	    = &combination->config;
				indicators  = &combination->indicators;

				memset(indicators, 0, sizeof(*indicators));

				if (config->present.ttq)
					indicators->copy_of_ttq = config->ttq;
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

void emv_ep_register_hal(struct emv_ep *ep, struct emv_hal *hal)
{
	ep->hal = hal;
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

	combination_set = &ep->combination_set[ep->txn_type];


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
	uint8_t adf[32];
	size_t adf_len = 0;
	uint8_t fci[256];
	size_t fci_len = sizeof(fci);
	uint8_t sw[2];
	int rc = EMV_RC_OK, i;

	candidate = &ep->candidates[ep->num_candidates - 1];
	config = &candidate->combination->config;

	adf_len = candidate->adf_name_len;
	memcpy(adf, candidate->adf_name, adf_len);

	if (candidate->extended_selection_len &&
					config->present.ext_selection_support &&
				      config->supported.ext_selection_support) {
		memcpy(&adf[adf_len], candidate->extended_selection,
					     candidate->extended_selection_len);
		adf_len += candidate->extended_selection_len;
	}

	rc = emv_transceive_apdu(ep->hal, EMV_CMD_SELECT_CLA,
				  EMV_CMD_SELECT_INS, EMV_CMD_SELECT_P1_BY_NAME,
					  EMV_CMD_SELECT_P2_FIRST, adf, adf_len,
							     fci, &fci_len, sw);
	printf("sw: %02x%02x\n", sw[0], sw[1]);
	for (i = 0; i < fci_len; i++)
		printf("%02X", fci[i]);
	printf("\n");

	return rc;
}

int emv_ep_activate(struct emv_ep *ep, enum emv_txn_type txn_type,
			   uint8_t amount_authorise[6], uint8_t amount_other[6],
			uint8_t currency_code[2], uint32_t unpredictable_number)
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
