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
#define AID_A0000000 .aid = { 0xA0, 0x00, 0x00, 0x00 }, .aid_len = 4

#define AID_A0000000010001 \
	.aid = { 0xA0, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01 }, .aid_len = 7
#define AID_A000000001000101 \
	.aid = { 0xA0, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x01 }, .aid_len = 8
#define AID_A0000000010001010203040506070809				       \
	.aid = {							       \
		0xA0, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x01, 0x02, 0x03,    \
		0x04, 0x05, 0x06, 0x07, 0x08, 0x09			       \
	}, .aid_len = 16
#define AID_A000000001000103 \
	.aid = { 0xA0, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x03 }, .aid_len = 8
#define AID_A0000000020002 \
	.aid = { 0xA0, 0x00, 0x00, 0x00, 0x02, 0x00, 0x02 }, .aid_len = 7
#define AID_A000000002000201 \
	.aid = { 0xA0, 0x00, 0x00, 0x00, 0x02, 0x00, 0x02, 0x01 }, .aid_len = 8
#define AID_A0000000020002010203040506070809				       \
	.aid = {							       \
		0xA0, 0x00, 0x00, 0x00, 0x02, 0x00, 0x02, 0x01, 0x02, 0x03,    \
		0x04, 0x05, 0x06, 0x07, 0x08, 0x09			       \
	}, .aid_len = 16
#define AID_A000000002000202 \
	.aid = { 0xA0, 0x00, 0x00, 0x00, 0x02, 0x00, 0x02, 0x02 }, .aid_len = 8
#define AID_A0000000030003 \
	.aid = { 0xA0, 0x00, 0x00, 0x00, 0x03, 0x00, 0x03 }, .aid_len = 7
#define AID_A000000003000301 \
	.aid = { 0xA0, 0x00, 0x00, 0x00, 0x03, 0x00, 0x03, 0x01 }, .aid_len = 8
#define AID_A0000000030003010203040506070809				       \
	.aid = {							       \
		0xA0, 0x00, 0x00, 0x00, 0x03, 0x00, 0x03, 0x01, 0x02, 0x03,    \
		0x04, 0x05, 0x06, 0x07, 0x08, 0x09			       \
	}, .aid_len = 16
#define AID_A0000000031010 \
	.aid = { 0xA0, 0x00, 0x00, 0x00, 0x03, 0x10, 0x10 }, .aid_len = 7
#define AID_A000000003101001 \
	.aid = { 0xA0, 0x00, 0x00, 0x00, 0x03, 0x10, 0x10, 0x01 }, .aid_len = 8
#define AID_A0000000040004 \
	.aid = { 0xA0, 0x00, 0x00, 0x00, 0x04, 0x00, 0x04 }, .aid_len = 7
#define AID_A000000004000401 \
	.aid = { 0xA0, 0x00, 0x00, 0x00, 0x04, 0x00, 0x04, 0x01 }, .aid_len = 8
#define AID_A000000004000402 \
	.aid = { 0xA0, 0x00, 0x00, 0x00, 0x04, 0x00, 0x04, 0x02 }, .aid_len = 8
#define AID_A000000004000404 \
	.aid = { 0xA0, 0x00, 0x00, 0x00, 0x04, 0x00, 0x04, 0x04 }, .aid_len = 8
#define AID_A0000000040004010203040506070809				       \
	.aid = {							       \
		0xA0, 0x00, 0x00, 0x00, 0x04, 0x00, 0x04, 0x01, 0x02, 0x03,    \
		0x04, 0x05, 0x06, 0x07, 0x08, 0x09			       \
	}, .aid_len = 16
#define AID_A00000000400040102030405060708090A				       \
	.aid = {							       \
		0xA0, 0x00, 0x00, 0x00, 0x04, 0x00, 0x04, 0x01, 0x02, 0x03,    \
		0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A		       \
	},								       \
	.aid_len = 17
#define AID_A0000000041010 \
	.aid = { 0xA0, 0x00, 0x00, 0x00, 0x04, 0x10, 0x10 }, .aid_len = 7
#define AID_A000000004101001 \
	.aid = { 0xA0, 0x00, 0x00, 0x00, 0x04, 0x10, 0x10, 0x01 }, .aid_len = 8
#define AID_A000000006101001 \
	.aid = { 0xA0, 0x00, 0x00, 0x00, 0x06, 0x10, 0x10, 0x01 }, .aid_len = 8
#define AID_A000000005 \
	.aid = { 0xA0, 0x00, 0x00, 0x00, 0x05 }, .aid_len = 5
