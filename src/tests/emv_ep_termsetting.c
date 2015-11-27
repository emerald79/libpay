#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <assert.h>

#include <tlv.h>
#include <emv.h>

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(*(x)))

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

struct emv_ep_config_flags {
	bool	status_check_support:1;
	bool	zero_amount_allowed:1;
	bool	reader_ctls_txn_limit:1;
	bool	reader_ctls_floor_limit:1;
	bool	terminal_floor_limit:1;
	bool	reader_cvm_reqd_limit:1;
	bool	ttq:1;
	bool	ext_selection_support:1;
};

struct emv_ep_config {
	struct emv_ep_config_flags present;
	struct emv_ep_config_flags enabled;
	uint64_t		   reader_ctls_txn_limit;
	uint64_t		   reader_ctls_floor_limit;
	uint64_t		   terminal_floor_limit;
	uint64_t		   reader_cvm_reqd_limit;
	uint8_t			   ttq[4];
};

struct emv_ep_aid_kernel {
	uint8_t aid[16];
	size_t  aid_len;
	uint8_t kernel_id[8];
	size_t  kernel_id_len;
};

struct emv_ep_combination {
	enum emv_txn_type txn_types[4];
	struct emv_ep_aid_kernel combinations[5];
	struct emv_ep_config config;
};

