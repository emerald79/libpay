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
| Global Data								       |
+-----------------------------------------------------------------------------*/

extern uint32_t transaction_sequence_counter;

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
#define PDOL_D1029F66049F02069F03069C019F37049F2A08			       \
	.pdol = {							       \
		0xD1, 0x02, 0x9F, 0x66, 0x04, 0x9F, 0x02, 0x06, 0x9F, 0x03,    \
		0x06, 0x9C, 0x01, 0x9F, 0x37, 0x04, 0x9F, 0x2A, 0x08	       \
	},								       \
	.pdol_len = 19

/* PDOL with Acquirer Identifier, Additional Terminal Capabilities, Application
 * Identifier (AID) – terminal, Application Version Number, Interface Device
 * (IFD) Serial Number, Merchant Category Code, Merchant Identifier, Merchant
 * Name and Location, Point-of-Service (POS) Entry Mode, Terminal Capabilities,
 * Terminal Country Code, Terminal Identification, Terminal Type, Transaction
 * Date, Transaction Sequence Counter, Transaction Status Information,
 * Transaction Time, Transaction Type					      */
#define PDOL_9F01069F40059F06079F0902___9F21039C01			       \
	.pdol = {							       \
		0x9F, 0x01, 0x06, 0x9F, 0x40, 0x05, 0x9F, 0x06, 0x07, 0x9F,    \
		0x09, 0x02, 0x9F, 0x1E, 0x08, 0x9F, 0x15, 0x02, 0x9F, 0x16,    \
		0x0F, 0x9F, 0x4E, 0x60, 0x9F, 0x39, 0x01, 0x9F, 0x33, 0x03,    \
		0x9F, 0x1A, 0x02, 0x9F, 0x1C, 0x08, 0x9F, 0x35, 0x01, 0x9A,    \
		0x03, 0x9F, 0x41, 0x04, 0x9B, 0x02, 0x9F, 0x21, 0x03, 0x9C,    \
		0x01							       \
	},								       \
	.pdol_len = 51

/* PDOL with Pre-Processing indicator, TTQ */
#define PDOL_D1029F6604 .pdol = { 0xD1, 0x02, 0x9F, 0x66, 0x04 }, .pdol_len = 5

/* PDOL with TTQ, kernel ID , Unpredictable Number */
#define PDOL_9F66049F2A089F3704						       \
	.pdol = {							       \
		0x9F, 0x66, 0x04, 0x9F, 0x2A, 0x08, 0x9F, 0x37, 0x04	       \
	},								       \
	.pdol_len = 9

/* PDOL with TTQ */
#define PDOL_9F6604 .pdol = { 0x9F, 0x66, 0x04 }, .pdol_len = 3

/* PDOL with Pre-Processing indicator, Start Point ,TTQ, Issuer Authentication
 * Data */
#define PDOL_D102D2019F66049110						       \
	.pdol = {							       \
		0xD1, 0x02, 0xD2, 0x01, 0x9F, 0x66, 0x04, 0x91, 0x10	       \
	},								       \
	.pdol_len = 9

/* PDOL with Pre-Processing indicator, start Point, Issuer Script 71          */
#define PDOL_D102D2017110						       \
	.pdol = {							       \
		0xD1, 0x02, 0xD2, 0x01, 0x71, 0x10			       \
	},								       \
	.pdol_len = 6

/* PDOL with Pre-Processing indicator, start Point, Issuer Script 72          */
#define PDOL_D102D2017210						       \
	.pdol = {							       \
		0xD1, 0x02, 0xD2, 0x01, 0x72, 0x10			       \
	},								       \
	.pdol_len = 6

/* PDOL with Pre-Processing indicator, restart, start, Issuer Authentication
 * Data, Issuer script data						      */
#define PDOL_D102D20191107110						       \
	.pdol = {							       \
		0xD1, 0x02, 0xD2, 0x01, 0x91, 0x10, 0x71, 0x10		       \
	},								       \
	.pdol_len = 8

/* PDOL with Pre-Processing indicator, restart, start, Issuer Authentication
 * Data, Issuer script data						      */
#define PDOL_D102D20191107210						       \
	.pdol = {							       \
		0xD1, 0x02, 0xD2, 0x01, 0x91, 0x10, 0x72, 0x10		       \
	},								       \
	.pdol_len = 8

/* PDOL with Pre-Processing indicator, start Point			      */
#define PDOL_D102D201 .pdol = { 0xD1, 0x02, 0xD2, 0x01 }, .pdol_len = 4

/* PDOL with Pre-Processing indicator, Start Point, TTQ			      */
#define PDOL_D102D2019F6604						       \
	.pdol = {							       \
		0xD1, 0x02, 0xD2, 0x01, 0x9F, 0x66, 0x04		       \
	},								       \
	.pdol_len = 7

/*-----------------------------------------------------------------------------+
| TTQs									       |
+-----------------------------------------------------------------------------*/

