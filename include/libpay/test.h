/*
 * LibPAY - The Toolkit for Smart Payment Applications
 *
 * Copyright (C) 2015, 2016  Michael Jung <mijung@gmx.net>
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

#ifndef __LIBPAY__TEST_H__
#define __LIBPAY__TEST_H__

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include <libpay/emv.h>

/*-----------------------------------------------------------------------------+
| Entry Point Test Pass Criteria Checker Interface			       |
+-----------------------------------------------------------------------------*/

struct chk;

struct chk_ops {
	void (*ep_start)(struct chk *chk);
	void (*ep_restart)(struct chk *chk);
	void (*ep_txn_end)(struct chk *chk);
	void (*field_on)(struct chk *chk);
	void (*field_off)(struct chk *chk, int hold_time);
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

/*-----------------------------------------------------------------------------+
| Terminal Settings (aka Entry Point Configuration)			       |
+-----------------------------------------------------------------------------*/

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
	struct emv_ep_aid_kernel combinations[63];
	struct emv_ep_config config;
};

struct emv_ep_autorun {
	bool		  enabled;
	enum emv_txn_type txn_type;
	uint64_t	  amount_authorized;
};

struct emv_ep_terminal_data {
	uint8_t	    acquirer_identifier[6];
	uint8_t	    merchant_category_code[2];
	char	    merchant_identifier[15];
	uint8_t	    terminal_country_code[2];
	char	    terminal_identification[8];
	uint8_t	    terminal_type;
	uint8_t	    pos_entry_mode;
	uint8_t	    terminal_capabilities[3];
	uint8_t	    additional_terminal_capabilities[5];
	const char *merchant_name_and_location;
};

struct termset {
	struct emv_ep_terminal_data	*terminal_data;
	struct emv_ep_autorun		 autorun;
	struct emv_ep_combination	*combination_sets;
	size_t				 num_combination_sets;
};


/*-----------------------------------------------------------------------------+
| Interface for wrappers around entry point implementations, which are to be   |
| tested with the LibPAY entry point test suite.			       |
+-----------------------------------------------------------------------------*/

struct emv_ep_wrapper;

struct emv_ep_wrapper_ops {
	int  (*register_kernel)	(struct emv_ep_wrapper *wrapper,
				 struct emv_kernel *kernel,
				 const uint8_t *kernel_id,
				 size_t kernel_id_len,
				 const uint8_t app_ver_num[2]);

	int  (*setup)		(struct emv_ep_wrapper *wrapper,
				 struct emv_hal *lt,
				 struct chk *chk,
				 const struct termset *termsetting);

	int  (*activate)	(struct emv_ep_wrapper *wrapper,
				 struct emv_txn *txn);

	void (*teardown)	(struct emv_ep_wrapper *wrapper);

	void (*free)		(struct emv_ep_wrapper *wrapper);
};

struct emv_ep_wrapper {
	const struct emv_ep_wrapper_ops *ops;
};

extern struct emv_ep_wrapper *new_emv_ep_wrapper(void);

static inline void emv_ep_wrapper_free(struct emv_ep_wrapper *wrapper)
{
	wrapper->ops->free(wrapper);
}

#endif						   /* ndef __LIBPAY__TEST_H__ */
