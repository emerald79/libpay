#ifndef __EMV_H__
#define __EMV_H__

#include <stdint.h>
#include <stdlib.h>

#define EMV_RC_OK				0
#define EMV_RC_UNSUPPORTED_TRANSACTION_TYPE	1
#define EMV_RC_UNSUPPORTED_CURRENCY_CODE	2

#define EMV_EP_CONFIG_STATUS_CHECK_SUPPORT_FLAG			0x01u
#define EMV_EP_CONFIG_ZERO_AMOUNT_ALLOWED_FLAG			0x02u
#define EMV_EP_CONFIG_READER_CONTACTLESS_TRANSACTION_LIMIT	0x04u
#define EMV_EP_CONFIG_READER_CONTACTLESS_FLOOR_LIMIT		0x08u
#define EMV_EP_CONFIG_TERMINAL_FLOOR_LIMIT			0x10u
#define EMV_EP_CONFIG_READER_CVM_REQUIRED_LIMIT			0x20u
#define EMV_EP_CONFIG_TERMINAL_TRANSACTION_QUALIFIERS		0x40u
#define EMV_EP_CONFIG_EXTENDED_SELECTION_SUPPORT_FLAG		0x80u

#define TTQ_MAG_STRIPE_MODE_SUPPORTED		0x80000000u
#define TTQ_EMV_MODE_SUPPORTED			0x20000000u
#define TTQ_EMV_CONTACT_CHIP_SUPPORTED		0x10000000u
#define TTQ_OFFLINE_ONLY_READER			0x08000000u
#define TTQ_ONLINE_PIN_SUPPORTED		0x04000000u
#define TTQ_SIGNATURE_SUPPORTED			0x02000000u
#define TTQ_ODA_FOR_ONLINE_AUTH_SUPPORTED	0x01000000u
#define TTQ_ONLINE_CRYPTOGRAM_REQUIRED		0x00800000u
#define TTQ_CVM_REQUIRED			0x00400000u
#define TTQ_OFFLINE_PIN_SUPPORTED		0x00200000u
#define TTQ_ISSUER_UPDATE_PROCESSING_SUPPORTED	0x00008000u
#define TTQ_CONSUMER_DEVICE_CVM_SUPPORTED	0x00004000u

struct emv_ep_config {
	uint8_t		presence_flags;
	uint8_t		support_flags;
	uint64_t	reader_contactless_transaction_limit;
	uint64_t	reader_contactless_floor_limit;
	uint64_t	terminal_floor_limit;
	uint64_t	reader_cvm_required_limit;
	uint32_t	terminal_transaction_qualifiers;
};

#define EMV_EP_PREPROC_INDICATORS_STATUS_CHECK_REQUESTED		   0x01u
#define EMV_EP_PREPROC_INDICATORS_CONTACTLESS_APPLICATION_NOT_ALLOWED	   0x02u
#define EMV_EP_PREPROC_INDICATORS_ZERO_AMOUNT				   0x04u
#define EMV_EP_PREPROC_INDICATORS_READER_CVM_REQUIRED_LIMIT_EXCEEDED	   0x08u
#define EMV_EP_PREPROC_INDICATORS_READER_CONTACTLESS_FLOOR_LIMIT_EXCEEDED  0x10u

struct emv_ep_preproc_indicators {
	uint8_t		flags;
	uint32_t	copy_of_ttq;
};

#define EMV_EP_TX_TYPE_IDX_PURCHASE			0
#define EMV_EP_TX_TYPE_IDX_PURCHASE_WITH_CASHBACK	1
#define EMV_EP_TX_TYPE_IDX_CASH_ADVANCE			2
#define EMV_EP_TX_TYPE_IDX_REFUND			3

struct emv_ep_combination {
	uint8_t					aid[16];
	size_t					aid_len;
	uint8_t					kernel_id[8];
	size_t					kernel_id_len;
	struct emv_ep_config			config[4];
	struct emv_ep_preproc_indicators	indicators;
};

struct emv_transaction_data {
	uint8_t		transaction_type;
	uint64_t	amount_authorised;
	uint64_t	amount_other;
	uint32_t	unpredictable_number;
	uint16_t	currency_code;
};

struct emv_ep {
	struct emv_ep_combination	*combinations;
	int				num_combinations;
};

#define ISO4217_USD	840
#define ISO4217_EUR	978


