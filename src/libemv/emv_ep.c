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

static int get_tx_type_idx(struct emv_transaction_data *tx, int *type_idx)
{
	/* See EMV Contactless Book A v2.5, Table 5-6: Type of Transaction.   */
	switch (tx->transaction_type) {
	case 0x00:
		if (tx->amount_other)
			*type_idx = EMV_EP_TX_TYPE_IDX_PURCHASE_WITH_CASHBACK;
		else
			*type_idx = EMV_EP_TX_TYPE_IDX_PURCHASE;
		break;
	case 0x09:
		*type_idx = EMV_EP_TX_TYPE_IDX_PURCHASE_WITH_CASHBACK;
		break;
	case 0x01:
		*type_idx = EMV_EP_TX_TYPE_IDX_CASH_ADVANCE;
		break;
	case 0x20:
		*type_idx = EMV_EP_TX_TYPE_IDX_REFUND;
		break;
	default:
		return EMV_RC_UNSUPPORTED_TRANSACTION_TYPE;
	}

	return EMV_RC_OK;
}

int emv_ep_preprocessing(struct emv_ep *ep, struct emv_transaction_data *tx,
					      struct emv_outcome_parms *outcome)
{
	uint64_t amount_authorised = 0, unit_of_currency = 0;
	bool ctls_app_allowed = 0;
	int i = 0, tx_type_idx = 0, rc = EMV_RC_OK;

	amount_authorised = bcd_to_u64(tx->amount_authorised, 6);
	unit_of_currency  = bcd_to_u64(
				 single_unit_of_currency(tx->currency_code), 6);

	rc = get_tx_type_idx(tx, &tx_type_idx);
	if (rc != EMV_RC_OK)
		return rc;

	if (!is_currency_code_supported(tx->currency_code))
		return EMV_RC_UNSUPPORTED_CURRENCY_CODE;

	for (i = 0; i < ep->num_combinations; i++) {
		struct emv_ep_preproc_indicators *indicators = NULL;
		struct emv_ep_config *cfg = NULL;
		uint64_t ctls_tx_limit = 0, ctls_floor_limit = 0;
		uint64_t floor_limit = 0, cvm_reqd_limit = 0;
		uint32_t *ttq = NULL;

		cfg	   = &ep->combinations[i].config[tx_type_idx];
		indicators = &ep->combinations[i].indicators;

		ctls_tx_limit	 = bcd_to_u64(cfg->reader_ctls_floor_limit, 6);
		ctls_floor_limit = bcd_to_u64(cfg->reader_ctls_floor_limit, 6);
		floor_limit	 = bcd_to_u64(cfg->terminal_floor_limit, 6);
		cvm_reqd_limit	 = bcd_to_u64(cfg->reader_cvm_reqd_limit, 6);


		REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.1.1.1");

		memset(&ep->combinations[i].indicators, 0,
					sizeof(ep->combinations[i].indicators));


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


			REQUIREMENT(EMV_CTRL_BOOK_B_V2_5, "3.1.1.12");

			if (indicators->cvm_reqd_limit_exceeded)
				*ttq |= TTQ_CVM_REQUIRED;
		}

		if (!indicators->ctls_app_not_allowed)
			ctls_app_allowed = true;
	}


	REQUIREMENT(EMV_CTRL_BOOK_B_V2_5, "3.1.1.13");

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
	REQUIREMENT(EMV_CTRL_BOOK_B_V2_5, "3.2.1.1");

	if (!ep->restart) {
		if (started_by_reader) {
			int i, tx_type_idx, rc;

			rc = get_tx_type_idx(tx, &tx_type_idx);
			if (rc != EMV_RC_OK)
				return rc;

			for (i = 0; i < ep->num_combinations; i++) {
				struct emv_ep_config *cfg = NULL;
				struct emv_ep_preproc_indicators *indicators;

				cfg = &ep->combinations[i].config[tx_type_idx];
				indicators = &ep->combinations[i].indicators;

				memset(indicators, 0, sizeof(*indicators));

				if (cfg->present.ttq)
					indicators->copy_of_ttq = cfg->ttq;
			}
		}

		/* FIXME: Entry Point shall clear the Candidate list */
	}


	/* FIXME: The following requirements should be done before calling the
	 * entry point.
	 */
	REQUIREMENT(EMV_CTRL_BOOK_B_V2_5, "3.2.1.2"); /* FIXME */


	REQUIREMENT(EMV_CTRL_BOOK_B_V2_5, "3.2.1.3"); /* FIXME */


	REQUIREMENT(EMV_CTRL_BOOK_B_V2_5, "3.2.1.4"); /* FIXME */


	REQUIREMENT(EMV_CTRL_BOOK_B_V2_5, "3.2.1.5"); /* FIXME */


	REQUIREMENT(EMV_CTRL_BOOK_B_V2_5, "3.2.1.6"); /* FIXME */

	return EMV_RC_OK;
}

