/*
 * libtlv - Support Library for EMV TLV handling.
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
#include <time.h>
#include <log4c.h>

#include <tlv.h>
#include <emv.h>

#include "emvco_ep_ta.h"

struct checker {
	const struct chk_ops *ops;

	enum pass_criteria    pass_criteria;
	bool		      pass_criteria_checked;
	bool		      pass_criteria_met;

	bool		      field_is_on;
	struct emv_ui_request ui_request;

	uint32_t	      uns[1000];
	size_t		      num_uns;

	log4c_category_t     *log_cat;
};

static bool get_value(struct tlv *tlv, const char *tag, void *value,
								   size_t *size)
{
	struct tlv *mytlv = tlv_find(tlv, tag);
	int rc = EMV_RC_OK;

	if (!mytlv) {
		*size = 0;
		return false;
	}

	rc = tlv_encode_value(mytlv, value, size);
	if (rc != EMV_RC_OK)
		return false;

	return true;
}

static bool check_value(struct checker *chk, struct tlv *tlv, const char *tag,
						 const void *value, size_t size)
{
	uint8_t val[256], dol_val[256];
	size_t val_sz = sizeof(val);
	bool ok = false;

	if (!get_value(tlv, tag, val, &val_sz))
		goto done;

	libtlv_get_dol_field(tag, value, size, dol_val, val_sz);

	if (memcmp(dol_val, val, val_sz))
		goto done;

	ok = true;

done:
	if (!ok) {
		char hex_tag[TLV_MAX_TAG_LENGTH * 2 + 1];
		char hex_dol_val[val_sz * 2 + 1];
		char hex_val[val_sz * 2 + 1];

		libtlv_bin_to_hex(tag, libtlv_get_tag_length(tag), hex_tag);
		libtlv_bin_to_hex(dol_val, val_sz, hex_dol_val);
		libtlv_bin_to_hex(val, val_sz, hex_val);

		if (val_sz)
			log4c_category_log(chk->log_cat, LOG4C_PRIORITY_NOTICE,
			       "Wrong value: tag '%s', expected '%s', got '%s'",
						 hex_tag, hex_dol_val, hex_val);
		else
			log4c_category_log(chk->log_cat, LOG4C_PRIORITY_NOTICE,
						  "Missing: tag '%s'", hex_tag);
	}

	return ok;
}

static bool check_terminal_data(struct checker *chk, struct tlv *data)
{
	uint8_t pos_entry_mode = POS_ENTRY_MODE;
	uint8_t terminal_type = TERMINAL_TYPE;
	uint8_t app_ver_num[2] = TK_APPLICATION_VERSION_NUMBER;

	if (!check_value(chk, data, EMV_ID_ACQUIRER_IDENTIFIER,
			      ACQUIRER_IDENTIFIER, sizeof(ACQUIRER_IDENTIFIER)))
		return false;

	if (!check_value(chk, data, EMV_ID_ADDITIONAL_TERMINAL_CAPABILITIES,
					       ADDITIONAL_TERMINAL_CAPABILITIES,
				      sizeof(ADDITIONAL_TERMINAL_CAPABILITIES)))
		return false;

	if (!check_value(chk, data, EMV_ID_MERCHANT_CATEGORY_CODE,
			MERCHANT_CATEGORY_CODE, sizeof(MERCHANT_CATEGORY_CODE)))
		return false;

	if (!check_value(chk, data, EMV_ID_MERCHANT_IDENTIFIER,
			      MERCHANT_IDENTIFIER, strlen(MERCHANT_IDENTIFIER)))
		return false;

	if (!check_value(chk, data, EMV_ID_MERCHANT_NAME_AND_LOCATION,
		MERCHANT_NAME_AND_LOCATION, strlen(MERCHANT_NAME_AND_LOCATION)))
		return false;

	if (!check_value(chk, data, EMV_ID_POS_ENTRY_MODE, &pos_entry_mode,
							sizeof(pos_entry_mode)))
		return false;

	if (!check_value(chk, data, EMV_ID_TERMINAL_CAPABILITIES,
			 TERMINAL_CAPABILITIES,  sizeof(TERMINAL_CAPABILITIES)))
		return false;

	if (!check_value(chk, data, EMV_ID_TERMINAL_COUNTRY_CODE,
			 TERMINAL_COUNTRY_CODE,  sizeof(TERMINAL_COUNTRY_CODE)))
		return false;

	if (!check_value(chk, data, EMV_ID_TERMINAL_IDENTIFICATION,
		     TERMINAL_IDENTIFICATION,  sizeof(TERMINAL_IDENTIFICATION)))
		return false;

	if (!check_value(chk, data, EMV_ID_TERMINAL_TYPE, &terminal_type,
							 sizeof(terminal_type)))
		return false;

	if (!check_value(chk, data, EMV_ID_APPLICATION_VERSION_NUMBER_TERM,
					      app_ver_num, sizeof(app_ver_num)))
		return false;

	if (!check_value(chk, data, EMV_ID_INTERFACE_DEVICE_SERIAL_NUMBER,
					     INTERFACE_DEVICE_SERIAL_NUMBER, 8))
		return false;

	/* FIXME: TSI (tag '9B') not checked for now.			      */

	return true;
}