struct emv_ep_combination termsetting2[] = {
	{
		.txn_types = {
			txn_purchase,
			txn_purchase_with_cashback,
			txn_cash_advance,
			txn_refund
		},
		.combinations = {
			{ AID_A0000000651010, KERNEL_ID_TK1 },
			{ AID_A0000000030003, KERNEL_ID_21  }
		},
		.config = {
			.present = {
				.zero_amount_allowed = true,
				.reader_ctls_txn_limit = true,
				.reader_ctls_floor_limit = true,
				.ttq = true,
				.ext_selection_support = true,
			},
			.enabled = {
				.zero_amount_allowed = true,
				.ext_selection_support = true,
			},
			.reader_ctls_txn_limit = 120,
			.reader_ctls_floor_limit = 20,
			.ttq = { 0x84, 0x00, 0x80, 0x00 }
		}
	},
	{
		.txn_types = {
			txn_purchase,
			txn_purchase_with_cashback,
			txn_cash_advance,
			txn_refund
		},
		.combinations = {
			{ AID_A0000000041010, KERNEL_ID_TK2 },
			{ AID_A0000000020002, KERNEL_ID_22  },
			{ AID_A0000000041010, KERNEL_ID_2B  },
			{ AID_A0000000020002, KERNEL_ID_2B  }
		},
		.config = {
			.present = {
				.status_check_support = true,
				.reader_cvm_reqd_limit = true
			},
			.reader_cvm_reqd_limit = 10,
		}
	},
	{
		.txn_types = {
			txn_purchase,
			txn_purchase_with_cashback,
			txn_cash_advance,
			txn_refund
		},
		.combinations = {
			{ AID_A0000000031010, KERNEL_ID_TK3 },
			{ AID_A0000000010001, KERNEL_ID_23  },
			{ AID_A0000000031010, KERNEL_ID_2B  },
			{ AID_A0000000010001, KERNEL_ID_2B  }
		},
		.config = {
			.present = {
				.status_check_support = true,
				.zero_amount_allowed = true,
				.reader_ctls_txn_limit = true,
				.terminal_floor_limit = true,
				.ttq = true,
				.ext_selection_support = true
			},
			.enabled = {
				.status_check_support = true,
			},
			.reader_ctls_txn_limit = 200,
			.terminal_floor_limit = 25,
			.ttq = { 0x28, 0x00, 0x00, 0x00 }
		}
	},
	{
		.txn_types = {
			txn_purchase,
			txn_purchase_with_cashback,
			txn_cash_advance,
			txn_refund
		},
		.combinations = {
			{ AID_A0000000251010, KERNEL_ID_TK4 },
			{ AID_A0000000651010, KERNEL_ID_TK5 },
			{ AID_A0000000040004, KERNEL_ID_24  },
			{ AID_A0000000030003, KERNEL_ID_25  }
		},
		.config = {
			.present = {
				.zero_amount_allowed = true,
				.reader_ctls_floor_limit = true,
				.reader_cvm_reqd_limit = true,
				.ttq = true,
				.ext_selection_support = true,
			},
			.enabled = {
				.ext_selection_support = true,
			},
			.reader_ctls_floor_limit = 50,
			.reader_cvm_reqd_limit = 20,
			.ttq = { 0x84, 0xC0, 0x80, 0x00 }
		}
	},
	{
		.txn_types = {
			txn_purchase,
			txn_purchase_with_cashback,
			txn_cash_advance,
			txn_refund
		},
		.combinations = {
			{ AID_A0000000041010, KERNEL_ID_TK4 },
			{ AID_A0000000020002, KERNEL_ID_24  }
		},
		.config = {
			.present = {
				.zero_amount_allowed = true,
				.reader_ctls_floor_limit = true,
				.ttq = true,
				.ext_selection_support = true
			},
			.enabled = {
				.zero_amount_allowed = true,
				.ext_selection_support = true
			},
			.reader_ctls_floor_limit = 20,
			.ttq = { 0x84, 0x00, 0x80, 0x00 }
		}
	},
	{
		.txn_types = {
			txn_purchase,
			txn_purchase_with_cashback,
			txn_cash_advance,
			txn_refund
		},
		.combinations = {
			{ AID_A0000000031010, KERNEL_ID_TK1 },
			{ AID_A0000000010001, KERNEL_ID_21  }
		},
		.config = {
			.present = {
				.status_check_support = true,
				.reader_cvm_reqd_limit = true,
				.ext_selection_support = true
			},
			.reader_cvm_reqd_limit = 10
		}
	},
	{
		.txn_types = {
			txn_purchase,
			txn_purchase_with_cashback,
			txn_cash_advance,
			txn_refund
		},
		.combinations = {
			{ AID_A0000000651010, KERNEL_ID_TK2 },
			{ AID_A0000000251010, KERNEL_ID_TK5 },
			{ AID_A0000000030003, KERNEL_ID_22  },
			{ AID_A0000000040004, KERNEL_ID_25  }
		},
		.config = {
			.present = {
				.status_check_support = true,
				.zero_amount_allowed = true,
				.reader_ctls_txn_limit = true,
				.terminal_floor_limit = true,
				.ttq = true
			},
			.enabled = {
				.status_check_support = true,
				.zero_amount_allowed = true
			},
			.reader_ctls_txn_limit = 200,
			.terminal_floor_limit = 25,
			.ttq = { 0x28, 0x00, 0x00, 0x00 }
		}
	},
	{
		.txn_types = {
			txn_purchase,
			txn_purchase_with_cashback,
			txn_cash_advance,
			txn_refund
		},
		.combinations = {
			{ AID_A0000000251010, KERNEL_ID_TK3 },
			{ AID_A0000000040004, KERNEL_ID_23  }
		},
		.config = {
			.present = {
				.status_check_support = true,
				.zero_amount_allowed = true,
				.reader_ctls_floor_limit = true,
				.reader_cvm_reqd_limit = true,
				.ext_selection_support = true
			},
			.enabled = {
				.status_check_support = true,
				.zero_amount_allowed = true,
				.ext_selection_support = true
			},
			.reader_ctls_floor_limit = 50,
			.reader_cvm_reqd_limit = 20
		}
	},
	{
		.txn_types = {
			txn_purchase,
			txn_purchase_with_cashback,
			txn_cash_advance,
			txn_refund
		},
		.combinations = {
			{ AID_A0000000251010, KERNEL_ID_2B  },
			{ AID_A0000000040004, KERNEL_ID_2B  },
			{ AID_A0000001523010, KERNEL_ID_TK6 },
			{ AID_A0000003330101, KERNEL_ID_TK7 },
			{ AID_A0000003241010, KERNEL_ID_TK6 }
		},
		.config = {
			.present = {
				.status_check_support = true,
				.reader_ctls_txn_limit = true,
				.reader_cvm_reqd_limit = true,
				.ext_selection_support = true
			},
			.reader_ctls_txn_limit = 120,
			.reader_cvm_reqd_limit = 10
		}
	}
};

static struct tlv *get_combinations(struct emv_ep_aid_kernel *aid_kernel)
{
	struct tlv *tlv_combinations = NULL;

	for (; aid_kernel; aid_kernel++) {
		struct tlv *tlv = NULL, *tail = NULL;

		if (!aid_kernel->aid_len)
			break;

		tlv = tlv_new(TLV_ID_LIBEMV_COMBINATION, 0, NULL);
		tail = tlv_insert_below(tlv, tlv_new(TLV_ID_LIBEMV_AID,
					 aid_kernel->aid_len, aid_kernel->aid));
		tail = tlv_insert_after(tail, tlv_new(TLV_ID_LIBEMV_KERNEL_ID,
			     aid_kernel->kernel_id_len, aid_kernel->kernel_id));

		if (!tlv_combinations)
			tlv_combinations = tlv;
		else
			tlv_insert_after(tlv_combinations, tlv);
	}

	return tlv_combinations;
}

static struct tlv *get_txn_types(enum emv_txn_type *txn_types)
{
	size_t len, i;
	uint8_t value[4];

	for (len = 0; len < num_txn_types; len++)
		if (txn_types[len] == num_txn_types)
			break;

