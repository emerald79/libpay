/*
 * libemv - Support Library containing EMVCo L2 Kernels
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

#ifndef __EMV_H__
#define __EMV_H__

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <tlv.h>

#define EMV_RC_OK				0
#define EMV_RC_UNSUPPORTED_TRANSACTION_TYPE	1
#define EMV_RC_UNSUPPORTED_CURRENCY_CODE	2
#define EMV_RC_RF_COMMUNICATION_ERROR		3
#define EMV_RC_CARD_PROTOCOL_ERROR		4
#define EMV_RC_OVERFLOW				5
#define EMV_RC_OUT_OF_MEMORY			6
#define EMV_RC_SYNTAX_ERROR			7
#define EMV_RC_NO_KERNEL			8
#define EMV_RC_INVALID_ARG			9
#define EMV_RC_COLLISION			10
#define EMV_RC_CONTINUE				11

#define EMV_MAX_ONLINE_RESPONSE_LEN	256
#define EMV_MAX_DATA_RECORD_LEN		512
#define	EMV_MAX_DISCRETIONARY_DATA_LEN	1024

#define ISO4217_USD	((const uint8_t []){ '\x08', '\x40' })
#define ISO4217_EUR	((const uint8_t []){ '\x09', '\x78' })

#define DF_NAME_2PAY_SYS_DDF01 "2PAY.SYS.DDF01"

#define EMV_SW(sw1, sw2) ((const uint8_t []){ sw1, sw2 })
#define EMV_SW_6700_WRONG_LENGTH	EMV_SW('\x67', '\x00')
#define EMV_SW_6A82_FILE_NOT_FOUND	EMV_SW('\x6A', '\x82')
#define EMV_SW_6D00_INS_NOT_SUPPORTED	EMV_SW('\x6E', '\x00')
#define EMV_SW_6E00_CLA_NOT_SUPPORTED	EMV_SW('\x6E', '\x00')
#define EMV_SW_9000_OK			EMV_SW('\x90', '\x00')

#define EMV_ID_ADF_NAME				"\x4F"
#define EMV_ID_APPLICATION_LABEL		"\x50"
#define EMV_ID_DIRECTORY_ENTRY			"\x61"
#define EMV_ID_FCI_TEMPLATE			"\x6F"
#define EMV_ID_DF_NAME				"\x84"
#define EMV_ID_APPLICATION_PRIORITY_INDICATOR	"\x87"
#define EMV_ID_TRANSACTION_TYPE			"\x9C"
#define EMV_ID_AMOUNT_AUTHORIZED		"\x9F\x02"
#define EMV_ID_AMOUNT_OTHER			"\x9F\x03"
#define EMV_ID_KERNEL_IDENTIFIER		"\x9F\x2A"
#define EMV_ID_EXTENDED_SELECTION		"\x9F\x29"
#define EMV_ID_UN				"\x9F\x37"
#define EMV_ID_PDOL				"\x9F\x38"
#define EMV_ID_TTQ				"\x9F\x66"
#define EMV_ID_FCI_PROPRIETARY_TEMPLATE		"\xA5"
#define EMV_ID_FCI_ISSUER_DISCRETIONARY_DATA	"\xBF\x0C"

#define EMV_ID_LIBEMV_CONFIGURATION		"\xFF\x81\xE3\x71"
#define EMV_ID_LIBEMV_COMBINATION_SET		"\xFF\x82\xE3\x71"
#define EMV_ID_LIBEMV_TRANSACTION_TYPES		"\xDF\x83\xE3\x71"
#define EMV_ID_LIBEMV_COMBINATION		"\xFF\x84\xE3\x71"
#define EMV_ID_LIBEMV_AID			"\xDF\x85\xE3\x71"
#define EMV_ID_LIBEMV_KERNEL_ID			"\xDF\x86\xE3\x71"
#define EMV_ID_LIBEMV_STATUS_CHECK_SUPPORTED	"\xDF\x87\xE3\x71"
#define EMV_ID_LIBEMV_ZERO_AMOUNT_ALLOWED	"\xDF\x88\xE3\x71"
#define EMV_ID_LIBEMV_EXT_SELECTION_SUPPORTED	"\xDF\x89\xE3\x71"
#define EMV_ID_LIBEMV_RDR_CTLS_TXN_LIMIT	"\xDF\x8A\xE3\x71"
#define EMV_ID_LIBEMV_RDR_CTLS_FLOOR_LIMIT	"\xDF\x8B\xE3\x71"
#define EMV_ID_LIBEMV_TERMINAL_FLOOR_LIMIT	"\xDF\x8C\xE3\x71"
#define EMV_ID_LIBEMV_RDR_CVM_REQUIRED_LIMIT	"\xDF\x8D\xE3\x71"
#define EMV_ID_LIBEMV_TTQ			"\xDF\x8E\xE3\x71"
#define EMV_ID_LIBEMB_AUTORUN			"\xFF\x8F\xE3\x71"
#define EMV_ID_LIBEMV_AUTORUN_TRANSACTION_TYPE	"\xDF\x90\xE3\x71"
#define EMV_ID_LIBEMV_AUTORUN_AMOUNT_AUTHORISED	"\xDF\x91\xE3\x71"

enum emv_message_identifier {
	msg_approved			= 0x03,
	msg_not_authorized		= 0x07,
	msg_enter_pin			= 0x09,
	msg_processing_error		= 0x0f,
	msg_remove_card			= 0x10,
	msg_welcome			= 0x14,
	msg_present_card		= 0x15,
	msg_processing			= 0x16,
	msg_card_read_ok		= 0x17,
	msg_insert_or_swipe_card	= 0x18,
	msg_present_one_card_only	= 0x19,
	msg_approved_please_sign	= 0x1a,
	msg_authorising			= 0x1b,
	msg_try_another_card		= 0x1c,
	msg_insert_card			= 0x1d,
	msg_clear_display		= 0x1e,
	msg_see_phone			= 0x20,
	msg_present_card_again		= 0x21,
};

enum emv_value_qualifier {
	val_amount = 0,
	val_balance
};

enum emv_status {
	sts_not_ready = 0,
	sts_idle,
	sts_ready_to_read,
	sts_processing,
	sts_card_read_successfully,
	sts_processing_error
};

struct emv_ui_request {
	enum	emv_message_identifier	msg_id;
	enum	emv_status		status;
	uint8_t				hold_time;
	char				lang_pref[8];
	size_t				lang_pref_len;
	enum	emv_value_qualifier	value_qualifier;
	uint8_t				value[6];
	uint8_t				currency_code[2];
};

enum emv_outcome {
	out_na = 0,
	out_select_next,
	out_try_again,
	out_approved,
	out_declined,
	out_online_request,
	out_try_another_interface,
	out_end_application
};

enum emv_start {
	start_na = 0,
	start_a,
	start_b,
	start_c,
	start_d
};

enum emv_cvm {
	cvm_na = 0,
	cvm_online_pin,
	cvm_confirmation_code_verified,
	cvm_obtain_signature,
	cvm_no_cvm
};

enum emv_online_response_type {
	ort_na = 0,
	ort_emv_data,
	ort_any
};

struct emv_online_response {
	enum emv_online_response_type	type;
	uint8_t				data[EMV_MAX_ONLINE_RESPONSE_LEN];
	size_t				len;
};

struct emv_data_record {
	uint8_t	data[EMV_MAX_DATA_RECORD_LEN];
	size_t	len;
};

struct emv_discretionary_data {
	uint8_t data[EMV_MAX_DISCRETIONARY_DATA_LEN];
	size_t len;
};

enum emv_alternate_interface_pref {
	aip_na = 0,
	aip_contact_chip,
	aip_magstripe
};

struct emv_field_off_request {
	bool	requested;
	int	hold_time_value;
};

struct emv_outcome_parms {
	enum	emv_outcome			outcome;
	enum	emv_start			start;
	struct	emv_online_response		online_response;
	enum	emv_cvm				cvm;
	bool					ui_request_on_outcome_present;
	struct	emv_ui_request			ui_request_on_outcome;
	bool					ui_request_on_restart_present;
	struct	emv_ui_request			ui_request_on_restart;
	struct	emv_data_record			data_record;
	struct	emv_discretionary_data		discretionary_data;
	enum	emv_alternate_interface_pref	alternate_interface_pref;
	bool					receipt;
	struct	emv_field_off_request		field_off_request;
	int					removal_timeout;
};

struct emv_ep_preproc_indicators {
	bool	status_check_requested:1;
	bool	ctls_app_not_allowed:1;
	bool	zero_amount:1;
	bool	cvm_reqd_limit_exceeded:1;
	bool	floor_limit_exceeded:1;
	bool	txn_limit_exceeded:1;
	uint8_t ttq[4];
};

struct emv_hal;

struct emv_hal_ops {
	int	(*start_polling)(struct emv_hal *hal);

	int	(*wait_for_card)(struct emv_hal *hal);

	int	(*transceive)(struct emv_hal *hal,
			      const void     *capdu,
			      size_t	     capdu_len,
			      void	     *rapdu,
			      size_t	     *rapdu_len);

	void	(*ui_request)(struct emv_hal	    *hal,
			      struct emv_ui_request *ui_request);
};

struct emv_hal {
	const struct emv_hal_ops *ops;
};

enum emv_txn_type {
	txn_purchase		   = 0,
	txn_purchase_with_cashback = 1,
	txn_cash_advance	   = 2,
	txn_refund		   = 3,
	num_txn_types		   = 4
};

struct emv_kernel_parms {
	bool					restart;
	enum emv_start				start;
	enum emv_txn_type			txn_type;
	uint64_t				amount_authorized;
	uint64_t				amount_other;
	uint8_t					currency[2];
	uint32_t				unpredictable_number;
	uint8_t					kernel_id[8];
	size_t					kernel_id_len;
	uint8_t					fci[256];
	size_t					fci_len;
	uint8_t					sw[2];
	const struct emv_ep_preproc_indicators *preproc_indicators;
};

struct emv_kernel;

struct emv_kernel_ops {
	int (*configure)(struct emv_kernel *kernel,
			 const void	   *configuration,
			 size_t		    length);

	int (*activate)	(struct emv_kernel	  *kernel,
			 struct emv_hal		  *hal,
			 struct emv_kernel_parms  *parms,
			 struct emv_outcome_parms *outcome,
			 void			  *txn_data,
			 size_t			  *txn_data_len);
};

struct emv_kernel {
	const struct emv_kernel_ops *ops;
};

struct emv_ep;

struct emv_ep *emv_ep_new(const char *logging_category);

void emv_ep_free(struct emv_ep *ep);

int emv_ep_register_hal(struct emv_ep *ep, struct emv_hal *hal);

int emv_ep_register_kernel(struct emv_ep *ep, struct emv_kernel *kernel,
				const uint8_t *kernel_id, size_t kernel_id_len);

int emv_ep_configure(struct emv_ep *ep, const void *config, size_t len);

int emv_ep_activate(struct emv_ep    *ep,
		    enum emv_start    start,
		    enum emv_txn_type txn_type,
		    uint64_t	      amount_authorised,
		    uint64_t	      amount_other,
		    const uint8_t     currency[2],
		    uint32_t	      unpredictable_number);

#define EMV_CMD_SELECT_CLA		0x00u
#define EMV_CMD_SELECT_INS		0xA4u
#define EMV_CMD_SELECT_P1_BY_NAME	0x04u
#define EMV_CMD_SELECT_P2_FIRST		0x00u
#define EMV_CMD_SELECT_P2_NEXT		0x02u

#define EMV_CMD_GPO_CLA			0x80u
#define EMV_CMD_GPO_INS			0xA8u
#define EMV_CMD_P1_NONE			0x00u
#define EMV_CMD_P2_NONE			0x00u

int emv_transceive_apdu(struct emv_hal *hal, uint8_t cla, uint8_t ins,
		      uint8_t p1, uint8_t p2, const void *data, size_t data_len,
			void *response, size_t *response_length, uint8_t sw[2]);

/*-----------------------------------------------------------------------------+
| EMV TLV Tag Description Handling					       |
+-----------------------------------------------------------------------------*/

enum emv_tag_source {
	src_icc	     = 0,
	src_terminal = 1
};

struct emv_tag {
	void	*value;
	size_t	len;
};

struct emv_tag_descriptor {
	char			*name;
	char			*description;
	enum emv_tag_source	source;
	enum tlv_fmt		format;
	struct emv_tag		*templates;
	size_t			num_templates;
	struct emv_tag		tag;
};

int emv_tag_parse_descriptors(const char *json_string,
	      struct emv_tag_descriptor **descriptors, size_t *num_descriptors);

const struct tlv_id_to_fmt *libemv_get_id_fmts(void);

#endif							    /* ndef __EMV_H__ */
