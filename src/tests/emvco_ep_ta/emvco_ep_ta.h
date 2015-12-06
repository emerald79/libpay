/*
 * libemv - Support Library for EMV TLV handling.
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

#ifndef __EMVCO_EP_TA_H__
#define __EMVCO_EP_TA_H__

#include <emv.h>

/*-----------------------------------------------------------------------------+
| Helper Macros                                                                |
+-----------------------------------------------------------------------------*/
#define ARRAY_SIZE(x) (sizeof(x)/sizeof(*(x)))


/*-----------------------------------------------------------------------------+
| Proprietary Tags for EMVCo Type Approval - Book A & Book B		       |
+-----------------------------------------------------------------------------*/
#define EMV_ID_TEST_FLAGS	  "\xD1"
#define EMV_ID_START_POINT	  "\xD2"
#define EMV_ID_SELECT_RESPONSE_SW "\xD3"
#define EMV_ID_OUTCOME_DATA	  "\xD4"
#define EMV_ID_UI_REQ_ON_OUTCOME  "\xD5"
#define EMV_ID_UI_REQ_ON_RESTART  "\xD6"


/*-----------------------------------------------------------------------------+
| Application Identifiers used in EMVCo Type Approval - Book A & Book B	       |
+-----------------------------------------------------------------------------*/
#define AID_A0000000010001 \
	.aid = { 0xA0, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01 }, .aid_len = 7
#define AID_A0000000020002 \
	.aid = { 0xA0, 0x00, 0x00, 0x00, 0x02, 0x00, 0x02 }, .aid_len = 7
#define AID_A0000000030003 \
	.aid = { 0xA0, 0x00, 0x00, 0x00, 0x03, 0x00, 0x03 }, .aid_len = 7
#define AID_A0000000040004 \
	.aid = { 0xA0, 0x00, 0x00, 0x00, 0x04, 0x00, 0x04 }, .aid_len = 7
#define AID_A0000000031010 \
	.aid = { 0xA0, 0x00, 0x00, 0x00, 0x03, 0x10, 0x10 }, .aid_len = 7
#define AID_A0000000041010 \
	.aid = { 0xA0, 0x00, 0x00, 0x00, 0x04, 0x10, 0x10 }, .aid_len = 7
#define AID_A00000002501   \
	.aid = { 0xA0, 0x00, 0x00, 0x00, 0x25, 0x01 },	     .aid_len = 6
#define AID_A0000000251010 \
	.aid = { 0xA0, 0x00, 0x00, 0x00, 0x25, 0x10, 0x10 }, .aid_len = 7
#define AID_A0000000651010 \
	.aid = { 0xA0, 0x00, 0x00, 0x00, 0x65, 0x10, 0x10 }, .aid_len = 7
#define AID_A0000001523010 \
	.aid = { 0xA0, 0x00, 0x00, 0x01, 0x52, 0x30, 0x10 }, .aid_len = 7
#define AID_A0000001532010 \
	.aid = { 0xA0, 0x00, 0x00, 0x01, 0x53, 0x20, 0x10 }, .aid_len = 7
#define AID_A0000003241010 \
	.aid = { 0xA0, 0x00, 0x00, 0x03, 0x24, 0x10, 0x10 }, .aid_len = 7
#define AID_A0000003330101 \
	.aid = { 0xA0, 0x00, 0x00, 0x03, 0x33, 0x01, 0x01 }, .aid_len = 7
#define AID_B0000000010101 \
	.aid = { 0xB0, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01 }, .aid_len = 7

/*-----------------------------------------------------------------------------+
| Application Labels							       |
+-----------------------------------------------------------------------------*/

#define APP_LABEL_APP1 \
	.app_label = "APP1", .app_label_len = 4
#define APP_LABEL_APP2 \
	.app_label = "APP2", .app_label_len = 4
#define APP_LABEL_APP3 \
	.app_label = "APP3", .app_label_len = 4
#define APP_LABEL_APP4 \
	.app_label = "APP4", .app_label_len = 4

/*-----------------------------------------------------------------------------+
| PDOLs									       |
+-----------------------------------------------------------------------------*/

/* PDOL with Pre-Processing indicator, TTQ, Amount, Amount Other, Transaction
 * Type, Restart flag, Unpredictable Number, Kernel ID			      */
#define PDOL_1								       \
	.pdol = {							       \
		0xD1, 0x02, 0x9F, 0x66, 0x04, 0x9F, 0x02, 0x06, 0x9F, 0x03,    \
		0x06, 0x9C, 0x01, 0x9F, 0x37, 0x04, 0x9F, 0x2A, 0x08	       \
	},								       \
	.pdol_len = 19


/*-----------------------------------------------------------------------------+
| TTQs									       |
+-----------------------------------------------------------------------------*/

/* TTQ value '84008000': Mag-stripe mode supported, EMV mode not supported, EMV
 * contact chip not supported, Online capable reader, Online PIN supported,
 * Signature not supported, Online cryptogram not required, CVM not required,
 * (Contact Chip) Offline PIN not supported, Issuer Update Processing supported.
 */
#define TTQ_1 .ttq = { 0x84, 0x00, 0x80, 0x00 }

/* TTQ value '28000000': Mag-stripe mode not supported, EMV mode supported, EMV
 * contact chip not supported, Offline-only reader, Online PIN not supported,
 * Signature not supported, Online cryptogram not required, CVM not required,
 * (Contact Chip) Offline PIN not supported, Issuer Update Processing not
 * supported.								      */
#define TTQ_2 .ttq = { 0x28, 0x00, 0x00, 0x00 }

/* TTQ value '84C08000': Mag-stripe mode supported, EMV mode not supported, EMV
 * contact chip not supported, Online capable reader, Online PIN supported,
 * Signature not supported, Online cryptogram required, CVM required, (Contact
 * Chip) Offline PIN not supported, Issuer Update Processing supported.	      */