static bool check_txn_date_and_time(struct checker *chk, struct tlv *data)
{
	uint8_t txn_time[3], txn_date[3];
	size_t txn_time_sz = sizeof(txn_time), txn_date_sz = sizeof(txn_date);
	struct tm tm_txn;
	time_t time_txn, time_now = time(NULL);
	uint64_t sec, min, hour, mday, mon, year;

	if (!get_value(data, EMV_ID_TRANSACTION_TIME, txn_time, &txn_time_sz) ||
	    txn_time_sz != sizeof(txn_time))
		return false;

	if (!get_value(data, EMV_ID_TRANSACTION_DATE, txn_date, &txn_date_sz) ||
	    txn_date_sz != sizeof(txn_date))
		return false;

	libtlv_bcd_to_u64(&txn_time[2], 1, &sec);
	libtlv_bcd_to_u64(&txn_time[1], 1, &min);
	libtlv_bcd_to_u64(&txn_time[0], 1, &hour);
	libtlv_bcd_to_u64(&txn_date[2], 1, &mday);
	libtlv_bcd_to_u64(&txn_date[1], 1, &mon);
	libtlv_bcd_to_u64(&txn_date[0], 1, &year);

	memset(&tm_txn, 0, sizeof(tm_txn));
	tm_txn.tm_sec  = (int)sec;
	tm_txn.tm_min  = (int)min;
	tm_txn.tm_hour = (int)hour;
	tm_txn.tm_mday = (int)mday;
	tm_txn.tm_mon  = (int)mon - 1;
	tm_txn.tm_year = (int)year + 100;
	time_txn = mktime(&tm_txn);

	if (time_now - time_txn > 3)
		return false;

	if (time_now - time_txn < 0)
		return false;

	return true;
}

static void store_unpredictable_number(struct checker *chk, struct tlv *data)
{
	size_t un_sz = sizeof(chk->uns[0]);

	if (chk->num_uns >= ARRAY_SIZE(chk->uns))
		return;

	if (!get_value(data, EMV_ID_UNPREDICTABLE_NUMBER,
					       &chk->uns[chk->num_uns], &un_sz))
		return;

	if (un_sz != sizeof(chk->uns[0]))
		return;

	chk->num_uns++;
	return;
}

static bool check_unpredictable_numbers(struct checker *chk)
{
	uint32_t and;
	int i, j;

	if (chk->num_uns != 1000)
		return false;

	for (i = 0; i < chk->num_uns; i++)
		for (j = i + 1; j < chk->num_uns; j++)
			if (chk->uns[i] == chk->uns[j])
				return false;

	for (and = 0xffffffffu, i = 0; i < chk->num_uns; i++)
		and &= chk->uns[i];

	if (!!and)
		return false;

	for (j = 0, i = 0; i < chk->num_uns; i++)
		j += __builtin_popcount(chk->uns[i]);

	if (j < 15000)
		return false;

	if (j > 17000)
		return false;

	return true;
}

