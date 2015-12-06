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
#include <log4c.h>

#include <tlv.h>
#include <emv.h>

#include "emvco_ep_ta.h"

struct checker {
	const struct chk_ops *ops;
	enum pass_criteria    pass_criteria;
	bool		      pass_criteria_checked;
	bool		      pass_criteria_met;
};

static bool get_value(struct tlv *tlv, const char *tag, void *value,
								   size_t *size)
{
	struct tlv *mytlv = tlv_find(tlv, tag);
	int rc = EMV_RC_OK;

	if (!mytlv)
		return false;

	rc = tlv_encode_value(mytlv, value, size);
	if (rc != EMV_RC_OK)
		return false;

	return true;
}

static bool check_value(struct tlv *tlv, const char *tag, const void *value,
								    size_t size)
{
	uint8_t myval[256];
	size_t myval_sz = sizeof(myval);

	if (!get_value(tlv, tag, myval, &myval_sz))
		return false;

	if (size != myval_sz)
		return false;

	if (memcmp(value, myval, size))
		return false;

	return true;
}

static void checker_check_gpo_data(struct chk *chk, struct tlv *gpo_data)
{
	struct checker *checker = (struct checker *)chk;
	uint8_t val[256];
	size_t val_sz = sizeof(val);

	switch (checker->pass_criteria) {

	case pc_2ea_001_00_case01:
		if (!check_value(gpo_data, EMV_ID_TRANSACTION_TYPE, "\x00", 1)
									      ||
		    !check_value(gpo_data, EMV_ID_AMOUNT_AUTHORIZED,
					       "\x00\x00\x00\x00\x00\x10", 6) ||
		    !check_value(gpo_data, EMV_ID_AMOUNT_OTHER,
						 "\x00\x00\x00\x00\x00\x00", 6))
			checker->pass_criteria_met = false;
		checker->pass_criteria_checked = true;
		break;

	case pc_2ea_001_00_case02:
		if (!check_value(gpo_data, EMV_ID_TRANSACTION_TYPE, "\x00", 1)
									      ||
		    !check_value(gpo_data, EMV_ID_AMOUNT_AUTHORIZED,
					       "\x00\x00\x00\x00\x00\x75", 6) ||
		    !check_value(gpo_data, EMV_ID_AMOUNT_OTHER,
						 "\x00\x00\x00\x00\x00\x00", 6))
			checker->pass_criteria_met = false;
		checker->pass_criteria_checked = true;
		break;

	case pc_2ea_001_00_case03:
		if (!check_value(gpo_data, EMV_ID_TRANSACTION_TYPE, "\x00", 1)
									      ||
		    !check_value(gpo_data, EMV_ID_AMOUNT_AUTHORIZED,
					       "\x00\x00\x00\x00\x00\x45", 6) ||
		    !check_value(gpo_data, EMV_ID_AMOUNT_OTHER,
						 "\x00\x00\x00\x00\x00\x00", 6))
			checker->pass_criteria_met = false;
		checker->pass_criteria_checked = true;
		break;

	case pc_2ea_002_00:
		if (!check_value(gpo_data, EMV_ID_TRANSACTION_TYPE, "\x00", 1)
									      ||
		    !check_value(gpo_data, EMV_ID_AMOUNT_AUTHORIZED,
					       "\x00\x00\x00\x00\x00\x10", 6) ||
		    !check_value(gpo_data, EMV_ID_AMOUNT_OTHER,
						 "\x00\x00\x00\x00\x00\x00", 6))
			checker->pass_criteria_met = false;
		checker->pass_criteria_checked = true;
		break;

	case pc_2ea_002_01:
		if (!check_value(gpo_data, EMV_ID_TRANSACTION_TYPE, "\x01", 1)
									      ||
		    !check_value(gpo_data, EMV_ID_AMOUNT_AUTHORIZED,
					       "\x00\x00\x00\x00\x00\x20", 6) ||
		    !check_value(gpo_data, EMV_ID_AMOUNT_OTHER,
						 "\x00\x00\x00\x00\x00\x00", 6))
			checker->pass_criteria_met = false;
		checker->pass_criteria_checked = true;
		break;

	case pc_2ea_002_02:
		if (!check_value(gpo_data, EMV_ID_TRANSACTION_TYPE, "\x20", 1)
									      ||
		    !check_value(gpo_data, EMV_ID_AMOUNT_AUTHORIZED,
					       "\x00\x00\x00\x00\x00\x30", 6) ||
		    !check_value(gpo_data, EMV_ID_AMOUNT_OTHER,
						 "\x00\x00\x00\x00\x00\x00", 6))
			checker->pass_criteria_met = false;
		checker->pass_criteria_checked = true;
		break;

	case pc_2ea_003_00_case01:
		if (!check_value(gpo_data, EMV_ID_TRANSACTION_TYPE, "\x09", 1)
									      ||
		    !check_value(gpo_data, EMV_ID_AMOUNT_AUTHORIZED,
					       "\x00\x00\x00\x00\x00\x30", 6) ||
		    !check_value(gpo_data, EMV_ID_AMOUNT_OTHER,
						 "\x00\x00\x00\x00\x00\x10", 6))
			checker->pass_criteria_met = false;
		checker->pass_criteria_checked = true;
		break;

	case pc_2ea_003_00_case02:
		if (!check_value(gpo_data, EMV_ID_TRANSACTION_TYPE, "\x09", 1)
									      ||
		    !check_value(gpo_data, EMV_ID_AMOUNT_AUTHORIZED,
					       "\x00\x00\x00\x00\x00\x70", 6) ||
		    !check_value(gpo_data, EMV_ID_AMOUNT_OTHER,
						 "\x00\x00\x00\x00\x00\x20", 6))
			checker->pass_criteria_met = false;
		checker->pass_criteria_checked = true;
		break;

	case pc_2ea_004_00_case01:
	case pc_2ea_004_00_case02:
		if (!get_value(gpo_data, EMV_ID_TEST_FLAGS, val, &val_sz) ||
		    (val_sz != 2) || (val[1] & 0x80u))
			checker->pass_criteria_met = false;
		checker->pass_criteria_checked = true;
		break;

	default:
		break;
	}
}

static bool checker_check_pass_criteria_met(struct chk *chk)
{
	struct checker *checker = (struct checker *)chk;

	return checker->pass_criteria_checked && checker->pass_criteria_met;
}

void checker_free(struct chk *chk)
{
	free(chk);
}

static const struct chk_ops checker_ops = {
	.check_gpo_data	   = checker_check_gpo_data,
	.pass_criteria_met = checker_check_pass_criteria_met,
	.free		   = checker_free
};

struct chk *chk_pass_criteria_new(enum pass_criteria pass_criteria)
{
	struct checker *checker = NULL;

	checker = malloc(sizeof(struct checker));
	if (!checker)
		return (struct chk *)checker;

	checker->ops			= &checker_ops;
	checker->pass_criteria		= pass_criteria;
	checker->pass_criteria_checked	= false;
	checker->pass_criteria_met	= true;

	return (struct chk *)checker;
};