#define TTQ_3 .ttq = { 0x84, 0xC0, 0x80, 0x00 }

/*-----------------------------------------------------------------------------+
| Test Checker (chk)							       |
+-----------------------------------------------------------------------------*/

struct chk;

struct chk_ops {
	void (*check_gpo_data)(struct chk *chk, struct tlv *gpo_data);
	void (*check_ui_request)(struct chk *chk,
				       const struct emv_ui_request *ui_request);
	void (*check_outcome)(struct chk *chk,
				       const struct emv_outcome_parms *outcome);
	bool (*pass_criteria_met)(struct chk *chk);
	void (*free)(struct chk *chk);
};

struct chk {
	const struct chk_ops *ops;
};


enum pass_criteria {
	pc_2ea_001_00_case01,
	pc_2ea_001_00_case02,
	pc_2ea_001_00_case03,
	pc_2ea_002_00,
	pc_2ea_002_01,
	pc_2ea_002_02,
	pc_2ea_003_00_case01,
	pc_2ea_003_00_case02,
	pc_2ea_004_00_case01,
	pc_2ea_004_00_case02,
};

struct chk *chk_pass_criteria_new(enum pass_criteria pass_criteria);

/*-----------------------------------------------------------------------------+
| Terminal Settings (term)						       |
+-----------------------------------------------------------------------------*/

enum termsetting {
	termsetting2 = 0,
	termsetting3 = 1,
	termsetting4 = 2,
	num_termsettings
};

int term_get_setting(enum termsetting termsetting, void *buffer, size_t *size);


/*-----------------------------------------------------------------------------+
| Lower Tester (lt)							       |
+-----------------------------------------------------------------------------*/

enum ltsetting {
	ltsetting1_1 = 0,
	ltsetting1_2 = 1,
	ltsetting1_3 = 2,
	ltsetting1_4 = 3,
	ltsetting1_97 = 4,
	num_ltsettings
};

struct emv_hal *lt_new(enum ltsetting ltsetting, struct chk *checker,
						    const char *log4c_category);
void lt_free(struct emv_hal *lt);


/*-----------------------------------------------------------------------------+
| Test Kernel (tk)							       |
+-----------------------------------------------------------------------------*/

#define KERNEL_ID_TK1 .kernel_id = { 0x01 }, .kernel_id_len = 1
#define KERNEL_ID_TK2 .kernel_id = { 0x02 }, .kernel_id_len = 1
#define KERNEL_ID_TK3 .kernel_id = { 0x03 }, .kernel_id_len = 1
#define KERNEL_ID_TK4 .kernel_id = { 0x04 }, .kernel_id_len = 1
#define KERNEL_ID_TK5 .kernel_id = { 0x05 }, .kernel_id_len = 1
#define KERNEL_ID_TK6 .kernel_id = { 0x06 }, .kernel_id_len = 1
#define KERNEL_ID_TK7 .kernel_id = { 0x07 }, .kernel_id_len = 1
#define KERNEL_ID_21  .kernel_id = { 0x21 }, .kernel_id_len = 1
#define KERNEL_ID_22  .kernel_id = { 0x22 }, .kernel_id_len = 1
#define KERNEL_ID_23  .kernel_id = { 0x23 }, .kernel_id_len = 1
#define KERNEL_ID_24  .kernel_id = { 0x24 }, .kernel_id_len = 1
#define KERNEL_ID_25  .kernel_id = { 0x25 }, .kernel_id_len = 1
#define KERNEL_ID_2B  .kernel_id = { 0x26 }, .kernel_id_len = 1

struct tk_id {
	uint8_t kernel_id[8];
	size_t  kernel_id_len;
};

struct emv_kernel *tk_new(const char *log4c_category);

void tk_free(struct emv_kernel *tk);


/*-----------------------------------------------------------------------------+
| Outcome data as provided by Lower Tester to Test Kernel in		       |
| EMV_ID_OUTCOME_DATA type TLV nodes.					       |
+-----------------------------------------------------------------------------*/

struct outcome_gpo_resp {
	uint8_t	 outcome;
	uint8_t	 start;
	uint8_t	 online_resp;
	uint8_t	 cvm;
	uint8_t	 alt_iface_pref;
	uint8_t	 receipt;
	uint16_t field_off_request;
	uint16_t removal_timeout;
} __attribute__((packed));

void gpo_outcome_to_outcome(const struct outcome_gpo_resp *gpo_outcome,
					     struct emv_outcome_parms *outcome);

void outcome_to_gpo_outcome(const struct emv_outcome_parms *outcome,
					  struct outcome_gpo_resp *gpo_outcome);


/*-----------------------------------------------------------------------------+
| UI Request data as provided by Lower Tester to Test Kernel in		       |
| EMV_ID_UI_REQ_ON_OUTCOME and EMV_ID_UI_REQ_ON_RESTART type TLV nodes.	       |
+-----------------------------------------------------------------------------*/

struct ui_req_gpo_resp {
	uint8_t	 msg_id;
	uint8_t	 status;
	uint16_t hold_time;
	uint8_t	 lang_pref[2];
	uint8_t	 value_qual;
	uint8_t	 value[6];
	uint8_t	 currency_code[2];
} __attribute__((packed));

void gpo_ui_req_to_ui_req(const struct ui_req_gpo_resp *gpo_ui_req,
						 struct emv_ui_request *ui_req);

void ui_req_to_gpo_ui_req(const struct emv_ui_request *ui_req,
					    struct ui_req_gpo_resp *gpo_ui_req);


#endif						    /* ndef __EMVCO_EP_TA_H__ */