#define AID_A00000002501   \
	.aid = { 0xA0, 0x00, 0x00, 0x00, 0x25, 0x01 },	     .aid_len = 6
#define AID_A0000000251010 \
	.aid = { 0xA0, 0x00, 0x00, 0x00, 0x25, 0x10, 0x10 }, .aid_len = 7
#define AID_A0000000651010 \
	.aid = { 0xA0, 0x00, 0x00, 0x00, 0x65, 0x10, 0x10 }, .aid_len = 7
#define AID_A000000065101001 \
	.aid = { 0xA0, 0x00, 0x00, 0x00, 0x65, 0x10, 0x10, 0x01 }, .aid_len = 8
#define AID_A000000025101001 \
	.aid = { 0xA0, 0x00, 0x00, 0x00, 0x25, 0x10, 0x10, 0x01 }, .aid_len = 8
#define AID_A0000000661010 \
	.aid = { 0xA0, 0x00, 0x00, 0x00, 0x66, 0x10, 0x10 }, .aid_len = 7
#define AID_A0000001523010 \
	.aid = { 0xA0, 0x00, 0x00, 0x01, 0x52, 0x30, 0x10 }, .aid_len = 7
#define AID_A000000152301001 \
	.aid = { 0xA0, 0x00, 0x00, 0x01, 0x52, 0x30, 0x10, 0x01 }, .aid_len = 8
#define AID_A0000001532010 \
	.aid = { 0xA0, 0x00, 0x00, 0x01, 0x53, 0x20, 0x10 }, .aid_len = 7
#define AID_A0000003241010 \
	.aid = { 0xA0, 0x00, 0x00, 0x03, 0x24, 0x10, 0x10 }, .aid_len = 7
#define AID_A000000324101001 \
	.aid = { 0xA0, 0x00, 0x00, 0x03, 0x24, 0x10, 0x10, 0x01 }, .aid_len = 8
#define AID_A0000003330101 \
	.aid = { 0xA0, 0x00, 0x00, 0x03, 0x33, 0x01, 0x01 }, .aid_len = 7
#define AID_B0000000010101 \
	.aid = { 0xB0, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01 }, .aid_len = 7
#define AID_B000000001010102 \
	.aid = { 0xB0, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x02}, .aid_len = 8
#define AID_B0000000010102 \
	.aid = { 0xB0, 0x00, 0x00, 0x00, 0x01, 0x01, 0x02 }, .aid_len = 7
#define AID_B0000000010103 \
	.aid = { 0xB0, 0x00, 0x00, 0x00, 0x01, 0x01, 0x03 }, .aid_len = 7
#define AID_B0000000010104 \
	.aid = { 0xB0, 0x00, 0x00, 0x00, 0x01, 0x01, 0x04 }, .aid_len = 7
#define AID_B0000000010105 \
	.aid = { 0xB0, 0x00, 0x00, 0x00, 0x01, 0x01, 0x04 }, .aid_len = 7

#define EXT_SEL_01	.ext_select = { 0x01 }, .ext_select_len = 1
#define EXT_SEL_02	.ext_select = { 0x02 }, .ext_select_len = 1
#define EXT_SEL_03	.ext_select = { 0x03 }, .ext_select_len = 1
#define EXT_SEL_04	.ext_select = { 0x04 }, .ext_select_len = 1

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
#define APP_LABEL_VISA \
	.app_label = "VISA", .app_label_len = 4
#define APP_LABEL_MASTERCARD \
	.app_label = "MASTERCARD", .app_label_len = 10
#define APP_LABEL_JCB \
	.app_label = "JCB", .app_label_len = 3
#define APP_LABEL_AMEX \
	.app_label = "AMEX", .app_label_len = 4
#define APP_LABEL_DISCOVER \
	.app_label = "DISCOVER", .app_label_len = 8
#define APP_LABEL_CUP \
	.app_label = "CUP", .app_label_len = 3
#define APP_LABEL_DOMESTIC \
	.app_label = "Domestic", .app_label_len = 8

/*-----------------------------------------------------------------------------+
| Application Priority Indicators					       |
+-----------------------------------------------------------------------------*/

#define API_00 \
	.app_prio = { 0x00 }, .app_prio_len = 1
#define API_01 \
	.app_prio = { 0x01 }, .app_prio_len = 1
#define API_02 \
	.app_prio = { 0x02 }, .app_prio_len = 1
#define API_03 \
	.app_prio = { 0x03 }, .app_prio_len = 1
#define API_04 \
	.app_prio = { 0x04 }, .app_prio_len = 1
#define API_05 \
	.app_prio = { 0x05 }, .app_prio_len = 1
