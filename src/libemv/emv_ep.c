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
#include <log4c.h>

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
	struct emv_ep_config_flags present;
	struct emv_ep_config_flags enabled;
	uint64_t		   reader_ctls_txn_limit;
	uint64_t		   reader_ctls_floor_limit;
	uint64_t		   terminal_floor_limit;
	uint64_t		   reader_cvm_reqd_limit;
	uint8_t			   ttq[4];
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

struct emv_ep_candidate_list {
	struct emv_ep_candidate *candidates;
	size_t			 size;
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

enum emv_ep_state {
	eps_preprocessing = 0,
	eps_protocol_activation,
	eps_combination_selection,
	eps_final_combination_selection,
	eps_kernel_activation,
	eps_outcome_processing,
	eps_done
};

struct emv_ep {
	enum emv_ep_state		state;
	struct emv_hal			*hal;
	log4c_category_t		*log_cat;
	struct emv_ep_combination_set	combination_set[num_txn_types];
	struct emv_ep_reg_kernel_set	reg_kernel_set;
	struct emv_ep_candidate_list	candidate_list;
	struct emv_kernel_parms		parms;
	struct emv_outcome_parms	outcome;
};

int emv_ep_register_kernel(struct emv_ep *ep, struct emv_kernel *kernel,
				 const uint8_t *kernel_id, size_t kernel_id_len)
{
	struct emv_ep_reg_kernel_set *set = &ep->reg_kernel_set;
	int rc = EMV_RC_OK;
	size_t i_kernel = set->size++;

	set->kernel = (struct emv_ep_reg_kernel *)realloc(set->kernel,
				  set->size * sizeof(struct emv_ep_reg_kernel));
	if (!set->kernel) {
		rc = EMV_RC_OUT_OF_MEMORY;
		goto done;
	}

	set->kernel[i_kernel].kernel = kernel;
	set->kernel[i_kernel].kernel_id_len = kernel_id_len;
	memcpy(set->kernel[i_kernel].kernel_id, kernel_id, kernel_id_len);

done:
	if (rc != EMV_RC_OK) {
		if (set->kernel) {
			free(set->kernel);
			set->kernel = NULL;
			set->size = 0;
		}
	}

	if (rc == EMV_RC_OK)
		log4c_category_log(ep->log_cat, LOG4C_PRIORITY_TRACE,
						  "%s('%s'): success", __func__,
				      tlv_bin_to_hex(kernel_id, kernel_id_len));
	else
		log4c_category_log(ep->log_cat, LOG4C_PRIORITY_WARN,
					   "%s('%s'): failed. rc %d.", __func__,
				  tlv_bin_to_hex(kernel_id, kernel_id_len), rc);

	return rc;
};

static struct emv_kernel *get_kernel(struct emv_ep *ep,
					   const uint8_t *kernel_id, size_t len)
{
	struct emv_kernel *kernel = NULL;
	size_t i_krn;

	for (i_krn = 0; i_krn < ep->reg_kernel_set.size; i_krn++) {
		if ((len == ep->reg_kernel_set.kernel[i_krn].kernel_id_len) &&
		    (!memcmp(ep->reg_kernel_set.kernel[i_krn].kernel_id,
							     kernel_id, len))) {
			kernel = ep->reg_kernel_set.kernel[i_krn].kernel;
			break;
		}
	}

	if (kernel) {
		log4c_category_log(ep->log_cat, LOG4C_PRIORITY_TRACE,
						  "%s('%s'): success", __func__,
						tlv_bin_to_hex(kernel_id, len));
	} else {
		log4c_category_log(ep->log_cat, LOG4C_PRIORITY_WARN,
						   "%s('%s'): failed", __func__,
						tlv_bin_to_hex(kernel_id, len));
	}

	return kernel;
}

static bool is_currency_code_supported(const uint8_t *currency_code)
{
	if (!memcmp(ISO4217_USD, currency_code, sizeof(ISO4217_USD)) ||
	    !memcmp(ISO4217_EUR, currency_code, sizeof(ISO4217_EUR)))
		return true;

	return false;
}

static uint64_t unit_of_currency(const uint8_t currency_code[2])
{
	return 100;
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

int emv_ep_preprocessing(struct emv_ep *ep)
{
	struct emv_ep_combination_set *combination_set = NULL;
	bool ctls_app_allowed = 0;
	int rc = EMV_RC_OK;
	int i = 0;

	log4c_category_log(ep->log_cat, LOG4C_PRIORITY_TRACE, "%s(): start",
								      __func__);

	assert(ep->parms.txn_type < num_txn_types);
	combination_set = &ep->combination_set[ep->parms.txn_type];

	if (!is_currency_code_supported(ep->parms.currency)) {
		rc = EMV_RC_UNSUPPORTED_CURRENCY_CODE;
		goto done;
	}

	for (i = 0; i < combination_set->size; i++) {
		struct emv_ep_combination *combination = NULL;
		struct emv_ep_preproc_indicators *indicators = NULL;
		struct emv_ep_config *cfg = NULL;

		combination = &combination_set->combinations[i];
		cfg	    = &combination->config;
		indicators  = &combination->indicators;


		REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.1.1.1");
		/* For each Combination, Entry Point shall reset the Entry Point
		 * Pre-Processing Indicators to 0.			      */
		memset(&combination->indicators, 0,
					       sizeof(combination->indicators));


		REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.1.1.2");
		/* If Terminal Transaction Qualifiers (TTQ) is part of the
		 * configuration data for a Combination, then:
		 *  - Entry Point shall copy the TTQ from Entry Point
		 *    Configuration Data into the Copy of TTQ in the Entry Point
		 *    Pre-Processing Indicators.
		 *  - Entry Point shall reset byte 2, bit 8 and bit 7 in the
		 *    Copy of TTQ to 00b ('Online cryptogram not required' and
		 *    'CVM not required').
		 * The other bits are unchanged.			      */
		if (cfg->present.ttq) {
			memcpy(indicators->ttq, cfg->ttq, sizeof(cfg->ttq));
			indicators->ttq[1] = indicators->ttq[1] &
					   ~(TTQ_B2_ONLINE_CRYPTOGRAM_REQUIRED |
							   TTQ_B2_CVM_REQUIRED);
		}


		REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.1.1.3");
		/* If all of the following are true:
		 *  - the Status Check Support flag is present,
		 *  - and the Status Check Support flag is 1,
		 *  - and the Amount, Authorised is a single unit of currency,
		 * then Entry Point shall set the 'Status Check Requested'
		 * indicator for the Combination to 1.			      */
		if (cfg->present.status_check_support &&
		    cfg->enabled.status_check_support &&
		    (ep->parms.amount_authorized ==
					  unit_of_currency(ep->parms.currency)))
			indicators->status_check_requested = true;


		REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.1.1.4");
		/* If the value of Amount, Authorised is zero, then:
		 *   - If the Zero Amount Allowed flag is present and the Zero
		 *     Amount Allowed flag is 0, then Entry Point shall set the
		 *     'Contactless Application Not Allowed' indicator for the
		 *     Combination to 1.
		 *   - Otherwise, Entry Point shall set the 'Zero Amount'
		 *     indicator for the Combination to 1.		      */
		if (!ep->parms.amount_authorized) {
			if (cfg->present.zero_amount_allowed &&
			    !cfg->enabled.zero_amount_allowed)
				indicators->ctls_app_not_allowed = true;
			else
				indicators->zero_amount = true;
		}


		REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.1.1.5");
		/* If the Reader Contactless Transaction Limit is present and
		 * the value of Amount, Authorised is greater than or equal to
		 * this limit, then Entry Point shall set the 'Contactless
		 * Application Not Allowed' indicator for the Combination to
		 * 1.							      */
		if (cfg->present.reader_ctls_txn_limit &&
		    (ep->parms.amount_authorized >= cfg->reader_ctls_txn_limit))
			indicators->ctls_app_not_allowed = true;


		REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.1.1.6");
		/* If the Reader Contactless Floor Limit is present and the
		 * value of Amount, Authorised is greater than this limit,
		 * then Entry Point shall set the 'Reader Contactless Floor
		 * Limit Exceeded' indicator for the Combination to 1.	      */
		if (cfg->present.reader_ctls_floor_limit &&
		    (ep->parms.amount_authorized >
						  cfg->reader_ctls_floor_limit))
			indicators->floor_limit_exceeded = true;


		REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.1.1.7");
		/* If all of the following are true:
		 *   - the Reader Contactless Floor Limit is not present,
		 *   - and the Terminal Floor Limit (Tag '9F1B') is present,
		 *   - and the value of Amount, Authorised is greater than the
		 *     Terminal Floor Limit (Tag '9F1B'),
		 * then Entry Point shall set the 'Reader Contactless Floor
		 * Limit Exceeded' indicator for the Combination to 1.	      */
		if (!cfg->present.reader_ctls_floor_limit &&
		    cfg->present.terminal_floor_limit &&
		    (ep->parms.amount_authorized > cfg->terminal_floor_limit))
			indicators->floor_limit_exceeded = true;


		REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.1.1.8");
		/* If the Reader CVM Required Limit is present and the value of
		 * Amount, Authorised is greater than or equal to this limit,
		 * then Entry Point shall set the 'Reader CVM Required Limit
		 * Exceeded' indicator for the Combination to 1.	      */
		if (cfg->present.reader_cvm_reqd_limit &&
		    (ep->parms.amount_authorized >= cfg->reader_cvm_reqd_limit))
			indicators->cvm_reqd_limit_exceeded = true;


		if (cfg->present.ttq) {


			REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.1.1.9");
			/* If the 'Reader Contactless Floor Limit Exceeded'
			 * indicator is 1, then Entry Point shall set byte 2,
			 * bit 8 in the Copy of TTQ for the Combination to 1b
			 * ('Online cryptogram required').		      */
			if (indicators->floor_limit_exceeded)
				indicators->ttq[1] |=
					      TTQ_B2_ONLINE_CRYPTOGRAM_REQUIRED;


			REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.1.1.10");
			/* If the 'Status Check Requested' indicator is 1, then
			 * Entry Point shall set byte 2, bit 8 in the Copy of
			 * TTQ for the Combination to 1b ('Online cryptogram
			 * required').					      */
			if (indicators->status_check_requested)
				indicators->ttq[1] |=
					      TTQ_B2_ONLINE_CRYPTOGRAM_REQUIRED;


			REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.1.1.11");
			/* If the 'Zero Amount' indicator is 1, then:
			 *   - If byte 1, bit 4 of the Copy of TTQ is 0b
			 *     ('Online capable reader'), then Entry Point shall
			 *     set byte 2, bit 8 in the Copy of TTQ for the
			 *     Combination to 1b ('Online cryptogram required').
			 *   - Otherwise (byte 1 bit 4 of the Copy of TTQ is 1b
			 *     ('Offline-only reader')), Entry Point shall set
			 *     the 'Contactless Application Not Allowed'
			 *     indicator for the Combination to 1.	      */
			if (indicators->zero_amount) {
				if (indicators->ttq[0] &
						     TTQ_B1_OFFLINE_ONLY_READER)
					indicators->ctls_app_not_allowed = true;
				else
					indicators->ttq[1] |=
					      TTQ_B2_ONLINE_CRYPTOGRAM_REQUIRED;
			}


			REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.1.1.12");
			/* If the 'Reader CVM Required Limit Exceeded' indicator
			 * is 1, then Entry Point shall set byte 2, bit 7 in the
			 * Copy of TTQ for the Combination to 1b ('CVM
			 * required').					      */
			if (indicators->cvm_reqd_limit_exceeded)
				indicators->ttq[1] |= TTQ_B2_CVM_REQUIRED;
		}


		if (!indicators->ctls_app_not_allowed)
			ctls_app_allowed = true;
	}


	REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.1.1.13");
	/* If, for all the Combinations, the 'Contactless Application Not
	 * Allowed' indicator is 1, then Entry Point shall provide a Try Another
	 * Interface Outcome with the following Outcome parameter values and
	 * shall continue with Outcome Processing, section 3.5.
	 *
	 * Try Another Interface:
	 *   - Start: N/A
	 *   - Online Response Data: N/A
	 *   - CVM: N/A
	 *   - UI Request on Outcome Present: Yes
	 *     - Message Identifier: '18' ("Please Insert or Swipe Card")
	 *     - Status: Processing Error
	 *   - UI Request on Restart Present: No
	 *   - Data Record Present: No
	 *   - Discretionary Data Present: No
	 *   - Alternate Interface Preference: N/A
	 *   - Receipt: N/A
	 *   - Field Off Request: N/A
	 *   - Removal Timeout: Zero
	 *
	 * Otherwise (at least one Combination is allowed) Entry Point shall
	 * retain the Entry Point Pre-Processing Indicators for each allowed
	 * Combination.							      */
	if (!ctls_app_allowed) {
		struct emv_ui_request *ui_req = NULL;

		memset(&ep->outcome, 0, sizeof(ep->outcome));
		ep->outcome.outcome = out_try_another_interface;
		ep->outcome.ui_request_on_outcome_present = true;
		ui_req = &ep->outcome.ui_request_on_outcome;
		ui_req->msg_id = msg_insert_or_swipe_card;
		ui_req->status = sts_processing_error;

		ep->state = eps_outcome_processing;
	} else {
		ep->state = eps_protocol_activation;
	}

done:
	if (rc == EMV_RC_OK)
		log4c_category_log(ep->log_cat, LOG4C_PRIORITY_TRACE,
						     "%s(): success", __func__);
	else
		log4c_category_log(ep->log_cat, LOG4C_PRIORITY_WARN,
					  "%s(): failed. rc %d.", __func__, rc);
	return rc;
}

int emv_ep_protocol_activation(struct emv_ep *ep, bool started_by_reader)
{
	bool collision = false;
	int rc = EMV_RC_OK;

	log4c_category_log(ep->log_cat, LOG4C_PRIORITY_TRACE, "%s(): start",
								      __func__);

	REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.2.1.1");
	/* If the Restart flag is 0, then:
	 *   - If Entry Point is activated by the reader 4 at Start B, then:
	 *     - For each Combination, Entry Point shall reset the Entry Point
	 *       Pre-Processing Indicators to 0.
	 *     - For each Combination, if Terminal Transaction Qualifiers (TTQ)
	 *       is configured, then Entry Point shall copy the TTQ from Entry
	 *       Point Configuration Data into the Copy of TTQ in the Entry
	 *       Point Pre-Processing Indicators.
	 *   - Entry Point shall clear the Candidate List.
	 */
	if (!ep->parms.restart) {
		if (started_by_reader) {
			struct emv_ep_combination_set *combination_set = NULL;
			int i = 0;

			combination_set =
				       &ep->combination_set[ep->parms.txn_type];

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

		if (ep->candidate_list.candidates)
			free(ep->candidate_list.candidates);
		ep->candidate_list.candidates = NULL;
		ep->candidate_list.size = 0;
	}


	REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.2.1.2");
	/* If the Restart flag is 1, and the value of the retained UI Request on
	 * Restart Present parameter is 'Yes', then Entry Point shall send the
	 * retained User Interface Request.
	 * Otherwise (the Restart flag is 0 or the value of the retained UI
	 * Request on Restart Present parameter is 'No'), Entry Point shall send
	 * a User Interface Request with the following parameters:
	 *   - Message Identifier: '15' (“Present Card”)
	 *   - Status: Ready to Read
	 */
	if (ep->parms.restart && ep->outcome.ui_request_on_restart_present) {
		ep->hal->ops->ui_request(ep->hal,
					    &ep->outcome.ui_request_on_restart);
	} else {
		struct emv_ui_request ui_request;

		memset(&ui_request, 0, sizeof(ui_request));
		ui_request.msg_id = msg_present_card;
		ui_request.status = sts_ready_to_read;

		ep->hal->ops->ui_request(ep->hal, &ui_request);
	}


	REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.2.1.3");
	/* The field shall be powered up and polling performed as defined in the
	 * Main Loop of Book D.						      */
	rc = ep->hal->ops->start_polling(ep->hal);
	if (rc != EMV_RC_OK)
		goto done;

	do {
		rc = ep->hal->ops->wait_for_card(ep->hal);


		REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.2.1.4");
		/* If a collision as defined in Book D is reported to Entry
		 * Point, then Entry Point shall send a User Interface Request
		 * with the following parameters:
		 *   - Message Identifier: '19' ("Please Present One Card Only")
		 *   - Status: Contactless collision detected (Processing Error)
		 */
		if (rc == EMV_RC_COLLISION) {
			struct emv_ui_request ui_request;

			memset(&ui_request, 0, sizeof(ui_request));
			ui_request.msg_id = msg_present_one_card_only;
			ui_request.status = sts_processing_error;

			ep->hal->ops->ui_request(ep->hal, &ui_request);

			collision = true;
		}


		REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.2.1.5");
		/* When the collision condition is no longer indicated, then
		 * Entry Point shall send a User Interface Request with the
		 * following parameters:
		 *   - Message Identifier: '19' ("Please Present One Card Only")
		 *   - Status: Ready to Read				      */
		if ((rc == EMV_RC_CONTINUE) && (collision)) {
			struct emv_ui_request ui_request;

			memset(&ui_request, 0, sizeof(ui_request));
			ui_request.msg_id = msg_present_one_card_only;
			ui_request.status = sts_ready_to_read;

			ep->hal->ops->ui_request(ep->hal, &ui_request);

			collision = false;
		}
	} while ((rc == EMV_RC_COLLISION) || (rc == EMV_RC_CONTINUE));

	if (rc != EMV_RC_OK)
		goto done;


	REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.2.1.6");
	/* As described in Book D requirement 6.4.1.12, Entry Point shall not
	 * use a higher layer command in the Higher layer - INF field of the
	 * ATTRIB command.						      */
	/* Hm, would not even know how to do this. This is L1 stuff, right?   */

	ep->state = eps_combination_selection;

done:
	if (rc == EMV_RC_OK)
		log4c_category_log(ep->log_cat, LOG4C_PRIORITY_TRACE,
						     "%s(): success", __func__);
	else
		log4c_category_log(ep->log_cat, LOG4C_PRIORITY_WARN,
					  "%s(): failed. rc %d.", __func__, rc);
	return rc;
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

static int emv_ep_parse_ppse(struct emv_ep *ep, const void *fci, size_t fci_len,
			    struct ppse_dir_entry *entries, size_t *num_entries)
{
	struct tlv *ppse = NULL, *i_tlv = NULL;
	size_t num = 0;
	int rc = EMV_RC_OK;

	assert(fci);
	assert(entries);
	assert(num_entries);

	log4c_category_log(ep->log_cat, LOG4C_PRIORITY_TRACE, "%s(fci: '%s')",
					__func__, tlv_bin_to_hex(fci, fci_len));

	rc = tlv_parse(fci, fci_len, &ppse);
	if (rc != TLV_RC_OK) {
		log4c_category_log(ep->log_cat, LOG4C_PRIORITY_NOTICE,
			      "%s(): Failed to parse 2PAY.SYS. rc %d", __func__,
									    rc);
		rc = EMV_RC_CARD_PROTOCOL_ERROR;
		goto done;
	}

	i_tlv = tlv_find(tlv_get_child(tlv_find(tlv_get_child(tlv_find(
			     tlv_get_child(tlv_find(ppse, EMV_ID_FCI_TEMPLATE)),
					      EMV_ID_FCI_PROPRIETARY_TEMPLATE)),
					 EMV_ID_FCI_ISSUER_DISCRETIONARY_DATA)),
							EMV_ID_DIRECTORY_ENTRY);

	for (num = 0;
	     i_tlv && (num < *num_entries);
	     i_tlv = tlv_find(tlv_get_next(i_tlv), EMV_ID_DIRECTORY_ENTRY)) {
		struct tlv *adf_name, *label, *prio, *kernel_id, *ext_sel;
		struct tlv *entry;
		struct ppse_dir_entry *dir_entry;
		int rc = TLV_RC_OK;

		entry	  = tlv_get_child(i_tlv);
		adf_name  = tlv_find(entry, EMV_ID_ADF_NAME);
		label	  = tlv_find(entry, EMV_ID_APPLICATION_LABEL);
		kernel_id = tlv_find(entry, EMV_ID_KERNEL_IDENTIFIER);
		ext_sel	  = tlv_find(entry, EMV_ID_EXTENDED_SELECTION);
		prio      = tlv_find(entry,
					 EMV_ID_APPLICATION_PRIORITY_INDICATOR);

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

	if (ppse)
		tlv_free(ppse);

	if ((num == *num_entries) && (i_tlv))
		return EMV_RC_OVERFLOW;

	*num_entries = num;

done:
	return rc;
}

static uint8_t rid_to_kernel_id(const void *rid)
{
	uint8_t rids[][5] = {
		{ 0xA0, 0x00, 0x00, 0x00, 0x04 },	/* MasterCard	      */
		{ 0xA0, 0x00, 0x00, 0x00, 0x03 },	/* Visa		      */
		{ 0xA0, 0x00, 0x00, 0x00, 0x25 },	/* American Express   */
		{ 0xA0, 0x00, 0x00, 0x00, 0x65 },	/* JCB		      */
		{ 0xA0, 0x00, 0x00, 0x00, 0x15 },	/* Discover	      */
		{ 0xA0, 0x00, 0x00, 0x03, 0x33 }	/* UnionPay	      */
	};
	int i = 0;

	for (i = 0; i < ARRAY_SIZE(rid); i++)
		if (!memcmp(rids[i], rid, sizeof(rids[i])))
			return (uint8_t)(i + 2);

	return 0;
}

bool emv_ep_is_combination_candidate(struct emv_ep_combination *combination,
					       struct ppse_dir_entry *dir_entry)

{
	uint8_t requested_kernel_id[8];
	size_t requested_kernel_id_len;
	bool is_candidate = false;


	REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.3.2.5 A");
	/* Entry Point shall examine the format of the ADF Name of the Directory
	 * Entry. If the ADF Name is missing or is not coded according to
	 * [EMV 4.2 Book 1], section 12.2.1, then Entry Point shall proceed with
	 * the next Directory Entry.					      */
	if (dir_entry->adf_name_len < 5)
		goto done;


	REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.3.2.5 B");
	/* Entry Point shall examine whether the ADF Name matches the AID of the
	 * reader Combination. If the ADF Name has the same length and value as
	 * the AID (full match), or the ADF Name begins with the AID (partial
	 * match), then the ADF Name matches the AID and the AID is referred to
	 * as the "matching AID". Otherwise Entry Point shall return to bullet A
	 * and proceed with the next Directory Entry.			      */
	if ((dir_entry->adf_name_len < combination->aid_len) ||
	    memcmp(combination->aid, dir_entry->adf_name, combination->aid_len))
		goto done;


	REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.3.2.5 C");
	/* Entry Point shall examine the presence and format of the Kernel
	 * Identifier (Tag '9F2A') to determine the Requested Kernel ID.      */

	if (dir_entry->kernel_identifier_len == 0) {
		/* If the Kernel Identifier (Tag '9F2A') is absent in the
		 * Directory Entry, then Entry Point shall use a default value
		 * for the Requested Kernel ID, based on the matching AID, as
		 * indicated in Table 3-6.				      */
		/* If the length of the Kernel Identifier value field is zero,
		 * then Entry Point shall use a default value for the Requested
		 * Kernel ID, based on the matching AID, as indicated in Table
		 * 3-6.							      */
		requested_kernel_id[0] = rid_to_kernel_id(combination->aid);
		requested_kernel_id_len = 1;
	} else {
		/* If the Kernel Identifier (Tag '9F2A') is present in the
		 * Directory Entry, then Entry Point shall examine the value
		 * field as follows:					     */
		if (((dir_entry->kernel_identifier[0] & 0xc0) == 0x00) ||
			   ((dir_entry->kernel_identifier[0] & 0xc0) == 0x40)) {
			/* If byte 1, b8 and b7 of the Kernel Identifier have
			 * the value 00b or 01b 12, then Requested Kernel ID is
			 * equal to the value of byte 1 of the Kernel Identifier
			 * (i.e. b8b7||Short Kernel ID).		      */
			*requested_kernel_id = dir_entry->kernel_identifier[0];
			requested_kernel_id_len = 1;
		} else {
			/* If byte 1, b8 and b7 of the Kernel Identifier have
			 * the value 10b or 11b, then			      */
			if (dir_entry->kernel_identifier_len < 3) {
				/* If the length of the Kernel Identifier value
				 * field is less than 3 bytes, then Entry Point
				 * shall return to bullet A and proceed with the
				 * next Directory Entry.		      */
				goto done;
			} else if (dir_entry->kernel_identifier[0] & 0x3f) {
				/* If the Short Kernel ID is different from
				 * 000000b, then the Requested Kernel ID is
				 * equal to value of the byte 1 to byte 3 of the
				 * Kernel Identifier (i.e.
				 * b8b7||Short Kernel ID||Extended Kernel ID).*/
				memcpy(requested_kernel_id,
					       dir_entry->kernel_identifier, 3);
				requested_kernel_id_len = 3;
			} else {
				/* If the Short Kernel ID is equal to 000000b,
				 * then the determination of the Requested
				 * Kernel ID is out of scope of this
				 * specification.			      */
				memcpy(requested_kernel_id,
						   dir_entry->kernel_identifier,
					      dir_entry->kernel_identifier_len);
				requested_kernel_id_len =
					       dir_entry->kernel_identifier_len;
			}
		}
	}


	REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.3.2.5 D");
	/* Entry Point shall examine whether the Requested Kernel ID is
	 * supported for the reader Combination.
	 *   - If the value of the Requested Kernel ID is zero, then the kernel
	 *     requested by the card is supported by the reader;
	 *   - If the value of the Requested Kernel ID is non-zero and the value
	 *     of the Requested Kernel ID is equal to the value of the Kernel
	 *     ID, then the kernel requested by the card is supported by the
	 *     reader;
	 * Otherwise Entry Point shall return to bullet A and proceed with the
	 * next Directory Entry.					      */
	if ((requested_kernel_id[0] == 0) ||
	    ((requested_kernel_id_len == combination->kernel_id_len) &&
	     (!memcmp(requested_kernel_id, combination->kernel_id,
						     requested_kernel_id_len))))
		is_candidate = true;

done:
	return is_candidate;
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

	log4c_category_log(ep->log_cat, LOG4C_PRIORITY_TRACE, "%s(): start",
								      __func__);

	combination_set = &ep->combination_set[ep->parms.txn_type];


	/* Step 1 */

	REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.3.2.2");
	/* Entry Point shall send a SELECT (PPSE) command (as described in
	 * [EMV 4.2 Book 1], section 11.3.2) to the card, with a file name of
	 * '2PAY.SYS.DDF01'.						     */
	rc = emv_transceive_apdu(ep->hal, EMV_CMD_SELECT_CLA,
				  EMV_CMD_SELECT_INS, EMV_CMD_SELECT_P1_BY_NAME,
				      EMV_CMD_SELECT_P2_FIRST, "2PAY.SYS.DDF01",
							 14, fci, &fci_len, sw);


	REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.3.3.7");
	/* If at any time during Protocol Activation or Combination Selection
	 * a communications error as defined in Book D (Transmission, Protocol,
	 * or Time-out) is reported to Entry Point, then Entry Point shall
	 * return to Start B (Protocol Activation, section 3.2.1).	      */
	if (rc != EMV_RC_OK) {
		ep->state = eps_protocol_activation;
		goto done;
	}


	REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.3.2.3");
	/* If Entry Point receives SW1 SW2 = '9000' in response to the SELECT
	 * (PPSE) command, then Entry Point shall proceed to Step 2.
	 * Otherwise, Entry Point shall add no Combinations to the Candidate
	 * List and shall proceed to Step 3.				      */
	if ((sw[0] != 0x90) || (sw[1] != 0x00)) {
		log4c_category_log(ep->log_cat, LOG4C_PRIORITY_NOTICE,
				 "%s(): Select 2PAY.SYS sw: %02x%02x", __func__,
								  sw[0], sw[1]);
		ep->state = eps_final_combination_selection;
		goto done;
	}


	rc = emv_ep_parse_ppse(ep, fci, fci_len, dir_entry, &num_dir_entries);
	assert(rc == EMV_RC_OK);


	REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.3.2.4");
	/* If there is no Directory Entry (Tag '61') in the FCI, then
	 * Entry Point shall add no Combinations to the Candidate List
	 * and shall proceed to Step 3. */
	if (!num_dir_entries) {
		log4c_category_log(ep->log_cat, LOG4C_PRIORITY_NOTICE,
				"%s(): No entries in 2PAY.SYS found", __func__);
		ep->state = eps_final_combination_selection;
		goto done;
	}

	ep->candidate_list.size = 0;
	ep->candidate_list.candidates = (struct emv_ep_candidate *)
				 calloc(combination_set->size * num_dir_entries,
					       sizeof(struct emv_ep_candidate));
	if (!ep->candidate_list.candidates) {
		rc = EMV_RC_OUT_OF_MEMORY;
		goto done;
	}

	log4c_category_log(ep->log_cat, LOG4C_PRIORITY_TRACE,
		       "%s(): Combinations: %d, 2PAY.SYS entries: %d", __func__,
					combination_set->size, num_dir_entries);

	REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.3.2.5");
	/* For each reader Combination {AID - Kernel ID} supported by the reader
	 * for which the 'Contactless Application Not Allowed' indicator is 0,
	 * Entry Point shall process each Directory Entry (Tag '61') from th
	 * FCI. When the Directory Entries have been processed for all supported
	 * reader Combinations, Entry Point shall proceed to Step 3.
	 *
	 * To process the Directory Entries, Entry Point shall begin with the
	 * first Directory Entry of the FCI and process sequentially for each
	 * Directory Entry in the FCI as described in bullet A thru E below.  */
	for (i_comb = 0; i_comb < combination_set->size; i_comb++) {
		struct emv_ep_combination *comb =
					 &combination_set->combinations[i_comb];

		if (comb->indicators.ctls_app_not_allowed)
			continue;

		for (i_dir = 0; i_dir < num_dir_entries; i_dir++) {
			struct ppse_dir_entry *entry = &dir_entry[i_dir];
			struct emv_ep_candidate_list *list = NULL;
			struct emv_ep_candidate *candidate = NULL;

			if (!emv_ep_is_combination_candidate(comb, entry))
				continue;


			REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.3.2.5 E");
			/* Entry Point shall add a Combination to the Candidate
			 * List for final selection, consisting of:
			 *   - the ADF Name
			 *   - the AID
			 *   - the Kernel ID
			 *   - the Application Priority Indicator (if present)
			 *   - the Extended Selection (if present)	      */
			list = &ep->candidate_list;
			candidate = &list->candidates[list->size++];
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

	qsort(ep->candidate_list.candidates, ep->candidate_list.size,
			   sizeof(struct emv_ep_candidate), compare_candidates);

	ep->state = eps_final_combination_selection;

done:
	if (rc == EMV_RC_OK)
		log4c_category_log(ep->log_cat, LOG4C_PRIORITY_TRACE,
				       "%s(): success. Number of candiates: %d",
					     __func__, ep->candidate_list.size);
	else
		log4c_category_log(ep->log_cat, LOG4C_PRIORITY_WARN,
					  "%s(): failed. rc %d.", __func__, rc);
	return rc;
}

int emv_ep_final_combination_selection(struct emv_ep *ep)
{
	struct emv_ep_candidate *candidate = NULL;
	struct emv_ep_config *config = NULL;
	uint8_t adf[32];
	size_t adf_len = 0;
	int rc = EMV_RC_OK;

	/* Step 3 */

	if (!ep->candidate_list.size) {


		REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.3.2.7");
		/* If the Candidate List is empty, then Entry Point shall send
		 * an End Application Outcome with the following Outcome
		 * parameter values and shall continue with Outcome Processing,
		 * section 3.5.
		 *
		 * End Application:
		 *   - Start: N/A
		 *   - Online Response Data: N/A
		 *   - CVM: N/A
		 *   - UI Request on Outcome Present: Yes
		 *     - Message Identifier: '1C' ("Insert, Swipe or Try Another
		 *       Card")
		 *   - Status: Ready To Read
		 *   - UI Request on Restart Present: No
		 *   - Data Record Present: No
		 *   - Discretionary Data Present: No
		 *   - Alternate Interface Preference: N/A
		 *   - Receipt: N/A
		 *   - Field Off Request: N/A
		 *   - Removal Timeout: Zero				      */
		struct emv_ui_request *ui_req = NULL;

		memset(&ep->outcome, 0, sizeof(ep->outcome));
		ep->outcome.outcome = out_end_application;
		ep->outcome.ui_request_on_outcome_present = true;
		ui_req = &ep->outcome.ui_request_on_outcome;
		ui_req->msg_id = msg_try_another_card;
		ui_req->status = sts_ready_to_read;

		ep->state = eps_outcome_processing;

		return EMV_RC_OK;
	}


	REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.3.2.6");
	/* If the Candidate List contains at least one entry, then Entry Point
	 * shall retain the Candidate List and shall continue with Final
	 * Combination Selection, section 3.3.3.			      */


	REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.3.3.1");
	/* If there is only one Combination in the Candidate List, then Entry
	 * Point shall select the Combination.				      */



	REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.3.3.2");
	/* If there are multiple Combinations in the Candidate List, then Entry
	 * Point shall select the Combination as follows:
	 *   - Consider each Combination that has an Application Priority
	 *     Indicator with a value of 0 or no Application Priority Indicator
	 *     to be of equal lowest priority.
	 *   - If a single Combination has a higher priority than any other
	 *     Combination in the Candidate List, then select that Combination.
	 *   - Otherwise multiple Combinations in the Candidate List have the
	 *     highest priority, and Entry Point shall select a Combination as
	 *     as follows:
	 *     - Determine the order of these Combinations' ADF Names and Kernel
	 *       IDs in the PPSE, where the order is the position in the PPSE,
	 *       with the lowest order being the first.
	 *     - Select any one of the Combinations that have the lowest order*/
	candidate = &ep->candidate_list.candidates[ep->candidate_list.size - 1];
	config = &candidate->combination->config;

	adf_len = candidate->adf_name_len;
	memcpy(adf, candidate->adf_name, adf_len);


	REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.3.3.3");
	/* If all of the following are true:
	 *   - the Extended Selection data element (Tag '9F29') is present in
	 *     the Combination selected,
	 *   - and the Extended Selection Support flag is present for this
	 *     Combination,
	 *   - and the Extended Selection Support flag is 1,
	 * then Entry Point shall append the value contained in Extended
	 * Selection to the ADF Name in the data field of the SELECT command. */
	if (candidate->extended_selection_len &&
					config->present.ext_selection_support &&
					config->enabled.ext_selection_support) {
		memcpy(&adf[adf_len], candidate->extended_selection,
					     candidate->extended_selection_len);
		adf_len += candidate->extended_selection_len;
	}



	REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.3.3.4");
	/* Entry Point shall send the SELECT (AID) command with the ADF Name of
	 * the selected Combination (with Extended Selection if appended).    */
	ep->parms.fci_len = sizeof(ep->parms.fci);
	rc = emv_transceive_apdu(ep->hal, EMV_CMD_SELECT_CLA,
				  EMV_CMD_SELECT_INS, EMV_CMD_SELECT_P1_BY_NAME,
			   EMV_CMD_SELECT_P2_FIRST, adf, adf_len, ep->parms.fci,
					      &ep->parms.fci_len, ep->parms.sw);


	REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.3.3.7");
	/* If at any time during Protocol Activation or Combination Selection
	 * a communications error as defined in Book D (Transmission, Protocol,
	 * or Time-out) is reported to Entry Point, then Entry Point shall
	 * return to Start B (Protocol Activation, section 3.2.1).	      */
	if (rc != EMV_RC_OK) {
		ep->state = eps_protocol_activation;
		return EMV_RC_OK;
	}



	REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.3.3.5");
	/* If the response to the SELECT (AID) command includes an SW1 SW2 other
	 * than '9000', then:
	 *   - FIXME: If issuer [...] data is present [...]
	 *   - Otherwise Entry Point shall remove the selected Combination from
	 *     the Candidate List and shall return to Start C (Step 3 of
	 *     Combination Selection (requirement 3.3.2.6)).		      */
	if ((ep->parms.sw[0] != 0x90) || (ep->parms.sw[1] != 0x00)) {
		ep->candidate_list.size--;
		if (!ep->candidate_list.size) {
			free(ep->candidate_list.candidates);
			ep->candidate_list.candidates = NULL;
		}
		ep->state = eps_final_combination_selection;
		return EMV_RC_OK;
	}



	REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.3.3.6");
	/* If all of the following are true:
	 *   - the selected AID indicates Visa AID,
	 *   - and the kernel in the selected Combination is Kernel 3,
	 *   - and the PDOL in the FCI is absent or the PDOL in the FCI does not
	 *     include Tag '9F66',
	 * then:
	 *   - If Kernel 1 is supported, then Entry Point shall change the
	 *     Kernel ID for this AID from the initial Kernel 3 to Kernel 1.
	 *   - If Kernel 1 is not supported, then Entry Point shall remove the
	 *     selected Combination from the Candidate List and shall return to
	 *     Start C (Step 3 of Combination Selection (requirement 3.3.2.6))*/
	/* FIXME */

	ep->state = eps_kernel_activation;
	return EMV_RC_OK;
}

int emv_ep_outcome_processing(struct emv_ep *ep)
{
	ep->state = eps_done;
	return EMV_RC_OK;
}

int emv_ep_kernel_activation(struct emv_ep *ep)
{
	struct emv_kernel *kernel = NULL;
	struct emv_ep_candidate *candidate = NULL;
	int rc = EMV_RC_OK;

	log4c_category_log(ep->log_cat, LOG4C_PRIORITY_TRACE, "%s(): start",
								      __func__);

	candidate = &ep->candidate_list.candidates[ep->candidate_list.size - 1];
	ep->parms.kernel_id_len = candidate->combination->kernel_id_len;
	memcpy(ep->parms.kernel_id, candidate->combination->kernel_id,
						       ep->parms.kernel_id_len);
	ep->parms.preproc_indicators = &candidate->combination->indicators;

	REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.4.1.1");
	/* Entry Point shall activate the kernel identified in the selected
	 * Combination.							      */
	kernel = get_kernel(ep, ep->parms.kernel_id, ep->parms.kernel_id_len);
	if (!kernel) {
		rc = EMV_RC_NO_KERNEL;
		goto done;
	}


	REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.4.1.2");
	/* Entry Point shall make the Entry Point Pre-Processing Indicators (as
	 * specified in Book A, Table 5-3) for the selected Combination
	 * available to the selected kernel.				      */
	REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.4.1.3");
	/* Entry Point shall make available the FCI and the Status Word SW1 SW2
	 * (both received from the card in the SELECT (AID) response) to the
	 * selected kernel. This requirement does not apply if Entry Point is
	 * restarted at Start D after Outcome Processing.		      */
	rc = kernel->ops->activate(kernel, ep->hal, &ep->parms, NULL, NULL,
									  NULL);
	ep->state = eps_outcome_processing;

done:
	if (rc == EMV_RC_OK)
		log4c_category_log(ep->log_cat, LOG4C_PRIORITY_TRACE,
						     "%s(): success", __func__);
	else
		log4c_category_log(ep->log_cat, LOG4C_PRIORITY_WARN,
					  "%s(): failed. rc %d.", __func__, rc);
	return rc;
}

int emv_ep_activate(struct emv_ep *ep, enum emv_start start,
			 enum emv_txn_type txn_type, uint64_t amount_authorized,
			       uint64_t amount_other, const uint8_t currency[2],
						  uint32_t unpredictable_number)
{
	bool started_at_b = (start == start_b);
	int rc = EMV_RC_OK;

	log4c_category_log(ep->log_cat, LOG4C_PRIORITY_TRACE, "%s(): start",
								      __func__);

	switch (start) {
	case start_a:
		ep->state = eps_preprocessing;
		break;
	case start_b:
		ep->state = eps_protocol_activation;
		break;
	case start_d:
		ep->state = eps_kernel_activation;
		break;
	case start_c:
	default:
		rc = EMV_RC_INVALID_ARG;
		goto done;
	}

	ep->parms.start		       = start;
	ep->parms.txn_type	       = txn_type;
	ep->parms.amount_authorized    = amount_authorized;
	ep->parms.amount_other	       = amount_other;
	ep->parms.unpredictable_number = unpredictable_number;
	memcpy(ep->parms.currency, currency, sizeof(ep->parms.currency));

	do {
		switch (ep->state) {

		case eps_preprocessing:
			rc = emv_ep_preprocessing(ep);
			break;

		case eps_protocol_activation:
			rc = emv_ep_protocol_activation(ep, started_at_b);
			started_at_b = false;
			break;

		case eps_combination_selection:
			rc = emv_ep_combination_selection(ep);
			break;

		case eps_final_combination_selection:
			rc = emv_ep_final_combination_selection(ep);
			break;

		case eps_kernel_activation:
			rc = emv_ep_kernel_activation(ep);
			break;

		case eps_outcome_processing:
			rc = emv_ep_outcome_processing(ep);
			break;

		case eps_done:
			break;

		default:
			assert(false);
		}
	} while ((rc == EMV_RC_OK) && (ep->state != eps_done));

done:
	if (rc == EMV_RC_OK)
		log4c_category_log(ep->log_cat, LOG4C_PRIORITY_TRACE,
						     "%s(): success", __func__);
	else
		log4c_category_log(ep->log_cat, LOG4C_PRIORITY_WARN,
					  "%s(): failed. rc %d.", __func__, rc);
	return rc;
}

static int parse_combination(struct tlv *tlv_combination,
	       const struct emv_ep_config *cfg, struct emv_ep_combination *comb)
{
	int rc = EMV_RC_OK;
	struct tlv *tlv_attr = NULL;

	memcpy(&comb->config, cfg, sizeof(struct emv_ep_config));

	for (tlv_attr = tlv_get_child(tlv_combination);
	     tlv_attr;
	     tlv_attr = tlv_get_next(tlv_attr)) {
		uint8_t tag[4];
		size_t tag_size = sizeof(tag);

		rc = tlv_encode_identifier(tlv_attr, tag, &tag_size);
		if ((rc != TLV_RC_OK) || (tag_size != sizeof(tag)))
			return EMV_RC_SYNTAX_ERROR;

		if (!memcmp(tag, EMV_ID_LIBEMV_AID, tag_size)) {
			comb->aid_len = sizeof(comb->aid);
			rc = tlv_encode_value(tlv_attr, comb->aid,
								&comb->aid_len);
			if (rc != TLV_RC_OK)
				return EMV_RC_SYNTAX_ERROR;

			continue;
		}

		if (!memcmp(tag, EMV_ID_LIBEMV_KERNEL_ID, tag_size)) {
			comb->kernel_id_len = sizeof(comb->kernel_id);
			rc = tlv_encode_value(tlv_attr, comb->kernel_id,
							  &comb->kernel_id_len);
			if (rc != TLV_RC_OK)
				return EMV_RC_SYNTAX_ERROR;

			continue;
		}
	}

	return EMV_RC_OK;
}

static int parse_combination_set(struct tlv *tlv_set,
	    const struct emv_ep_config *cfg, struct emv_ep_combination_set *set)
{
	struct tlv *tlv_comb = NULL;
	int rc = EMV_RC_OK;

	for (tlv_comb = tlv_find(tlv_get_child(tlv_set),
						     EMV_ID_LIBEMV_COMBINATION);
	     tlv_comb;
	     tlv_comb = tlv_find(tlv_get_next(tlv_comb),
						   EMV_ID_LIBEMV_COMBINATION)) {

		set->combinations = (struct emv_ep_combination *)realloc(
							      set->combinations,
			   sizeof(struct emv_ep_combination) * (set->size + 1));

		rc = parse_combination(tlv_comb, cfg,
						 &set->combinations[set->size]);
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

static int parse_emv_ep_config(struct tlv *tlv_set, struct emv_ep_config *cfg)
{
	struct tlv *tlv = NULL;
	int rc = EMV_RC_OK;

	for (tlv = tlv_get_child(tlv_set); tlv; tlv = tlv_get_next(tlv)) {
		uint8_t tag[4];
		size_t tag_size = sizeof(tag);

		rc = tlv_encode_identifier(tlv, tag, &tag_size);
		if ((rc != TLV_RC_OK) || (tag_size != sizeof(tag)))
			return EMV_RC_SYNTAX_ERROR;

		if (!memcmp(tag, EMV_ID_LIBEMV_STATUS_CHECK_SUPPORTED,
								    tag_size)) {
			uint8_t enabled;
			size_t enabled_sz = sizeof(enabled);

			rc = tlv_encode_value(tlv, &enabled, &enabled_sz);
			if ((rc != TLV_RC_OK) || (!enabled_sz))
				return EMV_RC_SYNTAX_ERROR;

			cfg->present.status_check_support = 1;
			cfg->enabled.status_check_support = !!enabled;

			continue;
		}

		if (!memcmp(tag, EMV_ID_LIBEMV_ZERO_AMOUNT_ALLOWED, tag_size)) {
			uint8_t enabled;
			size_t enabled_sz = sizeof(enabled);

			rc = tlv_encode_value(tlv, &enabled, &enabled_sz);
			if ((rc != TLV_RC_OK) || (!enabled_sz))
				return EMV_RC_SYNTAX_ERROR;

			cfg->present.zero_amount_allowed = 1;
			cfg->enabled.zero_amount_allowed = !!enabled;

			continue;
		}

		if (!memcmp(tag, EMV_ID_LIBEMV_EXT_SELECTION_SUPPORTED,
								    tag_size)) {
			uint8_t enabled;
			size_t enabled_sz = sizeof(enabled);

			rc = tlv_encode_value(tlv, &enabled, &enabled_sz);
			if ((rc != TLV_RC_OK) || (!enabled_sz))
				return EMV_RC_SYNTAX_ERROR;

			cfg->present.ext_selection_support = 1;
			cfg->enabled.ext_selection_support = !!enabled;

			continue;
		}

		if (!memcmp(tag, EMV_ID_LIBEMV_RDR_CTLS_TXN_LIMIT, tag_size)) {
			uint8_t amount[6];
			size_t size = sizeof(amount);

			rc = tlv_encode_value(tlv, amount, &size);
			if ((rc != TLV_RC_OK) || (size != sizeof(amount)))
				return EMV_RC_SYNTAX_ERROR;

			cfg->present.reader_ctls_txn_limit = 1;
			cfg->reader_ctls_txn_limit = bcd_to_u64(amount, size);

			continue;
		}

		if (!memcmp(tag, EMV_ID_LIBEMV_RDR_CTLS_FLOOR_LIMIT,
								    tag_size)) {
			uint8_t amount[6];
			size_t size = sizeof(amount);

			rc = tlv_encode_value(tlv, amount, &size);
			if ((rc != TLV_RC_OK) || (size != sizeof(amount)))
				return EMV_RC_SYNTAX_ERROR;

			cfg->present.reader_ctls_floor_limit = 1;
			cfg->reader_ctls_floor_limit = bcd_to_u64(amount, size);

			continue;
		}

		if (!memcmp(tag, EMV_ID_LIBEMV_TERMINAL_FLOOR_LIMIT,
								    tag_size)) {
			uint8_t amount[6];
			size_t size = sizeof(amount);

			rc = tlv_encode_value(tlv, amount, &size);
			if ((rc != TLV_RC_OK) || (size != sizeof(amount)))
				return EMV_RC_SYNTAX_ERROR;

			cfg->present.terminal_floor_limit = 1;
			cfg->terminal_floor_limit = bcd_to_u64(amount, size);

			continue;
		}

		if (!memcmp(tag, EMV_ID_LIBEMV_RDR_CVM_REQUIRED_LIMIT,
								    tag_size)) {
			uint8_t amount[6];
			size_t size = sizeof(amount);

			rc = tlv_encode_value(tlv, amount, &size);
			if ((rc != TLV_RC_OK) || (size != sizeof(amount)))
				return EMV_RC_SYNTAX_ERROR;

			cfg->present.reader_cvm_reqd_limit = 1;
			cfg->reader_cvm_reqd_limit = bcd_to_u64(amount, size);

			continue;
		}

		if (!memcmp(tag, EMV_ID_LIBEMV_TTQ, tag_size)) {
			size_t size = sizeof(cfg->ttq);

			cfg->present.ttq = 1;

			rc = tlv_encode_value(tlv, cfg->ttq, &size);
			if ((rc != TLV_RC_OK) || (size != sizeof(cfg->ttq)))
				return EMV_RC_SYNTAX_ERROR;

			continue;
		}

		if (memcmp(tag, EMV_ID_LIBEMV_TRANSACTION_TYPES, tag_size) &&
		    memcmp(tag, EMV_ID_LIBEMV_COMBINATION, tag_size))
			return EMV_RC_SYNTAX_ERROR;
	}

	return EMV_RC_OK;
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
		  EMV_ID_LIBEMV_CONFIGURATION)), EMV_ID_LIBEMV_COMBINATION_SET);
	     tlv_combination_set;
	     tlv_combination_set = tlv_find(tlv_get_next(tlv_combination_set),
					       EMV_ID_LIBEMV_COMBINATION_SET)) {
		struct emv_ep_config cfg;
		uint8_t txn_types[4] = { 0, 0, 0, 0 };
		size_t txn_types_sz = sizeof(txn_types);
		struct tlv *tlv_txn_type = NULL;
		int i_txn_type = 0;

		rc = parse_emv_ep_config(tlv_combination_set, &cfg);
		if (rc != EMV_RC_OK)
			goto error;

		tlv_txn_type = tlv_find(tlv_get_child(tlv_combination_set),
					       EMV_ID_LIBEMV_TRANSACTION_TYPES);
		if (!tlv_txn_type) {
			rc = EMV_RC_SYNTAX_ERROR;
			goto error;
		}

		rc = tlv_encode_value(tlv_txn_type, &txn_types, &txn_types_sz);
		if ((rc != TLV_RC_OK) || (txn_types_sz < 1)) {
			rc = EMV_RC_SYNTAX_ERROR;
			goto error;
		}

		for (i_txn_type = 0; i_txn_type < txn_types_sz; i_txn_type++) {
			enum emv_txn_type emv_txn_type = num_txn_types;

			emv_txn_type = get_emv_txn_type(txn_types[i_txn_type]);
			if (emv_txn_type >= num_txn_types) {
				rc = EMV_RC_SYNTAX_ERROR;
				goto error;
			}

			rc = parse_combination_set(tlv_combination_set, &cfg,
					    &ep->combination_set[emv_txn_type]);
			if (rc != EMV_RC_OK)
				goto error;
		}
	}

	log4c_category_log(ep->log_cat, LOG4C_PRIORITY_TRACE, "%s(): success",
								      __func__);

	return EMV_RC_OK;

error:
	if (tlv_config)
		tlv_free(tlv_config);

	log4c_category_log(ep->log_cat, LOG4C_PRIORITY_ERROR,
					  "%s(): failed. rc: %d", __func__, rc);

	return rc;
}

struct emv_ep *emv_ep_new(const char *log_cat)
{
	struct emv_ep *ep = (struct emv_ep *)calloc(1, sizeof(struct emv_ep));
	char cat[64];

	if (!ep)
		return NULL;

	snprintf(cat, sizeof(cat), "%s.libemv.emv_ep", log_cat);
	ep->log_cat = log4c_category_get(cat);

	return ep;
}

void emv_ep_free(struct emv_ep *ep)
{
	free(ep);
}