void emv_ep_register_hal(struct emv_ep *ep, struct emv_hal *hal)
{
	ep->hal = hal;
}

struct emv_candidate {
	uint8_t adf_name[16];
	size_t	adf_name_len;
	char	label[16];
	size_t	label_len;
	uint8_t	prio;
	uint8_t kernel_identifier[8];
	size_t	kernel_identifier_len;
	uint8_t extended_selection[16];
	size_t	extended_selection_len;
};

static int emv_transceive_apdu(struct emv_ep *ep, uint8_t cla, uint8_t ins,
		      uint8_t p1, uint8_t p2, const void *data, size_t data_len,
			 void *response, size_t *response_length, uint8_t sw[2])
{
	uint8_t capdu[262], rapdu[258];
	size_t capdu_len = 0, rapdu_len = sizeof(rapdu);
	int rc = EMV_RC_OK;

	assert(ep && ep->hal && ep->hal->emv_transceive);
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

	rc = ep->hal->emv_transceive(ep->hal, capdu, capdu_len, rapdu,
								    &rapdu_len);
	if (rc != EMV_RC_OK)
		return rc;

	memcpy(response, rapdu, rapdu_len - 2);
	*response_length = rapdu_len - 2;
	sw[0] = rapdu[rapdu_len - 2];
	sw[1] = rapdu[rapdu_len - 1];

	return EMV_RC_OK;
}