	for (i = 0; i < len; i++) {
		switch (txn_types[i]) {
		case txn_purchase:
			value[i] = 0x00;
			break;
		case txn_purchase_with_cashback:
			value[i] = 0x09;
			break;
		case txn_cash_advance:
			value[i] = 0x01;
			break;
		case txn_refund:
			value[i] = 0x20;
			break;
		default:
			assert(false);
		}
	}

	return tlv_new(TLV_ID_LIBEMV_TRANSACTION_TYPES, len, value);
}

static struct tlv *get_combination_set(struct emv_ep_combination *comb)
{
	struct tlv *tlv = NULL, *tail = NULL;

	tlv = tlv_new(TLV_ID_LIBEMV_COMBINATION_SET, 0, NULL);

	tail = tlv_insert_below(tlv, get_txn_types(comb->txn_types));

	tail = tlv_insert_after(tail, get_combinations(comb->combinations));

	while (tlv_get_next(tail))
		tail = tlv_get_next(tail);

	if (comb->config.present.status_check_support) {
		uint8_t enabled = comb->config.enabled.status_check_support;

		tail = tlv_insert_after(tail, tlv_new(
			    TLV_ID_LIBEMV_STATUS_CHECK_SUPPORTED, 1, &enabled));
	}

	if (comb->config.present.zero_amount_allowed) {
		uint8_t enabled = comb->config.enabled.zero_amount_allowed;

		tail = tlv_insert_after(tail, tlv_new(
			       TLV_ID_LIBEMV_ZERO_AMOUNT_ALLOWED, 1, &enabled));
	}

	if (comb->config.present.ext_selection_support) {
		uint8_t enabled = comb->config.enabled.ext_selection_support;

		tail = tlv_insert_after(tail, tlv_new(
			   TLV_ID_LIBEMV_EXT_SELECTION_SUPPORTED, 1, &enabled));
	}

	if (comb->config.present.reader_ctls_txn_limit) {
		uint8_t amount[6];
		int rc;

		rc = emv_u64_to_bcd(comb->config.reader_ctls_txn_limit,
							amount, sizeof(amount));
		if (rc != EMV_RC_OK)
			goto error;

		tail = tlv_insert_after(tail, tlv_new(
					       TLV_ID_LIBEMV_RDR_CTLS_TXN_LIMIT,
						       sizeof(amount), amount));
	}

	if (comb->config.present.reader_ctls_floor_limit) {
		uint8_t amount[6];
		int rc;

		rc = emv_u64_to_bcd(comb->config.reader_ctls_floor_limit,
							amount, sizeof(amount));
		if (rc != EMV_RC_OK)
			goto error;

		tail = tlv_insert_after(tail, tlv_new(
					     TLV_ID_LIBEMV_RDR_CTLS_FLOOR_LIMIT,
						       sizeof(amount), amount));
	}

	if (comb->config.present.terminal_floor_limit) {
		uint8_t amount[6];
		int rc;

		rc = emv_u64_to_bcd(comb->config.terminal_floor_limit,
							amount, sizeof(amount));
		if (rc != EMV_RC_OK)
			goto error;

		tail = tlv_insert_after(tail, tlv_new(
					     TLV_ID_LIBEMV_TERMINAL_FLOOR_LIMIT,
						       sizeof(amount), amount));
	}

	if (comb->config.present.reader_cvm_reqd_limit) {
		uint8_t amount[6];
		int rc;

		rc = emv_u64_to_bcd(comb->config.reader_cvm_reqd_limit,
							amount, sizeof(amount));
		if (rc != EMV_RC_OK)
			goto error;

		tail = tlv_insert_after(tail, tlv_new(
					   TLV_ID_LIBEMV_RDR_CVM_REQUIRED_LIMIT,
						       sizeof(amount), amount));
	}

	return tlv;

error:
	if (tlv)
		tlv_free(tlv);

	return NULL;
}

int get_termsetting2(void *buffer, size_t *size)
{
	struct tlv *tlv = NULL, *tail = NULL;
	int i;

	tlv  = tlv_new(TLV_ID_LIBEMV_CONFIGURATION, 0, NULL);
	tail = tlv_insert_below(tlv, get_combination_set(&termsetting2[0]));

	for (i = 1; i < ARRAY_SIZE(termsetting2); i++)
		tail = tlv_insert_after(tail,
					 get_combination_set(&termsetting2[i]));

	tlv_encode(tlv, buffer, size);
	tlv_free(tlv);

	return TLV_RC_OK;
}

int main(void)
{
	uint8_t buffer[4096];
	size_t size = sizeof(buffer), i;

	get_termsetting2(buffer, &size);

	for (i = 0; i < size; i++)
		printf("%02X", buffer[i]);
	printf("\n");

	return EXIT_SUCCESS;
}
