#include <log4c.h>
#include <emv.h>

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

	log4c_category_log(tk->log_cat,
			       LOG4C_PRIORITY_TRACE, "%s(): success", __func__);
	return EMV_RC_OK;
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