#define API_06 \
	.app_prio = { 0x06 }, .app_prio_len = 1
#define API_07 \
	.app_prio = { 0x07 }, .app_prio_len = 1
#define API_08 \
	.app_prio = { 0x08 }, .app_prio_len = 1
#define API_09 \
	.app_prio = { 0x09 }, .app_prio_len = 1
#define API_0A \
	.app_prio = { 0x0A }, .app_prio_len = 1
#define API_0B \
	.app_prio = { 0x0B }, .app_prio_len = 1
#define API_0C \
	.app_prio = { 0x0C }, .app_prio_len = 1
#define API_0D \
	.app_prio = { 0x0D }, .app_prio_len = 1
#define API_0E \
	.app_prio = { 0x0E }, .app_prio_len = 1
#define API_0F \
	.app_prio = { 0x0F }, .app_prio_len = 1
#define API_41 \
	.app_prio = { 0x41 }, .app_prio_len = 1
#define API_F2 \
	.app_prio = { 0xF2 }, .app_prio_len = 1
#define API_FF \
	.app_prio = { 0xFF }, .app_prio_len = 1

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

/* PDOL with Pre-Processing indicator */
#define PDOL_D102 .pdol = { 0xD1, 0x02 }, .pdol_len = 2

/* PDOL with TTQ, kernel ID , Unpredictable Number */
#define PDOL_9F66049F2A089F3704						       \
	.pdol = {							       \
		0x9F, 0x66, 0x04, 0x9F, 0x2A, 0x08, 0x9F, 0x37, 0x04	       \
	},								       \
	.pdol_len = 9

/* PDOL with TTQ, kernel ID */
#define PDOL_9F66049F2A08						       \
	.pdol = {							       \
		0x9F, 0x66, 0x04, 0x9F, 0x2A, 0x08			       \
	},								       \
	.pdol_len = 6

/* PDOL with kernel ID */
#define PDOL_9F2A08 .pdol = { 0x9F, 0x2A, 0x08 }, .pdol_len = 3

/* PDOL with TTQ */
#define PDOL_9F6604 .pdol = { 0x9F, 0x66, 0x04 }, .pdol_len = 3

/* PDOL with Pre-Processing indicator, Start Point ,TTQ, Issuer Authentication
 * Data									      */
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

/* PDOL with Pre-Processing indicator, TTQ, kernel ID, FCI		      */
#define PDOL_D1029F66049F2A086F23					       \
	.pdol = {							       \
		0xD1, 0x02, 0x9F, 0x66, 0x04, 0x9F, 0x2A, 0x08, 0x6F, 0x23     \
	},								       \
	.pdol_len = 10

/* PDOL with Pre-Processing indicator, TTQ, kernel ID, FCI		      */
#define PDOL_D1029F66049F2A086F24					       \
	.pdol = {							       \
		0xD1, 0x02, 0x9F, 0x66, 0x04, 0x9F, 0x2A, 0x08, 0x6F, 0x24     \
	},								       \
	.pdol_len = 10

/* PDOL with Amount Authorized, Amount Other, Unpredictable Number, DF01, C1,
 * 85, 9F25								      */
#define PDOL_9F66049F02069F03069F3704DF0101C102__9F2502			       \
	.pdol = {							       \
		0x9F, 0x66, 0x04, 0x9F, 0x02, 0x06, 0x9F, 0x03, 0x06, 0x9F,    \
		0x37, 0x04, 0xDF, 0x01, 0x01, 0xC1, 0x02, 0x85, 0x01, 0x9F,    \
		0x25, 0x02						       \
	},								       \
	.pdol_len = 22

/* PDOL with: Amount Authorized (tag 9F02) with length = 7, Unpredictable Number
 * (tag 9F37) with length = 5, Merchant Identifier (tag 9F16) with length = 16,
 * Terminal Identification (tag 9F1C) with length = 9			      */
#define PDOL_9F02079F37059F16109F1C09					       \
	.pdol = {							       \
		0x9F, 0x02, 0x07, 0x9F, 0x37, 0x05, 0x9F, 0x16, 0x10, 0x9F,    \
		0x1C, 0x09						       \
	},								       \
	.pdol_len = 12

/* PDOL with: Amount Authorized (tag 9F02) with length = 16, Unpredictable
 * Number (tag 9F37) with length = 14, Merchant Identifier (tag 9F16) with
 * length = 25, Terminal Identification (tag 9F1C) with length = 18	      */
