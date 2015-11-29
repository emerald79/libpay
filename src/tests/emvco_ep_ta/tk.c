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

static int tk_activate(struct emv_kernel *kernel, struct emv_hal *hal,
				 const uint8_t *kernel_id, size_t kernel_id_len,
			   const void *fci, size_t fci_len, const uint8_t sw[2],
			    struct emv_ep_preproc_indicators *prepoc_indicators,
			      struct emv_outcome_parms *outcome, void *txn_data,
							   size_t *txn_data_len)
{
	struct tk *tk = (struct tk *)kernel;
	struct tlv *tlv_fci = NULL, *tlv = NULL;
	uint8_t pdol[256], gpo_data[256];
	size_t pdol_sz = sizeof(pdol), gpo_data_sz = sizeof(gpo_data);
	int rc = EMV_RC_OK;

	rc = tlv_parse(fci, fci_len, &tlv_fci);
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
		    "%s(): PDOL='%s'", __func__, tlv_bin_to_hex(pdol, pdol_sz));

	rc = tlv_process_dol(NULL, pdol, pdol_sz, gpo_data, &gpo_data_sz);
	if (rc != TLV_RC_OK) {
		rc = EMV_RC_CARD_PROTOCOL_ERROR;
		goto done;
	}

done:
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

struct emv_kernel *tk_new(void)
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