/* TTQ value '84008000': Mag-stripe mode supported, EMV mode not supported, EMV
 * contact chip not supported, Online capable reader, Online PIN supported,
 * Signature not supported, Online cryptogram not required, CVM not required,
 * (Contact Chip) Offline PIN not supported, Issuer Update Processing supported.
 */
#define TTQ_84008000 .ttq = { 0x84, 0x00, 0x80, 0x00 }

/* TTQ value '28000000': Mag-stripe mode not supported, EMV mode supported, EMV
 * contact chip not supported, Offline-only reader, Online PIN not supported,
 * Signature not supported, Online cryptogram not required, CVM not required,
 * (Contact Chip) Offline PIN not supported, Issuer Update Processing not
 * supported.								      */
#define TTQ_28000000 .ttq = { 0x28, 0x00, 0x00, 0x00 }

/* TTQ value '84C08000': Mag-stripe mode supported, EMV mode not supported, EMV
 * contact chip not supported, Online capable reader, Online PIN supported,
 * Signature not supported, Online cryptogram required, CVM required, (Contact
 * Chip) Offline PIN not supported, Issuer Update Processing supported.	      */
#define TTQ_84C08000 .ttq = { 0x84, 0xC0, 0x80, 0x00 }

/* TTQ: Mag-stripe mode supported, EMV mode supported, EMV contact chip
 * supported, Online capable reader, Online PIN supported, Signature supported,
 * Online cryptogram not required, CVM not required, (Contact Chip) Offline PIN
 * supported, Issuer Update Processing supported.			      */
#define TTQ_B6208000 .ttq = { 0xB6, 0x20, 0x80, 0x00 }

/*-----------------------------------------------------------------------------+
| Test Checker (chk)							       |
+-----------------------------------------------------------------------------*/

struct chk;

struct chk_ops {
	void (*txn_start)(struct chk *chk);
	void (*field_on)(struct chk *chk);
	void (*field_off)(struct chk *chk);
	void (*select)(struct chk *chk, const uint8_t *data, size_t len);
	void (*gpo_data)(struct chk *chk, struct tlv *gpo_data);
	void (*ui_request)(struct chk *chk,
				       const struct emv_ui_request *ui_request);
	void (*outcome)(struct chk *chk,
				       const struct emv_outcome_parms *outcome);
	bool (*pass_criteria_met)(struct chk *chk);
	void (*free)(struct chk *chk);
};

struct chk {
	const struct chk_ops *ops;
};


enum pass_criteria {
	pc_2ea_001_00_case01 =  0,
	pc_2ea_001_00_case02 =  1,
	pc_2ea_001_00_case03 =  2,
	pc_2ea_002_00	     =  3,
	pc_2ea_002_01	     =  4,
	pc_2ea_002_02	     =  5,
	pc_2ea_003_00_case01 =  6,
	pc_2ea_003_00_case02 =  7,
	pc_2ea_004_00_case01 =  8,
	pc_2ea_004_00_case02 =  9,
	pc_2ea_005_00	     = 10,
	pc_2ea_005_01	     = 11,
	pc_2ea_006_00	     = 12,
	pc_2ea_006_01	     = 13,
	pc_2ea_006_02_case01 = 14,
	pc_2ea_006_02_case02 = 15,
	pc_2ea_006_03	     = 16,
	pc_2ea_006_04	     = 17,
	pc_2ea_006_05	     = 18,
	pc_2ea_007_00	     = 19,
	pc_2ea_011_00_case01 = 20,
	pc_2ea_011_00_case02 = 21,
	pc_2ea_011_00_case03 = 22,
	pc_2ea_012_00_case01 = 23,
	pc_2ea_012_00_case02 = 24,
	pc_2ea_012_00_case03 = 25,
	pc_2ea_012_00_case04 = 26,
	pc_2ea_013_00_case01 = 27,
	pc_2ea_013_00_case02 = 28,
	pc_2ea_013_00_case03 = 29,
	pc_2ea_013_01_case01 = 30,
	pc_2ea_013_01_case02 = 31,
	pc_2ea_014_00_case01 = 32,
	pc_2ea_014_00_case02 = 33,
	pc_2ea_014_00_case03 = 34,
	pc_2ea_014_00_case04 = 35,
	pc_2ea_014_00_case05 = 36,
	pc_2ea_014_00_case06 = 37,
	pc_2ea_014_01_case01 = 38,
	pc_2ea_014_01_case02 = 39,
	pc_2ea_015_00_case01 = 40,
	pc_2ea_015_00_case02 = 41,
	pc_2ea_015_00_case03 = 42,
	pc_2ea_015_00_case04 = 43,
};

struct chk *chk_pass_criteria_new(enum pass_criteria pass_criteria,
						    const char *log4c_category);

/*-----------------------------------------------------------------------------+
| Terminal Settings (term)						       |
+-----------------------------------------------------------------------------*/

