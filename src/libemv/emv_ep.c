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

int emv_ep_preprocessing(struct emv_ep *ep, struct emv_transaction_data *tx)
{
	int i, tx_type_idx;


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
	}

	return EMV_RC_OK;
}
