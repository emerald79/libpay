#include <arpa/inet.h>
#include <log4c.h>

#include <emv.h>
#include <tlv.h>

#include "emvco_ep_ta.h"

struct tk {
	const struct emv_kernel_ops *ops;
	log4c_category_t *log_cat;
};

static int tk_configure(struct emv_kernel *kernel, const void *config,
								     size_t len)
{
	struct tk *tk = (struct tk *)kernel;

	log4c_category_log(tk->log_cat,
			       LOG4C_PRIORITY_TRACE, "%s(): success", __func__);
	return EMV_RC_OK;
}

static struct tlv *tlv_kernel_parms(struct emv_kernel_parms *parms)
{
	const struct emv_ep_preproc_indicators *ind = parms->preproc_indicators;
	struct tlv *tlv_kernel_parms = NULL, *tlv = NULL;
	uint8_t amount_authorized[6], amount_other[6], txn_type, start;
	uint8_t test_flags[2] = { 0, 0 };
	uint32_t un = ntohl(parms->unpredictable_number);
	int rc = TLV_RC_OK;

	switch (parms->start) {
	case start_a:
		start = 0xA;
		break;
	case start_b:
		start = 0xB;
		break;
	case start_c:
		start = 0xC;
		break;
	case start_d:
		start = 0xD;
		break;
	default:
		rc = TLV_RC_INVALID_ARG;
		goto done;
	}

	if (ind->floor_limit_exceeded)
		test_flags[0] |= 0x80u;

	if (ind->txn_limit_exceeded)
		test_flags[0] |= 0x40u;

	if (ind->cvm_reqd_limit_exceeded)
		test_flags[0] |= 0x20u;

	if (ind->status_check_requested)
		test_flags[0] |= 0x10u;

	if (ind->zero_amount)
		test_flags[0] |= 0x08u;

	if (ind->ctls_app_not_allowed)
		test_flags[0] |= 0x40u;

	if (parms->restart)
		test_flags[1] |= 0x80u;

	switch (parms->txn_type) {
	case txn_purchase:
		if (parms->amount_other)
			txn_type = 0x09;
		else
			txn_type = 0x00;
		break;
	case txn_purchase_with_cashback:
		txn_type = 0x09;
		break;
	case txn_cash_advance:
		txn_type = 0x01;
		break;
	case txn_refund:
		txn_type = 0x20;
		break;
	default:
		rc = TLV_RC_INVALID_ARG;
		goto done;
	}

	rc = libtlv_u64_to_bcd(parms->amount_authorized, amount_authorized,
						     sizeof(amount_authorized));
	if (rc != TLV_RC_OK)
		goto done;

	rc = libtlv_u64_to_bcd(parms->amount_other, amount_other,
							  sizeof(amount_other));
	if (rc != TLV_RC_OK)
		goto done;

	tlv = tlv_kernel_parms =
		     tlv_new(EMV_ID_TEST_FLAGS, sizeof(test_flags), test_flags);
	tlv = tlv_insert_after(tlv, tlv_new(EMV_ID_TTQ, sizeof(ind->ttq),
								     ind->ttq));
	tlv = tlv_insert_after(tlv, tlv_new(EMV_ID_KERNEL_IDENTIFIER,
				       parms->kernel_id_len, parms->kernel_id));
	tlv = tlv_insert_after(tlv, tlv_new(EMV_ID_AMOUNT_AUTHORIZED,
				 sizeof(amount_authorized), amount_authorized));
	tlv = tlv_insert_after(tlv, tlv_new(EMV_ID_AMOUNT_OTHER,
					   sizeof(amount_other), amount_other));
	tlv = tlv_insert_after(tlv, tlv_new(EMV_ID_TRANSACTION_TYPE,
						  sizeof(txn_type), &txn_type));
	tlv = tlv_insert_after(tlv, tlv_new(EMV_ID_UN, sizeof(un), &un));

	tlv = tlv_insert_after(tlv, tlv_new(EMV_ID_START_POINT,
							sizeof(start), &start));
	tlv = tlv_insert_after(tlv, tlv_new(EMV_ID_FCI_TEMPLATE,
						   parms->fci_len, parms->fci));
	tlv = tlv_insert_after(tlv, tlv_new(EMV_ID_SELECT_RESPONSE_SW,
						 sizeof(parms->sw), parms->sw));

done:
	if (rc != TLV_RC_OK) {
		if (tlv_kernel_parms) {
			tlv_free(tlv_kernel_parms);
			tlv_kernel_parms = NULL;
		}
	}

