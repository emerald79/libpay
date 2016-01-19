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
	int		      state;

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

	if (tlv_is_constructed(mytlv))
		rc = tlv_encode(tlv_get_child(mytlv), value, size);
	else
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

static bool check_value_under_mask(struct checker *chk, struct tlv *tlv,
			  const char *tag, void *value, void *mask, size_t size)
{
	uint8_t val[256], dol_val[256], mask_val[256];
	size_t val_sz = sizeof(val), i;
	bool ok = false;

	if (!get_value(tlv, tag, val, &val_sz))
		goto done;

	libtlv_get_dol_field(tag, value, size, dol_val, val_sz);
	libtlv_get_dol_field(tag, mask, size, mask_val, val_sz);

	for (i = 0; i < val_sz; i++) {
		val[i] &= mask_val[i];
		dol_val[i] &= mask_val[i];
	}

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

static void checker_select(struct chk *checker, const uint8_t *data, size_t len)
{
	struct checker *chk = (struct checker *)checker;

	switch (chk->pass_criteria) {

	case pc_2ea_014_00_case01:

		switch (chk->state) {
		/* Case01: The LT shall receive the SELECT command with PPSE
		 * before the Select AID 'A0000000010001' (corresponding to the
		 * APP1 AID) at restart					      */
		case 0:
			if ((len == strlen(DF_NAME_2PAY_SYS_DDF01)) &&
			    (!memcmp(data, DF_NAME_2PAY_SYS_DDF01, len)))
				chk->state = 1;

			break;

		case 1:
			if ((len == 7) &&
			    (!memcmp(data, "\xA0\x00\x00\x00\x01\x00\x01", 7)))
				chk->state = 2;
			break;

		case 2:
			if ((len == strlen(DF_NAME_2PAY_SYS_DDF01)) &&
			    (!memcmp(data, DF_NAME_2PAY_SYS_DDF01, len)))
				chk->state = 3;

			break;

		case 3:
			if ((len == 7) &&
			    (!memcmp(data, "\xA0\x00\x00\x00\x01\x00\x01", 7)))
				chk->state = 4;
			break;

		default:
			chk->pass_criteria_met = false;
			chk->pass_criteria_checked = true;
		}
		break;

	case pc_2ea_014_00_case02:
		/* Case02: At restart, the LT shall receive the Select AID
		 * 'A0000000020002' (corresponding to the the APP2 AID) without
		 * a previous SELECT command with PPSE			      */
		switch (chk->state) {

		case 0:
			if ((len == strlen(DF_NAME_2PAY_SYS_DDF01)) &&
			    (!memcmp(data, DF_NAME_2PAY_SYS_DDF01, len)))
				chk->state = 1;

			break;

		case 1:
			if ((len == 7) &&
			    (!memcmp(data, "\xA0\x00\x00\x00\x02\x00\x02", 7)))
				chk->state = 2;
			break;

		case 2:
			break;

		case 3:
			if ((len == strlen(DF_NAME_2PAY_SYS_DDF01)) &&
			    (!memcmp(data, DF_NAME_2PAY_SYS_DDF01, len)))
				chk->pass_criteria_met = false;

			if ((len == 7) &&
			    (!memcmp(data, "\xA0\x00\x00\x00\x02\x00\x02", 7)))
				chk->state = 4;
			break;

		default:
			chk->pass_criteria_met = false;
			chk->pass_criteria_checked = true;
		}
		break;

	case pc_2ea_014_00_case03:
		/* Case03: At restart, the LT shall receive the Select AID
		 * 'A0000000030003' (corresponding to the the APP3 AID) without
		 * a previous SELECT command with PPSE			      */
		switch (chk->state) {

		case 0:
			if ((len == strlen(DF_NAME_2PAY_SYS_DDF01)) &&
			    (!memcmp(data, DF_NAME_2PAY_SYS_DDF01, len)))
				chk->state = 1;

			break;

		case 1:
			if ((len == 7) &&
			    (!memcmp(data, "\xA0\x00\x00\x00\x03\x00\x03", 7)))
				chk->state = 2;
			break;

		case 2:
			break;

		case 3:
			if ((len == strlen(DF_NAME_2PAY_SYS_DDF01)) &&
			    (!memcmp(data, DF_NAME_2PAY_SYS_DDF01, len)))
				chk->pass_criteria_met = false;

			if ((len == 7) &&
			    (!memcmp(data, "\xA0\x00\x00\x00\x03\x00\x03", 7)))
				chk->state = 4;
			break;

		default:
			chk->pass_criteria_met = false;
			chk->pass_criteria_checked = true;
		}
		break;

	case pc_2ea_014_00_case04:
		/* Case04: The LT shall not receive the SELECT command with PPSE
		 * at restart.
		 * The LT shall not receive the SELECT AID 'A0000000040004'
		 * (corresponding to the APP4 AID) at restart		      */
		switch (chk->state) {

		case 0:
			if ((len == strlen(DF_NAME_2PAY_SYS_DDF01)) &&
			    (!memcmp(data, DF_NAME_2PAY_SYS_DDF01, len)))
				chk->state = 1;

			break;

		case 1:
			if ((len == 7) &&
			    (!memcmp(data, "\xA0\x00\x00\x00\x04\x00\x04", 7)))
				chk->state = 2;
			break;

		default:
			chk->pass_criteria_met = false;
			chk->pass_criteria_checked = true;
		}
		break;

	case pc_2ea_014_00_case05:
		/* Case05: At restart, the LT shall receive the SELECT AID
		 * 'A0000000030003' (corresponding to the the APP3 AID) without
		 * a previous SELECT command with PPSE.			      */
		switch (chk->state) {

		case 0:
			if ((len == strlen(DF_NAME_2PAY_SYS_DDF01)) &&
			    (!memcmp(data, DF_NAME_2PAY_SYS_DDF01, len)))
				chk->state = 1;

			break;

		case 1:
			if ((len == 7) &&
			    (!memcmp(data, "\xA0\x00\x00\x00\x03\x00\x03", 7)))
				chk->state = 2;
			break;

		case 2:
			if ((len == 7) &&
			    (!memcmp(data, "\xA0\x00\x00\x00\x03\x00\x03", 7)))
				chk->state = 3;
			break;

		default:
			chk->pass_criteria_met = false;
			chk->pass_criteria_checked = true;
		}
		break;

	case pc_2ea_014_00_case06:
		/* Case06: The LT shall not receive the SELECT command with
		 * PPSE at restart
		 * Case06: The LT shall not receive the SELECT AID
		 * 'A0000000040004' (corresponding to the APP4 AID) at restart*/
		switch (chk->state) {

		case 0:
			if ((len == strlen(DF_NAME_2PAY_SYS_DDF01)) &&
			    (!memcmp(data, DF_NAME_2PAY_SYS_DDF01, len)))
				chk->state = 1;

			break;

		case 1:
			if ((len == 7) &&
			    (!memcmp(data, "\xA0\x00\x00\x00\x04\x00\x04", 7)))
				chk->state = 2;
			break;

		default:
			chk->pass_criteria_met = false;
			chk->pass_criteria_checked = true;
		}
		break;

	case pc_2ea_014_01_case01:
		switch (chk->state) {
		case 0:
			break;

		case 2:
			if ((len == strlen(DF_NAME_2PAY_SYS_DDF01)) &&
			    (!memcmp(data, DF_NAME_2PAY_SYS_DDF01, len)))
				chk->state = 3;

			break;

		case 3:
			if ((len == 7) &&
			    (!memcmp(data, "\xA0\x00\x00\x00\x01\x00\x01", 7)))
				chk->state = 4;
			break;

		default:
			chk->pass_criteria_met = false;
			chk->pass_criteria_checked = true;
		}
		break;

	case pc_2ea_014_01_case02:
		switch (chk->state) {
		case 0:
			break;

		case 1:
			if ((len == strlen(DF_NAME_2PAY_SYS_DDF01)) &&
			    (!memcmp(data, DF_NAME_2PAY_SYS_DDF01, len)))
				chk->state = 2;

			break;

		case 2:
			if ((len == 7) &&
			    (!memcmp(data, "\xA0\x00\x00\x00\x02\x00\x02", 7)))
				chk->state = 3;
			break;

		default:
			chk->pass_criteria_met = false;
			chk->pass_criteria_checked = true;
		}
		break;

	case pc_2ea_016_00_case01:
		switch (chk->state) {

		case 0:
			if ((len == strlen(DF_NAME_2PAY_SYS_DDF01)) &&
			    (!memcmp(data, DF_NAME_2PAY_SYS_DDF01, len)))
				chk->state = 1;

			break;

		case 1:
			if ((len == 7) &&
			    (!memcmp(data, "\xA0\x00\x00\x00\x01\x00\x01", 7)))
				chk->state = 2;
			break;

		case 2:
			if ((len == strlen(DF_NAME_2PAY_SYS_DDF01)) &&
			    (!memcmp(data, DF_NAME_2PAY_SYS_DDF01, len)))
				chk->state = 3;

			break;

		case 3:
			if ((len == 7) &&
			    (!memcmp(data, "\xA0\x00\x00\x00\x01\x00\x01", 7)))
				chk->state = 4;
			break;

		default:
			chk->pass_criteria_met = false;
			chk->pass_criteria_checked = true;
		}
		break;

	case pc_2ea_016_00_case02:
		switch (chk->state) {

		case 0:
			if ((len == strlen(DF_NAME_2PAY_SYS_DDF01)) &&
			    (!memcmp(data, DF_NAME_2PAY_SYS_DDF01, len)))
				chk->state = 1;

			break;

		case 1:
			if ((len == 7) &&
			    (!memcmp(data, "\xA0\x00\x00\x00\x02\x00\x02", 7)))
				chk->state = 2;
			break;

		case 2:
			if ((len == strlen(DF_NAME_2PAY_SYS_DDF01)) &&
			    (!memcmp(data, DF_NAME_2PAY_SYS_DDF01, len)))
				chk->state = 3;

			break;

		case 3:
			if ((len == 7) &&
			    (!memcmp(data, "\xA0\x00\x00\x00\x02\x00\x02", 7)))
				chk->state = 4;
			break;

		default:
			chk->pass_criteria_met = false;
			chk->pass_criteria_checked = true;
		}
		break;

	case pc_2ea_016_00_case03:
		switch (chk->state) {

		case 0:
			if ((len == strlen(DF_NAME_2PAY_SYS_DDF01)) &&
			    (!memcmp(data, DF_NAME_2PAY_SYS_DDF01, len)))
				chk->state = 1;

			break;

		case 1:
			if ((len == 7) &&
			    (!memcmp(data, "\xA0\x00\x00\x00\x03\x00\x03", 7)))
				chk->state = 2;
			break;

		case 2:
			if ((len == strlen(DF_NAME_2PAY_SYS_DDF01)) &&
			    (!memcmp(data, DF_NAME_2PAY_SYS_DDF01, len)))
				chk->pass_criteria_met = false;

			if ((len == 7) &&
			    (!memcmp(data, "\xA0\x00\x00\x00\x03\x00\x03", 7)))
				chk->state = 3;
			break;

		default:
			chk->pass_criteria_met = false;
			chk->pass_criteria_checked = true;
		}
		break;

	case pc_2ea_016_00_case04:
		switch (chk->state) {

		case 0:
			if ((len == strlen(DF_NAME_2PAY_SYS_DDF01)) &&
			    (!memcmp(data, DF_NAME_2PAY_SYS_DDF01, len)))
				chk->state = 1;

			break;

		case 1:
			if ((len == 7) &&
			    (!memcmp(data, "\xA0\x00\x00\x00\x04\x00\x04", 7)))
				chk->state = 2;
			break;

		default:
			chk->pass_criteria_met = false;
			chk->pass_criteria_checked = true;
		}
		break;

	case pc_2ea_016_00_case05:

		switch (chk->state) {

		case 0:
			if ((len == strlen(DF_NAME_2PAY_SYS_DDF01)) &&
			    (!memcmp(data, DF_NAME_2PAY_SYS_DDF01, len)))
				chk->state = 1;

			break;

		case 1:
			if ((len == 7) &&
			    (!memcmp(data, "\xA0\x00\x00\x00\x01\x00\x01", 7)))
				chk->state = 2;
			break;

		case 3:
			if ((len == strlen(DF_NAME_2PAY_SYS_DDF01)) &&
			    (!memcmp(data, DF_NAME_2PAY_SYS_DDF01, len)))
				chk->state = 4;

			break;

		case 4:
			if ((len == 7) &&
			    (!memcmp(data, "\xA0\x00\x00\x00\x01\x00\x01", 7)))
				chk->state = 5;
			break;

		default:
			chk->pass_criteria_met = false;
			chk->pass_criteria_checked = true;
		}
		break;

	case pc_2ea_016_00_case06:

		switch (chk->state) {

		case 0:
			if ((len == strlen(DF_NAME_2PAY_SYS_DDF01)) &&
			    (!memcmp(data, DF_NAME_2PAY_SYS_DDF01, len)))
				chk->state = 1;

			break;

		case 1:
			if ((len == 7) &&
			    (!memcmp(data, "\xA0\x00\x00\x00\x02\x00\x02", 7)))
				chk->state = 2;
			break;

		case 3:
			if ((len == 7) &&
			    (!memcmp(data, "\xA0\x00\x00\x00\x02\x00\x02", 7)))
				chk->state = 4;
			break;

		default:
			chk->pass_criteria_met = false;
			chk->pass_criteria_checked = true;
		}
		break;

	case pc_2ea_016_00_case07:

		switch (chk->state) {

		case 0:
			if ((len == strlen(DF_NAME_2PAY_SYS_DDF01)) &&
			    (!memcmp(data, DF_NAME_2PAY_SYS_DDF01, len)))
				chk->state = 1;

			break;

		case 1:
			if ((len == 7) &&
			    (!memcmp(data, "\xA0\x00\x00\x00\x03\x00\x03", 7)))
				chk->state = 2;
			break;

		case 3:
			if ((len == 7) &&
			    (!memcmp(data, "\xA0\x00\x00\x00\x03\x00\x03", 7)))
				chk->state = 4;
			break;

		default:
			chk->pass_criteria_met = false;
			chk->pass_criteria_checked = true;
		}
		break;

	case pc_2ea_016_00_case08:

		switch (chk->state) {

		case 0:
			if ((len == strlen(DF_NAME_2PAY_SYS_DDF01)) &&
			    (!memcmp(data, DF_NAME_2PAY_SYS_DDF01, len)))
				chk->state = 1;

			break;

		case 1:
			if ((len == 7) &&
			    (!memcmp(data, "\xA0\x00\x00\x00\x04\x00\x04", 7)))
				chk->state = 2;
			break;

		default:
			chk->pass_criteria_met = false;
			chk->pass_criteria_checked = true;
		}
		break;

	case pc_2ea_017_00_case01:

		switch (chk->state) {

		case 0:
			if ((len == strlen(DF_NAME_2PAY_SYS_DDF01)) &&
			    (!memcmp(data, DF_NAME_2PAY_SYS_DDF01, len)))
				chk->state = 1;

			break;

		case 1:
			if ((len == 7) &&
			    (!memcmp(data, "\xA0\x00\x00\x00\x03\x00\x03", 7)))
				chk->state = 2;
			break;

		default:
			chk->pass_criteria_met = false;
			chk->pass_criteria_checked = true;
		}
		break;

	case pc_2ea_017_00_case02:

		switch (chk->state) {

		case 0:
			if ((len == strlen(DF_NAME_2PAY_SYS_DDF01)) &&
			    (!memcmp(data, DF_NAME_2PAY_SYS_DDF01, len)))
				chk->state = 1;

			break;

		case 1:
			if ((len == 7) &&
			    (!memcmp(data, "\xA0\x00\x00\x00\x02\x00\x02", 7)))
				chk->state = 2;
			break;

		default:
			chk->pass_criteria_met = false;
			chk->pass_criteria_checked = true;
		}
		break;

	case pc_2ea_017_00_case03:

		switch (chk->state) {

		case 0:
			if ((len == strlen(DF_NAME_2PAY_SYS_DDF01)) &&
			    (!memcmp(data, DF_NAME_2PAY_SYS_DDF01, len)))
				chk->state = 1;

			break;

		case 1:
			if ((len == 7) &&
			    (!memcmp(data, "\xA0\x00\x00\x00\x03\x00\x03", 7)))
				chk->state = 2;
			break;

		default:
			chk->pass_criteria_met = false;
			chk->pass_criteria_checked = true;
		}
		break;

	case pc_2ea_017_01:

		switch (chk->state) {

		case 0:
			if ((len == strlen(DF_NAME_2PAY_SYS_DDF01)) &&
			    (!memcmp(data, DF_NAME_2PAY_SYS_DDF01, len)))
				chk->state = 1;

			break;

		case 1:
			if ((len == 7) &&
			    (!memcmp(data, "\xA0\x00\x00\x00\x03\x00\x03", 7)))
				chk->state = 2;
			break;

		case 3:
			break;

		default:
			chk->pass_criteria_met = false;
			chk->pass_criteria_checked = true;
		}
		break;

	case pc_2ea_018_00:

		switch (chk->state) {

		case 0:
			if ((len == strlen(DF_NAME_2PAY_SYS_DDF01)) &&
			    (!memcmp(data, DF_NAME_2PAY_SYS_DDF01, len)))
				chk->state = 1;

			break;

		case 1:
			if ((len == 7) &&
			    (!memcmp(data, "\xA0\x00\x00\x00\x01\x00\x01", 7)))
				chk->state = 2;
			break;

		default:
			chk->pass_criteria_met = false;
			chk->pass_criteria_checked = true;
		}
		break;

	case pc_2ea_020_00_case01:

		switch (chk->state) {

		case 0:
			if ((len == strlen(DF_NAME_2PAY_SYS_DDF01)) &&
			    (!memcmp(data, DF_NAME_2PAY_SYS_DDF01, len)))
				chk->state = 1;

			break;

		case 1:
			if ((len == 7) &&
			    (!memcmp(data, "\xA0\x00\x00\x00\x02\x00\x02", 7)))
				chk->state = 2;
			break;

		default:
			chk->pass_criteria_met = false;
			chk->pass_criteria_checked = true;
		}
		break;


	case pc_2ea_020_00_case02:

		switch (chk->state) {

		case 0:
			if ((len == strlen(DF_NAME_2PAY_SYS_DDF01)) &&
			    (!memcmp(data, DF_NAME_2PAY_SYS_DDF01, len)))
				chk->state = 1;

			break;

		case 1:
			if ((len == 7) &&
			    (!memcmp(data, "\xA0\x00\x00\x00\x04\x00\x04", 7)))
				chk->state = 2;
			break;

		default:
			chk->pass_criteria_met = false;
			chk->pass_criteria_checked = true;
		}
		break;

	case pc_2ea_021_00:

		switch (chk->state) {

		case 0:
			if ((len == strlen(DF_NAME_2PAY_SYS_DDF01)) &&
			    (!memcmp(data, DF_NAME_2PAY_SYS_DDF01, len)))
				chk->state = 1;

			break;

		case 1:
			if ((len == 7) &&
			    (!memcmp(data, "\xA0\x00\x00\x00\x03\x00\x03", 7)))
				chk->state = 2;
			break;

		default:
			chk->pass_criteria_met = false;
			chk->pass_criteria_checked = true;
		}
		break;

	case pc_2eb_010_00_case01:
	case pc_2eb_010_01_case01:
	case pc_2eb_011_00_case03:
	case pc_2ed_003_01_case01:
	case pc_2ed_007_00:
	case pc_2ed_007_01:
		switch (chk->state) {

		case 0:
			if ((len == strlen(DF_NAME_2PAY_SYS_DDF01)) &&
			    (!memcmp(data, DF_NAME_2PAY_SYS_DDF01, len)))
				chk->state = 1;
			break;

		case 1:
			if ((len == 7) &&
			    (!memcmp(data, "\xA0\x00\x00\x00\x01\x00\x01", 7)))
				chk->state = 2;
			else
				chk->pass_criteria_met = false;
			chk->pass_criteria_checked = true;
			break;


		default:
			chk->pass_criteria_met = false;
			chk->pass_criteria_checked = true;
		}
		break;

	case pc_2eb_007_00_case01:
	case pc_2eb_007_01_case01:
	case pc_2eb_010_00_case02:
	case pc_2eb_010_00_case03:
	case pc_2eb_010_00_case05:
	case pc_2eb_010_01_case02:
	case pc_2eb_010_01_case03:
	case pc_2eb_011_01:
	case pc_2eb_012_00_case01:
	case pc_2eb_021_00_case01:
	case pc_2ed_003_01_case02:
	case pc_2ed_008_00:
		switch (chk->state) {

		case 0:
			if ((len == strlen(DF_NAME_2PAY_SYS_DDF01)) &&
			    (!memcmp(data, DF_NAME_2PAY_SYS_DDF01, len)))
				chk->state = 1;
			break;

		case 1:
			if ((len == 7) &&
			    (!memcmp(data, "\xA0\x00\x00\x00\x02\x00\x02", 7)))
				chk->state = 2;
			else
				chk->pass_criteria_met = false;
			chk->pass_criteria_checked = true;
			break;


		default:
			chk->pass_criteria_met = false;
			chk->pass_criteria_checked = true;
		}
		break;

	case pc_2eb_007_00_case02:
	case pc_2eb_011_00_case01:
	case pc_2eb_011_00_case02:
	case pc_2eb_011_02:
	case pc_2eb_012_00_case02:
	case pc_2eb_021_00_case02:
	case pc_2ed_003_01_case03:
		switch (chk->state) {

		case 0:
			if ((len == strlen(DF_NAME_2PAY_SYS_DDF01)) &&
			    (!memcmp(data, DF_NAME_2PAY_SYS_DDF01, len)))
				chk->state = 1;
			break;

		case 1:
			if ((len == 7) &&
			    (!memcmp(data, "\xA0\x00\x00\x00\x03\x00\x03", 7)))
				chk->state = 2;
			else
				chk->pass_criteria_met = false;
			chk->pass_criteria_checked = true;
			break;

		default:
			chk->pass_criteria_met = false;
			chk->pass_criteria_checked = true;
		}
		break;

	case pc_2eb_007_01_case02:
	case pc_2eb_010_00_case04:
	case pc_2eb_011_00_case04:
	case pc_2eb_012_01:
		switch (chk->state) {

		case 0:
			if ((len == strlen(DF_NAME_2PAY_SYS_DDF01)) &&
			    (!memcmp(data, DF_NAME_2PAY_SYS_DDF01, len)))
				chk->state = 1;
			break;

		case 1:
			if ((len == 7) &&
			    (!memcmp(data, "\xA0\x00\x00\x00\x04\x00\x04", 7)))
				chk->state = 2;
			else
				chk->pass_criteria_met = false;
			chk->pass_criteria_checked = true;
			break;

		default:
			chk->pass_criteria_met = false;
			chk->pass_criteria_checked = true;
		}
		break;

	case pc_2eb_022_00:
		chk->pass_criteria_met = false;
		break;

	case pc_2ec_005_00_case01:
	case pc_2ec_005_00_case02:
	case pc_2ec_005_00_case03:
	case pc_2ec_005_00_case04:
		if (chk->state == 0) {
			if ((len != strlen(DF_NAME_2PAY_SYS_DDF01)) ||
			    (memcmp(data, DF_NAME_2PAY_SYS_DDF01, len)))
				chk->pass_criteria_met = false;
			chk->pass_criteria_checked = true;
			chk->state = 1;
		}
		break;

	case pc_2ec_006_00_case01:
	case pc_2ec_006_00_case02:
	case pc_2ec_006_00_case03:
	case pc_2ec_006_00_case04:
	case pc_2ec_007_00_case01:
	case pc_2ec_007_00_case02:
	case pc_2ec_007_00_case03:
	case pc_2ec_007_00_case04:
		if (chk->state == 1)
			chk->pass_criteria_checked = true;
		break;

	case pc_2ed_001_00_case01:
	case pc_2ed_001_00_case02:
	case pc_2ed_001_00_case04:
	case pc_2ed_001_00_case05:
		if (chk->state == 1) {
			if ((len == 7) &&
			    (!memcmp(data, "\xA0\x00\x00\x00\x03\x00\x03", 7)))
				chk->state = 2;
			else
				chk->pass_criteria_met = false;
		}
		break;

	case pc_2ed_001_00_case03:
	case pc_2ed_001_00_case06:
		if (chk->state == 1) {
			if ((len == 7) &&
			    (!memcmp(data, "\xA0\x00\x00\x00\x01\x00\x01", 7)))
				chk->state = 2;
			else
				chk->pass_criteria_met = false;
		}
		break;

	case pc_2ed_001_01_case01:
	case pc_2ed_001_01_case02:
	case pc_2ed_001_01_case03:
	case pc_2ed_001_01_case04:
	case pc_2ed_001_01_case05:
	case pc_2ed_001_01_case06:
	case pc_2ed_002_01:
		if (chk->state == 1) {
			if ((len == 8) &&
			    (!memcmp(data, "\xA0\x00\x00\x00\x04\x00\x04\x04",
									    8)))
				chk->state = 2;
			else
				chk->pass_criteria_met = false;
		}
		break;

	case pc_2ed_002_00:
		if (chk->state == 1) {
			if ((len == 7) &&
			    (!memcmp(data, "\xA0\x00\x00\x00\x02\x00\x02", 7)))
				chk->state = 2;
			else
				chk->pass_criteria_met = false;
		}
		break;

	case pc_2ed_003_00:
		if (chk->state == 0) {
			chk->state = 1;

			if ((len != strlen(DF_NAME_2PAY_SYS_DDF01)) ||
			    (memcmp(data, DF_NAME_2PAY_SYS_DDF01, len)))
				chk->pass_criteria_met = false;
			chk->pass_criteria_checked = true;
		}
		break;

	case pc_2ed_009_00:
		if (chk->state == 0) {
			if ((len == 7) &&
			    (!memcmp(data, "\xA0\x00\x00\x00\x03\x10\x10", 7)))
				chk->state = 1;
		}
		break;

	case pc_2ed_009_02:
		if (chk->state == 0) {
			if ((len == 8) &&
			    (!memcmp(data, "\xA0\x00\x00\x00\x04\x10\x10\x01",
									    8)))
				chk->state = 1;
		}
		break;

	case pc_2ed_009_04:
		if (chk->state == 0) {
			if ((len == 7) &&
			    (!memcmp(data, "\xA0\x00\x00\x00\x65\x10\x10", 7)))
				chk->state = 1;
		}
		break;

	case pc_2ed_009_06_case01:
	case pc_2ed_009_16:
		if (chk->state == 0) {
			if ((len == 8) &&
			    (!memcmp(data, "\xA0\x00\x00\x00\x25\x10\x10\x01",
									    8)))
				chk->state = 1;
		}
		break;

	case pc_2ed_009_06_case02:
		if (chk->state == 0) {
			if ((len == 7) &&
			    (!memcmp(data, "\xA0\x00\x00\x00\x25\x10\x10", 7)))
				chk->state = 1;
		}
		break;

	case pc_2ed_009_10:
		if (chk->state == 0) {
			if ((len == 8) &&
			    (!memcmp(data, "\xA0\x00\x00\x00\x03\x10\x10\x01",
									    8)))
				chk->state = 1;
		}
		break;

	case pc_2ed_009_12:
		if (chk->state == 0) {
			if ((len == 7) &&
			    (!memcmp(data, "\xA0\x00\x00\x00\x04\x10\x10", 7)))
				chk->state = 1;
		}
		break;

	case pc_2ed_009_14:
		if (chk->state == 0) {
			if ((len == 8) &&
			    (!memcmp(data, "\xA0\x00\x00\x00\x65\x10\x10\x01",
									    8)))
				chk->state = 1;
		}
		break;

	case pc_2ed_009_18:
		if (chk->state == 0) {
			if ((len == 7) &&
			    (!memcmp(data, "\xA0\x00\x00\x01\x52\x30\x10", 7)))
				chk->state = 1;
		}
		break;

	case pc_2ed_009_20:
		if (chk->state == 0) {
			if ((len == 8) &&
			    (!memcmp(data, "\xA0\x00\x00\x01\x52\x30\x10\x01",
									    8)))
				chk->state = 1;
		}
		break;

	case pc_2ed_009_22:
	case pc_2ed_009_24:
		if (chk->state == 0) {
			if ((len == 7) &&
			    (!memcmp(data, "\xA0\x00\x00\x03\x33\x01\x01", 7)))
				chk->state = 1;
		}
		break;

	case pc_2ed_009_26:
		if (chk->state == 0) {
			if ((len == 7) &&
			    (!memcmp(data, "\xA0\x00\x00\x03\x24\x10\x10", 7)))
				chk->state = 1;
		}
		break;

	case pc_2ed_009_28:
		if (chk->state == 0) {
			if ((len == 8) &&
			    (!memcmp(data, "\xA0\x00\x00\x03\x24\x10\x10\x01",
									    8)))
				chk->state = 1;
		}
		break;

	case pc_2ed_010_00:
	case pc_2ed_011_00:
		if (chk->state == 0) {
			if ((len == 7) &&
			    (!memcmp(data, "\xA0\x00\x00\x00\x04\x00\x04", 7)))
				chk->state = 1;
		}
		break;

	default:
		break;
	}

	if (!chk->pass_criteria_met) {
		char hex[2 * len + 1];

		log4c_category_log(chk->log_cat, LOG4C_PRIORITY_NOTICE,
			       "%s('%s): pass criteria check failed!", __func__,
					     libtlv_bin_to_hex(data, len, hex));
	}
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

	case pc_2ea_014_00_case01:
		if (chk->state != 4)
			break;

		if (!check_value_under_mask(chk, data, EMV_ID_TEST_FLAGS,
						   "\x00\x80", "\x00\x80", 2) ||
		    !check_value(chk, data, EMV_ID_START_POINT, "\x0A", 1)    ||
		    !check_value(chk, data, EMV_ID_ISSUER_AUTHENTICATION_DATA,
				  "\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B"
						    "\x0C\x0D\x0E\x0F\x10", 16))
			chk->pass_criteria_met = false;
		chk->pass_criteria_checked = true;
		break;

	case pc_2ea_014_00_case02:
		if (chk->state != 4)
			break;

		if (!check_value_under_mask(chk, data, EMV_ID_TEST_FLAGS,
						   "\x00\x80", "\x00\x80", 2) ||
		    !check_value(chk, data, EMV_ID_START_POINT, "\x0B", 1)    ||
		    !check_value(chk, data, EMV_ID_ISSUER_SCRIPT_TEMPLATE_1,
				  "\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B"
						    "\x1C\x1D\x1E\x1F\x20", 16))
			chk->pass_criteria_met = false;
		chk->pass_criteria_checked = true;
		break;

	case pc_2ea_014_00_case03:
		if (chk->state != 4)
			break;

		if (!check_value_under_mask(chk, data, EMV_ID_TEST_FLAGS,
						   "\x00\x80", "\x00\x80", 2) ||
		    !check_value(chk, data, EMV_ID_START_POINT, "\x0C", 1)    ||
		    !check_value(chk, data, EMV_ID_ISSUER_SCRIPT_TEMPLATE_2,
				  "\x21\x22\x23\x24\x25\x26\x27\x28\x29\x2A\x2B"
						    "\x2C\x2D\x2E\x2F\x30", 16))
			chk->pass_criteria_met = false;
		chk->pass_criteria_checked = true;
		break;

	case pc_2ea_014_00_case04:
		if (chk->state == 2) {
			chk->state = 3;
			break;
		}

		if (chk->state != 3)
			break;

		if (!check_value_under_mask(chk, data, EMV_ID_TEST_FLAGS,
						   "\x00\x80", "\x00\x80", 2) ||
		    !check_value(chk, data, EMV_ID_START_POINT, "\x0D", 1)    ||
		    !check_value(chk, data, EMV_ID_ISSUER_AUTHENTICATION_DATA,
				  "\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B"
						  "\x0C\x0D\x0E\x0F\x10", 16) ||
		    !check_value(chk, data, EMV_ID_ISSUER_SCRIPT_TEMPLATE_1,
				  "\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B"
						    "\x1C\x1D\x1E\x1F\x20", 16))
			chk->pass_criteria_met = false;
		chk->pass_criteria_checked = true;
		break;

	case pc_2ea_014_00_case05:
		if (chk->state != 3)
			break;

		if (!check_value_under_mask(chk, data, EMV_ID_TEST_FLAGS,
						   "\x00\x80", "\x00\x80", 2) ||
		    !check_value(chk, data, EMV_ID_START_POINT, "\x0C", 1)    ||
		    !check_value(chk, data, EMV_ID_ISSUER_AUTHENTICATION_DATA,
				  "\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B"
						  "\x0C\x0D\x0E\x0F\x10", 16) ||
		    !check_value(chk, data, EMV_ID_ISSUER_SCRIPT_TEMPLATE_2,
				  "\x21\x22\x23\x24\x25\x26\x27\x28\x29\x2A\x2B"
						    "\x2C\x2D\x2E\x2F\x30", 16))
			chk->pass_criteria_met = false;
		chk->pass_criteria_checked = true;
		break;

	case pc_2ea_014_00_case06:
		if (chk->state != 3)
			break;

		if (!check_value_under_mask(chk, data, EMV_ID_TEST_FLAGS,
						   "\x00\x80", "\x00\x80", 2) ||
		    !check_value(chk, data, EMV_ID_START_POINT, "\x0D", 1)    ||
		    !check_value(chk, data, EMV_ID_ISSUER_AUTHENTICATION_DATA,
				  "\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B"
						  "\x0C\x0D\x0E\x0F\x10", 16) ||
		    !check_value(chk, data, EMV_ID_ISSUER_SCRIPT_TEMPLATE_2,
				  "\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B"
						    "\x1C\x1D\x1E\x1F\x20", 16))
			chk->pass_criteria_met = false;
		chk->pass_criteria_checked = true;
		break;

	case pc_2ea_014_01_case01:
		if (chk->state != 4)
			break;

		if (!check_value_under_mask(chk, data, EMV_ID_TEST_FLAGS,
						   "\x00\x80", "\x00\x80", 2) ||
		    !check_value(chk, data, EMV_ID_START_POINT, "\x0A", 1))
			chk->pass_criteria_met = false;
		chk->pass_criteria_checked = true;
		break;

	case pc_2ea_014_01_case02:
		if (chk->state != 3)
			break;

		if (!check_value_under_mask(chk, data, EMV_ID_TEST_FLAGS,
						   "\x00\x80", "\x00\x80", 2) ||
		    !check_value(chk, data, EMV_ID_START_POINT, "\x0B", 1))
			chk->pass_criteria_met = false;
		chk->pass_criteria_checked = true;
		break;

	case pc_2ea_016_00_case01:
		if (chk->state != 4)
			break;

		if (!check_value_under_mask(chk, data, EMV_ID_TEST_FLAGS,
						   "\x00\x80", "\x00\x80", 2) ||
		    !check_value(chk, data, EMV_ID_START_POINT, "\x0A", 1))
			chk->pass_criteria_met = false;
		chk->pass_criteria_checked = true;
		break;

	case pc_2ea_016_00_case02:
		if (chk->state != 4)
			break;

		if (!check_value_under_mask(chk, data, EMV_ID_TEST_FLAGS,
						   "\x00\x80", "\x00\x80", 2) ||
		    !check_value(chk, data, EMV_ID_START_POINT, "\x0B", 1))
			chk->pass_criteria_met = false;
		chk->pass_criteria_checked = true;
		break;

	case pc_2ea_016_00_case03:
		if (chk->state != 3)
			break;

		if (!check_value_under_mask(chk, data, EMV_ID_TEST_FLAGS,
						   "\x00\x80", "\x00\x80", 2) ||
		    !check_value(chk, data, EMV_ID_START_POINT, "\x0C", 1))
			chk->pass_criteria_met = false;
		chk->pass_criteria_checked = true;
		break;

	case pc_2ea_016_00_case04:
		if (chk->state != 3)
			break;

		if (!check_value_under_mask(chk, data, EMV_ID_TEST_FLAGS,
						   "\x00\x80", "\x00\x80", 2) ||
		    !check_value(chk, data, EMV_ID_START_POINT, "\x0D", 1))
			chk->pass_criteria_met = false;
		chk->pass_criteria_checked = true;
		break;

	case pc_2ea_016_00_case05:
		if (chk->state != 5)
			break;

		if (!check_value_under_mask(chk, data, EMV_ID_TEST_FLAGS,
						   "\x00\x80", "\x00\x80", 2) ||
		    !check_value(chk, data, EMV_ID_START_POINT, "\x0A", 1))
			chk->pass_criteria_met = false;
		chk->pass_criteria_checked = true;
		break;

	case pc_2ea_016_00_case06:
		if (chk->state != 4)
			break;

		if (!check_value_under_mask(chk, data, EMV_ID_TEST_FLAGS,
						   "\x00\x80", "\x00\x80", 2) ||
		    !check_value(chk, data, EMV_ID_START_POINT, "\x0B", 1))
			chk->pass_criteria_met = false;
		chk->pass_criteria_checked = true;
		break;

	case pc_2ea_016_00_case07:
		if (chk->state != 4)
			break;

		if (!check_value_under_mask(chk, data, EMV_ID_TEST_FLAGS,
						   "\x00\x80", "\x00\x80", 2) ||
		    !check_value(chk, data, EMV_ID_START_POINT, "\x0C", 1))
			chk->pass_criteria_met = false;
		chk->pass_criteria_checked = true;
		break;

	case pc_2ea_016_00_case08:
		if (chk->state != 3)
			break;

		if (!check_value_under_mask(chk, data, EMV_ID_TEST_FLAGS,
						   "\x00\x80", "\x00\x80", 2) ||
		    !check_value(chk, data, EMV_ID_START_POINT, "\x0D", 1))
			chk->pass_criteria_met = false;
		chk->pass_criteria_checked = true;
		break;

	case pc_2ea_017_00_case01:
		if (chk->state != 2)
			break;

		if (!check_value(chk, data, EMV_ID_KERNEL_IDENTIFIER, "\x21",
									     1))
			chk->pass_criteria_met = false;
		chk->pass_criteria_checked = true;
		break;

	case pc_2ea_017_00_case02:
		if (chk->state != 2)
			break;

		if (!check_value(chk, data, EMV_ID_KERNEL_IDENTIFIER, "\x24",
									     1))
			chk->pass_criteria_met = false;
		chk->pass_criteria_checked = true;
		break;

	case pc_2ea_017_00_case03:
		if (chk->state != 2)
			break;

		if (!check_value(chk, data, EMV_ID_KERNEL_IDENTIFIER, "\x21",
									   1) ||
		    !check_value(chk, data,
					 EMV_ID_TERMINAL_TRANSACTION_QUALIFIERS,
							 "\xF7\x2A\x8A\x55", 4))
			chk->pass_criteria_met = false;
		chk->pass_criteria_checked = true;
		break;

	case pc_2ea_017_01:
		if (chk->state != 2)
			break;

		if (!check_value(chk, data, EMV_ID_FCI_TEMPLATE,
					  "\x6F\x21\x84\x07\xA0\x00\x00\x00\x03"
					  "\x00\x03\xA5\x16\x50\x04\x41\x50\x50"
					  "\x33\x87\x01\x01\x9F\x38\x0A\xD1\x02"
				      "\x9F\x66\x04\x9F\x2A\x08\x6F\x23", 35) ||
		    !check_value(chk, data, EMV_ID_KERNEL_IDENTIFIER, "\x21",
									   1) ||
		    !check_value(chk, data,
					 EMV_ID_TERMINAL_TRANSACTION_QUALIFIERS,
							 "\xB6\x20\x80\x00", 4))
			chk->pass_criteria_met = false;
		chk->pass_criteria_checked = true;
		chk->state = 3;
		break;

	case pc_2ea_018_00:
		if (chk->state != 2)
			break;

		if (!check_value(chk, data, EMV_ID_AMOUNT_AUTHORIZED,
					       "\x00\x00\x00\x00\x00\x02", 6) ||
		    !check_value(chk, data, EMV_ID_AMOUNT_OTHER,
					       "\x00\x00\x00\x00\x00\x00", 6) ||
		    check_value(chk, data, EMV_ID_UNPREDICTABLE_NUMBER,
						       "\x11\x22\x33\x44", 4) ||
		    !check_value(chk, data, "\xDF\x01", "\x00", 1)	      ||
		    !check_value(chk, data, "\xC1", "\x00\x00", 2)	      ||
		    !check_value(chk, data, "\x85", "\x00", 1)		      ||
		    !check_value(chk, data, "\x9F\x25", "\x00\x00", 2))
			chk->pass_criteria_met = false;
		chk->pass_criteria_checked = true;
		break;

	case pc_2ea_020_00_case01:
		if (chk->state != 2)
			break;

		if (!check_value(chk, data, EMV_ID_AMOUNT_AUTHORIZED,
					   "\x00\x00\x00\x00\x00\x00\x02", 7) ||
		    !check_value_under_mask(chk, data,
			    EMV_ID_UNPREDICTABLE_NUMBER, "\x00\x00\x00\x00\xFF",
						   "\x00\x00\x00\x00\x00", 5) ||
		    !check_value_under_mask(chk, data,
						 EMV_ID_TERMINAL_IDENTIFICATION,
					 "\x00\x00\x00\x00\x00\x00\x00\x00\xFF",
				   "\x00\x00\x00\x00\x00\x00\x00\x00\x00", 9) ||
		    !check_value_under_mask(chk, data,
						     EMV_ID_MERCHANT_IDENTIFIER,
			  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
								 "\x00\x00\xFF",
			  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
							    "\x00\x00\x00", 16))
			chk->pass_criteria_met = false;
		chk->pass_criteria_checked = true;
		break;

	case pc_2ea_020_00_case02:
		if (chk->state != 2)
			break;

		if (!check_value(chk, data, EMV_ID_AMOUNT_AUTHORIZED,
			  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
							  "\x00\x00\x02", 16) ||
		    !check_value_under_mask(chk, data,
			    EMV_ID_UNPREDICTABLE_NUMBER,
				  "\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
								 "\xFF\xFF\xFF",
				  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
							  "\x00\x00\x00", 14) ||
		    !check_value_under_mask(chk, data,
						 EMV_ID_TERMINAL_IDENTIFICATION,
				  "\x00\x00\x00\x00\x00\x00\x00\x00\xFF\xFF\xFF"
						 "\xFF\xFF\xFF\xFF\xFF\xFF\xFF",
				  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
					  "\x00\x00\x00\x00\x00\x00\x00", 18) ||
		    !check_value_under_mask(chk, data,
						     EMV_ID_MERCHANT_IDENTIFIER,
			  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
			     "\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF",
			  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
			     "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
									    25))
			chk->pass_criteria_met = false;
		chk->pass_criteria_checked = true;
		break;

	case pc_2ea_021_00:
		if (chk->state != 2)
			break;

		if (!check_value(chk, data, EMV_ID_AMOUNT_AUTHORIZED,
						 "\x00\x00\x00\x00\x00\x02", 6))
			chk->pass_criteria_met = false;
		chk->pass_criteria_checked = true;
		break;

	case pc_2eb_001_00_case01:
		if (chk->state == 0) {
			if (!check_value_under_mask(chk, data,
				  EMV_ID_TEST_FLAGS, "\x10\x00", "\x10\x00", 2))
				chk->pass_criteria_met = false;
			chk->state = 1;
			break;
		}

		if (chk->state == 1) {
			if (!check_value(chk, data, EMV_ID_TEST_FLAGS,
								 "\x00\x00", 2))
				chk->pass_criteria_met = false;
			chk->pass_criteria_checked = true;
			break;
		}

		break;

	case pc_2eb_001_00_case02:
		if (chk->state == 0) {
			if (!check_value(chk, data, EMV_ID_TEST_FLAGS,
								 "\x00\x00", 2))
				chk->pass_criteria_met = false;
			chk->pass_criteria_checked = true;
			chk->state = 1;
			break;
		}

		if (chk->state == 1)
			chk->pass_criteria_met = false;

		break;

	case pc_2eb_001_00_case03:
		if (chk->state == 0) {
			if (!check_value(chk, data, EMV_ID_TEST_FLAGS,
								 "\x08\x00", 2))
				chk->pass_criteria_met = false;
			chk->state = 1;
			break;
		}

		if (chk->state == 1) {
			if (!check_value(chk, data, EMV_ID_TEST_FLAGS,
								 "\x00\x00", 2))
				chk->pass_criteria_met = false;
			chk->pass_criteria_checked = true;
			break;
		}

		break;

	case pc_2eb_001_00_case04:
		if (chk->state == 0) {
			if (!check_value(chk, data, EMV_ID_TEST_FLAGS,
								 "\xA0\x00", 2))
				chk->pass_criteria_met = false;
			chk->state = 1;
			break;
		}

		if (chk->state == 1) {
			if (!check_value(chk, data, EMV_ID_TEST_FLAGS,
								 "\x00\x00", 2))
				chk->pass_criteria_met = false;
			chk->pass_criteria_checked = true;
			break;
		}

		break;

	case pc_2eb_001_00_case05:
		if (chk->state == 0) {
			if (!check_value(chk, data, EMV_ID_TEST_FLAGS,
								 "\x80\x00", 2))
				chk->pass_criteria_met = false;
			chk->state = 1;
			break;
		}

		if (chk->state == 1) {
			if (!check_value(chk, data, EMV_ID_TEST_FLAGS,
								 "\x00\x00", 2))
				chk->pass_criteria_met = false;
			chk->pass_criteria_checked = true;
			break;
		}

		break;

	case pc_2eb_002_00_case01:
		if (!check_value(chk, data,
					 EMV_ID_TERMINAL_TRANSACTION_QUALIFIERS,
							 "\xB6\x20\x80\x00", 4))
			chk->pass_criteria_met = false;
		chk->pass_criteria_checked = true;
		break;

	case pc_2eb_002_00_case02:
		if (!check_value(chk, data,
					 EMV_ID_TERMINAL_TRANSACTION_QUALIFIERS,
							 "\x28\x00\x00\x00", 4))
			chk->pass_criteria_met = false;
		chk->pass_criteria_checked = true;
		break;

	case pc_2eb_002_00_case03:
		if (!check_value(chk, data,
					 EMV_ID_TERMINAL_TRANSACTION_QUALIFIERS,
							 "\x84\x00\x80\x00", 4))
			chk->pass_criteria_met = false;
		chk->pass_criteria_checked = true;
		break;

	case pc_2eb_003_00_case01:
	case pc_2eb_003_00_case02:
	case pc_2eb_003_00_case03:
	case pc_2eb_003_00_case05:
	case pc_2eb_003_01:
		if (!check_value_under_mask(chk, data, EMV_ID_TEST_FLAGS,
						   "\x10\x00", "\x10\x00", 2) ||
		    !check_value_under_mask(chk, data,
					 EMV_ID_TERMINAL_TRANSACTION_QUALIFIERS,
				     "\x00\x80\x00\x00", "\x00\x80\x00\x00", 4))
			chk->pass_criteria_met = false;
		chk->pass_criteria_checked = true;
		break;

	case pc_2eb_003_00_case04:
	case pc_2eb_003_02:
		if (!check_value_under_mask(chk, data, EMV_ID_TEST_FLAGS,
						     "\x10\x00", "\x10\x00", 2))
			chk->pass_criteria_met = false;
		chk->pass_criteria_checked = true;
		break;

	case pc_2eb_004_00_case01:
	case pc_2eb_004_00_case02:
	case pc_2eb_004_00_case03:
	case pc_2eb_004_01_case01:
	case pc_2eb_004_01_case02:
	case pc_2eb_004_02:
	case pc_2eb_005_00_case01:
	case pc_2eb_005_00_case02:
	case pc_2eb_005_00_case03:
	case pc_2eb_005_01:
	case pc_2eb_006_00_case04:
		if (!check_value_under_mask(chk, data, EMV_ID_TEST_FLAGS,
						     "\x00\x00", "\x10\x00", 2))
			chk->pass_criteria_met = false;
		chk->pass_criteria_checked = true;
		break;

	case pc_2eb_006_00_case01:
	case pc_2eb_006_00_case02:
	case pc_2eb_006_00_case03:
	case pc_2eb_006_00_case05:
	case pc_2eb_006_01:
		if (!check_value_under_mask(chk, data, EMV_ID_TEST_FLAGS,
						   "\x00\x00", "\x10\x00", 2) ||
		    !check_value_under_mask(chk, data,
					 EMV_ID_TERMINAL_TRANSACTION_QUALIFIERS,
				     "\x00\x00\x00\x00", "\x00\x80\x00\x00", 4))
			chk->pass_criteria_met = false;
		chk->pass_criteria_checked = true;
		break;

	case pc_2eb_008_00_case01:
	case pc_2eb_008_00_case03:
	case pc_2eb_008_01_case01:
	case pc_2eb_008_02_case01:
	case pc_2eb_009_00_case01:
	case pc_2eb_009_01_case01:
	case pc_2eb_009_01_case03:
	case pc_2eb_009_02_case01:
		if (!check_value_under_mask(chk, data, EMV_ID_TEST_FLAGS,
						     "\x08\x00", "\x08\x00", 2))
			chk->pass_criteria_met = false;
		chk->pass_criteria_checked = true;
		break;

	case pc_2eb_008_00_case02:
	case pc_2eb_008_00_case04:
	case pc_2eb_008_01_case02:
	case pc_2eb_008_02_case02:
	case pc_2eb_009_00_case02:
	case pc_2eb_009_01_case02:
	case pc_2eb_009_01_case04:
	case pc_2eb_009_02_case02:
	case pc_2eb_014_00_case02:
	case pc_2eb_014_00_case04:
	case pc_2eb_015_00_case03:
	case pc_2eb_015_00_case04:
	case pc_2eb_015_00_case06:
	case pc_2eb_015_01_case03:
	case pc_2eb_015_01_case04:
	case pc_2eb_015_02_case03:
	case pc_2eb_015_02_case04:
	case pc_2eb_016_00:
	case pc_2eb_016_01:
	case pc_2eb_016_02_case01:
	case pc_2eb_016_02_case02:
		if (!check_value_under_mask(chk, data, EMV_ID_TEST_FLAGS,
						     "\x00\x00", "\x08\x00", 2))
			chk->pass_criteria_met = false;
		chk->pass_criteria_checked = true;
		break;

	case pc_2eb_013_00_case01:
	case pc_2eb_013_00_case02:
	case pc_2eb_013_00_case03:
	case pc_2eb_013_00_case04:
	case pc_2eb_013_00_case05:
	case pc_2eb_013_00_case06:
		if (!check_value_under_mask(chk, data,
					 EMV_ID_TERMINAL_TRANSACTION_QUALIFIERS,
				     "\x00\x80\x00\x00", "\x00\x80\x00\x00", 4))
			chk->pass_criteria_met = false;
		chk->pass_criteria_checked = true;
		break;

	case pc_2eb_013_01:
		if (!check_value_under_mask(chk, data,
					 EMV_ID_TERMINAL_TRANSACTION_QUALIFIERS,
				   "\x00\x80\x00\x00", "\x00\x80\x00\x00", 4) ||
		    !check_value_under_mask(chk, data, EMV_ID_TEST_FLAGS,
						     "\x80\x00", "\x80\x00", 2))
			chk->pass_criteria_met = false;
		chk->pass_criteria_checked = true;
		break;

	case pc_2eb_013_02:
	case pc_2eb_015_00_case01:
	case pc_2eb_015_00_case02:
	case pc_2eb_015_00_case05:
	case pc_2eb_015_01_case01:
	case pc_2eb_015_01_case02:
	case pc_2eb_015_02_case01:
	case pc_2eb_015_02_case02:
		if (!check_value_under_mask(chk, data, EMV_ID_TEST_FLAGS,
						     "\x80\x00", "\x80\x00", 2))
			chk->pass_criteria_met = false;
		chk->pass_criteria_checked = true;
		break;

	case pc_2eb_014_00_case01:
	case pc_2eb_014_00_case03:
	case pc_2eb_014_00_case05:
	case pc_2eb_014_01:
	case pc_2eb_014_02_case01:
	case pc_2eb_014_02_case02:
		if (!check_value_under_mask(chk, data,
					 EMV_ID_TERMINAL_TRANSACTION_QUALIFIERS,
				   "\x00\x00\x00\x00", "\x00\x80\x00\x00", 4) ||
		    !check_value_under_mask(chk, data, EMV_ID_TEST_FLAGS,
						     "\x00\x00", "\x80\x00", 2))
			chk->pass_criteria_met = false;
		chk->pass_criteria_checked = true;
		break;

	case pc_2eb_018_00_case01:
	case pc_2eb_018_00_case02:
	case pc_2eb_018_00_case03:
	case pc_2eb_018_00_case04:
	case pc_2eb_018_01:
	case pc_2eb_018_02:
	case pc_2eb_019_00:
	case pc_2eb_019_01:
	case pc_2eb_019_02:
		if (!check_value_under_mask(chk, data, EMV_ID_TEST_FLAGS,
						     "\x00\x00", "\x20\x00", 2))
			chk->pass_criteria_met = false;
		chk->pass_criteria_checked = true;
		break;

	case pc_2eb_017_00_case02:
	case pc_2eb_017_02:
		if (!check_value_under_mask(chk, data, EMV_ID_TEST_FLAGS,
						     "\x20\x00", "\x20\x00", 2))
			chk->pass_criteria_met = false;
		chk->pass_criteria_checked = true;
		break;

	case pc_2eb_017_00_case01:
	case pc_2eb_017_00_case03:
	case pc_2eb_017_00_case04:
	case pc_2eb_017_01:
		if (!check_value_under_mask(chk, data, EMV_ID_TEST_FLAGS,
						   "\x20\x00", "\x20\x00", 2) ||
		    !check_value_under_mask(chk, data,
					 EMV_ID_TERMINAL_TRANSACTION_QUALIFIERS,
				     "\x00\x40\x00\x00", "\x00\x40\x00\x00", 4))
			chk->pass_criteria_met = false;
		chk->pass_criteria_checked = true;
		break;

	case pc_2eb_020_00_case01:
	case pc_2eb_020_01_case01:
		if (!check_value_under_mask(chk, data, EMV_ID_TEST_FLAGS,
						   "\x08\x00", "\x08\x00", 2) ||
		    !check_value_under_mask(chk, data,
					 EMV_ID_TERMINAL_TRANSACTION_QUALIFIERS,
				     "\x00\x80\x00\x00", "\x00\x80\x00\x00", 4))
			chk->pass_criteria_met = false;
		chk->pass_criteria_checked = true;
		break;

	case pc_2eb_020_00_case02:
	case pc_2eb_020_01_case02:
		if (!check_value_under_mask(chk, data, EMV_ID_TEST_FLAGS,
						   "\x00\x00", "\x08\x00", 2) ||
		    !check_value_under_mask(chk, data,
					 EMV_ID_TERMINAL_TRANSACTION_QUALIFIERS,
				     "\x00\x00\x00\x00", "\x00\x80\x00\x00", 4))
			chk->pass_criteria_met = false;
		chk->pass_criteria_checked = true;
		break;

	case pc_2ec_001_00_case01:
	case pc_2ec_001_01:
	case pc_2ec_001_02:
		if (!check_value(chk, data, EMV_ID_TEST_FLAGS, "\x00\x00", 2) ||
		    !check_value(chk, data,
					 EMV_ID_TERMINAL_TRANSACTION_QUALIFIERS,
							 "\x28\x00\x00\x00", 4))
			chk->pass_criteria_met = false;
		chk->pass_criteria_checked = true;
		break;

	case pc_2ec_001_00_case02:
	case pc_2ec_001_03:
	case pc_2ec_001_04:
		if (!check_value(chk, data, EMV_ID_TEST_FLAGS, "\x00\x00", 2))
			chk->pass_criteria_met = false;
		chk->pass_criteria_checked = true;
		break;

	case pc_2ec_001_00_case03:
		if (!check_value(chk, data, EMV_ID_TEST_FLAGS, "\x00\x00", 2) ||
		    !check_value(chk, data,
					 EMV_ID_TERMINAL_TRANSACTION_QUALIFIERS,
							 "\x84\x00\x80\x00", 4))
			chk->pass_criteria_met = false;
		chk->pass_criteria_checked = true;
		break;

	case pc_2ec_001_00_case04:
	case pc_2ec_001_00_case05:
		if (!check_value(chk, data, EMV_ID_TEST_FLAGS, "\x00\x00", 2) ||
		    !check_value(chk, data,
					 EMV_ID_TERMINAL_TRANSACTION_QUALIFIERS,
							 "\x84\xC0\x80\x00", 4))
			chk->pass_criteria_met = false;
		chk->pass_criteria_checked = true;
		break;

	case pc_2ec_002_00_case01:
	case pc_2ec_002_00_case02:
	case pc_2ec_002_00_case03:
	case pc_2ec_002_00_case04:
	case pc_2ec_003_00_case01:
	case pc_2ec_003_00_case02:
	case pc_2ec_003_00_case03:
	case pc_2ec_003_00_case04:
		if (chk->state == 2) {
			if (!check_value_under_mask(chk, data,
				 EMV_ID_TEST_FLAGS, "\x00\x80", "\x00\x80", 2))
				chk->pass_criteria_met = false;
			chk->pass_criteria_checked = true;
		}
		break;

	case pc_2ec_004_00_case01:
	case pc_2ec_004_00_case02:
		if (chk->state == 1) {
			if (!check_value_under_mask(chk, data,
				 EMV_ID_TEST_FLAGS, "\x00\x00", "\x00\x80", 2))
				chk->pass_criteria_met = false;
			chk->pass_criteria_checked = true;
		}
		break;

	case pc_2ed_001_00_case01:
	case pc_2ed_001_00_case02:
	case pc_2ed_001_00_case04:
	case pc_2ed_001_00_case05:
		if (chk->state == 2) {
			if (!check_value(chk, data, EMV_ID_KERNEL_IDENTIFIER,
								   "\x21", 1) ||
			    !check_value(chk, data, EMV_ID_FCI_TEMPLATE,
					  "\x6F\x21\x84\x07\xA0\x00\x00\x00\x03"
					  "\x00\x03\xA5\x16\x50\x04\x41\x50\x50"
					  "\x33\x87\x01\x01\x9F\x38\x0A\xD1\x02"
					"\x9F\x66\x04\x9F\x2A\x08\x6F\x23", 35))
				chk->pass_criteria_met = false;
			chk->pass_criteria_checked = true;
		}
		break;

	case pc_2ed_001_00_case03:
	case pc_2ed_001_00_case06:
		if (chk->state == 2) {
			if (!check_value(chk, data, EMV_ID_KERNEL_IDENTIFIER,
								   "\x23", 1) ||
			    !check_value(chk, data, EMV_ID_FCI_TEMPLATE,
					  "\x6F\x21\x84\x07\xA0\x00\x00\x00\x01"
					  "\x00\x01\xA5\x16\x50\x04\x41\x50\x50"
					  "\x31\x87\x01\x01\x9F\x38\x0A\xD1\x02"
					"\x9F\x66\x04\x9F\x2A\x08\x6F\x23", 35))
				chk->pass_criteria_met = false;
			chk->pass_criteria_checked = true;
		}
		break;

	case pc_2ed_001_01_case01:
	case pc_2ed_001_01_case02:
	case pc_2ed_001_01_case03:
	case pc_2ed_001_01_case04:
	case pc_2ed_001_01_case05:
		if (chk->state == 2) {
			if (!check_value(chk, data, EMV_ID_KERNEL_IDENTIFIER,
								   "\x24", 1) ||
			    !check_value(chk, data, EMV_ID_FCI_TEMPLATE,
					  "\x6F\x22\x84\x08\xA0\x00\x00\x00\x04"
					  "\x00\x04\x04\xA5\x16\x50\x04\x41\x50"
					  "\x50\x34\x87\x01\x01\x9F\x38\x0A\xD1"
				    "\x02\x9F\x66\x04\x9F\x2A\x08\x6F\x24", 36))
				chk->pass_criteria_met = false;
			chk->pass_criteria_checked = true;
		}
		break;

	case pc_2ed_001_01_case06:
		if (chk->state == 2) {
			if (!check_value(chk, data, EMV_ID_KERNEL_IDENTIFIER,
								   "\x25", 1) ||
			    !check_value(chk, data, EMV_ID_FCI_TEMPLATE,
					  "\x6F\x22\x84\x08\xA0\x00\x00\x00\x04"
					  "\x00\x04\x04\xA5\x16\x50\x04\x41\x50"
					  "\x50\x34\x87\x01\x01\x9F\x38\x0A\xD1"
				    "\x02\x9F\x66\x04\x9F\x2A\x08\x6F\x24", 36))
				chk->pass_criteria_met = false;
			chk->pass_criteria_checked = true;
		}
		break;

	case pc_2ed_002_00:
		if (chk->state == 2) {
			if (!check_value(chk, data, EMV_ID_KERNEL_IDENTIFIER,
								   "\x24", 1) ||
			    !check_value(chk, data, EMV_ID_FCI_TEMPLATE,
					  "\x6F\x21\x84\x07\xA0\x00\x00\x00\x02"
					  "\x00\x02\xA5\x16\x50\x04\x41\x50\x50"
					  "\x32\x87\x01\x01\x9F\x38\x0A\xD1\x02"
					"\x9F\x66\x04\x9F\x2A\x08\x6F\x23", 35))
				chk->pass_criteria_met = false;
			chk->pass_criteria_checked = true;
		}
		break;

	case pc_2ed_002_01:
		if (chk->state == 2) {
			if (!check_value(chk, data, EMV_ID_KERNEL_IDENTIFIER,
								   "\x24", 1) ||
			    !check_value(chk, data, EMV_ID_FCI_TEMPLATE,
					  "\x6F\x22\x84\x08\xA0\x00\x00\x00\x04"
					  "\x00\x04\x04\xA5\x16\x50\x04\x41\x50"
					  "\x50\x34\x87\x01\x01\x9F\x38\x0A\xD1"
				    "\x02\x9F\x66\x04\x9F\x2A\x08\x6F\x24", 36))
				chk->pass_criteria_met = false;
			chk->pass_criteria_checked = true;
		}
		break;

	case pc_2ed_003_01_case01:
		if (chk->state == 2) {
			chk->state = 3;
			if (!check_value(chk, data, EMV_ID_KERNEL_IDENTIFIER,
								     "\x23", 1))
				chk->pass_criteria_met = false;
			chk->pass_criteria_checked = true;
		}
		break;

	case pc_2ed_003_01_case02:
		if (chk->state == 2) {
			chk->state = 3;
			if (!check_value(chk, data, EMV_ID_KERNEL_IDENTIFIER,
								     "\x22", 1))
				chk->pass_criteria_met = false;
			chk->pass_criteria_checked = true;
		}
		break;

	case pc_2ed_003_01_case03:
		if (chk->state == 2) {
			chk->state = 3;
			if (!check_value(chk, data, EMV_ID_KERNEL_IDENTIFIER,
								     "\x21", 1))
				chk->pass_criteria_met = false;
			chk->pass_criteria_checked = true;
		}
		break;

	case pc_2ed_009_00:
		if (chk->state == 1) {
			chk->state = 2;
			if (!check_value(chk, data, EMV_ID_KERNEL_IDENTIFIER,
								     "\x03", 1))
				chk->pass_criteria_met = false;
			chk->pass_criteria_checked = true;
		}
		break;

	case pc_2ed_009_02:
	case pc_2ed_009_12:
		if (chk->state == 1) {
			chk->state = 2;
			if (!check_value(chk, data, EMV_ID_KERNEL_IDENTIFIER,
								     "\x02", 1))
				chk->pass_criteria_met = false;
			chk->pass_criteria_checked = true;
		}
		break;

	case pc_2ed_009_04:
	case pc_2ed_009_14:
		if (chk->state == 1) {
			chk->state = 2;
			if (!check_value(chk, data, EMV_ID_KERNEL_IDENTIFIER,
								     "\x05", 1))
				chk->pass_criteria_met = false;
			chk->pass_criteria_checked = true;
		}
		break;

	case pc_2ed_009_06_case01:
	case pc_2ed_009_06_case02:
	case pc_2ed_009_16:
		if (chk->state == 1) {
			chk->state = 2;
			if (!check_value(chk, data, EMV_ID_KERNEL_IDENTIFIER,
								     "\x04", 1))
				chk->pass_criteria_met = false;
			chk->pass_criteria_checked = true;
		}
		break;

	case pc_2ed_009_10:
		if (chk->state == 1) {
			chk->state = 2;
			if (!check_value(chk, data, EMV_ID_KERNEL_IDENTIFIER,
								     "\x03", 1))
				chk->pass_criteria_met = false;
			chk->pass_criteria_checked = true;
		}
		break;

	case pc_2ed_009_18:
	case pc_2ed_009_20:
	case pc_2ed_009_26:
	case pc_2ed_009_28:
		if (chk->state == 1) {
			chk->state = 2;
			if (!check_value(chk, data, EMV_ID_KERNEL_IDENTIFIER,
								     "\x06", 1))
				chk->pass_criteria_met = false;
			chk->pass_criteria_checked = true;
		}
		break;

	case pc_2ed_009_22:
	case pc_2ed_009_24:
		if (chk->state == 1) {
			chk->state = 2;
			if (!check_value(chk, data, EMV_ID_KERNEL_IDENTIFIER,
								     "\x07", 1))
				chk->pass_criteria_met = false;
			chk->pass_criteria_checked = true;
		}
		break;

	case pc_2ed_010_00:
	case pc_2ed_011_00:
		if (chk->state == 1) {
			chk->state = 2;
			if (!check_value(chk, data, EMV_ID_KERNEL_IDENTIFIER,
								     "\x24", 1))
				chk->pass_criteria_met = false;
			chk->pass_criteria_checked = true;
		}
		break;

	default:
		break;
	}

	if (!chk->pass_criteria_met) {
		uint8_t bin[256];
		size_t len = sizeof(bin);
		int rc;

		rc = tlv_encode(data, bin, &len);
		if (rc == TLV_RC_OK) {
			char hex[2 * len + 1];

			log4c_category_log(chk->log_cat, LOG4C_PRIORITY_NOTICE,
					"%s('%s): pass criteria check failed!",
				   __func__, libtlv_bin_to_hex(bin, len, hex));

		}
	}
}

