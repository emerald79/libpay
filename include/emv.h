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

#define EMV_RC_OK				0
#define EMV_RC_UNSUPPORTED_TRANSACTION_TYPE	1
#define EMV_RC_UNSUPPORTED_CURRENCY_CODE	2
#define EMV_RC_RF_COMMUNICATION_ERROR		3
#define EMV_RC_CARD_PROTOCOL_ERROR		4
#define EMV_RC_BUFFER_OVERFLOW			5

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

struct emv_ep_config_flags {
	bool	status_check_support:1;
	bool	zero_amount_allowed:1;
	bool	reader_ctls_tx_limit:1;
	bool	reader_ctls_floor_limit:1;
	bool	terminal_floor_limit:1;
	bool	reader_cvm_reqd_limit:1;
	bool	ttq:1;
	bool	ext_selection_support:1;
};

struct emv_ep_config {
	struct emv_ep_config_flags	present;
	struct emv_ep_config_flags	supported;
	uint8_t				reader_ctls_tx_limit[6];
	uint8_t				reader_ctls_floor_limit[6];
	uint8_t				terminal_floor_limit[6];
	uint8_t				reader_cvm_reqd_limit[6];
	uint32_t			ttq;
};

struct emv_ep_preproc_indicators {
	bool	 status_check_requested:1;
	bool	 ctls_app_not_allowed:1;
	bool	 zero_amount:1;
	bool	 cvm_reqd_limit_exceeded:1;
	bool	 floor_limit_exceeded:1;
	uint32_t copy_of_ttq;
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
	uint8_t	 transaction_type;
	uint8_t	 amount_authorised[6];
	uint8_t	 amount_other[6];
	uint32_t unpredictable_number;
	uint8_t	 currency_code[2];
};

struct emv_hal {
	void	*priv;
	int	(*emv_transceive)(struct emv_hal *hal,
				  const uint8_t	 *capdu,
				  size_t	 capdu_len,
				  uint8_t	 *rapdu,
				  size_t	 *rapdu_len);
};

struct emv_ep {
	struct emv_hal			*hal;
	bool				restart;
	struct emv_ep_combination	*combinations;
	int				num_combinations;
};

#define EMV_MAX_ONLINE_RESPONSE_LEN	256
#define EMV_MAX_DATA_RECORD_LEN		512
#define	EMV_MAX_DISCRETIONARY_DATA_LEN	1024

#define ISO4217_USD	((const uint8_t []){ '\x08', '\x40' })
#define ISO4217_EUR	((const uint8_t []){ '\x09', '\x78' })

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
	bool				present;
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
	out_select_next = 0,
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
	struct	emv_ui_request			ui_request_on_outcome;
	struct	emv_ui_request			ui_request_on_restart;
	struct	emv_data_record			data_record;
	struct	emv_discretionary_data		discretionary_data;
	enum	emv_alternate_interface_pref	alternate_interface_pref;
	bool					receipt;
	struct	emv_field_off_request		field_off_request;
	int					removal_timeout;
};

int emv_ep_preprocessing(struct emv_ep *ep, struct emv_transaction_data *tx,
					     struct emv_outcome_parms *outcome);

int emv_ep_protocol_activation(struct emv_ep *ep,
		       struct emv_transaction_data *tx, bool started_by_reader);

int emv_ep_combination_selection(struct emv_ep *ep);

void emv_ep_register_hal(struct emv_ep *ep, struct emv_hal *hal);

#endif							    /* ndef __EMV_H__ */
