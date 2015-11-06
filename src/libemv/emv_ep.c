#include <string.h>

#include <emv.h>

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

int emv_ep_preprocessing(struct emv_ep *ep, struct emv_transaction_data *tx,
					      struct emv_outcome_parms *outcome)
{
	uint64_t amount_authorised = 0, unit_of_currency = 0;
	bool ctls_app_allowed = 0;
	int i = 0, tx_type_idx = 0;

	amount_authorised = bcd_to_u64(tx->amount_authorised, 6);
	unit_of_currency  = bcd_to_u64(
				 single_unit_of_currency(tx->currency_code), 6);

	/* See EMV Contactless Book A v2.5, Table 5-6: Type of Transaction.   */
	if (tx->transaction_type == 0x00)
		if (tx->amount_other)
			tx_type_idx = EMV_EP_TX_TYPE_IDX_PURCHASE_WITH_CASHBACK;
		else
			tx_type_idx = EMV_EP_TX_TYPE_IDX_PURCHASE;
	else if (tx->transaction_type == 0x09)
		tx_type_idx = EMV_EP_TX_TYPE_IDX_PURCHASE_WITH_CASHBACK;
	else if (tx->transaction_type == 0x01)
		tx_type_idx = EMV_EP_TX_TYPE_IDX_CASH_ADVANCE;
	else if (tx->transaction_type == 0x20)
		tx_type_idx = EMV_EP_TX_TYPE_IDX_REFUND;
	else
		return EMV_RC_UNSUPPORTED_TRANSACTION_TYPE;

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