static void checker_gpo_data(struct chk *checker, struct tlv *data)
{
	struct checker *chk = (struct checker *)checker;
	uint8_t val[256];
	size_t val_sz = sizeof(val);

	switch (chk->pass_criteria) {

	case pc_2ea_001_00_case01:
		if (!check_value(chk, data, EMV_ID_TRANSACTION_TYPE, "\x00", 1)
									      ||
		    !check_value(chk, data, EMV_ID_AMOUNT_AUTHORIZED,
					       "\x00\x00\x00\x00\x00\x10", 6) ||
		    !check_value(chk, data, EMV_ID_AMOUNT_OTHER,
						 "\x00\x00\x00\x00\x00\x00", 6))
			chk->pass_criteria_met = false;
		chk->pass_criteria_checked = true;
		break;

	case pc_2ea_001_00_case02:
		if (!check_value(chk, data, EMV_ID_TRANSACTION_TYPE, "\x00", 1)
									      ||
		    !check_value(chk, data, EMV_ID_AMOUNT_AUTHORIZED,
					       "\x00\x00\x00\x00\x00\x75", 6) ||
		    !check_value(chk, data, EMV_ID_AMOUNT_OTHER,
						 "\x00\x00\x00\x00\x00\x00", 6))
			chk->pass_criteria_met = false;
		chk->pass_criteria_checked = true;
		break;

	case pc_2ea_001_00_case03:
		if (!check_value(chk, data, EMV_ID_TRANSACTION_TYPE, "\x00", 1)
									      ||
		    !check_value(chk, data, EMV_ID_AMOUNT_AUTHORIZED,
					       "\x00\x00\x00\x00\x00\x45", 6) ||
		    !check_value(chk, data, EMV_ID_AMOUNT_OTHER,
						 "\x00\x00\x00\x00\x00\x00", 6))
			chk->pass_criteria_met = false;
		chk->pass_criteria_checked = true;
		break;

	case pc_2ea_002_00:
		if (!check_value(chk, data, EMV_ID_TRANSACTION_TYPE, "\x00", 1)
									      ||
		    !check_value(chk, data, EMV_ID_AMOUNT_AUTHORIZED,
					       "\x00\x00\x00\x00\x00\x10", 6) ||
		    !check_value(chk, data, EMV_ID_AMOUNT_OTHER,
						 "\x00\x00\x00\x00\x00\x00", 6))
			chk->pass_criteria_met = false;
		chk->pass_criteria_checked = true;
		break;

	case pc_2ea_002_01:
		if (!check_value(chk, data, EMV_ID_TRANSACTION_TYPE, "\x01", 1)
									      ||
		    !check_value(chk, data, EMV_ID_AMOUNT_AUTHORIZED,
					       "\x00\x00\x00\x00\x00\x20", 6) ||
		    !check_value(chk, data, EMV_ID_AMOUNT_OTHER,
						 "\x00\x00\x00\x00\x00\x00", 6))
			chk->pass_criteria_met = false;
		chk->pass_criteria_checked = true;
		break;

	case pc_2ea_002_02:
		if (!check_value(chk, data, EMV_ID_TRANSACTION_TYPE, "\x20", 1)
									      ||
		    !check_value(chk, data, EMV_ID_AMOUNT_AUTHORIZED,
					       "\x00\x00\x00\x00\x00\x30", 6) ||
		    !check_value(chk, data, EMV_ID_AMOUNT_OTHER,
						 "\x00\x00\x00\x00\x00\x00", 6))
			chk->pass_criteria_met = false;
		chk->pass_criteria_checked = true;
		break;

	case pc_2ea_003_00_case01:
		if (!check_value(chk, data, EMV_ID_TRANSACTION_TYPE, "\x09", 1)
									      ||
		    !check_value(chk, data, EMV_ID_AMOUNT_AUTHORIZED,
					       "\x00\x00\x00\x00\x00\x30", 6) ||
		    !check_value(chk, data, EMV_ID_AMOUNT_OTHER,
						 "\x00\x00\x00\x00\x00\x10", 6))
			chk->pass_criteria_met = false;
		chk->pass_criteria_checked = true;
		break;

	case pc_2ea_003_00_case02:
		if (!check_value(chk, data, EMV_ID_TRANSACTION_TYPE, "\x09", 1)
									      ||
		    !check_value(chk, data, EMV_ID_AMOUNT_AUTHORIZED,
					       "\x00\x00\x00\x00\x00\x70", 6) ||
		    !check_value(chk, data, EMV_ID_AMOUNT_OTHER,
						 "\x00\x00\x00\x00\x00\x20", 6))
			chk->pass_criteria_met = false;
		chk->pass_criteria_checked = true;
		break;

	case pc_2ea_004_00_case01:
	case pc_2ea_004_00_case02:
		if (!get_value(data, EMV_ID_TEST_FLAGS, val, &val_sz) ||
		    (val_sz != 2) || (val[1] & 0x80u))
			chk->pass_criteria_met = false;
		chk->pass_criteria_checked = true;
		break;

	case pc_2ea_006_02_case01:
		if (!check_terminal_data(chk, data)			      ||
		    !check_txn_date_and_time(chk, data)			      ||
		    !check_value(chk, data,
					 EMV_ID_APPLICATION_IDENTIFIER_TERMINAL,
					   "\xA0\x00\x00\x00\x01\x00\x01", 7) ||
		    !check_value(chk, data, EMV_ID_TRANSACTION_TYPE, "\x00", 1))
			chk->pass_criteria_met = false;
		chk->pass_criteria_checked = true;
		break;

	case pc_2ea_006_02_case02:
		if (!check_terminal_data(chk, data)			      ||
		    !check_txn_date_and_time(chk, data)			      ||
		    !check_value(chk, data,
					 EMV_ID_APPLICATION_IDENTIFIER_TERMINAL,
					   "\xA0\x00\x00\x00\x03\x00\x03", 7) ||
		    !check_value(chk, data, EMV_ID_TRANSACTION_TYPE, "\x00", 1))
			chk->pass_criteria_met = false;
		chk->pass_criteria_checked = true;
		break;

	case pc_2ea_006_03:
		if (!check_terminal_data(chk, data)			      ||
		    !check_txn_date_and_time(chk, data)			      ||
		    !check_value(chk, data,
					 EMV_ID_APPLICATION_IDENTIFIER_TERMINAL,
					   "\xA0\x00\x00\x00\x03\x00\x03", 7) ||
		    !check_value(chk, data, EMV_ID_TRANSACTION_TYPE, "\x09", 1))
			chk->pass_criteria_met = false;
		chk->pass_criteria_checked = true;
		break;

	case pc_2ea_006_04:
		if (!check_terminal_data(chk, data)			      ||
		    !check_txn_date_and_time(chk, data)			      ||
		    !check_value(chk, data,
					 EMV_ID_APPLICATION_IDENTIFIER_TERMINAL,
					   "\xA0\x00\x00\x00\x01\x00\x01", 7) ||
		    !check_value(chk, data, EMV_ID_TRANSACTION_TYPE, "\x01", 1))
			chk->pass_criteria_met = false;
		chk->pass_criteria_checked = true;
		break;

	case pc_2ea_006_05:
		if (!check_terminal_data(chk, data)			      ||
		    !check_txn_date_and_time(chk, data)			      ||
		    !check_value(chk, data,
					 EMV_ID_APPLICATION_IDENTIFIER_TERMINAL,
					   "\xA0\x00\x00\x00\x03\x00\x03", 7) ||
		    !check_value(chk, data, EMV_ID_TRANSACTION_TYPE, "\x20", 1))
			chk->pass_criteria_met = false;
		chk->pass_criteria_checked = true;
		break;

	case pc_2ea_007_00:
		store_unpredictable_number(chk, data);
		break;

	default:
		break;
	}
}

