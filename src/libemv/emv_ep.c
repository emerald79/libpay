#include <string.h>

#include <emv.h>

#define REQUIREMENT(book, id)

static int emv_is_currency_code_supported(uint16_t currency_code)
{
	switch (currency_code) {
	case ISO4217_USD:
	case ISO4217_EUR:
		return 1;
	}

	return 0;
}

static uint64_t emv_single_unit_of_currency(uint16_t currency_code)
{
	return 100;
}

int emv_ep_preprocessing(struct emv_ep *ep, struct emv_transaction_data *tx, struct emv_outcome *outcome)
{
	int i = 0, tx_type_idx = 0, contactless_application_allowed = 0;


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

	if (!emv_is_currency_code_supported(tx->currency_code))
		return EMV_RC_UNSUPPORTED_CURRENCY_CODE;

	for (i = 0; i < ep->num_combinations; i++) {
		struct emv_ep_config *config = NULL;

		config = &ep->combinations[i].config[tx_type_idx];


		REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.1.1.1");

		memset(&ep->combinations[i].indicators, 0,
					sizeof(ep->combinations[i].indicators));


		REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.1.1.2");

		if (config->presence_flags &
				EMV_EP_CONFIG_TERMINAL_TRANSACTION_QUALIFIERS) {
			ep->combinations[i].indicators.copy_of_ttq =
					config->terminal_transaction_qualifiers;
			ep->combinations[i].indicators.copy_of_ttq &=
					      ~(TTQ_ONLINE_CRYPTOGRAM_REQUIRED |
							      TTQ_CVM_REQUIRED);
		}


		REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.1.1.3");

		if ((config->presence_flags &
				     EMV_EP_CONFIG_STATUS_CHECK_SUPPORT_FLAG) &&
		    (config->support_flags &
				     EMV_EP_CONFIG_STATUS_CHECK_SUPPORT_FLAG) &&
		    (tx->amount_authorised == emv_single_unit_of_currency(
							    tx->currency_code)))
			ep->combinations[i].indicators.flags |=
			       EMV_EP_PREPROC_INDICATORS_STATUS_CHECK_REQUESTED;


		REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.1.1.4");

		if ((tx->amount_authorised == 0) &&
		    (config->presence_flags &
				      EMV_EP_CONFIG_ZERO_AMOUNT_ALLOWED_FLAG)) {
			if (config->support_flags &
				       EMV_EP_CONFIG_ZERO_AMOUNT_ALLOWED_FLAG) {
				ep->combinations[i].indicators.flags |=
					  EMV_EP_PREPROC_INDICATORS_ZERO_AMOUNT;
			} else {
				ep->combinations[i].indicators.flags |=
					EMV_EP_PREPROC_INDICATORS_CONTACTLESS_APPLICATION_NOT_ALLOWED;
			}
		}


		REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.1.1.5");

		if ((config->presence_flags &
			  EMV_EP_CONFIG_READER_CONTACTLESS_TRANSACTION_LIMIT) &&
			(tx->amount_authorised >=
				  config->reader_contactless_transaction_limit))
			ep->combinations[i].indicators.flags |=
					EMV_EP_PREPROC_INDICATORS_CONTACTLESS_APPLICATION_NOT_ALLOWED;


		REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.1.1.6");

		if ((config->presence_flags &
				EMV_EP_CONFIG_READER_CONTACTLESS_FLOOR_LIMIT) &&
			(tx->amount_authorised >
					config->reader_contactless_floor_limit))
			ep->combinations[i].indicators.flags |=
					EMV_EP_PREPROC_INDICATORS_READER_CONTACTLESS_FLOOR_LIMIT_EXCEEDED;


		REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.1.1.7");

		if (!(config->presence_flags &
				EMV_EP_CONFIG_READER_CONTACTLESS_FLOOR_LIMIT) &&
		    (config->presence_flags &
					  EMV_EP_CONFIG_TERMINAL_FLOOR_LIMIT) &&
			 (tx->amount_authorised > config->terminal_floor_limit))
			ep->combinations[i].indicators.flags |=
					EMV_EP_PREPROC_INDICATORS_READER_CONTACTLESS_FLOOR_LIMIT_EXCEEDED;


		REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.1.1.8");

		if ((config->presence_flags &
				     EMV_EP_CONFIG_READER_CVM_REQUIRED_LIMIT) &&
			(tx->amount_authorised >= config->reader_cvm_required_limit))
			ep->combinations[i].indicators.flags |=
				EMV_EP_PREPROC_INDICATORS_READER_CVM_REQUIRED_LIMIT_EXCEEDED;


		if (config->presence_flags &
				EMV_EP_CONFIG_TERMINAL_TRANSACTION_QUALIFIERS) {

			REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.1.1.9");

			if (ep->combinations[i].indicators.flags &
					EMV_EP_PREPROC_INDICATORS_READER_CONTACTLESS_FLOOR_LIMIT_EXCEEDED)
				ep->combinations[i].indicators.copy_of_ttq |= TTQ_ONLINE_CRYPTOGRAM_REQUIRED;


			REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.1.1.10");

			if (ep->combinations[i].indicators.flags &
					EMV_EP_PREPROC_INDICATORS_STATUS_CHECK_REQUESTED)
				ep->combinations[i].indicators.copy_of_ttq |= TTQ_ONLINE_CRYPTOGRAM_REQUIRED;


			REQUIREMENT(EMV_CTLS_BOOK_B_V2_5, "3.1.1.11");

			if (ep->combinations[i].indicators.flags &
						EMV_EP_PREPROC_INDICATORS_ZERO_AMOUNT)
			{
				if (ep->combinations[i].indicators.copy_of_ttq & TTQ_OFFLINE_ONLY_READER)
					ep->combinations[i].indicators.flags |=
						EMV_EP_PREPROC_INDICATORS_CONTACTLESS_APPLICATION_NOT_ALLOWED;
				else
					ep->combinations[i].indicators.copy_of_ttq |= TTQ_ONLINE_CRYPTOGRAM_REQUIRED;
			}


			REQUIREMENT(EMV_CTRL_BOOK_B_V2_5, "3.1.1.12");

			if (ep->combinations[i].indicators.flags &
					EMV_EP_PREPROC_INDICATORS_READER_CVM_REQUIRED_LIMIT_EXCEEDED)
				ep->combinations[i].indicators.copy_of_ttq |= TTQ_CVM_REQUIRED;

		}


		if (!(ep->combinations[i].indicators.flags &
					EMV_EP_PREPROC_INDICATORS_CONTACTLESS_APPLICATION_NOT_ALLOWED))
			contactless_application_allowed = 1;
	}


	REQUIREMENT(EMV_CTRL_BOOK_B_V2_5, "3.1.1.13");

	if (!contactless_application_allowed) {
		outcome->start = EMV_OUTCOME_START_NA;
		outcome->online_response_data =	EMV_OUTCOME_ONLINE_RESPONSE_DATA_NA;
		outcome->cvm = EMV_OUTCOME_CVM_NA;
		outcome->ui_request_on_outcome_present = 1;
		outcome->ui_request_on_restart_present = 0;
		outcome->data_record_present = 0;
		outcome->discretionary_data_present = 0;
		outcome->field_off_request = 0;
		outcome->hold_time_value = 0;
		outcome->removal_timeout = 0;
	}

	return EMV_RC_OK;
}
