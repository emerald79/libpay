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

struct emv_chk;

struct emv_chk_ops {
	void (*start)		 (struct emv_chk *chk);
	void (*restart)		 (struct emv_chk *chk);
	void (*txn_end)		 (struct emv_chk *chk);
	void (*field_on)	 (struct emv_chk *chk);
	void (*field_off)	 (struct emv_chk *chk, int hold_time);
	void (*select)		 (struct emv_chk *chk, const uint8_t *data,
								    size_t len);
	void (*gpo_data)	 (struct emv_chk *chk, struct tlv *gpo_data);
	void (*ui_request)	 (struct emv_chk *chk,
				       const struct emv_ui_request *ui_request);
	void (*outcome)		 (struct emv_chk *chk,
				       const struct emv_outcome_parms *outcome);
	bool (*pass_criteria_met)(struct emv_chk *chk);
	void (*free)		 (struct emv_chk *chk);
};

struct emv_chk {
	const struct emv_chk_ops *ops;
};

#define emv_chk_start(CHK)						       \
		(((struct emv_chk *)(CHK))->ops->start((struct emv_chk *)(CHK)))

#define emv_chk_restart(CHK)						       \
	      (((struct emv_chk *)(CHK))->ops->restart((struct emv_chk *)(CHK)))

#define emv_chk_txn_end(CHK)						       \
	      (((struct emv_chk *)(CHK))->ops->txn_end((struct emv_chk *)(CHK)))

#define emv_chk_field_on(CHK)						       \
	     (((struct emv_chk *)(CHK))->ops->field_on((struct emv_chk *)(CHK)))

#define emv_chk_field_off(CHK, HOLDTIME)				       \
	    (((struct emv_chk *)(CHK))->ops->field_off((struct emv_chk *)(CHK),\
								    (HOLDTIME)))

#define emv_chk_select(CHK, DATA, LEN)					       \
	    (((struct emv_chk *)(CHK))->ops->select((struct emv_chk *)(CHK),\
								 (DATA), (LEN)))
#define emv_chk_gpo_data(CHK, DATA)					       \
	    (((struct emv_chk *)(CHK))->ops->gpo_data((struct emv_chk *)(CHK),\
									(DATA)))

#define emv_chk_ui_request(CHK, UI_REQ)					       \
	    (((struct emv_chk *)(CHK))->ops->ui_request(		       \
					     (struct emv_chk *)(CHK), (UI_REQ)))

#define emv_chk_outcome(CHK, OUTCOME)					       \
	      (((struct emv_chk *)(CHK))->ops->outcome((struct emv_chk *)(CHK),\
								     (OUTCOME)))

#define emv_chk_pass_criteria_met(CHK)					       \
			    (((struct emv_chk *)(CHK))->ops->pass_criteria_met(\
						       (struct emv_chk *)(CHK)))

#define emv_chk_free(CHK)						       \
		 (((struct emv_chk *)(CHK))->ops->free((struct emv_chk *)(CHK)))

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

struct emv_ep_terminal_settings {
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

typedef struct emv_ep_wrapper *(*emv_ep_wrapper_new_t)(
						    const char *log4c_category);

typedef int (*emv_ep_wrapper_register_kernel_t)(struct emv_ep_wrapper *wrapper,
						struct emv_kernel *kernel,
						const uint8_t *kernel_id,
						size_t kernel_id_len,
						const uint8_t app_ver_num[2]);

typedef int (*emv_ep_wrapper_setup_t)(struct emv_ep_wrapper *wrapper,
				      struct emv_hal *lt,
				      struct emv_chk *chk,
				      const struct emv_ep_terminal_settings
							    *terminal_settings);

typedef int (*emv_ep_wrapper_activate_t)(struct emv_ep_wrapper *wrapper,
					 const struct emv_txn *txn);

typedef int (*emv_ep_wrapper_free_t)(struct emv_ep_wrapper *wrapper);


struct emv_ep_wrapper_ops {
	emv_ep_wrapper_register_kernel_t register_kernel;
	emv_ep_wrapper_setup_t		 setup;
	emv_ep_wrapper_activate_t	 activate;
	emv_ep_wrapper_free_t		 free;
};

struct emv_ep_wrapper {
	const struct emv_ep_wrapper_ops *ops;
};

#define emv_ep_wrapper_register_kernel(WRAPPER, KERNEL, KERNEL_ID,	       \
						    KERNEL_ID_LEN, APP_VER_NUM)\
	(((struct emv_ep_wrapper *)(WRAPPER))->ops->register_kernel(	       \
				  (struct emv_ep_wrapper *)(WRAPPER), (KERNEL),\
				   (KERNEL_ID), (KERNEL_ID_LEN), (APP_VER_NUM)))

#define emv_ep_wrapper_setup(WRAPPER, LT, CHK, TERMSETTING)		       \
	(((struct emv_ep_wrapper *)(WRAPPER))->ops->setup(		       \
		(struct emv_ep_wrapper *)(WRAPPER), (LT), (CHK), (TERMSETTING)))

#define emv_ep_wrapper_activate(WRAPPER, TXN)				       \
	(((struct emv_ep_wrapper *)(WRAPPER))->ops->activate(		       \
				     (struct emv_ep_wrapper *)(WRAPPER), (TXN)))

#define emv_ep_wrapper_free(WRAPPER)					       \
	(((struct emv_ep_wrapper *)(WRAPPER))->ops->free(		       \
					    (struct emv_ep_wrapper *)(WRAPPER)))

#endif						   /* ndef __LIBPAY__TEST_H__ */