static void checker_ep_start(struct chk *chk)
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

static void checker_ep_restart(struct chk *chk)
{
	struct checker *checker = (struct checker *)chk;

	switch (checker->pass_criteria) {

	case pc_2ea_014_00_case03:
	case pc_2ea_016_00_case04:
	case pc_2ea_016_00_case05:
	case pc_2ea_016_00_case06:
	case pc_2ea_016_00_case07:
	case pc_2ea_016_00_case08:
		if (checker->state == 2)
			checker->state = 3;
		break;

	case pc_2ec_002_00_case01:
	case pc_2ec_002_00_case02:
	case pc_2ec_002_00_case03:
	case pc_2ec_002_00_case04:
	case pc_2ec_003_00_case01:
	case pc_2ec_003_00_case02:
	case pc_2ec_003_00_case03:
	case pc_2ec_003_00_case04:
	case pc_2ed_001_00_case01:
	case pc_2ed_001_00_case02:
	case pc_2ed_001_00_case03:
	case pc_2ed_001_00_case04:
	case pc_2ed_001_00_case05:
	case pc_2ed_001_00_case06:
	case pc_2ed_001_01_case01:
	case pc_2ed_001_01_case02:
	case pc_2ed_001_01_case03:
	case pc_2ed_001_01_case04:
	case pc_2ed_001_01_case05:
	case pc_2ed_001_01_case06:
	case pc_2ed_002_00:
	case pc_2ed_002_01:
		if (checker->state == 0)
			checker->state = 1;
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

static void checker_ui_request(struct chk *checker,
					const struct emv_ui_request *ui_request)
{
	struct checker *chk = (struct checker *)checker;

	switch (chk->pass_criteria) {

	case pc_2ea_011_00_case01:
		if (ui_request->msg_id == msg_approved) {
			chk->pass_criteria_checked = true;
			if (ui_request->hold_time != 100)
				chk->pass_criteria_met = false;
		}
		break;

	case pc_2ea_011_00_case02:
		if (ui_request->msg_id == msg_not_authorized) {
			chk->pass_criteria_checked = true;
			if (ui_request->hold_time != 200)
				chk->pass_criteria_met = false;
		}
		break;

	case pc_2ea_011_00_case03:
		switch (chk->state) {
		case 0:
			if (ui_request->msg_id == msg_present_card_again &&
			    ui_request->status == sts_ready_to_read)
				chk->state = 1;
			break;
		case 1:
			if (ui_request->msg_id == msg_approved &&
			    ui_request->status == sts_card_read_successfully)
				chk->pass_criteria_checked = true;
			break;
		default:
			chk->pass_criteria_met = false;
			chk->pass_criteria_checked = true;
		}
		break;

	case pc_2ea_012_00_case01:
		if (ui_request->msg_id == msg_approved) {
			chk->pass_criteria_checked = true;
			if (memcmp(ui_request->lang_pref, "en", 2))
				chk->pass_criteria_met = false;
		}
		break;

	case pc_2ea_012_00_case02:
		switch (chk->state) {
		case 0:
			if (ui_request->msg_id == msg_see_phone &&
			    !memcmp(ui_request->lang_pref, "en", 2))
				chk->state = 1;
			break;
		case 1:
			if (ui_request->msg_id == msg_not_authorized &&
			    !memcmp(ui_request->lang_pref, "en", 2))
				chk->pass_criteria_checked = true;
			break;
		default:
			chk->pass_criteria_met = false;
			chk->pass_criteria_checked = true;
		}
		break;

	case pc_2ea_012_00_case03:
		if (ui_request->msg_id == msg_approved) {
			chk->pass_criteria_checked = true;
			if (memcmp(ui_request->lang_pref, "de", 2))
				chk->pass_criteria_met = false;
		}
		break;

	case pc_2ea_012_00_case04:
		switch (chk->state) {
		case 0:
			if (ui_request->msg_id == msg_see_phone &&
			    !memcmp(ui_request->lang_pref, "de", 2))
				chk->state = 1;
			break;
		case 1:
			if (ui_request->msg_id == msg_not_authorized &&
			    !memcmp(ui_request->lang_pref, "de", 2))
				chk->pass_criteria_checked = true;
			break;
		default:
			chk->pass_criteria_met = false;
			chk->pass_criteria_checked = true;
		}
		break;

	case pc_2ea_013_00_case01:
		if (ui_request->msg_id == msg_approved) {
			chk->pass_criteria_checked = true;
			if (memcmp(ui_request->value,
					       "\x00\x00\x00\x00\x01\x23", 6) ||
			    (ui_request->value_qualifier != val_amount)	      ||
			    memcmp(ui_request->currency_code, ISO4217_USD, 2))
				chk->pass_criteria_met = false;
		}
		break;

	case pc_2ea_013_00_case02:
		if (ui_request->msg_id == msg_approved) {
			chk->pass_criteria_checked = true;
			if (memcmp(ui_request->value,
					       "\x00\x00\x00\x00\x00\x12", 6) ||
			    (ui_request->value_qualifier != val_balance)      ||
			    memcmp(ui_request->currency_code, ISO4217_EUR, 2))
				chk->pass_criteria_met = false;
		}
		break;

	case pc_2ea_013_00_case03:
		if (ui_request->msg_id == msg_approved) {
			chk->pass_criteria_checked = true;
			if (memcmp(ui_request->value,
					       "\x00\x00\x12\x34\x56\x00", 6) ||
			    (ui_request->value_qualifier != val_amount)	      ||
			    memcmp(ui_request->currency_code, ISO4217_USD, 2))
				chk->pass_criteria_met = false;
		}
		break;

	case pc_2ea_013_01_case01:
		if (ui_request->msg_id == msg_approved) {
			chk->pass_criteria_checked = true;
			if (memcmp(ui_request->value,
					       "\x00\x00\x00\x12\x34\x00", 6) ||
			    (ui_request->value_qualifier != val_balance)      ||
			    memcmp(ui_request->currency_code, ISO4217_USD, 2))
				chk->pass_criteria_met = false;
		}
		break;

	case pc_2ea_013_01_case02:
		if (ui_request->msg_id == msg_approved) {
			chk->pass_criteria_checked = true;
			if (memcmp(ui_request->value,
					       "\x00\x00\x00\x12\x34\x00", 6) ||
			    (ui_request->value_qualifier != val_balance)      ||
			    memcmp(ui_request->currency_code, ISO4217_EUR, 2))
				chk->pass_criteria_met = false;
		}
		break;

	case pc_2ea_014_00_case02:
		if ((chk->state == 2) &&
		    (ui_request->msg_id == msg_present_card_again))
			chk->state = 3;
		break;

	case pc_2ea_014_00_case06:
		if ((chk->state == 2) &&
		    (ui_request->msg_id == msg_processing))
			chk->state = 3;
		break;

	case pc_2ea_014_01_case01:
		if ((chk->state == 0) &&
		    (ui_request->msg_id == msg_remove_card))
			chk->state = 1;

		if ((chk->state == 1) &&
		    (ui_request->msg_id == msg_present_card_again))
			chk->state = 2;
		break;

	case pc_2ea_014_01_case02:
		if ((chk->state == 0) &&
		    (ui_request->msg_id == msg_present_card_again))
			chk->state = 1;
		break;

	case pc_2ea_015_00_case01:
	case pc_2ea_015_00_case02:
	case pc_2ea_015_00_case03:
	case pc_2ea_015_00_case04:
		if (ui_request->msg_id == msg_card_read_ok)
			chk->pass_criteria_checked = true;
		break;

	case pc_2eb_022_00:
		if (ui_request->msg_id == msg_insert_or_swipe_card)
			chk->pass_criteria_checked = true;
		break;

	case pc_2ec_002_00_case01:
	case pc_2ec_002_00_case02:
	case pc_2ec_002_00_case03:
		if ((chk->state == 1) &&
		    (ui_request->msg_id == msg_present_card_again))
			chk->state = 2;
		break;

	case pc_2ec_002_00_case04:
		if ((chk->state == 1) &&
		    (ui_request->msg_id == msg_processing))
			chk->state = 2;
		break;

	case pc_2ec_003_00_case01:
	case pc_2ec_003_00_case02:
	case pc_2ec_003_00_case03:
	case pc_2ec_003_00_case04:
		if ((chk->state == 1) &&
		    (ui_request->msg_id == msg_present_card))
			chk->state = 2;
		break;

	case pc_2ec_004_00_case01:
	case pc_2ec_004_00_case02:
		if ((chk->state == 0) &&
		    (ui_request->msg_id == msg_present_card))
			chk->state = 1;
		break;

	case pc_2ec_006_00_case01:
	case pc_2ec_006_00_case02:
	case pc_2ec_006_00_case03:
	case pc_2ec_006_00_case04:
	case pc_2ec_007_00_case01:
	case pc_2ec_007_00_case02:
	case pc_2ec_007_00_case03:
	case pc_2ec_007_00_case04:
		if ((chk->state == 0) &&
		    (ui_request->msg_id == msg_present_one_card_only))
			chk->state = 1;
		break;

	case pc_2ed_004_00:
	case pc_2ed_005_00:
		if (ui_request->msg_id == msg_try_another_card)
			chk->pass_criteria_checked = true;
		break;

	default:
		memcpy(&chk->ui_request, ui_request, sizeof(*ui_request));
	}
}

static void checker_outcome(struct chk *checker,
					const struct emv_outcome_parms *outcome)
{
	struct checker *chk = (struct checker *)checker;

	switch (chk->pass_criteria) {
	case pc_2ea_019_00_case01:
	case pc_2ea_019_00_case03:
		if (outcome->receipt) {
			if (outcome->cvm != cvm_no_cvm)
				chk->pass_criteria_met = false;
			chk->pass_criteria_checked = true;
		}
		break;

	case pc_2ea_019_00_case02:
	case pc_2ea_019_00_case04:
		if (outcome->receipt) {
			if (outcome->cvm != cvm_obtain_signature)
				chk->pass_criteria_met = false;
			chk->pass_criteria_checked = true;
		}
		break;

	default:
		break;
	}
}

static bool checker_pass_criteria_met(struct chk *chk)
{
	struct checker *checker = (struct checker *)chk;
	bool ok = false;

	switch (checker->pass_criteria) {
	case pc_2ea_007_00:
		ok = check_unpredictable_numbers(checker);
		break;
	default:
		ok = checker->pass_criteria_checked &&
						     checker->pass_criteria_met;
	}

	if (!ok)
		log4c_category_log(checker->log_cat, LOG4C_PRIORITY_NOTICE,
			 "Pass criteria %d not met! checked %d met %d state %d",
						    (int)checker->pass_criteria,
					    (int)checker->pass_criteria_checked,
			       (int)checker->pass_criteria_met, checker->state);
	return ok;
}

void checker_free(struct chk *chk)
{
	free(chk);
}

static const struct chk_ops checker_ops = {
	.ep_start	   = checker_ep_start,
	.ep_restart	   = checker_ep_restart,
	.field_on	   = checker_field_on,
	.field_off	   = checker_field_off,
	.select		   = checker_select,
	.gpo_data	   = checker_gpo_data,
	.ui_request	   = checker_ui_request,
	.outcome	   = checker_outcome,
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
