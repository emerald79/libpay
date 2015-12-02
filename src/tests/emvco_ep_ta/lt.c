#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <log4c.h>

#include <emv.h>
#include <tlv.h>

#include "emvco_ep_ta.h"

struct ppse_entry_flags {
	bool app_label:1;
	bool app_prio:1;
	bool kernel_id:1;
	bool ext_select:1;
};

struct ppse_entry {
	struct ppse_entry_flags	present;
	uint8_t			aid[16];
	size_t			aid_len;
	uint8_t			app_label[17];
	size_t			app_label_len;
	uint8_t			app_prio;
	uint8_t			kernel_id[8];
	size_t			kernel_id_len;
	uint8_t			ext_select[16];
	size_t			ext_select_len;
};

struct aid_fci_flags {
	bool aid:1;
	bool app_label:1;
	bool app_prio:1;
	bool pdol:1;
};

struct aid_fci {
	struct aid_fci_flags present;
	uint8_t		     aid[16];
	size_t		     aid_len;
	uint8_t		     app_label[17];
	size_t		     app_label_len;
	uint8_t		     app_prio;
	uint8_t		     pdol[256];
	uint8_t		     pdol_len;
};

struct gpo_resp {
	struct emv_outcome_parms outcome_parms;
};

struct lt_setting {
	struct ppse_entry ppse_entries[8];
	size_t		  ppse_entries_num;
	struct aid_fci	  aid_fci[8];
	size_t		  aid_fci_num;
	struct gpo_resp	  gpo_resp;
};