int emv_ep_combination_selection(struct emv_ep *ep)
{
	struct emv_candidate candidates[16];
	int num_candidates = 0;
	uint8_t fci[256];
	size_t fci_len = sizeof(fci);
	uint8_t sw[2];
	int rc, i;
	struct tlv *tlv_fci = NULL, *tlv_cur = NULL, *tlv_dir = NULL;

	rc = emv_transceive_apdu(ep, 0x00, 0xA4, 0x04, 0x00, "2PAY.SYS.DDF01",
							 14, fci, &fci_len, sw);
	if (rc != EMV_RC_OK)
		goto done;

	rc = tlv_parse(fci, fci_len, &tlv_fci);
	if (rc != TLV_RC_OK) {
		rc = EMV_RC_CARD_PROTOCOL_ERROR;
		goto done;
	}

	tlv_cur = tlv_find(tlv_fci, TLV_ID_FCI_TEMPLATE);
	if (!tlv_cur) {
		rc = EMV_RC_CARD_PROTOCOL_ERROR;
		goto done;
	}

	tlv_cur = tlv_get_child(tlv_cur);
	if (!tlv_cur) {
		rc = EMV_RC_CARD_PROTOCOL_ERROR;
		goto done;
	}

	tlv_cur = tlv_find(tlv_cur, TLV_ID_FCI_PROPRIETARY_TEMPLATE);
	if (!tlv_cur) {
		rc = EMV_RC_CARD_PROTOCOL_ERROR;
		goto done;
	}

	tlv_cur = tlv_get_child(tlv_cur);
	if (!tlv_cur) {
		rc = EMV_RC_CARD_PROTOCOL_ERROR;
		goto done;
	}

	tlv_cur = tlv_find(tlv_cur, TLV_ID_FCI_ISSUER_DISCRETIONARY_DATA);
	if (!tlv_cur) {
		rc = EMV_RC_CARD_PROTOCOL_ERROR;
		goto done;
	}

	tlv_cur = tlv_get_child(tlv_cur);
	if (!tlv_cur) {
		rc = EMV_RC_CARD_PROTOCOL_ERROR;
		goto done;
	}

	num_candidates = 0;
	for (tlv_dir = tlv_find(tlv_cur, TLV_ID_DIRECTORY_ENTRY);
	     tlv_dir;
	     tlv_dir = tlv_find(tlv_get_next(tlv_dir),
						      TLV_ID_DIRECTORY_ENTRY)) {
		struct emv_candidate *candidate = &candidates[num_candidates];
		struct tlv *tlv_dir_entry = NULL;

		tlv_dir_entry = tlv_get_child(tlv_dir);
		if (!tlv_dir_entry) {
			rc = EMV_RC_CARD_PROTOCOL_ERROR;
			goto done;
		}

		tlv_cur = tlv_find(tlv_dir_entry, TLV_ID_ADF_NAME);
		if (!tlv_cur) {
			rc = EMV_RC_CARD_PROTOCOL_ERROR;
			goto done;
		}

		candidate->adf_name_len = sizeof(candidate->adf_name);
		rc = tlv_encode_value(tlv_cur, candidate->adf_name,
						      &candidate->adf_name_len);
		if (rc != TLV_RC_OK) {
			rc = EMV_RC_CARD_PROTOCOL_ERROR;
			goto done;
		}

		tlv_cur = tlv_find(tlv_dir_entry, TLV_ID_APPLICATION_LABEL);
		if (tlv_cur) {
			candidate->label_len = sizeof(candidate->label);
			rc = tlv_encode_value(tlv_cur, candidate->label,
							 &candidate->label_len);
			if (rc != TLV_RC_OK) {
				rc = EMV_RC_CARD_PROTOCOL_ERROR;
				goto done;
			}
		} else {
			candidate->label_len = 0;
		}

		tlv_cur = tlv_find(tlv_dir_entry,
					 TLV_ID_APPLICATION_PRIORITY_INDICATOR);
		if (tlv_cur) {
			size_t prio_len = sizeof(candidate->prio);

			rc = tlv_encode_value(tlv_cur, &candidate->prio,
								     &prio_len);
			if ((rc != TLV_RC_OK) || !prio_len) {
				rc = EMV_RC_CARD_PROTOCOL_ERROR;
				goto done;
			}

			if (!candidate->prio)
				candidate->prio = 15;
		} else {
			candidate->prio = 15;
		}

		tlv_cur = tlv_find(tlv_dir_entry, TLV_ID_KERNEL_IDENTIFIER);
		if (tlv_cur) {
			candidate->kernel_identifier_len =
					   sizeof(candidate->kernel_identifier);
			rc = tlv_encode_value(tlv_cur,
						   candidate->kernel_identifier,
					     &candidate->kernel_identifier_len);
			if (rc != TLV_RC_OK) {
				rc = EMV_RC_CARD_PROTOCOL_ERROR;
				goto done;
			}
		} else {
			candidate->kernel_identifier_len = 0;
		}

		tlv_cur = tlv_find(tlv_dir_entry, TLV_ID_EXTENDED_SELECTION);
		if (tlv_cur) {
			candidate->extended_selection_len =
					  sizeof(candidate->extended_selection);
			rc = tlv_encode_value(tlv_cur,
						  candidate->extended_selection,
					    &candidate->extended_selection_len);
			if (rc != TLV_RC_OK) {
				rc = EMV_RC_CARD_PROTOCOL_ERROR;
				goto done;
			}
		} else {
			candidate->extended_selection_len = 0;
		}

		num_candidates++;
	}

	if (!num_candidates) {
		rc = EMV_RC_CARD_PROTOCOL_ERROR;
		goto done;
	}

	for (i = 0; i < num_candidates; i++) {
		int j;

		for (j = 0; j < candidates[i].adf_name_len; j++)
			printf("%02X", candidates[i].adf_name[j]);

		printf(" ");

		for (j = 0; j < candidates[i].label_len; j++)
			printf("%c", candidates[i].label[j]);

		printf(" %u", (unsigned)candidates[i].prio);

		for (j = 0; j < candidates[i].kernel_identifier_len; j++)
			printf("%02X", candidates[i].kernel_identifier[j]);

		printf(" ");

		for (j = 0; j < candidates[i].extended_selection_len; j++)
			printf("%02X", candidates[i].extended_selection[j]);

		printf("\n");
	}

done:
	if (tlv_fci)
		tlv_free(tlv_fci);

	return rc;
}