static void checker_txn_start(struct chk *chk)
{
	struct checker *checker = (struct checker *)chk;

	switch (checker->pass_criteria) {

	case pc_2ea_005_00:
		if (checker->field_is_on ||
		    (checker->ui_request.msg_id != msg_welcome))
			checker->pass_criteria_met = false;
		checker->pass_criteria_checked = true;
		break;

	case pc_2ea_005_01:
		if (checker->field_is_on ||
		    (checker->ui_request.status != sts_idle))
			checker->pass_criteria_met = false;
		checker->pass_criteria_checked = true;
		break;

	case pc_2ea_006_00:
		if (!checker->field_is_on ||
		    (checker->ui_request.msg_id != msg_present_card))
			checker->pass_criteria_met = false;
		checker->pass_criteria_checked = true;
		break;

	case pc_2ea_006_01:
		if (!checker->field_is_on ||
		    (checker->ui_request.status != sts_ready_to_read))
			checker->pass_criteria_met = false;
		checker->pass_criteria_checked = true;
		break;

	default:
		break;
	}
}

static void checker_field_on(struct chk *chk)
{
	struct checker *checker = (struct checker *)chk;

	checker->field_is_on = true;
}

static void checker_field_off(struct chk *chk)
{
	struct checker *checker = (struct checker *)chk;

	checker->field_is_on = false;
}

static void checker_ui_request(struct chk *chk,
					const struct emv_ui_request *ui_request)
{
	struct checker *checker = (struct checker *)chk;

	memcpy(&checker->ui_request, ui_request, sizeof(*ui_request));
}

static bool checker_pass_criteria_met(struct chk *chk)
{
	struct checker *checker = (struct checker *)chk;

	switch (checker->pass_criteria) {
	case pc_2ea_007_00:
		return check_unpredictable_numbers(checker);
	default:
		return checker->pass_criteria_checked &&
						     checker->pass_criteria_met;
	}
}

void checker_free(struct chk *chk)
{
	free(chk);
}

static const struct chk_ops checker_ops = {
	.txn_start	   = checker_txn_start,
	.field_on	   = checker_field_on,
	.field_off	   = checker_field_off,
	.gpo_data	   = checker_gpo_data,
	.ui_request	   = checker_ui_request,
	.pass_criteria_met = checker_pass_criteria_met,
	.free		   = checker_free
};

struct chk *chk_pass_criteria_new(enum pass_criteria pass_criteria,
						     const char *log4c_category)
{
	struct checker *checker = NULL;
	char cat[64];

	checker = malloc(sizeof(struct checker));
	if (!checker)
		return (struct chk *)checker;

	memset(checker, 0, sizeof(*checker));
	checker->ops			= &checker_ops;

	snprintf(cat, sizeof(cat), "%s.chk", log4c_category);
	checker->log_cat = log4c_category_get(cat);

	checker->pass_criteria		= pass_criteria;
	checker->pass_criteria_checked	= false;
	checker->pass_criteria_met	= true;

	return (struct chk *)checker;
};