static const struct lt_setting ltsetting[] = {
	/* LTsetting1.1 */
	{
		.ppse_entries = {
			{
				.present = {
					.app_label = true,
					.app_prio  = true,
					.kernel_id = true,
				},
				AID_A0000000010001,
				APP_LABEL_APP1,
				KERNEL_ID_23,
				.app_prio = 1,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				.app_prio = 1,
				.pdol = {
					0xD1, 0x02, 0x9F, 0x66, 0x04, 0x9F,
					0x02, 0x06, 0x9F, 0x03, 0x06, 0x9C,
					0x01, 0x9F, 0x37, 0x04, 0x9F, 0x2A,
					0x08
				},
				.pdol_len = 19
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			.outcome_parms = {
				.present = {
					.ui_request_on_outcome = true
				},
				.outcome = out_approved,
				.ui_request_on_outcome = {
					.msg_id = msg_approved,
					.status = sts_card_read_successfully
				}
			}
		}
	}
};

static struct tlv *tlv_get_ppse_entry(const struct ppse_entry *ent)
{
	struct tlv *tlv_ppse_entry = NULL, *tlv = NULL;

	tlv_ppse_entry = tlv_new(EMV_ID_DIRECTORY_ENTRY, 0, NULL);

	tlv = tlv_insert_below(tlv_ppse_entry,
			      tlv_new(EMV_ID_ADF_NAME, ent->aid_len, ent->aid));

	if (ent->present.app_label)
		tlv = tlv_insert_after(tlv, tlv_new(EMV_ID_APPLICATION_LABEL,
					   ent->app_label_len, ent->app_label));

	if (ent->present.app_prio)
		tlv = tlv_insert_after(tlv,
				  tlv_new(EMV_ID_APPLICATION_PRIORITY_INDICATOR,
							    1, &ent->app_prio));

	if (ent->present.kernel_id)
		tlv = tlv_insert_after(tlv, tlv_new(EMV_ID_KERNEL_IDENTIFIER,
					   ent->kernel_id_len, ent->kernel_id));

	if (ent->present.ext_select)
		tlv = tlv_insert_after(tlv, tlv_new(EMV_ID_EXTENDED_SELECTION,
					 ent->ext_select_len, ent->ext_select));

	return tlv_ppse_entry;
}

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

struct ui_req_gpo_resp {
	uint8_t	 msg_id;
	uint8_t	 status;
	uint16_t hold_time;
	uint8_t	 lang_pref[2];
	uint8_t	 value_qual;
	uint8_t	 value[6];
	uint8_t	 currency_code[2];
} __attribute__((packed));

static int ber_get_gpo_resp(const struct gpo_resp *resp, void *ber,
								 size_t *ber_sz)
{
	const struct emv_outcome_parms *out_parms = &resp->outcome_parms;
	struct outcome_gpo_resp out_resp;
	struct tlv *gpo_resp = NULL, *tlv = NULL;
	int rc = TLV_RC_OK;

	out_resp.outcome	 = (uint8_t)out_parms->outcome;
	out_resp.start		 = (uint8_t)out_parms->start;
	out_resp.online_resp	 = (uint8_t)out_parms->online_response.type;
	out_resp.cvm		 = (uint8_t)out_parms->cvm;
	out_resp.alt_iface_pref	 = (uint8_t)out_parms->alternate_interface_pref;
	out_resp.receipt	 = (uint8_t)out_parms->present.receipt;
	out_resp.field_off_request = out_parms->present.field_off_request ?
			       htons((uint16_t)out_parms->field_off_hold_time) :
									0xffffu;
	out_resp.removal_timeout = htons((uint16_t)out_parms->removal_timeout);

	gpo_resp = tlv_new(EMV_ID_RESP_MSG_TEMPLATE_FMT_2, 0, NULL);
	if (!gpo_resp)
		return TLV_RC_OUT_OF_MEMORY;

	tlv = tlv_insert_below(gpo_resp, tlv_new(EMV_ID_OUTCOME_DATA,
						  sizeof(out_resp), &out_resp));

	if (out_parms->present.ui_request_on_outcome) {
		struct ui_req_gpo_resp ui_req_resp;
		const struct emv_ui_request *ui_req = NULL;

		ui_req = &out_parms->ui_request_on_outcome;

		ui_req_resp.msg_id     = (uint8_t)ui_req->msg_id;
		ui_req_resp.status     = (uint8_t)ui_req->status;
		ui_req_resp.hold_time  = htons(ui_req->hold_time);
		ui_req_resp.value_qual = (uint8_t)ui_req->value_qualifier;
		memcpy(ui_req_resp.lang_pref, ui_req->lang_pref,
						 sizeof(ui_req_resp.lang_pref));
		memcpy(ui_req_resp.value, ui_req->value,
						     sizeof(ui_req_resp.value));
		memcpy(ui_req_resp.currency_code, ui_req->currency_code,
						 sizeof(ui_req->currency_code));
		tlv = tlv_insert_after(tlv, tlv_new(EMV_ID_UI_REQ_ON_OUTCOME,
					    sizeof(ui_req_resp), &ui_req_resp));
	}

	rc = tlv_encode(gpo_resp, ber, ber_sz);

	tlv_free(gpo_resp);

	return rc;
}

static int ber_get_ppse(const struct ppse_entry *entries, size_t num_entries,
						      void *ber, size_t *ber_sz)
{
	struct tlv *tlv_ppse = NULL, *tlv = NULL;
	size_t i = 0;
	int rc = TLV_RC_OK;

	tlv_ppse = tlv_new(EMV_ID_FCI_TEMPLATE, 0, NULL);

	tlv = tlv_insert_below(tlv_ppse, tlv_new(EMV_ID_DF_NAME,
		       strlen(DF_NAME_2PAY_SYS_DDF01), DF_NAME_2PAY_SYS_DDF01));
	tlv = tlv_insert_after(tlv,
			     tlv_new(EMV_ID_FCI_PROPRIETARY_TEMPLATE, 0, NULL));
	tlv = tlv_insert_below(tlv,
			tlv_new(EMV_ID_FCI_ISSUER_DISCRETIONARY_DATA, 0, NULL));

	tlv = tlv_insert_below(tlv, tlv_get_ppse_entry(&entries[0]));
	for (i = 1; i < num_entries; i++)
		tlv = tlv_insert_after(tlv, tlv_get_ppse_entry(&entries[i]));

	rc = tlv_encode(tlv_ppse, ber, ber_sz);

	tlv_free(tlv_ppse);

	return rc;
}

static int ber_get_aid_fci(const struct aid_fci *aid_fci, void *ber,
							       size_t *ber_size)
{
	struct tlv *tlv_aid_fci = NULL, *tlv = NULL;
	int rc = TLV_RC_OK;

	tlv_aid_fci = tlv_new(EMV_ID_FCI_TEMPLATE, 0, NULL);

	tlv = tlv_insert_below(tlv_aid_fci, tlv_new(EMV_ID_DF_NAME,
					       aid_fci->aid_len, aid_fci->aid));
	tlv = tlv_insert_after(tlv, tlv_new(EMV_ID_FCI_PROPRIETARY_TEMPLATE, 0,
									 NULL));
	tlv = tlv_insert_below(tlv, tlv_new(EMV_ID_APPLICATION_LABEL,
				   aid_fci->app_label_len, aid_fci->app_label));
	tlv = tlv_insert_after(tlv, tlv_new(
		 EMV_ID_APPLICATION_PRIORITY_INDICATOR, 1, &aid_fci->app_prio));
	tlv = tlv_insert_after(tlv, tlv_new(EMV_ID_PDOL, aid_fci->pdol_len,
								aid_fci->pdol));

	rc = tlv_encode(tlv_aid_fci, ber, ber_size);

	tlv_free(tlv_aid_fci);

	return rc;
}

struct lt {
	const struct emv_hal_ops *ops;
	const struct lt_setting *setting;
	log4c_category_t *log_cat;
};

int lt_start_polling(struct emv_hal *hal)
{
	struct lt *lt = (struct lt *)hal;

	log4c_category_log(lt->log_cat, LOG4C_PRIORITY_TRACE,
						     "%s(): success", __func__);
	return EMV_RC_OK;
}

int lt_wait_for_card(struct emv_hal *hal)
{
	struct lt *lt = (struct lt *)hal;

	log4c_category_log(lt->log_cat, LOG4C_PRIORITY_TRACE,
						     "%s(): success", __func__);
	return EMV_RC_OK;
}

static int lt_select_application(struct lt *lt, uint8_t p1, uint8_t p2,
		      size_t lc, const uint8_t *data, size_t *le, uint8_t *resp,
								    uint8_t *sw)
{
	size_t i_aid;
	int rc = EMV_RC_OK;

	if ((lc == strlen(DF_NAME_2PAY_SYS_DDF01)) &&
	    !memcmp(data, DF_NAME_2PAY_SYS_DDF01, lc)) {
		rc = ber_get_ppse(lt->setting->ppse_entries,
				       lt->setting->ppse_entries_num, resp, le);
		if (rc != EMV_RC_OK)
			goto done;

		memcpy(sw, EMV_SW_9000_OK, 2);
		goto done;
	}

	for (i_aid = 0; i_aid < lt->setting->aid_fci_num; i_aid++) {
		if ((lc == lt->setting->aid_fci[i_aid].aid_len) &&
		    !memcmp(data, lt->setting->aid_fci[i_aid].aid, lc)) {
			rc = ber_get_aid_fci(&lt->setting->aid_fci[i_aid], resp,
									    le);
			if (rc != EMV_RC_OK)
				goto done;

			memcpy(sw, EMV_SW_9000_OK, 2);
			goto done;
		}
	}

	*le = 0;
	memcpy(sw, EMV_SW_6A82_FILE_NOT_FOUND, 2);

done:
	return rc;
}

static int lt_get_processing_options(struct lt *lt, uint8_t p1, uint8_t p2,
		      size_t lc, const uint8_t *data, size_t *le, uint8_t *resp,
								    uint8_t *sw)
{
	int rc = EMV_RC_OK;

	log4c_category_log(lt->log_cat, LOG4C_PRIORITY_TRACE,
		  "%s(PDOL data: '%s')", __func__, libtlv_bin_to_hex(data, lc));

	rc = ber_get_gpo_resp(&lt->setting->gpo_resp, resp, le);

	memcpy(sw, EMV_SW_9000_OK, 2);

	return rc;
}

struct lt_ins {
	uint8_t cla;
	uint8_t ins;
	int (*cmd)(struct lt	 *lt,
		   uint8_t	  p1,
		   uint8_t	  p2,
		   size_t	  lc,
		   const uint8_t *data,
		   size_t	 *le,
		   uint8_t	 *resp,
		   uint8_t	 *sw);
};

static struct lt_ins lt_ins[] = {
	{ EMV_CMD_SELECT_CLA, EMV_CMD_SELECT_INS, lt_select_application	      },
	{ EMV_CMD_GPO_CLA,    EMV_CMD_GPO_INS,	  lt_get_processing_options   }
};

int lt_transceive(struct emv_hal *hal, const void *capdu, size_t capdu_sz,
						  void *rapdu, size_t *rapdu_sz)
{
	struct lt *lt = (struct lt *)hal;
	uint8_t *requ = (uint8_t *)capdu;
	uint8_t resp[256];
	uint8_t sw[2];
	size_t resp_sz = sizeof(resp);
	int i = 0;
	int rc = EMV_RC_OK;

	log4c_category_log(lt->log_cat, LOG4C_PRIORITY_TRACE, "%s(capdu: '%s')",
				  __func__, libtlv_bin_to_hex(capdu, capdu_sz));

	memcpy(sw, EMV_SW_9000_OK, 2);

	if (capdu_sz < 4) {
		memcpy(resp, EMV_SW_6700_WRONG_LENGTH, 2);
		resp_sz = 0;
		goto done;
	}

	if (((requ[0] & 0xf0u) != 0x00u) && ((requ[0] & 0xf0) != 0x80u)) {
		memcpy(sw, EMV_SW_6E00_CLA_NOT_SUPPORTED, 2);
		resp_sz = 0;
		goto done;
	}

	for (i = 0; i < ARRAY_SIZE(lt_ins); i++)
		if ((requ[0] == lt_ins[i].cla) && (requ[1] == lt_ins[i].ins)) {
			rc = lt_ins[i].cmd(lt, requ[2], requ[3],
				(size_t)requ[4], &requ[5], &resp_sz, resp, sw);
			break;
		}

	if (rc != EMV_RC_OK)
		goto done;

	if (i == ARRAY_SIZE(lt_ins)) {
		memcpy(sw, EMV_SW_6D00_INS_NOT_SUPPORTED, 2);
		resp_sz = 0;
		goto done;
	}

done:
	memcpy(&resp[resp_sz], sw, 2);
	resp_sz += 2;

	if (*rapdu_sz < resp_sz) {
		rc = EMV_RC_OVERFLOW;
		goto done;
	}

	memcpy(rapdu, resp, resp_sz);
	*rapdu_sz = resp_sz;

	if (rc != EMV_RC_OK) {
		log4c_category_log(lt->log_cat, LOG4C_PRIORITY_ERROR,
					  "%s(): failed. rc: %d", __func__, rc);
	} else {
		log4c_priority_level_t prio = LOG4C_PRIORITY_TRACE;

		if (memcmp(sw, EMV_SW_9000_OK, 2))
			prio = LOG4C_PRIORITY_NOTICE;

		log4c_category_log(lt->log_cat, prio,
				    "%s(): success, sw: %02hhX%02hhX", __func__,
								  sw[0], sw[1]);
	}

	return rc;
}

void lt_ui_request(struct emv_hal *hal, struct emv_ui_request *ui_request)
{
	struct lt *lt = (struct lt *)hal;
	log4c_category_log(lt->log_cat,
			       LOG4C_PRIORITY_TRACE, "%s(): success", __func__);
}

const struct emv_hal_ops lt_ops  = {
	.start_polling = lt_start_polling,
	.wait_for_card = lt_wait_for_card,
	.transceive    = lt_transceive,
	.ui_request    = lt_ui_request
};

struct emv_hal *lt_new(enum ltsetting i_lts, const char *log4c_category)
{
	struct lt *lt = NULL;
	char cat[64];

	if (i_lts >= num_ltsettings)
		return NULL;

	lt = malloc(sizeof(struct lt));
	if (!lt)
		return NULL;

	lt->ops = &lt_ops;

	snprintf(cat, sizeof(cat), "%s.lt", log4c_category);
	lt->log_cat = log4c_category_get(cat);
	lt->setting = &ltsetting[i_lts];

	return (struct emv_hal *)lt;
}

void lt_free(struct emv_hal *lt)
{
	if (lt)
		free(lt);
}