#define ACQUIRER_IDENTIFIER		((const uint8_t [])		       \
					 { 0x00, 0x01, 0x23, 0x45, 0x67, 0x89 })
#define MERCHANT_CATEGORY_CODE		((const uint8_t []){ 0x30, 0x08 })
#define MERCHANT_IDENTIFIER		"Lufthansa AG   "
#define TERMINAL_COUNTRY_CODE		((const uint8_t []){ 0x02, 0x76 })
#define TERMINAL_IDENTIFICATION		"COUNTER3"
#define TERMINAL_CAPABILITIES		((const uint8_t []){ 0xDE, 0xAD, 0x22 })
#define TERMINAL_TYPE			0x25
#define POS_ENTRY_MODE			0x05
#define ADDITIONAL_TERMINAL_CAPABILITIES ((const uint8_t [])		       \
					       { 0x02, 0x00, 0x00, 0x00, 0x00 })
#define MERCHANT_NAME_AND_LOCATION	"Lufthansa - Rhein-Main Airport"
#define INTERFACE_DEVICE_SERIAL_NUMBER  "0000FE16"

enum termsetting {
	termsetting1 = 0,
	termsetting2 = 1,
	termsetting3 = 2,
	termsetting4 = 3,
	num_termsettings
};

int term_get_setting(enum termsetting termsetting, void *buffer, size_t *size);


/*-----------------------------------------------------------------------------+
| Lower Tester (lt)							       |
+-----------------------------------------------------------------------------*/

enum ltsetting {
	ltsetting1_1,
	ltsetting1_2,
	ltsetting1_3,
	ltsetting1_4,
	ltsetting1_11,
	ltsetting1_13,
	ltsetting1_15,
	ltsetting1_18,
	ltsetting1_20,
	ltsetting1_22,
	ltsetting1_24,
	ltsetting1_27,
	ltsetting1_28,
	ltsetting1_34,
	ltsetting1_35,
	ltsetting1_60,
	ltsetting1_61,
	ltsetting1_62,
	ltsetting1_70,
	ltsetting1_71,
	ltsetting1_72,
	ltsetting1_73,
	ltsetting1_80,
	ltsetting1_81,
	ltsetting1_82,
	ltsetting1_90,
	ltsetting1_91,
	ltsetting1_97,
	ltsetting1_98,
	ltsetting1_99,
	ltsetting2_40,
	ltsetting8_0,
	ltsetting8_1,
	num_ltsettings
};

struct emv_hal *lt_new(enum ltsetting ltsetting, struct chk *checker,
						    const char *log4c_category);
void lt_free(struct emv_hal *lt);


/*-----------------------------------------------------------------------------+
| Test Kernel (tk)							       |
+-----------------------------------------------------------------------------*/

#define KERNEL_ID_TK1	 .kernel_id = { 0x01 },		    .kernel_id_len = 1
#define KERNEL_ID_TK2	 .kernel_id = { 0x02 },		    .kernel_id_len = 1
#define KERNEL_ID_TK3	 .kernel_id = { 0x03 },		    .kernel_id_len = 1
#define KERNEL_ID_TK4	 .kernel_id = { 0x04 },		    .kernel_id_len = 1
#define KERNEL_ID_TK5	 .kernel_id = { 0x05 },		    .kernel_id_len = 1
#define KERNEL_ID_TK6	 .kernel_id = { 0x06 },		    .kernel_id_len = 1
#define KERNEL_ID_TK7	 .kernel_id = { 0x07 },		    .kernel_id_len = 1
#define KERNEL_ID_21	 .kernel_id = { 0x21 },		    .kernel_id_len = 1
#define KERNEL_ID_22	 .kernel_id = { 0x22 },		    .kernel_id_len = 1
#define KERNEL_ID_23	 .kernel_id = { 0x23 },		    .kernel_id_len = 1
#define KERNEL_ID_24	 .kernel_id = { 0x24 },		    .kernel_id_len = 1
#define KERNEL_ID_25	 .kernel_id = { 0x25 },		    .kernel_id_len = 1
#define KERNEL_ID_2B	 .kernel_id = { 0x26 },		    .kernel_id_len = 1
#define KERNEL_ID_32	 .kernel_id = { 0x32 },		    .kernel_id_len = 1
#define KERNEL_ID_810978 .kernel_id = { 0x81, 0x09, 0x78 }, .kernel_id_len = 3
#define KERNEL_ID_BF0840 .kernel_id = { 0xBF, 0x08, 0x40 }, .kernel_id_len = 3
#define KERNEL_ID_C11111 .kernel_id = { 0xC1, 0x11, 0x11 }, .kernel_id_len = 3
#define KERNEL_ID_FF2222 .kernel_id = { 0xFF, 0x22, 0x22 }, .kernel_id_len = 3

#define TK_APPLICATION_VERSION_NUMBER { 0x02, 0x03 }

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