#define PDOL_9F02109F370E9F16199F1C12					       \
	.pdol = {							       \
		0x9F, 0x02, 0x10, 0x9F, 0x37, 0x0E, 0x9F, 0x16, 0x19, 0x9F,    \
		0x1C, 0x12						       \
	},								       \
	.pdol_len = 12

/* PDOL with: ‘Amount Authorized (tag 9F02)				      */
#define PDOL_9F0206 .pdol = { 0x9F, 0x02, 0x06 }, .pdol_len = 3

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

/* TTQ: Mag-stripe mode supported, EMV mode supported, EMV contact chip
 * supported, Online capable reader, Online PIN supported, Signature supported,
 * Online cryptogram not required, CVM not required, (Contact Chip) Offline PIN
 * supported, Issuer Update Processing supported. (Several RFU bits set).     */
#define TTQ_F72A8A55 .ttq = { 0xF7, 0x2A, 0x8A, 0x55 }

/*-----------------------------------------------------------------------------+
| Test Checker (chk)							       |
+-----------------------------------------------------------------------------*/

struct chk;

struct chk_ops {
	void (*ep_start)(struct chk *chk);
	void (*ep_restart)(struct chk *chk);
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
	pc_2ea_001_00_case01 =	 0,
	pc_2ea_001_00_case02 =	 1,
	pc_2ea_001_00_case03 =	 2,
	pc_2ea_002_00	     =	 3,
	pc_2ea_002_01	     =	 4,
	pc_2ea_002_02	     =	 5,
	pc_2ea_003_00_case01 =	 6,
	pc_2ea_003_00_case02 =	 7,
	pc_2ea_004_00_case01 =	 8,
	pc_2ea_004_00_case02 =	 9,
	pc_2ea_005_00	     =	10,
	pc_2ea_005_01	     =	11,
	pc_2ea_006_00	     =	12,
	pc_2ea_006_01	     =	13,
	pc_2ea_006_02_case01 =	14,
	pc_2ea_006_02_case02 =	15,
	pc_2ea_006_03	     =	16,
	pc_2ea_006_04	     =	17,
	pc_2ea_006_05	     =	18,
	pc_2ea_007_00	     =	19,
	pc_2ea_011_00_case01 =	20,
	pc_2ea_011_00_case02 =	21,
	pc_2ea_011_00_case03 =	22,
	pc_2ea_012_00_case01 =	23,
	pc_2ea_012_00_case02 =	24,
	pc_2ea_012_00_case03 =	25,
	pc_2ea_012_00_case04 =	26,
	pc_2ea_013_00_case01 =	27,
	pc_2ea_013_00_case02 =	28,
	pc_2ea_013_00_case03 =	29,
	pc_2ea_013_01_case01 =	30,
	pc_2ea_013_01_case02 =	31,
	pc_2ea_014_00_case01 =	32,
	pc_2ea_014_00_case02 =	33,
	pc_2ea_014_00_case03 =	34,
	pc_2ea_014_00_case04 =	35,
	pc_2ea_014_00_case05 =	36,
	pc_2ea_014_00_case06 =	37,
	pc_2ea_014_01_case01 =	38,
	pc_2ea_014_01_case02 =	39,
	pc_2ea_015_00_case01 =	40,
	pc_2ea_015_00_case02 =	41,
	pc_2ea_015_00_case03 =	42,
	pc_2ea_015_00_case04 =	43,
	pc_2ea_016_00_case01 =	44,
	pc_2ea_016_00_case02 =	45,
	pc_2ea_016_00_case03 =	46,
	pc_2ea_016_00_case04 =	47,
	pc_2ea_016_00_case05 =	48,
	pc_2ea_016_00_case06 =	49,
	pc_2ea_016_00_case07 =	50,
	pc_2ea_016_00_case08 =	51,
	pc_2ea_017_00_case01 =	52,
	pc_2ea_017_00_case02 =	53,
	pc_2ea_017_00_case03 =	54,
	pc_2ea_017_01	     =	55,
	pc_2ea_018_00	     =	56,
	pc_2ea_019_00_case01 =	57,
	pc_2ea_019_00_case02 =	58,
	pc_2ea_019_00_case03 =	59,
	pc_2ea_019_00_case04 =	60,
	pc_2ea_020_00_case01 =	61,
	pc_2ea_020_00_case02 =	62,
	pc_2ea_021_00	     =	63,
	pc_2eb_001_00_case01 =	64,
	pc_2eb_001_00_case02 =	65,
	pc_2eb_001_00_case03 =	66,
	pc_2eb_001_00_case04 =	67,
	pc_2eb_001_00_case05 =	68,
	pc_2eb_002_00_case01 =	69,
	pc_2eb_002_00_case02 =	70,
	pc_2eb_002_00_case03 =	71,
	pc_2eb_003_00_case01 =	72,
	pc_2eb_003_00_case02 =	73,
	pc_2eb_003_00_case03 =	74,
	pc_2eb_003_00_case04 =	75,
	pc_2eb_003_00_case05 =	76,
	pc_2eb_003_01	     =	77,
	pc_2eb_003_02	     =	78,
	pc_2eb_004_00_case01 =	79,
	pc_2eb_004_00_case02 =	80,
	pc_2eb_004_00_case03 =	81,
	pc_2eb_004_01_case01 =	82,
	pc_2eb_004_01_case02 =	83,
	pc_2eb_004_02	     =	84,
	pc_2eb_005_00_case01 =	85,
	pc_2eb_005_00_case02 =	86,
	pc_2eb_005_00_case03 =	87,
	pc_2eb_005_01	     =	88,
	pc_2eb_006_00_case01 =	89,
	pc_2eb_006_00_case02 =	90,
	pc_2eb_006_00_case03 =	91,
	pc_2eb_006_00_case04 =	92,
	pc_2eb_006_00_case05 =	93,
	pc_2eb_006_01	     =	94,
	pc_2eb_007_00_case01 =	95,
	pc_2eb_007_00_case02 =	96,
	pc_2eb_007_01_case01 =	97,
	pc_2eb_007_01_case02 =	98,
	pc_2eb_008_00_case01 =	99,
	pc_2eb_008_00_case02 = 100,
	pc_2eb_008_00_case03 = 101,
	pc_2eb_008_00_case04 = 102,
	pc_2eb_008_01_case01 = 103,
	pc_2eb_008_01_case02 = 104,
	pc_2eb_008_02_case01 = 105,
	pc_2eb_008_02_case02 = 106,
	pc_2eb_009_00_case01 = 107,
	pc_2eb_009_00_case02 = 108,
	pc_2eb_009_01_case01 = 109,
	pc_2eb_009_01_case02 = 110,
	pc_2eb_009_01_case03 = 111,
	pc_2eb_009_01_case04 = 112,
	pc_2eb_009_02_case01 = 113,
	pc_2eb_009_02_case02 = 114,
	pc_2eb_010_00_case01 = 115,
	pc_2eb_010_00_case02 = 116,
	pc_2eb_010_00_case03 = 117,
	pc_2eb_010_00_case04 = 118,
	pc_2eb_010_00_case05 = 119,
	pc_2eb_010_01_case01 = 120,
	pc_2eb_010_01_case02 = 121,
	pc_2eb_010_01_case03 = 122,
	pc_2eb_011_00_case01 = 123,
	pc_2eb_011_00_case02 = 124,
	pc_2eb_011_00_case03 = 125,
	pc_2eb_011_00_case04 = 126,
	pc_2eb_011_01	     = 127,
	pc_2eb_011_02	     = 128,
	pc_2eb_012_00_case01 = 129,
	pc_2eb_012_00_case02 = 130,
	pc_2eb_012_01	     = 131,
	pc_2eb_013_00_case01 = 132,
	pc_2eb_013_00_case02 = 133,
	pc_2eb_013_00_case03 = 134,
	pc_2eb_013_00_case04 = 135,
	pc_2eb_013_00_case05 = 136,
	pc_2eb_013_00_case06 = 137,
	pc_2eb_013_01	     = 138,
	pc_2eb_013_02	     = 139,
	pc_2eb_014_00_case01 = 140,
	pc_2eb_014_00_case02 = 141,
	pc_2eb_014_00_case03 = 142,
	pc_2eb_014_00_case04 = 143,
	pc_2eb_014_00_case05 = 144,
	pc_2eb_014_01	     = 145,
	pc_2eb_014_02_case01 = 146,
	pc_2eb_014_02_case02 = 147,
	pc_2eb_015_00_case01 = 148,
	pc_2eb_015_00_case02 = 149,
	pc_2eb_015_00_case03 = 150,
	pc_2eb_015_00_case04 = 151,
	pc_2eb_015_00_case05 = 152,
	pc_2eb_015_00_case06 = 153,
	pc_2eb_015_01_case01 = 154,
	pc_2eb_015_01_case02 = 155,
	pc_2eb_015_01_case03 = 156,
	pc_2eb_015_01_case04 = 157,
	pc_2eb_015_02_case01 = 158,
	pc_2eb_015_02_case02 = 159,
	pc_2eb_015_02_case03 = 160,
	pc_2eb_015_02_case04 = 161,
	pc_2eb_016_00	     = 162,
	pc_2eb_016_01	     = 163,
	pc_2eb_016_02_case01 = 164,
	pc_2eb_016_02_case02 = 165,
	pc_2eb_017_00_case01 = 166,
	pc_2eb_017_00_case02 = 167,
	pc_2eb_017_00_case03 = 168,
	pc_2eb_017_00_case04 = 169,
	pc_2eb_017_01	     = 170,
	pc_2eb_017_02	     = 171,
	pc_2eb_018_00_case01 = 172,
	pc_2eb_018_00_case02 = 173,
	pc_2eb_018_00_case03 = 174,
	pc_2eb_018_00_case04 = 175,
	pc_2eb_018_01	     = 176,
	pc_2eb_018_02	     = 177,
	pc_2eb_019_00	     = 178,
	pc_2eb_019_01	     = 179,
	pc_2eb_019_02	     = 180,
	pc_2eb_020_00_case01 = 181,
	pc_2eb_020_00_case02 = 182,
	pc_2eb_020_01_case01 = 183,
	pc_2eb_020_01_case02 = 184,
	pc_2eb_021_00_case01 = 185,
	pc_2eb_021_00_case02 = 186,
	pc_2eb_022_00	     = 187,
	pc_2ec_001_00_case01 = 188,
	pc_2ec_001_00_case02 = 189,
	pc_2ec_001_00_case03 = 190,
	pc_2ec_001_00_case04 = 191,
	pc_2ec_001_00_case05 = 192,
	pc_2ec_001_01	     = 193,
	pc_2ec_001_02	     = 194,
	pc_2ec_001_03	     = 195,
	pc_2ec_001_04	     = 196,
	pc_2ec_002_00_case01 = 197,
	pc_2ec_002_00_case02 = 198,
	pc_2ec_002_00_case03 = 199,
	pc_2ec_002_00_case04 = 200,
	pc_2ec_003_00_case01 = 201,
	pc_2ec_003_00_case02 = 202,
	pc_2ec_003_00_case03 = 203,
	pc_2ec_003_00_case04 = 204,
	pc_2ec_004_00_case01 = 205,
	pc_2ec_004_00_case02 = 206,
	pc_2ec_005_00_case01 = 207,
	pc_2ec_005_00_case02 = 208,
	pc_2ec_005_00_case03 = 209,
	pc_2ec_005_00_case04 = 210,
	pc_2ec_006_00_case01 = 211,
	pc_2ec_006_00_case02 = 212,
	pc_2ec_006_00_case03 = 213,
	pc_2ec_006_00_case04 = 214,
	pc_2ec_007_00_case01 = 215,
	pc_2ec_007_00_case02 = 216,
	pc_2ec_007_00_case03 = 217,
	pc_2ec_007_00_case04 = 218,
	pc_2ed_001_00_case01 = 219,
	pc_2ed_001_00_case02 = 220,
	pc_2ed_001_00_case03 = 221,
	pc_2ed_001_00_case04 = 222,
	pc_2ed_001_00_case05 = 223,
	pc_2ed_001_00_case06 = 224,
	pc_2ed_001_01_case01 = 225,
	pc_2ed_001_01_case02 = 226,
	pc_2ed_001_01_case03 = 227,
	pc_2ed_001_01_case04 = 228,
	pc_2ed_001_01_case05 = 229,
	pc_2ed_001_01_case06 = 230,
	pc_2ed_002_00	     = 231,
	pc_2ed_002_01	     = 232,
	pc_2ed_003_00	     = 233,
	pc_2ed_003_01_case01 = 234,
	pc_2ed_003_01_case02 = 235,
	pc_2ed_003_01_case03 = 236,
	pc_2ed_004_00	     = 237,
	pc_2ed_005_00	     = 238,
	pc_2ed_007_00	     = 239,
	pc_2ed_007_01	     = 240,
	pc_2ed_008_00	     = 241,
	pc_2ed_009_00	     = 242,
	pc_2ed_009_02	     = 243,
	pc_2ed_009_04	     = 244,
	pc_2ed_009_06_case01 = 245,
	pc_2ed_009_06_case02 = 246,
	pc_2ed_009_10	     = 247,
	pc_2ed_009_12	     = 248,
	pc_2ed_009_14	     = 249,
	pc_2ed_009_16	     = 250,
	pc_2ed_009_18	     = 251,
	pc_2ed_009_20	     = 252,
	pc_2ed_009_22	     = 253,
	pc_2ed_009_24	     = 254,
	pc_2ed_009_26	     = 255,
	pc_2ed_009_28	     = 256,
	pc_2ed_010_00	     = 257,
	pc_2ed_011_00	     = 258,
	pc_2ed_012_00	     = 259,
	pc_2ed_012_02	     = 260,
	pc_2ed_012_04	     = 261,
	pc_2ed_012_06_case01 = 262,
	pc_2ed_012_06_case02 = 263,
	pc_2ed_012_08_case01 = 264,
	pc_2ed_012_08_case02 = 265,
	pc_2ed_012_08_case03 = 266,
	pc_2ed_012_08_case04 = 267,
	pc_2ed_012_10_case01 = 268,
	pc_2ed_012_10_case02 = 269,
	pc_2ed_012_10_case03 = 270,
	pc_2ed_012_10_case04 = 271,
	pc_2ed_012_10_case05 = 272,
	pc_2ed_012_10_case06 = 273,
	pc_2ed_012_10_case07 = 274,
	pc_2ed_012_10_case08 = 275,
	pc_2ed_012_10_case09 = 276,
	pc_2ed_012_10_case10 = 277,
	pc_2ed_012_11	     = 278,
	pc_2ed_012_13	     = 279,
	pc_2ed_012_15	     = 280,
	pc_2ed_013_00_case01 = 281,
	pc_2ed_013_00_case02 = 282,
	pc_2ed_013_00_case03 = 283,
	pc_2ed_013_00_case04 = 284,
	pc_2ed_013_00_case05 = 285,
	pc_2ed_013_00_case06 = 286,
	pc_2ed_013_00_case07 = 287,
	pc_2ed_013_00_case08 = 288,
	pc_2ed_013_00_case09 = 289,
	pc_2ed_013_00_case10 = 290,
	pc_2ed_014_00_case01 = 291,
	pc_2ed_014_00_case02 = 292,
	pc_2ed_014_00_case03 = 293,
	pc_2ed_014_00_case04 = 294,
	pc_2ed_014_00_case05 = 295,
	pc_2ed_015_00_case01 = 296,
	pc_2ed_015_00_case02 = 297,
	pc_2ed_015_00_case03 = 298,
	pc_2ed_015_00_case04 = 299,
	pc_2ed_015_00_case05 = 300,
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
	termsetting1	= 0,
	termsetting2	= 1,
	termsetting3	= 2,
	termsetting4	= 3,
	termsetting5_1	= 4,
	termsetting5_2	= 5,
	termsetting5_3	= 6,
	termsetting5_4	= 7,
	termsetting8	= 8,
	termsetting13	= 9,
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
	ltsetting1_5,
	ltsetting1_6,
	ltsetting1_7,
	ltsetting1_8,
	ltsetting1_9,
	ltsetting1_10,
	ltsetting1_11,
	ltsetting1_12,
	ltsetting1_13,
	ltsetting1_14,
	ltsetting1_15,
	ltsetting1_16,
	ltsetting1_17,
	ltsetting1_18,
	ltsetting1_19,
	ltsetting1_20,
	ltsetting1_21,
	ltsetting1_22,
	ltsetting1_23,
	ltsetting1_24,
	ltsetting1_25,
	ltsetting1_26,
	ltsetting1_27,
	ltsetting1_28,
	ltsetting1_29,
	ltsetting1_34,
	ltsetting1_35,
	ltsetting1_50,
	ltsetting1_51,
	ltsetting1_52,
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
	ltsetting1_84,
	ltsetting1_85,
	ltsetting1_86,
	ltsetting1_87,
	ltsetting1_88,
	ltsetting1_89,
	ltsetting1_90,
	ltsetting1_91,
	ltsetting1_92,
	ltsetting1_93,
	ltsetting1_94,
	ltsetting1_95,
	ltsetting1_96,
	ltsetting1_97,
	ltsetting1_98,
	ltsetting1_99,
	ltsetting1_100,
	ltsetting2_1,
	ltsetting2_2,
	ltsetting2_3,
	ltsetting2_4,
	ltsetting2_5,
	ltsetting2_6,
	ltsetting2_7,
	ltsetting2_8,
	ltsetting2_9,
	ltsetting2_10,
	ltsetting2_11,
	ltsetting2_12,
	ltsetting2_13,
	ltsetting2_14,
	ltsetting2_15,
	ltsetting2_16,
	ltsetting2_17,
	ltsetting2_18,
	ltsetting2_19,
	ltsetting2_20,
	ltsetting2_21,
	ltsetting2_22,
	ltsetting2_23,
	ltsetting2_24,
	ltsetting2_25,
	ltsetting2_40,
	ltsetting2_42,
	ltsetting2_43,
	ltsetting2_44,
	ltsetting3_1,
	ltsetting3_2,
	ltsetting3_3,
	ltsetting3_4,
	ltsetting3_6,
	ltsetting3_7,
	ltsetting4_1,
	ltsetting4_2,
	ltsetting5_1,
	ltsetting5_2,
	ltsetting5_3,
	ltsetting5_4,
	ltsetting5_5,
	ltsetting5_6,
	ltsetting5_7,
	ltsetting5_8,
	ltsetting5_10,
	ltsetting5_11,
	ltsetting5_12,
	ltsetting5_13,
	ltsetting5_14,
	ltsetting5_15,
	ltsetting5_16,
	ltsetting5_17,
	ltsetting5_18,
	ltsetting5_19,
	ltsetting5_20,
	ltsetting5_21,
	ltsetting5_22,
	ltsetting5_23,
	ltsetting5_24,
	ltsetting5_25,
	ltsetting6_1,
	ltsetting6_2,
	ltsetting6_3,
	ltsetting6_5,
	ltsetting6_6,
	ltsetting6_7,
	ltsetting6_10,
	ltsetting6_11,
	ltsetting6_12,
	ltsetting6_13,
	ltsetting6_14,
	ltsetting6_16,
	ltsetting8_0,
	ltsetting8_1,
	num_ltsettings
};