#define EMV_MSGID_APPROVED		0x03u
#define EMV_MSGID_NOT_AUTHORISED	0x07u
#define EMV_MSGID_ENTER_PIN		0x09u
#define EMV_MSGID_PROCESSING_ERROR	0x0Fu
#define EMV_MSGID_REMOVE_CARD		0x10u
#define EMV_MSGID_WELCOME		0x14u
#define EMV_MSGID_PRESENT_CARD		0x15u
#define EMV_MSGID_PROCESSING		0x16u
#define EMV_MSGID_CARD_READ_OK		0x17u
#define EMV_MSGID_INSERT_OR_SWIPE_CARD	0x18u
#define EMV_MSGID_PRESENT_ONE_CARD_ONLY	0x19u
#define EMV_MSGID_APPROVED_PLEASE_SIGN	0x1Au
#define EMV_MSGID_AUTHORISING		0x1Bu
#define EMV_MSGID_TRY_ANOTHER_CARD	0x1Cu
#define EMV_MSGID_INSERT_CARD		0x1Du
#define EMV_MSGID_CLEAR_DISPLAY		0x1Eu
#define EMV_MSGID_SEE_PHONE		0x20u
#define EMV_MSGID_PRESENT_CARD_AGAIN	0x21u

#define EMV_VALUE_AMOUNT		0x01u
#define EMV_VALUE_BALANCE		0x02u

#define EMV_STATUS_NOT_READY		0x01u
#define EMV_STATUS_IDLE			0x02u
#define EMV_STATUS_READY_TO_READ	0x03u
#define EMV_STATUS_PROCESSING		0x04u
#define EMV_STATUS_CARD_READ_SUCCESS	0x05u
#define EMV_STATUS_PROCESSING_ERROR	0x06u

struct emv_ui_req {
	int		present;
	uint8_t 	msgid;
	uint8_t 	status;
	int		hold_time;
	char		lang_pref[8];
	size_t		lang_pref_len;
	uint8_t		value_qualifier;
	uint64_t	value;
	uint16_t	currency_code;
};

#define EMV_OUTCOME_SELECT_NEXT		0x01u
#define EMV_OUTCOME_TRY_AGAIN		0x02u
#define EMV_OUTCOME_APPROVED		0x03u
#define EMV_OUTCOME_DECLINED		0x04u
#define EMV_OUTCOME_ONLINE_REQUEST	0x05u
#define EMV_OUTCOME_TRY_ANOTHER_IFACE	0x06u
#define EMV_OUTCOME_END_APPLICATION	0x07u

#define EMV_OUTCOME_START_NA	0x00u
#define EMV_OUTCOME_START_A	0x01u
#define EMV_OUTCOME_START_B	0x02u
#define EMV_OUTCOME_START_C	0x03u
#define EMV_OUTCOME_START_D	0x04u

#define EMV_OUTCOME_ONLINE_RESPONSE_DATA_NA		0x00u
#define EMV_OUTCOME_ONLINE_RESPONSE_DATA_EMV_DATA	0x01u
#define EMV_OUTCOME_ONLINE_RESPONSE_DATA_ANY		0x02u

#define EMV_OUTCOME_CVM_NA				0x00u
#define EMV_OUTCOME_CVM_ONLINE_PIN			0x01u
#define EMV_OUTCOME_CVM_CONFIRMATION_CODE_VERIFIED	0x02u
#define EMV_OUTCOME_CVM_OBTAIN_SIGNATURE		0x03u
#define EMV_OUTCOME_CVM_NO_CVM				0x04u

#define EMV_OUTCOME_ALTERNATE_INTERFACE_PREFERENCE_NA		0x00u
#define EMV_OUTCOME_ALTERNATE_INTERFACE_PREFERENCE_CONTACT_CHIP	0x01u
#define EMV_OUTCOME_ALTERNATE_INTERFACE_PREFERENCE_MAGSTRIPE	0x02u

#define EMV_OUTCOME_RECEIPT_NA	0x00
#define EMV_OUTCOME_RECEIPT_YES	0x01
#define EMV_OUTCOME_RECEIPT_NO	0x02

struct emv_outcome_parms {
	uint8_t			outcome;
	uint8_t			start;
	uint8_t			online_response_data;
	uint8_t			cvm;
	int			ui_req_on_outcome_present;
	struct emv_ui_req	ui_req_on_outcome;
	int			ui_req_on_restart_present;
	struct emv_ui_req	ui_req_on_restart;
	int			data_record_present;
	int			discretionary_data_present;
	int			alternate_interface_preference;
	uint8_t			receipt;
	int			field_off_request;
	int			removal_timeout;
};

#endif							    /* ndef __EMV_H__ */