	return tlv_kernel_parms;
}

static int tk_activate(struct emv_kernel *kernel, struct emv_hal *hal,
						 struct emv_kernel_parms *parms,
			      struct emv_outcome_parms *outcome, void *txn_data,
							   size_t *txn_data_len)
{
	struct tk *tk = (struct tk *)kernel;
	struct tlv *tlv_fci = NULL, *tlv = NULL, *tlv_parms = NULL;
	uint8_t pdol[256], gpo_data[256], gpo_resp[256], sw[2];
	size_t pdol_sz = sizeof(pdol), gpo_data_sz = sizeof(gpo_data);
	size_t gpo_resp_sz = sizeof(gpo_resp);
	int rc = EMV_RC_OK;

	rc = tlv_parse(parms->fci, parms->fci_len, &tlv_fci);
	if (rc != TLV_RC_OK) {
		rc = EMV_RC_CARD_PROTOCOL_ERROR;
		goto done;
	}

	tlv = tlv_find(tlv_get_child(tlv_find(tlv_get_child(tlv_find(tlv_fci,
		       EMV_ID_FCI_TEMPLATE)), EMV_ID_FCI_PROPRIETARY_TEMPLATE)),
								   EMV_ID_PDOL);
	if (!tlv) {
		rc = EMV_RC_CARD_PROTOCOL_ERROR;
		goto done;
	}

	rc = tlv_encode_value(tlv, pdol, &pdol_sz);
	if (rc != TLV_RC_OK) {
		rc = EMV_RC_CARD_PROTOCOL_ERROR;
		goto done;
	}

	log4c_category_log(tk->log_cat, LOG4C_PRIORITY_TRACE,
		 "%s(): PDOL='%s'", __func__, libtlv_bin_to_hex(pdol, pdol_sz));

	tlv_parms = tlv_kernel_parms(parms);
	if (!tlv_parms) {
		rc = EMV_RC_SYNTAX_ERROR;
		goto done;
	}

	rc = tlv_process_dol(tlv_parms, pdol, pdol_sz, gpo_data, &gpo_data_sz);
	if (rc != TLV_RC_OK) {
		rc = EMV_RC_CARD_PROTOCOL_ERROR;
		goto done;
	}

	log4c_category_log(tk->log_cat, LOG4C_PRIORITY_TRACE,
					       "%s(): GPO DATA ='%s'", __func__,
				      libtlv_bin_to_hex(gpo_data, gpo_data_sz));

	rc = emv_transceive_apdu(hal, EMV_CMD_GPO_CLA, EMV_CMD_GPO_INS,
			EMV_CMD_P1_NONE, EMV_CMD_P2_NONE, gpo_data, gpo_data_sz,
						    gpo_resp, &gpo_resp_sz, sw);
	if (rc != TLV_RC_OK) {
		rc = EMV_RC_CARD_PROTOCOL_ERROR;
		goto done;
	}

	log4c_category_log(tk->log_cat, LOG4C_PRIORITY_TRACE,
			    "%s(): GPO RESP = '%s' SW: %02hhX%02hhX", __func__,
		       libtlv_bin_to_hex(gpo_resp, gpo_resp_sz), sw[0], sw[1]);

done:
	if (tlv_parms)
		tlv_free(tlv_parms);

	if (tlv_fci)
		tlv_free(tlv_fci);

	if (rc == EMV_RC_OK) {
		log4c_category_log(tk->log_cat, LOG4C_PRIORITY_TRACE,
						     "%s(): success", __func__);
	} else {
		log4c_category_log(tk->log_cat, LOG4C_PRIORITY_NOTICE,
					     "%s(): fail. rc %d", __func__, rc);
	}

	return rc;
}

const struct emv_kernel_ops tk_ops = {
	.configure = tk_configure,
	.activate  = tk_activate
};

struct emv_kernel *tk_new(const char *log4c_category)
{
	struct tk *tk = NULL;
	char cat[64];

	tk = malloc(sizeof(struct tk));
	if (!tk)
		return NULL;

	tk->ops = &tk_ops;

	snprintf(cat, sizeof(cat), "%s.tk", log4c_category);
	tk->log_cat = log4c_category_get(cat);

	return (struct emv_kernel *)tk;
}

void tk_free(struct emv_kernel *tk)
{
	if (tk)
		free(tk);
}
