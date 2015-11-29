#ifndef __EMVCO_EP_TA_H__
#define __EMVCO_EP_TA_H__

#include <emv.h>

/*-----------------------------------------------------------------------------+
| Helper Macros                                                                |
+-----------------------------------------------------------------------------*/
#define ARRAY_SIZE(x) (sizeof(x)/sizeof(*(x)))

/*-----------------------------------------------------------------------------+
| Proprietary Tags                                                             |
+-----------------------------------------------------------------------------*/
#define EMV_ID_TEST_FLAGS	  "\xD1"
#define EMV_ID_START_POINT	  "\xD2"
#define EMV_ID_SELECT_RESPONSE_SW "\xD3"

/*-----------------------------------------------------------------------------+
| Application Identifiers						       |
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

/*-----------------------------------------------------------------------------+
| Kernel IDs								       |
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

enum ltsetting {
	ltsetting1_1 = 0,
	num_ltsettings
};

extern const char log4c_category[];

int get_termsetting_n(int n, void *buffer, size_t *size);

struct emv_hal *lt_new(enum ltsetting ltsetting);

void lt_free(struct emv_hal *lt);

struct emv_kernel *tk_new();

void tk_free(struct emv_kernel *tk);

#endif						    /* ndef __EMVCO_EP_TA_H__ */