#define LT_NORMAL			0
#define LT_COLLISION_THEN_WITHDRAW_BOTH	1
#define LT_COLLISION_THEN_WITHDRAW_ONE	2

struct emv_hal *lt_new(enum ltsetting ltsetting, struct chk *checker,
					  const char *log4c_category, int mode);
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
#define KERNEL_ID_00	 .kernel_id = { 0x00 },		    .kernel_id_len = 1
#define KERNEL_ID_21	 .kernel_id = { 0x21 },		    .kernel_id_len = 1
#define KERNEL_ID_2102030405060708					       \
	      .kernel_id = { 0x21, 0x02, 0x02, 0x04, 0x05, 0x06, 0x07, 0x08 }, \
	      .kernel_id_len = 8
#define KERNEL_ID_22	 .kernel_id = { 0x22 },		    .kernel_id_len = 1
#define KERNEL_ID_23	 .kernel_id = { 0x23 },		    .kernel_id_len = 1
#define KERNEL_ID_23FF00FF00FF00FF					       \
	      .kernel_id = { 0x23, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF }, \
	      .kernel_id_len = 8
#define KERNEL_ID_24	 .kernel_id = { 0x24 },		    .kernel_id_len = 1
#define KERNEL_ID_240000 .kernel_id = { 0x24, 0x00, 0x00 }, .kernel_id_len = 3
#define KERNEL_ID_24FFFF .kernel_id = { 0x24, 0xFF, 0xFF }, .kernel_id_len = 3
#define KERNEL_ID_25	 .kernel_id = { 0x25 },		    .kernel_id_len = 1
#define KERNEL_ID_2B	 .kernel_id = { 0x2B },		    .kernel_id_len = 1
#define KERNEL_ID_32	 .kernel_id = { 0x32 },		    .kernel_id_len = 1
#define KERNEL_ID_6	 .kernel_id = { 0x06 },		    .kernel_id_len = 1
#define KERNEL_ID_62	 .kernel_id = { 0x62 },		    .kernel_id_len = 1
#define KERNEL_ID_80	 .kernel_id = { 0x80 },		    .kernel_id_len = 1
#define KERNEL_ID_810977 .kernel_id = { 0x81, 0x09, 0x77 }, .kernel_id_len = 3
#define KERNEL_ID_810978 .kernel_id = { 0x81, 0x09, 0x78 }, .kernel_id_len = 3
#define KERNEL_ID_811110 .kernel_id = { 0x81, 0x11, 0x10 }, .kernel_id_len = 3
#define KERNEL_ID_811111 .kernel_id = { 0x81, 0x11, 0x11 }, .kernel_id_len = 3
#define KERNEL_ID_BF0840 .kernel_id = { 0xBF, 0x08, 0x40 }, .kernel_id_len = 3
#define KERNEL_ID_BF0841 .kernel_id = { 0xBF, 0x08, 0x41 }, .kernel_id_len = 3
#define KERNEL_ID_BF2222 .kernel_id = { 0xBF, 0x22, 0x22 }, .kernel_id_len = 3
#define KERNEL_ID_BF2223 .kernel_id = { 0xBF, 0x22, 0x23 }, .kernel_id_len = 3
#define KERNEL_ID_C001	 .kernel_id = { 0xC0, 0x01 },	    .kernel_id_len = 2
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
