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

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <log4c.h>
#include <assert.h>

#include "emvco_ep_ta.h"

struct lt;
struct lt_app;

struct lt_app_ops {
	void (*free)  (struct lt_app *app);
	int  (*select)(struct lt_app *app, uint8_t *resp, size_t *resp_sz,
								   uint8_t *sw);
};

struct lt_app {
	const struct lt_app_ops *ops;
};

typedef struct lt_app *(*lt_app_factory_t)(struct lt *lt, const uint8_t *aid,
					      size_t aid_sz, const void *parms);

struct ppse_entry {
	uint8_t			aid[17];
	size_t			aid_len;
	uint8_t			app_label[17];
	size_t			app_label_len;
	uint8_t			app_prio[1];
	size_t			app_prio_len;
	uint8_t			kernel_id[8];
	size_t			kernel_id_len;
	uint8_t			ext_select[16];
	size_t			ext_select_len;
};

struct aid_fci {
	const char	    *bin;
	size_t		     bin_len;
	uint8_t		     sw[2];
	uint8_t		     aid[16];
	size_t		     aid_len;
	uint8_t		     app_label[17];
	size_t		     app_label_len;
	uint8_t		     app_prio[1];
	size_t		     app_prio_len;
	uint8_t		     pdol[256];
	size_t		     pdol_len;
	char		     lang_pref[2];
	uint8_t		     other[64];
	size_t		     other_len;
	lt_app_factory_t     lt_app_factory;
};

struct gpo_resp {
	struct emv_outcome_parms outcome_parms;
};

struct lt_setting {
	const char	 *ppse;
	size_t		  ppse_len;
	uint8_t		  ppse_sw[2];
	struct ppse_entry ppse_entries[8];
	size_t		  ppse_entries_num;
	struct aid_fci	  aid_fci[8];
	size_t		  aid_fci_num;
	struct gpo_resp	  gpo_resp[3];
	size_t		  gpo_resp_num;
};

static struct lt_setting ltsetting[] = {
	/* LTsetting1.1 */
	{
		.ppse_entries = {
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				KERNEL_ID_23,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				PDOL_D1029F66049F02069F03069C019F37049F2A08,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.present = {
						.ui_request_on_outcome = true
					},
					.outcome = out_approved,
					.ui_request_on_outcome = {
						.msg_id = msg_approved,
						.status =
						      sts_card_read_successfully
					}
				}
			}
		},
		.gpo_resp_num = 1
	},
	/* LTsetting1.2 */
	{
		.ppse_entries = {
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				KERNEL_ID_22,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				PDOL_D1029F66049F02069F03069C019F37049F2A08,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.present = {
						.ui_request_on_outcome = true
					},
					.outcome = out_approved,
					.ui_request_on_outcome = {
						.msg_id = msg_approved,
						.status =
						      sts_card_read_successfully
					}
				}
			}
		},
		.gpo_resp_num = 1
	},
	/* LTsetting1.3 */
	{
		.ppse_entries = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				KERNEL_ID_21,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				PDOL_D1029F66049F02069F03069C019F37049F2A08,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.present = {
						.ui_request_on_outcome = true
					},
					.outcome = out_approved,
					.ui_request_on_outcome = {
						.msg_id = msg_approved,
						.status =
						      sts_card_read_successfully
					}
				}
			}
		},
		.gpo_resp_num = 1
	},
	/* LTsetting1.4 */
	{
		.ppse_entries = {
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				KERNEL_ID_24,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				PDOL_D1029F66049F02069F03069C019F37049F2A08,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.present = {
						.ui_request_on_outcome = true
					},
					.outcome = out_approved,
					.ui_request_on_outcome = {
						.msg_id = msg_approved,
						.status =
						      sts_card_read_successfully
					}
				}
			}
		},
		.gpo_resp_num = 1
	},
	/* LTsetting1.5 */
	{
		.ppse_entries = {
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				KERNEL_ID_24,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.present = {
						.ui_request_on_outcome = true
					},
					.outcome = out_approved,
					.receipt = true,
					.cvm = cvm_no_cvm,
					.ui_request_on_outcome = {
						.msg_id = msg_approved,
						.status =
						      sts_card_read_successfully
					}
				}
			}
		},
		.gpo_resp_num = 1
	},
	/* LTsetting1.6 */
	{
		.ppse_entries = {
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				KERNEL_ID_22,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.present = {
						.ui_request_on_outcome = true
					},
					.outcome = out_approved,
					.receipt = true,
					.cvm = cvm_obtain_signature,
					.ui_request_on_outcome = {
						.msg_id = msg_approved,
						.status =
						      sts_card_read_successfully
					}
				}
			}
		},
		.gpo_resp_num = 1
	},
	/* LTsetting1.7 */
	{
		.ppse_entries = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				KERNEL_ID_21,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				PDOL_D102D201,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_online_request,
					.start = start_c,
					.online_response_type = ort_any
				}
			},
			{
				.outcome_parms = {
					.outcome = out_approved,
					.receipt = true,
					.cvm = cvm_obtain_signature
				}
			}
		},
		.gpo_resp_num = 2
	},
	/* LTsetting1.8 */
	{
		.ppse_entries = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				KERNEL_ID_21,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				PDOL_D102D201,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_online_request,
					.start = start_c,
					.online_response_type = ort_any,
					.data_record = {
						.data = "\x91\x10\x01\x02\x03"
							"\x04\x05\x06\x07\x08"
							"\x09\x0A\x0B\x0C\x0D"
							"\x0E\x0F\x10\x71\x10"
							"\x11\x12\x13\x14\x15"
							"\x16\x17\x18\x19\x1A"
							"\x1B\x1C\x1D\x1E\x1F"
							"\x20",
						.len = 36
					},
				}
			},
			{
				.outcome_parms = {
					.outcome = out_approved,
					.receipt = true,
					.cvm = cvm_obtain_signature
				}
			}
		},
		.gpo_resp_num = 2
	},
	/* LTsetting1.9 */
	{
		.ppse_entries = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				KERNEL_ID_21,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				PDOL_D102D201,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.present = {
						.ui_request_on_restart = true
					},
					.outcome = out_online_request,
					.start = start_d,
					.online_response_type = ort_any,
					.ui_request_on_restart = {
						.msg_id = msg_processing,
						.status = sts_processing
					}
				}
			},
			{
				.outcome_parms = {
					.outcome = out_approved,
					.receipt = true,
					.cvm = cvm_obtain_signature
				}
			}
		},
		.gpo_resp_num = 2
	},
	/* LTsetting1.10 */
	{
		.ppse_entries = {
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				KERNEL_ID_23,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				PDOL_D102D2019F6604,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_online_request,
					.online_response_type = ort_any,
					.start = start_a,
				}
			},
			{
				.outcome_parms = {
					.outcome = out_approved,
				}
			},
		},
		.gpo_resp_num = 2
	},
	/* LTsetting1.11 */
	{
		.ppse_entries = {
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				KERNEL_ID_22,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				PDOL_D102D2017110,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.present = {
						.ui_request_on_restart = true
					},
					.outcome = out_online_request,
					.online_response_type = ort_emv_data,
					.start = start_b,
					.removal_timeout = 100,
					.data_record = {
						.data = "\x71\x10\x11\x12\x13"
							"\x14\x15\x16\x17\x18"
							"\x19\x1A\x1B\x1C\x1D"
							"\x1E\x1F\x20",
						.len = 18
					},
					.ui_request_on_restart = {
						.msg_id =
							 msg_present_card_again,
						.status = sts_ready_to_read
					}
				}
			},
			{
				.outcome_parms = {
					.outcome = out_approved,
				}
			},
		},
		.gpo_resp_num = 2
	},
	/* LTsetting1.12 */
	{
		.ppse_entries = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				KERNEL_ID_21,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				PDOL_D102D201,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.present = {
						.ui_request_on_restart = true
					},
					.outcome = out_online_request,
					.start = start_c,
					.online_response_type = ort_any,
					.ui_request_on_restart = {
						.msg_id =
							 msg_present_card_again,
						.status = sts_ready_to_read
					}
				}
			},
			{
				.outcome_parms = {
					.outcome = out_approved,
					.receipt = true,
					.cvm = cvm_obtain_signature
				}
			}
		},
		.gpo_resp_num = 2
	},
	/* LTsetting1.13 */
	{
		.ppse_entries = {
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				KERNEL_ID_24,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				PDOL_D102D20191107110,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.present = {
						.ui_request_on_restart = true
					},
					.outcome = out_online_request,
					.online_response_type = ort_emv_data,
					.start = start_d,
					.removal_timeout = 100,
					.data_record = {
						.data = "\x91\x10\x01\x02\x03"
							"\x04\x05\x06\x07\x08"
							"\x09\x0A\x0B\x0C\x0D"
							"\x0E\x0F\x10\x71\x10"
							"\x11\x12\x13\x14\x15"
							"\x16\x17\x18\x19\x1A"
							"\x1B\x1C\x1D\x1E\x1F"
							"\x20",
						.len = 36
					},
					.ui_request_on_restart = {
						.msg_id = msg_processing,
						.status = sts_processing
					}
				}
			},
			{
				.outcome_parms = {
					.outcome = out_approved,
					.receipt = true,
					.cvm = cvm_obtain_signature
				}
			},
		},
		.gpo_resp_num = 2
	},
	/* LTsetting1.14 */
	{
		.ppse_entries = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				KERNEL_ID_21,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				PDOL_D102D201,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.present = {
						.ui_request_on_restart = true
					},
					.outcome = out_online_request,
					.online_response_type = ort_emv_data,
					.start = start_d,
					.removal_timeout = 100,
					.data_record = {
						.data = "\x91\x10\x01\x02\x03"
							"\x04\x05\x06\x07\x08"
							"\x09\x0A\x0B\x0C\x0D"
							"\x0E\x0F\x10",
						.len = 18
					},
					.ui_request_on_restart = {
						.msg_id = msg_processing,
						.status = sts_processing
					}
				}
			},
			{
				.outcome_parms = {
					.outcome = out_approved,
					.receipt = true,
					.cvm = cvm_no_cvm
				}
			},
		},
		.gpo_resp_num = 2
	},
	/* LTsetting1.15 */
	{
		.ppse_entries = {
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				KERNEL_ID_23,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				PDOL_9F6604,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_online_request,
					.online_response_type = ort_emv_data,
					.start = start_a,
					.removal_timeout = 100,
				}
			}
		},
		.gpo_resp_num = 1
	},
	/* LTsetting1.16 */
	{
		.ppse_entries = {
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				KERNEL_ID_23,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				PDOL_D102D2019F6604,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_online_request,
					.online_response_type = ort_any,
					.start = start_a,
					.data_record = {
						.data = "\x91\x10\x01\x02\x03"
							"\x04\x05\x06\x07\x08"
							"\x09\x0A\x0B\x0C\x0D"
							"\x0E\x0F\x10",
						.len = 18
					}
				}
			},
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		},
		.gpo_resp_num = 2
	},
	/* LTsetting1.17 */
	{
		.ppse_entries = {
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				KERNEL_ID_23,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				PDOL_D102D2019F6604,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.present = {
						.ui_request_on_restart = true
					},
					.outcome = out_online_request,
					.online_response_type = ort_any,
					.start = start_a,
					.data_record = {
						.data = "\x91\x10\x01\x02\x03"
							"\x04\x05\x06\x07\x08"
							"\x09\x0A\x0B\x0C\x0D"
							"\x0E\x0F\x10",
						.len = 18
					},
					.ui_request_on_restart = {
						.msg_id =
							 msg_present_card_again,
						.status = sts_ready_to_read
					}
				}
			},
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		},
		.gpo_resp_num = 2
	},
	/* LTsetting1.18 */
	{
		.ppse_entries = {
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				KERNEL_ID_22,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_online_request,
					.online_response_type = ort_emv_data,
					.start = start_b,
					.removal_timeout = 100,
				}
			}
		},
		.gpo_resp_num = 1
	},
	/* LTsetting1.19 */
	{
		.ppse_entries = {
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				KERNEL_ID_22,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				PDOL_D102D201,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.present = {
						.ui_request_on_restart = true
					},
					.outcome = out_online_request,
					.online_response_type = ort_emv_data,
					.start = start_b,
					.removal_timeout = 100,
					.data_record = {
						.data = "\x91\x10\x01\x02\x03"
							"\x04\x05\x06\x07\x08"
							"\x09\x0A\x0B\x0C\x0D"
							"\x0E\x0F\x10",
						.len = 18
					},
					.ui_request_on_restart = {
						.msg_id =
							 msg_present_card_again,
						.status = sts_ready_to_read
					}
				}
			},
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		},
		.gpo_resp_num = 2
	},
	/* LTsetting1.20 */
	{
		.ppse_entries = {
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				KERNEL_ID_23,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				PDOL_D102D2019F66049110,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_online_request,
					.online_response_type = ort_emv_data,
					.start = start_a,
					.data_record = {
						.data = "\x91\x10\x01\x02\x03"
							"\x04\x05\x06\x07\x08"
							"\x09\x0A\x0B\x0C\x0D"
							"\x0E\x0F\x10",
						.len = 18
					}
				}
			},
			{
				.outcome_parms = {
					.outcome = out_approved,
				}
			},
		},
		.gpo_resp_num = 2
	},
	/* LTsetting1.21 */
	{
		.ppse_entries = {
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				KERNEL_ID_22,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				PDOL_D102D201,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_online_request,
					.online_response_type = ort_any,
					.start = start_b
				}
			},
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			},
		},
		.gpo_resp_num = 2
	},
	/* LTsetting1.22 */
	{
		.ppse_entries = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				KERNEL_ID_21,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				PDOL_D102D2017210,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_online_request,
					.online_response_type = ort_emv_data,
					.start = start_c,
					.data_record = {
						.data = "\x72\x10\x21\x22\x23"
							"\x24\x25\x26\x27\x28"
							"\x29\x2A\x2B\x2C\x2D"
							"\x2E\x2F\x30",
						.len = 18
					}
				}
			},
			{
				.outcome_parms = {
					.outcome = out_approved,
					.receipt = true,
					.cvm = cvm_obtain_signature
				}
			},
		},
		.gpo_resp_num = 2
	},
	/* LTsetting1.23 */
	{
		.ppse_entries = {
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				KERNEL_ID_24,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				PDOL_D102D201,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_online_request,
					.online_response_type = ort_any,
					.start = start_d
				}
			},
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			},
		},
		.gpo_resp_num = 2
	},
	/* LTsetting1.24 */
	{
		.ppse_entries = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				KERNEL_ID_21,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_online_request,
					.online_response_type = ort_emv_data,
					.start = start_d,
					.removal_timeout = 100,
				}
			}
		},
		.gpo_resp_num = 1
	},
	/* LTsetting1.25 */
	{
		.ppse_entries = {
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				KERNEL_ID_22,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				PDOL_D102D201,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.present = {
						.ui_request_on_restart = true
					},
					.outcome = out_online_request,
					.online_response_type = ort_any,
					.start = start_b,
					.data_record = {
						.data = "\x71\x10\x11\x12\x13"
							"\x14\x15\x16\x17\x18"
							"\x19\x1A\x1B\x1C\x1D"
							"\x1E\x1F\x20",
						.len = 18
					},
					.ui_request_on_restart = {
						.msg_id =
							 msg_present_card_again,
						.status = sts_ready_to_read
					}
				}
			},
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		},
		.gpo_resp_num = 1
	},
	/* LTsetting1.26 */
	{
		.ppse_entries = {
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				KERNEL_ID_22,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				PDOL_D102D201,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_online_request,
					.online_response_type = ort_any,
					.start = start_b,
					.data_record = {
						.data = "\x91\x10\x01\x02\x03"
							"\x04\x05\x06\x07\x08"
							"\x09\x0A\x0B\x0C\x0D"
							"\x0E\x0F\x10",
						.len = 18
					},
				}
			},
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			},
		},
		.gpo_resp_num = 2
	},
	/* LTsetting1.27 */
	{
		.ppse_entries = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				KERNEL_ID_21,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				PDOL_D102D20191107210,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_online_request,
					.online_response_type = ort_emv_data,
					.start = start_c,
					.data_record = {
						.data = "\x91\x10\x01\x02\x03"
							"\x04\x05\x06\x07\x08"
							"\x09\x0A\x0B\x0C\x0D"
							"\x0E\x0F\x10\x72\x10"
							"\x21\x22\x23\x24\x25"
							"\x26\x27\x28\x29\x2A"
							"\x2B\x2C\x2D\x2E\x2F"
							"\x30",
						.len = 36
					},
				}
			},
			{
				.outcome_parms = {
					.outcome = out_approved,
					.receipt = true,
					.cvm = cvm_obtain_signature
				}
			},
		},
		.gpo_resp_num = 2
	},
	/* LTsetting1.28 */
	{
		.ppse_entries = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				KERNEL_ID_21,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_online_request,
					.online_response_type = ort_emv_data,
					.start = start_c,
					.removal_timeout = 100,
				}
			}
		},
		.gpo_resp_num = 1
	},
	/* LTsetting1.29 */
	{
		.ppse_entries = {
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				KERNEL_ID_24,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				PDOL_D102D201,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_online_request,
					.online_response_type = ort_any,
					.start = start_d,
					.data_record = {
						.data = "\x91\x10\x01\x02\x03"
							"\x04\x05\x06\x07\x08"
							"\x09\x0A\x0B\x0C\x0D"
							"\x0E\x0F\x10\x72\x10"
							"\x21\x22\x23\x24\x25"
							"\x26\x27\x28\x29\x2A"
							"\x2B\x2C\x2D\x2E\x2F"
							"\x30",
						.len = 36
					},
				}
			},
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		},
		.gpo_resp_num = 2
	},
	/* LTsetting1.30 */
	{
		.ppse_entries = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				API_01,
				KERNEL_ID_21,
				EXT_SEL_010203040506070809
			},
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000030003010203040506070809,
				APP_LABEL_APP3,
				API_01,
				PDOL_9F2A08
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		},
		.gpo_resp_num = 1
	},
	/* LTsetting1.31 */
	{
		.ppse_entries = {
			{
				AID_B0000000010101,
				APP_LABEL_DOMESTIC,
				API_01,
				KERNEL_ID_C11111,
				EXT_SEL_1122
			},
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_B00000000101011122,
				APP_LABEL_DOMESTIC,
				API_01,
				PDOL_9F2A08
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		},
		.gpo_resp_num = 1
	},
	/* LTsetting1.32 */
	{
		.ppse_entries = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				API_01,
				KERNEL_ID_21,
			},
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				API_01,
				PDOL_6FFB,
				.bin =
				  "\x6F" "\x81\xFB"
				    "\x84" "\x07" "\xA0\x00\x00\x00\x03\x00\x03"
				    "\xA5" "\x81\xEF"
				      "\x50" "\x04" "\x41\x50\x50\x33"
				      "\x87" "\x01" "\x01"
				      "\x9F\x38" "\x02" "\x6F\xFB"
				      "\xBF\x0C" "\x81\xDD"
					"\x5F\x50" "\x81\xD9" "\x68\x74\x74\x70"
						  "\x3A\x2F\x2F\x61\x61\x61\x61"
						  "\x61\x2E\x61\x61\x61\x61\x61"
						  "\x2E\x61\x61\x61\x61\x61\x2E"
						  "\x61\x61\x61\x61\x61\x2E\x61"
						  "\x61\x61\x61\x61\x2E\x61\x61"
						  "\x61\x61\x61\x2E\x61\x61\x61"
						  "\x61\x61\x2E\x61\x61\x61\x61"
						  "\x61\x2E\x61\x61\x61\x61\x61"
						  "\x2E\x61\x61\x61\x61\x61\x2E"
						  "\x61\x61\x61\x61\x61\x2E\x61"
						  "\x61\x61\x61\x61\x2E\x61\x61"
						  "\x61\x61\x61\x2E\x61\x61\x61"
						  "\x61\x61\x2E\x61\x61\x61\x61"
						  "\x61\x2E\x61\x61\x61\x61\x61"
						  "\x2E\x61\x61\x61\x61\x61\x2E"
						  "\x61\x61\x61\x61\x61\x2E\x61"
						  "\x61\x61\x61\x61\x2E\x61\x61"
						  "\x61\x61\x61\x2E\x61\x61\x61"
						  "\x61\x61\x2E\x61\x61\x61\x61"
						  "\x61\x2E\x61\x61\x61\x61\x61"
						  "\x2E\x61\x61\x61\x61\x61\x2E"
						  "\x61\x61\x61\x61\x61\x2E\x61"
						  "\x61\x61\x61\x61\x2E\x61\x61"
						  "\x61\x61\x61\x2E\x61\x61\x61"
						  "\x61\x61\x2E\x61\x61\x61\x61"
						  "\x61\x2E\x61\x61\x61\x61\x61"
						  "\x2E\x61\x61\x61\x61\x61\x2E"
						  "\x61\x61\x61\x61\x61\x2E\x61"
						  "\x61\x61\x61\x61\x2E\x61\x61"
						  "\x61\x61\x61\x2E\x61\x61\x61"
								 "\x61\x61\x2E",
				.bin_len = 254
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		},
		.gpo_resp_num = 1
	},
	/* LTsetting1.33 */
	{
		.ppse_entries = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				API_01,
				KERNEL_ID_21,
			},
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				API_01,
				PDOL_D3026F1B
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		},
		.gpo_resp_num = 1
	},
	/* LTsetting1.34 */
	{
		.ppse_entries = {
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				KERNEL_ID_23,
				API_01
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				PDOL_D102D2019F6604,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.present = {
						.ui_request_on_outcome = true,
						.ui_request_on_restart = true,
						.field_off_request = true
					},
					.outcome = out_end_application,
					.start = start_a,
					.field_off_hold_time = 13,
					.removal_timeout = 600,
					.ui_request_on_outcome = {
						.msg_id = msg_remove_card,
						.status = sts_not_ready,
						.hold_time = 13
					},
					.ui_request_on_restart = {
						.msg_id =
							 msg_present_card_again,
						.status = sts_ready_to_read
					}
				}
			},
			{
				.outcome_parms = {
					.outcome = out_approved,
					.removal_timeout = 600
				}
			},
		},
		.gpo_resp_num = 2
	},
	/* LTsetting1.35 */
	{
		.ppse_entries = {
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				KERNEL_ID_22,
				API_01
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				PDOL_D102D201,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.present = {
						.ui_request_on_outcome = true,
						.ui_request_on_restart = true,
					},
					.outcome = out_end_application,
					.start = start_b,
					.ui_request_on_outcome = {
						.msg_id =
							 msg_present_card_again,
						.status = sts_not_ready,
						.hold_time = 13
					},
					.ui_request_on_restart = {
						.msg_id =
							 msg_present_card_again,
						.status = sts_ready_to_read
					}
				}
			},
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			},
		},
		.gpo_resp_num = 2
	},
	/* LTsetting1.40 */
	{
		.ppse_entries = {
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				KERNEL_ID_22,
				API_01
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.present = {
						.ui_request_on_outcome = true,
					},
					.outcome = out_declined,
					.ui_request_on_outcome = {
						.msg_id = msg_not_authorized,
						.status =
						      sts_card_read_successfully
					}
				}
			}
		},
		.gpo_resp_num = 1
	},
	/* LTsetting1.41 */
	{
		.ppse_entries = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				API_01,
				KERNEL_ID_21,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.present = {
						.ui_request_on_outcome = true,
					},
					.outcome = out_try_another_interface,
					.ui_request_on_outcome = {
						.msg_id =
						       msg_insert_or_swipe_card,
						.status = sts_ready_to_read
					}
				}
			}
		},
		.gpo_resp_num = 1
	},
	/* LTsetting1.42 */
	{
		.ppse_entries = {
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				API_01,
				KERNEL_ID_24,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.present = {
						.ui_request_on_outcome = true,
					},
					.outcome = out_online_request,
					.ui_request_on_outcome = {
						.msg_id = msg_authorising,
						.status =
						      sts_card_read_successfully
					}
				}
			}
		},
		.gpo_resp_num = 1
	},
	/* LTsetting1.43 */
	{
		.ppse_entries = {
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				API_01,
				KERNEL_ID_23,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				API_01,
				PDOL_9F37049F6604
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.present = {
						.ui_request_on_outcome = true,
					},
					.outcome = out_try_again,
					.start = start_b,
					.ui_request_on_outcome = {
						.msg_id =
							 msg_present_card_again,
						.status = sts_ready_to_read,
						.hold_time = 100,
					}
				}
			},
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		},
		.gpo_resp_num = 2
	},
	/* LTsetting1.45 */
	{
		.ppse_entries = {
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				API_01,
				KERNEL_ID_22,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				API_01,
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.present = {
						.ui_request_on_outcome = true,
					},
					.outcome = out_approved,
					.ui_request_on_outcome = {
						.msg_id = msg_approved,
						.status =
						     sts_card_read_successfully,
						.value_qualifier = val_balance,
						.value =
						     "\x00\x00\x00\x00\x01\x23",
						.currency_code = ISO4217_GBP
					}
				}
			}
		},
		.gpo_resp_num = 1
	},
	/* LTsetting1.47 */
	{
		.ppse_entries = {
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				API_01,
				KERNEL_ID_23
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				API_01,
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.present = {
						.ui_request_on_outcome = true,
						.ui_request_on_restart = true,
					},
					.outcome = out_online_request,
					.start = start_d,
					.online_response_type = ort_emv_data,
					.data_record = {
						.data = "\x91\x10\x01\x02\x03"
							"\x04\x05\x06\x07\x08"
							"\x09\x0A\x0B\x0C\x0D"
							"\x0E\x0F\x10",
						.len = 18
					},
					.ui_request_on_outcome = {
						.msg_id = msg_authorising,
						.status =
						      sts_card_read_successfully
					},
					.ui_request_on_restart = {
						.msg_id = msg_processing,
						.status =
						      sts_card_read_successfully
					}
				}
			},
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		},
		.gpo_resp_num = 2
	},
	/* LTsetting1.49 */
	{
		.ppse_entries = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				API_01,
				KERNEL_ID_21,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.present = {
						.ui_request_on_outcome = true,
					},
					.outcome = out_end_application,
					.ui_request_on_outcome = {
						.msg_id = msg_try_another_card,
						.status = sts_processing_error
					}
				}
			}
		},
		.gpo_resp_num = 1
	},
	/* LTsetting1.50 */
	{
		.ppse =
		  "\x6F""\x30"
		    "\x84""\x0E""\x32\x50\x41\x59\x2E\x53\x59\x53\x2E\x44\x44"
								  "\x46\x30\x31"
		    "\xA5""\x1E"
		      "\xBF\x0C""\x1B"
			"\x61""\x16"
			  "\x4F""\x07""\xA0\x00\x00\x00\x01\x00\x01"
			  "\x50""\x04""\x41\x50\x50\x31"
			  "\x87""\x01""\x01"
			  "\x9F\x2A""\x01""\x23"
			"\x88""\x01""\x01",
		.ppse_len = 50,
		.aid_fci = {
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				API_01,
				PDOL_9F66049F2A08
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved,
				}
			}
		},
		.gpo_resp_num = 1
	},
	/* LTsetting1.51 */
	{
		.ppse =
		  "\x6F""\x3E"
		    "\x84""\x0E""\x32\x50\x41\x59\x2E\x53\x59\x53\x2E\x44\x44"
								  "\x46\x30\x31"
		    "\xA5""\x2C"
		      "\x88""\x01""\x01"
		      "\x5F\x2D""\x02""\x65\x6E"
		      "\x9F\x11""\x01""\x00"
		      "\xBF\x0C""\x1D"
			"\x61""\x16"
			  "\x4F""\x07""\xA0\x00\x00\x00\x02\x00\x02"
			  "\x50""\x04""\x41\x50\x50\x32"
			  "\x87""\x01""\x01"
			  "\x9F\x2A""\x01""\x22"
			"\x5F\x55""\x02""\x55\x53",
		.ppse_len = 64,
		.aid_fci = {
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				API_01,
				PDOL_9F2A08
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved,
				}
			}
		},
		.gpo_resp_num = 1
	},
	/* LTsetting1.52 */
	{
		.ppse =
		  "\x6F""\x2D"
		    "\x84""\x0E""\x32\x50\x41\x59\x2E\x53\x59\x53\x2E\x44\x44"
								  "\x46\x30\x31"
		    "\xA5""\x1B"
		      "\xBF\x0C""\x18"
			"\x61""\x16"
			  "\x4F""\x07""\xA0\x00\x00\x00\x03\x00\x03"
			  "\x50""\x04""\x41\x50\x50\x33"
			  "\x87""\x01""\x01"
			  "\x9F\x2A""\x01""\x21"
		  "\x73""\x23"
		    "\x5F\x54""\x08""\x41\x42\x43\x44\x45\x46\x47\x48"
		    "\x5F\x53""\x05""\x12\x34\x56\x78\x90"
		    "\x5F\x55""\x02""\x55\x53"
		    "\x5F\x56""\x03""\x55\x53\x41"
		    "\x42""\x03""\x49\x72\x00",
		.ppse_len = 84,
		.aid_fci = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				API_01,
				PDOL_9F2A08
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved,
				}
			}
		},
		.gpo_resp_num = 1
	},
	/* LTsetting1.60 */
	{
		.ppse_entries = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				KERNEL_ID_21,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.present = {
						.ui_request_on_outcome = true
					},
					.outcome = out_approved,
					.ui_request_on_outcome = {
						.msg_id = msg_approved,
						.status =
						     sts_card_read_successfully,
						.hold_time = 100
					}
				}
			}
		},
		.gpo_resp_num = 1
	},
	/* LTsetting1.61 */
	{
		.ppse_entries = {
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				KERNEL_ID_22,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.present = {
						.ui_request_on_outcome = true
					},
					.outcome = out_declined,
					.ui_request_on_outcome = {
						.msg_id = msg_not_authorized,
						.status =
						     sts_card_read_successfully,
						.hold_time = 200
					}
				}
			}
		},
		.gpo_resp_num = 1
	},
	/* LTsetting1.62 */
	{
		.ppse_entries = {
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				KERNEL_ID_23,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				PDOL_9F6604,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.present = {
						.ui_request_on_restart = true
					},
					.outcome = out_online_request,
					.start = start_a,
					.online_response_type = ort_any,
					.ui_request_on_restart = {
						.msg_id =
							 msg_present_card_again,
						.status = sts_ready_to_read,
						.hold_time = 100
					}
				}
			},
			{
				.outcome_parms = {
					.present = {
						.ui_request_on_outcome = true
					},
					.outcome = out_approved,
					.ui_request_on_outcome = {
						.msg_id = msg_approved,
						.status =
						     sts_card_read_successfully,
					}
				}
			}
		},
		.gpo_resp_num = 2
	},
	/* LTsetting1.70 */
	{
		.ppse_entries = {
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				KERNEL_ID_23,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				PDOL_9F6604,
				API_01,
				.lang_pref = "en"
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.present = {
						.ui_request_on_outcome = true
					},
					.outcome = out_approved,
					.ui_request_on_outcome = {
						.msg_id = msg_approved,
						.status =
						     sts_card_read_successfully,
						.lang_pref = "en"
					}
				}
			}
		},
		.gpo_resp_num = 1
	},
	/* LTsetting1.71 */
	{
		.ppse_entries = {
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				KERNEL_ID_24,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				API_01,
				.lang_pref = "en"
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.present = {
						.ui_request_on_outcome = true
					},
					.outcome = out_try_again,
					.start = start_b,
					.ui_request_on_outcome = {
						.msg_id = msg_see_phone,
						.status =
						     sts_processing_error,
						.lang_pref = "en"
					}
				}
			},
			{
				.outcome_parms = {
					.present = {
						.ui_request_on_outcome = true
					},
					.outcome = out_declined,
					.ui_request_on_outcome = {
						.msg_id = msg_not_authorized,
						.status =
						     sts_card_read_successfully,
						.lang_pref = "en"
					}
				}
			}
		},
		.gpo_resp_num = 2
	},
	/* LTsetting1.72 */
	{
		.ppse_entries = {
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				KERNEL_ID_23,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				PDOL_9F6604,
				API_01,
				.lang_pref = "de"
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.present = {
						.ui_request_on_outcome = true
					},
					.outcome = out_approved,
					.ui_request_on_outcome = {
						.msg_id = msg_approved,
						.status =
						     sts_card_read_successfully,
						.lang_pref = "de"
					}
				}
			}
		},
		.gpo_resp_num = 1
	},
	/* LTsetting1.73 */
	{
		.ppse_entries = {
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				KERNEL_ID_24,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				API_01,
				.lang_pref = "de"
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.present = {
						.ui_request_on_outcome = true
					},
					.outcome = out_try_again,
					.start = start_b,
					.ui_request_on_outcome = {
						.msg_id = msg_see_phone,
						.status =
						     sts_processing_error,
						.lang_pref = "de"
					}
				}
			},
			{
				.outcome_parms = {
					.present = {
						.ui_request_on_outcome = true
					},
					.outcome = out_declined,
					.ui_request_on_outcome = {
						.msg_id = msg_not_authorized,
						.status =
						     sts_card_read_successfully,
						.lang_pref = "de"
					}
				}
			}
		},
		.gpo_resp_num = 2
	},
	/* LTsetting1.80 */
	{
		.ppse_entries = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				KERNEL_ID_21,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.present = {
						.ui_request_on_outcome = true
					},
					.outcome = out_approved,
					.ui_request_on_outcome = {
						.msg_id = msg_approved,
						.status =
						     sts_card_read_successfully,
						.value_qualifier = val_amount,
						.value =
						     "\x00\x00\x00\x00\x01\x23",
						.currency_code = ISO4217_USD
					}
				}
			}
		},
		.gpo_resp_num = 1
	},
	/* LTsetting1.81 */
	{
		.ppse_entries = {
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				KERNEL_ID_22,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.present = {
						.ui_request_on_outcome = true
					},
					.outcome = out_approved,
					.ui_request_on_outcome = {
						.msg_id = msg_approved,
						.status =
						     sts_card_read_successfully,
						.value_qualifier = val_balance,
						.value =
						     "\x00\x00\x00\x00\x00\x12",
						.currency_code = ISO4217_EUR
					}
				}
			}
		},
		.gpo_resp_num = 1
	},
	/* LTsetting1.82 */
	{
		.ppse_entries = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				KERNEL_ID_21,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.present = {
						.ui_request_on_outcome = true
					},
					.outcome = out_approved,
					.ui_request_on_outcome = {
						.msg_id = msg_approved,
						.status =
						     sts_card_read_successfully,
						.value_qualifier = val_amount,
						.value =
						     "\x00\x00\x12\x34\x56\x00",
						.currency_code = ISO4217_USD
					}
				}
			}
		},
		.gpo_resp_num = 1
	},
	/* LTsetting1.84 */
	{
		.ppse_entries = {
			{
				AID_A000000002000201,
				APP_LABEL_APP2,
				KERNEL_ID_32,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A000000002000201,
				APP_LABEL_APP2,
				PDOL_9F2A08,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		},
		.gpo_resp_num = 1
	},
	/* LTsetting1.85 */
	{
		.ppse_entries = {
			{
				AID_B0000000010101,
				APP_LABEL_DOMESTIC,
				KERNEL_ID_810978,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_B0000000010101,
				APP_LABEL_DOMESTIC,
				PDOL_9F2A08,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		},
		.gpo_resp_num = 1
	},
	/* LTsetting1.86 */
	{
		.ppse_entries = {
			{
				AID_A000000004000401,
				APP_LABEL_APP4,
				KERNEL_ID_BF0840,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A000000004000401,
				APP_LABEL_APP4,
				PDOL_9F2A08,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		},
		.gpo_resp_num = 1
	},
	/* LTsetting1.87 */
	{
		.ppse_entries = {
			{
				AID_B000000001010102,
				APP_LABEL_DOMESTIC,
				KERNEL_ID_C11111,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_B000000001010102,
				APP_LABEL_DOMESTIC,
				PDOL_9F2A08,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		},
		.gpo_resp_num = 1
	},
	/* LTsetting1.88 */
	{
		.ppse_entries = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				KERNEL_ID_FF2222,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				PDOL_9F2A08,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		},
		.gpo_resp_num = 1
	},
	/* LTsetting1.89 */
	{
		.ppse_entries = {
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				KERNEL_ID_2B,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				PDOL_D102,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.present = {
						.ui_request_on_outcome = true
					},
					.outcome = out_approved,
					.ui_request_on_outcome = {
						.msg_id = msg_approved,
						.status =
						     sts_card_read_successfully
					}
				}
			}
		},
		.gpo_resp_num = 1
	},
	/* LTsetting1.90 */
	{
		.ppse_entries = {
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				KERNEL_ID_23,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				PDOL_9F01069F40059F06079F0902___9F21039C01,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		},
		.gpo_resp_num = 1
	},
	/* LTsetting1.91 */
	{
		.ppse_entries = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				KERNEL_ID_21,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				PDOL_9F01069F40059F06079F0902___9F21039C01,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		},
		.gpo_resp_num = 1
	},
	/* LTsetting1.92 */
	{
		.ppse_entries = {
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				KERNEL_ID_24,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				PDOL_D102,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		},
		.gpo_resp_num = 1
	},
	/* LTsetting1.93 */
	{
		.ppse_entries = {
			{
				AID_B0000000010101,
				APP_LABEL_DOMESTIC,
				KERNEL_ID_00,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_B0000000010101,
				APP_LABEL_DOMESTIC,
				PDOL_9F2A08,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		},
		.gpo_resp_num = 1
	},
	/* LTsetting1.94 */
	{
		.ppse_entries = {
			{
				AID_B0000000010102,
				APP_LABEL_DOMESTIC,
				KERNEL_ID_00,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_B0000000010102,
				APP_LABEL_DOMESTIC,
				PDOL_9F2A08,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		},
		.gpo_resp_num = 1
	},
	/* LTsetting1.95 */
	{
		.ppse_entries = {
			{
				AID_B0000000010103,
				APP_LABEL_DOMESTIC,
				KERNEL_ID_00,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_B0000000010103,
				APP_LABEL_DOMESTIC,
				PDOL_9F2A08,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		},
		.gpo_resp_num = 1
	},
	/* LTsetting1.96 */
	{
		.ppse_entries = {
			{
				AID_B0000000010104,
				APP_LABEL_DOMESTIC,
				KERNEL_ID_00,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_B0000000010104,
				APP_LABEL_DOMESTIC,
				PDOL_9F2A08,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		},
		.gpo_resp_num = 1
	},
	/* LTsetting1.97 */
	{
		.ppse_entries = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				KERNEL_ID_25,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				PDOL_D1029F66049F02069F03069C019F37049F2A08,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.present = {
						.ui_request_on_outcome = true
					},
					.outcome = out_approved,
					.ui_request_on_outcome = {
						.msg_id = msg_approved,
						.status =
						      sts_card_read_successfully
					}
				}
			}
		},
		.gpo_resp_num = 1
	},
	/* LTsetting1.98 */
	{
		.ppse_entries = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				KERNEL_ID_25,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				PDOL_9F01069F40059F06079F0902___9F21039C01,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		},
		.gpo_resp_num = 1
	},
	/* LTsetting1.99 */
	{
		.ppse_entries = {
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				KERNEL_ID_25,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				PDOL_D102D20191107210,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.present = {
						.ui_request_on_restart = true
					},
					.outcome = out_online_request,
					.online_response_type = ort_emv_data,
					.start = start_d,
					.removal_timeout = 100,
					.data_record = {
						.data = "\x91\x10\x01\x02\x03"
							"\x04\x05\x06\x07\x08"
							"\x09\x0A\x0B\x0C\x0D"
							"\x0E\x0F\x10\x72\x10"
							"\x11\x12\x13\x14\x15"
							"\x16\x17\x18\x19\x1A"
							"\x1B\x1C\x1D\x1E\x1F"
							"\x20",
						.len = 36
					},
					.ui_request_on_restart = {
						.msg_id = msg_processing,
						.status = sts_processing
					}
				}
			},
			{
				.outcome_parms = {
					.outcome = out_approved,
					.receipt = true,
					.cvm = cvm_obtain_signature
				}
			},
		},
		.gpo_resp_num = 2
	},
	/* LTsetting1.100 */
	{
		.ppse_entries = {
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				KERNEL_ID_25,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				PDOL_D1029F66049F02069F03069C019F37049F2A08,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.present = {
						.ui_request_on_restart = true
					},
					.outcome = out_approved,
					.ui_request_on_restart = {
						.msg_id = msg_approved,
						.status =
						      sts_card_read_successfully
					}
				}
			},
		},
		.gpo_resp_num = 1
	},
	/* LTsetting1.101 */
	{
		.ppse_entries = {
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				API_01,
				KERNEL_ID_25,
				EXT_SEL_010203040506070809
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000040004010203040506070809,
				APP_LABEL_APP4,
				API_01,
				PDOL_9F2A08,
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			},
		},
		.gpo_resp_num = 1
	},
	/* LTsetting2.1 */
	{
		.ppse_entries = {
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				KERNEL_ID_22,
				API_01,
			},
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				KERNEL_ID_21,
				API_02,
			},
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				KERNEL_ID_22,
				API_03,
			},
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				KERNEL_ID_21,
				API_04,
			}
		},
		.ppse_entries_num = 4,
		.aid_fci = {
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				PDOL_D1029F6604,
				API_01
			},
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				PDOL_D1029F6604,
				API_02
			},
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				PDOL_D1029F6604,
				API_03
			},
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				PDOL_D1029F6604,
				API_04
			}
		},
		.aid_fci_num = 4,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting2.2 */
	{
		.ppse_entries = {
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				KERNEL_ID_21,
				API_04,
			},
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				KERNEL_ID_21,
				API_02,
			},
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				KERNEL_ID_23,
				API_03,
			},
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				KERNEL_ID_21,
				API_01,
			}
		},
		.ppse_entries_num = 4,
		.aid_fci = {
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				PDOL_D1029F6604,
				API_04
			},
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				PDOL_D1029F6604,
				API_02
			},
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				PDOL_D1029F6604,
				API_03
			},
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				PDOL_D1029F6604,
				API_01
			}
		},
		.aid_fci_num = 4,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting2.3 */
	{
		.ppse_entries = {
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				KERNEL_ID_22,
				API_01,
			},
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				KERNEL_ID_21,
				API_02,
			},
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				KERNEL_ID_23,
				API_03,
			},
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				KERNEL_ID_23,
				API_04,
			}
		},
		.ppse_entries_num = 4,
		.aid_fci = {
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				PDOL_D1029F6604,
				API_01
			},
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				PDOL_D1029F6604,
				API_02
			},
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				PDOL_D1029F6604,
				API_03
			},
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				PDOL_D1029F6604,
				API_04
			}
		},
		.aid_fci_num = 4,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting2.4 */
	{
		.ppse_entries = {
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				KERNEL_ID_24,
				API_01,
			},
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				KERNEL_ID_22,
				API_02,
			}
		},
		.ppse_entries_num = 2,
		.aid_fci = {
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				PDOL_D1029F6604,
				API_01
			},
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				PDOL_D1029F6604,
				API_02
			},
		},
		.aid_fci_num = 2,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting2.5 */
	{
		.ppse_entries = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				KERNEL_ID_22,
				API_01,
			},
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				KERNEL_ID_24,
				API_02,
			}
		},
		.ppse_entries_num = 2,
		.aid_fci = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				PDOL_D1029F6604,
				API_01
			},
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				PDOL_D1029F6604,
				API_02
			},
		},
		.aid_fci_num = 2,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting2.6 */
	{
		.ppse_entries = {
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				KERNEL_ID_23,
				API_01,
			},
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				KERNEL_ID_22,
				API_02,
			}
		},
		.ppse_entries_num = 2,
		.aid_fci = {
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				PDOL_D1029F6604,
				API_01
			},
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				PDOL_D1029F6604,
				API_02
			},
		},
		.aid_fci_num = 2,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting2.7 */
	{
		.ppse_entries = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				KERNEL_ID_21,
				API_01,
			},
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				KERNEL_ID_24,
				API_02,
			},
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				KERNEL_ID_23,
				API_03,
			},
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				KERNEL_ID_24,
				API_04,
			}
		},
		.ppse_entries_num = 4,
		.aid_fci = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				PDOL_D1029F66049F2A086F23,
				API_01
			},
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				PDOL_D1029F66049F2A086F23,
				API_02
			},
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				PDOL_D1029F66049F2A086F23,
				API_03
			},
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				PDOL_D1029F66049F2A086F23,
				API_04
			}
		},
		.aid_fci_num = 4,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_online_request,
					.start = start_b,
					.online_response_type = ort_emv_data,
						.data_record = {
						.data = "\x71\x10\x11\x12\x13"
							"\x14\x15\x16\x17\x18"
							"\x19\x1A\x1B\x1C\x1D"
							"\x1E\x1F\x20",
						.len = 18
					}
				}
			},
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting2.8 */
	{
		.ppse_entries = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				KERNEL_ID_21,
				API_01,
			},
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				KERNEL_ID_24,
				API_02,
			},
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				KERNEL_ID_23,
				API_03,
			},
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				KERNEL_ID_24,
				API_04,
			}
		},
		.ppse_entries_num = 4,
		.aid_fci = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				PDOL_D1029F66049F2A086F23,
				API_01
			},
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				PDOL_D1029F66049F2A086F23,
				API_02
			},
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				PDOL_D1029F66049F2A086F23,
				API_03
			},
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				PDOL_D1029F66049F2A086F23,
				API_04
			}
		},
		.aid_fci_num = 4,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_online_request,
					.start = start_b,
					.online_response_type = ort_emv_data,
					.data_record = {
						.data = "\x91\x10\x01\x02\x03"
							"\x04\x05\x06\x07\x08"
							"\x09\x0A\x0B\x0C\x0D"
							"\x0E\x0F\x10\x71\x10"
							"\x11\x12\x13\x14\x15"
							"\x16\x17\x18\x19\x1A"
							"\x1B\x1C\x1D\x1E\x1F"
							"\x20",
						.len = 36
					}
				}
			},
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting2.9 */
	{
		.ppse_entries = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				KERNEL_ID_21,
				API_01,
			},
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				KERNEL_ID_24,
				API_02,
			},
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				KERNEL_ID_23,
				API_03,
			},
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				KERNEL_ID_24,
				API_04,
			}
		},
		.ppse_entries_num = 4,
		.aid_fci = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				PDOL_D1029F66049F2A086F23,
				API_01
			},
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				PDOL_D1029F66049F2A086F23,
				API_02
			},
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				PDOL_D1029F66049F2A086F23,
				API_03
			},
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				PDOL_D1029F66049F2A086F23,
				API_04
			}
		},
		.aid_fci_num = 4,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_online_request,
					.start = start_b,
					.online_response_type = ort_emv_data,
					.data_record = {
						.data = "\x91\x10\x01\x02\x03"
							"\x04\x05\x06\x07\x08"
							"\x09\x0A\x0B\x0C\x0D"
							"\x0E\x0F\x10\x72\x10"
							"\x21\x22\x23\x24\x25"
							"\x26\x27\x28\x29\x2A"
							"\x2B\x2C\x2D\x2E\x2F"
							"\x30",
						.len = 36
					}
				}
			},
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting2.10 */
	{
		.ppse_entries = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				KERNEL_ID_21,
				API_01,
			},
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				KERNEL_ID_24,
				API_02,
			},
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				KERNEL_ID_23,
				API_03,
			},
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				KERNEL_ID_24,
				API_04,
			}
		},
		.ppse_entries_num = 4,
		.aid_fci = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				PDOL_D1029F66049F2A086F23,
				API_01
			},
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				PDOL_D1029F66049F2A086F23,
				API_02
			},
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				PDOL_D1029F66049F2A086F23,
				API_03
			},
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				PDOL_D1029F66049F2A086F23,
				API_04
			}
		},
		.aid_fci_num = 4,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_online_request,
					.start = start_b,
					.online_response_type = ort_emv_data,
					.data_record = {
						.data = "\x91\x10\x01\x02\x03"
							"\x04\x05\x06\x07\x08"
							"\x09\x0A\x0B\x0C\x0D"
							"\x0E\x0F\x10",
						.len = 18
					}
				}
			},
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting2.11 */
	{
		.ppse_entries = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				KERNEL_ID_21,
				API_04,
				EXT_SEL_01
			},
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				KERNEL_ID_24,
				API_03,
				EXT_SEL_02
			},
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				KERNEL_ID_23,
				API_02,
				EXT_SEL_03
			},
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				KERNEL_ID_24,
				API_01,
				EXT_SEL_04
			}
		},
		.ppse_entries_num = 4,
		.aid_fci = {
			{
				AID_A000000003000301,
				APP_LABEL_APP3,
				PDOL_D1029F66049F2A086F24,
				API_04
			},
			{
				AID_A000000002000202,
				APP_LABEL_APP2,
				PDOL_D1029F66049F2A086F24,
				API_03
			},
			{
				AID_A000000001000103,
				APP_LABEL_APP1,
				PDOL_D1029F66049F2A086F24,
				API_02
			},
			{
				AID_A000000004000404,
				APP_LABEL_APP4,
				PDOL_D1029F66049F2A086F24,
				API_01
			}
		},
		.aid_fci_num = 4,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_online_request,
					.start = start_b,
					.online_response_type = ort_emv_data,
					.data_record = {
						.data = "\x91\x10\x01\x02\x03"
							"\x04\x05\x06\x07\x08"
							"\x09\x0A\x0B\x0C\x0D"
							"\x0E\x0F\x10",
						.len = 18
					}
				}
			},
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting2.12 */
	{
		.ppse_entries = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				KERNEL_ID_21,
				API_04,
				EXT_SEL_01
			},
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				KERNEL_ID_24,
				API_03,
				EXT_SEL_02
			},
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				KERNEL_ID_23,
				API_02,
				EXT_SEL_03
			},
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				KERNEL_ID_24,
				API_01,
				EXT_SEL_04
			}
		},
		.ppse_entries_num = 4,
		.aid_fci = {
			{
				AID_A000000003000301,
				APP_LABEL_APP3,
				PDOL_D1029F66049F2A086F24,
				API_04
			},
			{
				AID_A000000002000202,
				APP_LABEL_APP2,
				PDOL_D1029F66049F2A086F24,
				API_03
			},
			{
				AID_A000000001000103,
				APP_LABEL_APP1,
				PDOL_D1029F66049F2A086F24,
				API_02
			},
			{
				AID_A000000004000404,
				APP_LABEL_APP4,
				PDOL_D1029F66049F2A086F24,
				API_01
			}
		},
		.aid_fci_num = 4,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_online_request,
					.start = start_b,
					.online_response_type = ort_emv_data,
					.data_record = {
						.data = "\x71\x10\x11\x12\x13"
							"\x14\x15\x16\x17\x18"
							"\x19\x1A\x1B\x1C\x1D"
							"\x1E\x1F\x20",
						.len = 18
					}
				}
			},
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting2.13 */
	{
		.ppse_entries = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				KERNEL_ID_21,
				API_04,
				EXT_SEL_01
			},
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				KERNEL_ID_24,
				API_03,
				EXT_SEL_02
			},
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				KERNEL_ID_23,
				API_02,
				EXT_SEL_03
			},
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				KERNEL_ID_24,
				API_01,
				EXT_SEL_04
			}
		},
		.ppse_entries_num = 4,
		.aid_fci = {
			{
				AID_A000000003000301,
				APP_LABEL_APP3,
				PDOL_D1029F66049F2A086F24,
				API_04
			},
			{
				AID_A000000002000202,
				APP_LABEL_APP2,
				PDOL_D1029F66049F2A086F24,
				API_03
			},
			{
				AID_A000000001000103,
				APP_LABEL_APP1,
				PDOL_D1029F66049F2A086F24,
				API_02
			},
			{
				AID_A000000004000404,
				APP_LABEL_APP4,
				PDOL_D1029F66049F2A086F24,
				API_01
			}
		},
		.aid_fci_num = 4,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_online_request,
					.start = start_b,
					.online_response_type = ort_emv_data,
					.data_record = {
						.data = "\x72\x10\x21\x22\x23"
							"\x24\x25\x26\x27\x28"
							"\x29\x2A\x2B\x2C\x2D"
							"\x2E\x2F\x30",
						.len = 18
					}
				}
			},
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting2.14 */
	{
		.ppse_entries = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				KERNEL_ID_21,
				API_04,
				EXT_SEL_01
			},
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				KERNEL_ID_24,
				API_03,
				EXT_SEL_02
			},
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				KERNEL_ID_23,
				API_02,
				EXT_SEL_03
			},
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				KERNEL_ID_24,
				API_01,
				EXT_SEL_04
			}
		},
		.ppse_entries_num = 4,
		.aid_fci = {
			{
				AID_A000000003000301,
				APP_LABEL_APP3,
				PDOL_D1029F66049F2A086F24,
				API_04
			},
			{
				AID_A000000002000202,
				APP_LABEL_APP2,
				PDOL_D1029F66049F2A086F24,
				API_03
			},
			{
				AID_A000000001000103,
				APP_LABEL_APP1,
				PDOL_D1029F66049F2A086F24,
				API_02
			},
			{
				AID_A000000004000404,
				APP_LABEL_APP4,
				PDOL_D1029F66049F2A086F24,
				API_01
			}
		},
		.aid_fci_num = 4,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_online_request,
					.start = start_b,
					.online_response_type = ort_emv_data,
					.data_record = {
						.data = "\x91\x10\x01\x02\x03"
							"\x04\x05\x06\x07\x08"
							"\x09\x0A\x0B\x0C\x0D"
							"\x0E\x0F\x10\x71\x10"
							"\x11\x12\x13\x14\x15"
							"\x16\x17\x18\x19\x1A"
							"\x1B\x1C\x1D\x1E\x1F"
							"\x20",
						.len = 36
					}
				}
			},
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting2.15 */
	{
		.ppse_entries = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				KERNEL_ID_21,
				API_04,
				EXT_SEL_01
			},
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				KERNEL_ID_24,
				API_03,
				EXT_SEL_02
			},
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				KERNEL_ID_23,
				API_02,
				EXT_SEL_03
			},
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				KERNEL_ID_24,
				API_01,
				EXT_SEL_04
			}
		},
		.ppse_entries_num = 4,
		.aid_fci = {
			{
				AID_A000000003000301,
				APP_LABEL_APP3,
				PDOL_D1029F66049F2A086F24,
				API_04
			},
			{
				AID_A000000002000202,
				APP_LABEL_APP2,
				PDOL_D1029F66049F2A086F24,
				API_03
			},
			{
				AID_A000000001000103,
				APP_LABEL_APP1,
				PDOL_D1029F66049F2A086F24,
				API_02
			},
			{
				AID_A000000004000404,
				APP_LABEL_APP4,
				PDOL_D1029F66049F2A086F24,
				API_01
			}
		},
		.aid_fci_num = 4,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_online_request,
					.start = start_b,
					.online_response_type = ort_emv_data,
					.data_record = {
						.data = "\x91\x10\x01\x02\x03"
							"\x04\x05\x06\x07\x08"
							"\x09\x0A\x0B\x0C\x0D"
							"\x0E\x0F\x10\x72\x10"
							"\x21\x22\x23\x24\x25"
							"\x26\x27\x28\x29\x2A"
							"\x2B\x2C\x2D\x2E\x2F"
							"\x30",
						.len = 36
					}
				}
			},
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting2.16 */
	{
		.ppse_entries = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				KERNEL_ID_21,
				API_04,
				EXT_SEL_01
			},
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				KERNEL_ID_24,
				API_03,
				EXT_SEL_02
			},
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				KERNEL_ID_23,
				API_02,
				EXT_SEL_03
			},
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				KERNEL_ID_25,
				API_01,
				EXT_SEL_04
			}
		},
		.ppse_entries_num = 4,
		.aid_fci = {
			{
				AID_A000000003000301,
				APP_LABEL_APP3,
				PDOL_D1029F66049F2A086F24,
				API_04
			},
			{
				AID_A000000002000202,
				APP_LABEL_APP2,
				PDOL_D1029F66049F2A086F24,
				API_03
			},
			{
				AID_A000000001000103,
				APP_LABEL_APP1,
				PDOL_D1029F66049F2A086F24,
				API_02
			},
			{
				AID_A000000004000404,
				APP_LABEL_APP4,
				PDOL_D1029F66049F2A086F24,
				API_01
			}
		},
		.aid_fci_num = 4,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_online_request,
					.start = start_b,
					.online_response_type = ort_emv_data,
					.data_record = {
						.data = "\x91\x10\x01\x02\x03"
							"\x04\x05\x06\x07\x08"
							"\x09\x0A\x0B\x0C\x0D"
							"\x0E\x0F\x10\x71\x10"
							"\x11\x12\x13\x14\x15"
							"\x16\x17\x18\x19\x1A"
							"\x1B\x1C\x1D\x1E\x1F"
							"\x20\x72\x10\x21\x22"
							"\x23\x24\x25\x26\x27"
							"\x28\x29\x2A\x2B\x2C"
							"\x2D\x2E\x2F\x30",
						.len = 54
					}
				}
			},
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting2.17 */
	{
		.ppse_entries = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				KERNEL_ID_21,
				API_02,
			},
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				KERNEL_ID_24,
				API_03,
			},
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				KERNEL_ID_23,
				API_01,
			},
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				KERNEL_ID_24,
				API_04,
			}
		},
		.ppse_entries_num = 4,
		.aid_fci = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				PDOL_D1029F66049F2A086F23,
				API_02
			},
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				PDOL_D1029F66049F2A086F23,
				API_03
			},
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				PDOL_D1029F66049F2A086F23,
				API_01
			},
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				PDOL_D1029F66049F2A086F23,
				API_04
			}
		},
		.aid_fci_num = 4,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_online_request,
					.start = start_b,
					.online_response_type = ort_emv_data,
					.data_record = {
						.data = "\x72\x10\x21\x22\x23"
							"\x24\x25\x26\x27\x28"
							"\x29\x2A\x2B\x2C\x2D"
							"\x2E\x2F\x30",
						.len = 18
					}
				}
			},
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting2.18 */
	{
		.ppse_entries = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				KERNEL_ID_21,
				API_02,
			},
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				KERNEL_ID_24,
				API_03,
			},
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				KERNEL_ID_23,
				API_01,
			},
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				KERNEL_ID_24,
				API_04,
			}
		},
		.ppse_entries_num = 4,
		.aid_fci = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				PDOL_D1029F66049F2A086F23,
				API_02
			},
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				PDOL_D1029F66049F2A086F23,
				API_03
			},
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				PDOL_D1029F66049F2A086F23,
				API_01
			},
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				PDOL_D1029F66049F2A086F23,
				API_04
			}
		},
		.aid_fci_num = 4,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_online_request,
					.start = start_b,
					.online_response_type = ort_emv_data,
					.data_record = {
						.data = "\x91\x10\x01\x02\x03"
							"\x04\x05\x06\x07\x08"
							"\x09\x0A\x0B\x0C\x0D"
							"\x0E\x0F\x10\x71\x10"
							"\x11\x12\x13\x14\x15"
							"\x16\x17\x18\x19\x1A"
							"\x1B\x1C\x1D\x1E\x1F"
							"\x20\x72\x10\x21\x22"
							"\x23\x24\x25\x26\x27"
							"\x28\x29\x2A\x2B\x2C"
							"\x2D\x2E\x2F\x30",
						.len = 54
					}
				}
			},
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting2.19 */
	{
		.ppse_entries = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				KERNEL_ID_21,
				API_03,
			},
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				KERNEL_ID_24,
				API_01,
			},
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				KERNEL_ID_23,
				API_03,
			},
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				KERNEL_ID_24,
				API_02,
			}
		},
		.ppse_entries_num = 4,
		.aid_fci = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				PDOL_D1029F66049F2A086F23,
				API_03
			},
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				PDOL_D1029F66049F2A086F23,
				API_01
			},
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				PDOL_D1029F66049F2A086F23,
				API_03
			},
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				PDOL_D1029F66049F2A086F23,
				API_02
			}
		},
		.aid_fci_num = 4,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_online_request,
					.start = start_c,
					.online_response_type = ort_any
				}
			},
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting2.20 */
	{
		.ppse_entries = {
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				API_03,
				KERNEL_ID_23,
			},
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				API_01,
				KERNEL_ID_21,
			},
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				API_02,
				KERNEL_ID_2B,
			},
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				API_04,
				KERNEL_ID_24,
			}
		},
		.ppse_entries_num = 4,
		.aid_fci = {
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				API_03,
				PDOL_9F66049F2A08,
			},
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				API_01,
				PDOL_9F66049F2A08,
			},
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				API_02,
				PDOL_9F66049F2A08,
			},
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				API_04,
				PDOL_9F66049F2A08,
			}
		},
		.aid_fci_num = 4,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting2.21 */
	{
		.ppse_entries = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				API_07,
				KERNEL_ID_21,
			},
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				API_00,
				KERNEL_ID_24,
			},
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				API_04,
				KERNEL_ID_23,
			},
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				API_05,
				KERNEL_ID_23,
			}
		},
		.ppse_entries_num = 4,
		.aid_fci = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				API_07,
				PDOL_9F66049F2A08
			},
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				API_00,
				PDOL_9F66049F2A08
			},
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				API_04,
				PDOL_9F66049F2A08
			},
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				API_05,
				PDOL_9F66049F2A08
			}
		},
		.aid_fci_num = 4,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting2.22 */
	{
		.ppse_entries = {
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				API_03,
				KERNEL_ID_23,
			},
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				API_01,
				KERNEL_ID_24,
			},
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				API_01,
				KERNEL_ID_24,
			},
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				API_01,
				KERNEL_ID_22,
			}
		},
		.ppse_entries_num = 4,
		.aid_fci = {
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				API_03,
				PDOL_9F66049F2A08
			},
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				API_01,
				PDOL_9F66049F2A08
			},
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				API_01,
				PDOL_9F66049F2A08
			},
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				API_01,
				PDOL_9F66049F2A08
			}
		},
		.aid_fci_num = 4,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting2.23 */
	{
		.ppse_entries = {
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				KERNEL_ID_23,
			},
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				API_00,
				KERNEL_ID_21,
			},
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				API_00,
				KERNEL_ID_24,
			},
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				KERNEL_ID_24,
			}
		},
		.ppse_entries_num = 4,
		.aid_fci = {
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				PDOL_9F66049F2A08
			},
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				API_00,
				PDOL_9F66049F2A08
			},
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				API_00,
				PDOL_9F66049F2A08
			},
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				PDOL_9F66049F2A08
			}
		},
		.aid_fci_num = 4,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting2.24 */
	{
		.ppse_entries = {
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				API_0C,
				KERNEL_ID_23
			},
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				API_0B,
				KERNEL_ID_BF0840
			},
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				API_0A,
				KERNEL_ID_810978
			},
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				API_0E,
				KERNEL_ID_24
			}
		},
		.ppse_entries_num = 4,
		.aid_fci = {
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				API_0C,
				PDOL_9F66049F2A08
			},
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				API_0B,
				PDOL_9F66049F2A08
			},
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				API_0A,
				PDOL_9F66049F2A08
			},
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				API_0E,
				PDOL_9F66049F2A08
			}
		},
		.aid_fci_num = 4,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting2.25 */
	{
		.ppse_entries = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				KERNEL_ID_21,
				API_04,
				EXT_SEL_01
			},
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				KERNEL_ID_24,
				API_03,
				EXT_SEL_02
			},
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				KERNEL_ID_23,
				API_02,
				EXT_SEL_03
			},
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				KERNEL_ID_24,
				API_01,
				EXT_SEL_04
			}
		},
		.ppse_entries_num = 4,
		.aid_fci = {
			{
				AID_A000000003000301,
				APP_LABEL_APP3,
				PDOL_D1029F66049F2A086F24,
				API_04
			},
			{
				AID_A000000002000202,
				APP_LABEL_APP2,
				PDOL_D1029F66049F2A086F24,
				API_03
			},
			{
				AID_A000000001000103,
				APP_LABEL_APP1,
				PDOL_D1029F66049F2A086F24,
				API_02
			},
			{
				AID_A000000004000404,
				APP_LABEL_APP4,
				PDOL_D1029F66049F2A086F24,
				API_01
			}
		},
		.aid_fci_num = 4,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_online_request,
					.start = start_c,
					.online_response_type = ort_any
				}
			},
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting2.30 */
	{
		.ppse_entries = {
			{
				AID_A0000000031010,
				APP_LABEL_VISA,
				KERNEL_ID_3,
				API_01,
			},
			{
				AID_A0000000251010,
				APP_LABEL_AMEX,
				KERNEL_ID_TK4,
				API_02,
			}
		},
		.ppse_entries_num = 2,
		.aid_fci = {
			{
				AID_A0000000031010,
				APP_LABEL_VISA,
				API_01,
				PDOL_9F2A08
			},
			{
				AID_A0000000251010,
				APP_LABEL_AMEX,
				API_02,
				PDOL_9F2A08
			}
		},
		.aid_fci_num = 2,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting2.31 */
	{
		.ppse_entries = {
			{
				AID_A000000003101001,
				APP_LABEL_VISA,
				KERNEL_ID_3,
				API_01,
			},
			{
				AID_A0000000251010,
				APP_LABEL_AMEX,
				KERNEL_ID_TK4,
				API_02,
			}
		},
		.ppse_entries_num = 2,
		.aid_fci = {
			{
				AID_A000000003101001,
				APP_LABEL_VISA,
				API_01
			},
			{
				AID_A0000000251010,
				APP_LABEL_AMEX,
				API_02,
			}
		},
		.aid_fci_num = 2,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting2.40 */
	{
		.ppse_entries = {
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				KERNEL_ID_23,
				API_01,
			},
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				KERNEL_ID_21,
				API_02,
			},
			{
				AID_A0000000020002,
				APP_LABEL_APP3,
				KERNEL_ID_22,
				API_03,
			}
		},
		.ppse_entries_num = 3,
		.aid_fci = {
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				PDOL_9F66049F2A089F3704,
				API_01
			},
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				PDOL_9F66049F2A089F3704,
				API_02
			},
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				PDOL_9F66049F2A089F3704,
				API_03
			}
		},
		.aid_fci_num = 3,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_select_next,
					.start	 = start_c
				}
			},
			{
				.outcome_parms = {
					.outcome = out_select_next,
					.start	 = start_c
				}
			},
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting2.41 */
	{
		.ppse_entries = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				API_01,
				KERNEL_ID_21,
			},
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				API_02,
				KERNEL_ID_22,
			}
		},
		.ppse_entries_num = 2,
		.aid_fci = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				API_01,
				PDOL_9F2A08
			},
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				API_02,
				PDOL_9F2A08
			}
		},
		.aid_fci_num = 3,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_select_next,
					.start	 = start_c
				}
			},
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting2.42 */
	{
		.ppse_entries = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				KERNEL_ID_25,
				API_01,
			},
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				KERNEL_ID_22,
				API_02,
			}
		},
		.ppse_entries_num = 2,
		.aid_fci = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				PDOL_D1029F6604,
				API_01
			},
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				PDOL_D1029F6604,
				API_02
			}
		},
		.aid_fci_num = 2,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting2.43 */
	{
		.ppse_entries = {
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				KERNEL_ID_25,
				API_01,
			},
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				KERNEL_ID_22,
				API_02,
			}
		},
		.ppse_entries_num = 2,
		.aid_fci = {
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				PDOL_D1029F6604,
				API_01
			},
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				PDOL_D1029F6604,
				API_02
			}
		},
		.aid_fci_num = 2,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting2.44 */
	{
		.ppse_entries = {
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				API_03,
				KERNEL_ID_23,
			},
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				API_04,
				KERNEL_ID_25,
			},
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				API_05,
				KERNEL_ID_2B,
			},
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				API_06,
				KERNEL_ID_24,
			}
		},
		.ppse_entries_num = 4,
		.aid_fci = {
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				API_03,
				PDOL_9F66049F2A08,
			},
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				API_04,
				PDOL_9F66049F2A08,
			},
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				API_05,
				PDOL_9F66049F2A08,
			},
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				API_06,
				PDOL_9F66049F2A08,
			}
		},
		.aid_fci_num = 4,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting2.45 */
	{
		.ppse_entries = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				API_01,
				KERNEL_ID_22,
			},
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				API_02,
				KERNEL_ID_25,
			}
		},
		.ppse_entries_num = 2,
		.aid_fci = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				API_01,
				PDOL_9F2A08
			},
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				API_02,
				PDOL_9F2A08
			}
		},
		.aid_fci_num = 2,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_select_next,
					.start	 = start_c
				}
			},
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting3.1 */
	{
		.ppse_entries = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				API_04,
				KERNEL_ID_21,
			},
			{
				AID_A0000000020002010203040506070809,
				APP_LABEL_APP2,
				API_02,
				KERNEL_ID_24,
			},
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				API_03,
				KERNEL_ID_23,
			},
			{
				AID_A0000000040004010203040506070809,
				APP_LABEL_APP4,
				API_01,
				KERNEL_ID_24,
			}
		},
		.ppse_entries_num = 4,
		.aid_fci = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				API_04,
				PDOL_9F66049F2A08,
			},
			{
				AID_A0000000020002010203040506070809,
				APP_LABEL_APP2,
				API_02,
				PDOL_9F66049F2A08,
			},
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				API_03,
				PDOL_9F66049F2A08,
			},
			{
				AID_A0000000040004010203040506070809,
				APP_LABEL_APP4,
				API_01,
				PDOL_9F66049F2A08,
			}
		},
		.aid_fci_num = 4,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting3.2 */
	{
		.ppse_entries = {
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				API_00,
				KERNEL_ID_24,
			},
			{
				AID_A0000000010001010203040506070809,
				APP_LABEL_APP1,
				API_0E,
				KERNEL_ID_23,
			},
			{
				AID_A0000000040004010203040506070809,
				APP_LABEL_APP4,
				API_0E,
				KERNEL_ID_23,
			},
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				API_0E,
				KERNEL_ID_22,
			}
		},
		.ppse_entries_num = 4,
		.aid_fci = {
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				API_00,
				PDOL_9F66049F2A08
			},
			{
				AID_A0000000010001010203040506070809,
				APP_LABEL_APP1,
				API_0E,
				PDOL_9F66049F2A08
			},
			{
				AID_A0000000040004010203040506070809,
				APP_LABEL_APP4,
				API_0E,
				PDOL_9F66049F2A08
			},
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				API_0E,
				PDOL_9F66049F2A08
			}
		},
		.aid_fci_num = 4,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting3.3 */
	{
		.ppse_entries = {
			{
				AID_A0000000010001010203040506070809,
				APP_LABEL_APP1,
				API_00,
				KERNEL_ID_23,
			},
			{
				AID_A0000000040004010203040506070809,
				APP_LABEL_APP4,
				KERNEL_ID_23,
			},
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				API_00,
				KERNEL_ID_21,
			},
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				KERNEL_ID_24,
			}
		},
		.ppse_entries_num = 4,
		.aid_fci = {
			{
				AID_A0000000010001010203040506070809,
				APP_LABEL_APP1,
				API_00,
				PDOL_9F66049F2A08
			},
			{
				AID_A0000000040004010203040506070809,
				APP_LABEL_APP4,
				PDOL_9F66049F2A08
			},
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				API_00,
				PDOL_9F66049F2A08
			},
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				PDOL_9F66049F2A08
			}
		},
		.aid_fci_num = 4,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting3.4 */
	{
		.ppse_entries = {
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				API_0C,
				KERNEL_ID_23,
			},
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				API_0B,
				KERNEL_ID_21,
			},
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				API_0A,
				KERNEL_ID_810978
			},
			{
				AID_A0000000040004010203040506070809,
				APP_LABEL_APP4,
				API_03,
				KERNEL_ID_FF2222,
			}
		},
		.ppse_entries_num = 4,
		.aid_fci = {
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				API_0C,
				PDOL_9F66049F2A08,
			},
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				API_0B,
				PDOL_9F66049F2A08,
			},
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				API_0A,
				PDOL_9F66049F2A08,
			},
			{
				AID_A0000000040004010203040506070809,
				APP_LABEL_APP4,
				API_03,
				PDOL_9F66049F2A08,
			}
		},
		.aid_fci_num = 4,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting3.5 */
	{
		.ppse_entries = {
			{
				AID_A000000001000101,
				APP_LABEL_APP1,
				API_01,
				KERNEL_ID_23,
				EXT_SEL_0303
			},
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				API_02,
				KERNEL_ID_22,
				EXT_SEL_0202
			},
		},
		.ppse_entries_num = 2,
		.aid_fci = {
			{
				AID_A0000000010001010303,
				APP_LABEL_APP1,
				API_01,
				PDOL_9F66049F2A08,
			},
			{
				AID_A00000000200020202,
				APP_LABEL_APP2,
				API_02,
				PDOL_9F66049F2A08,
			},
		},
		.aid_fci_num = 2,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting3.6 */
	{
		.ppse_entries = {
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				API_09,
				KERNEL_ID_24,
			},
			{
				AID_A0000000040004010203040506070809,
				APP_LABEL_APP4,
				API_08,
				KERNEL_ID_25,
			},
			{
				AID_A0000000010001010203040506070809,
				APP_LABEL_APP1,
				API_08,
				KERNEL_ID_23
			},
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				API_08,
				KERNEL_ID_22,
			}
		},
		.ppse_entries_num = 4,
		.aid_fci = {
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				API_09,
				PDOL_9F66049F2A08,
			},
			{
				AID_A0000000040004010203040506070809,
				APP_LABEL_APP4,
				API_08,
				PDOL_9F66049F2A08,
			},
			{
				AID_A0000000010001010203040506070809,
				APP_LABEL_APP1,
				API_08,
				PDOL_9F66049F2A08,
			},
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				API_08,
				PDOL_9F66049F2A08,
			}
		},
		.aid_fci_num = 4,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting3.7 */
	{
		.ppse_entries = {
			{
				AID_A0000000030003010203040506070809,
				APP_LABEL_APP3,
				KERNEL_ID_25,
			},
			{
				AID_A0000000040004010203040506070809,
				APP_LABEL_APP4,
				API_00,
				KERNEL_ID_23,
			},
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				API_00,
				KERNEL_ID_21
			},
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				KERNEL_ID_24,
			}
		},
		.ppse_entries_num = 4,
		.aid_fci = {
			{
				AID_A0000000030003010203040506070809,
				APP_LABEL_APP3,
				PDOL_9F66049F2A08,
			},
			{
				AID_A0000000040004010203040506070809,
				APP_LABEL_APP4,
				API_00,
				PDOL_9F66049F2A08,
			},
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				API_00,
				PDOL_9F66049F2A08,
			},
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				PDOL_9F66049F2A08,
			}
		},
		.aid_fci_num = 4,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting4.1 */
	{
		.ppse_entries = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				KERNEL_ID_6,
				API_01,
			},
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				KERNEL_ID_62,
				API_01,
			},
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				KERNEL_ID_810977,
				API_01,
			},
			{
				AID_A000000003000301,
				APP_LABEL_APP3,
				KERNEL_ID_BF0841,
				API_01,
			},
			{
				AID_A000000002000201,
				APP_LABEL_APP2,
				KERNEL_ID_811110,
				API_01,
			},
			{
				AID_A000000001000101,
				APP_LABEL_APP1,
				KERNEL_ID_BF2223,
				API_01,
			},
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				KERNEL_ID_24,
				API_02,
			},
		},
		.ppse_entries_num = 7,
		.aid_fci = {
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				PDOL_9F66049F2A08,
				API_02
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting4.2 */
	{
		.ppse_entries = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				KERNEL_ID_23,
				API_01,
			},
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				KERNEL_ID_25,
				API_01,
			},
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				KERNEL_ID_22,
				API_01,
			},
			{
				AID_A000000004000402,
				APP_LABEL_APP4,
				KERNEL_ID_21,
				API_01,
			},
			{
				AID_A000000003000301,
				APP_LABEL_APP3,
				KERNEL_ID_24,
				API_01,
			},
			{
				AID_A000000002000201,
				APP_LABEL_APP2,
				KERNEL_ID_23,
				API_01,
			},
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				KERNEL_ID_24,
				API_02,
			},
		},
		.ppse_entries_num = 7,
		.aid_fci = {
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				PDOL_9F66049F2A08,
				API_02
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting5.1 */
	{
		.ppse_entries = {
			{
				AID_A0000000031010,
				APP_LABEL_VISA,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000031010,
				APP_LABEL_VISA,
				PDOL_9F66049F2A08,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting5.2 */
	{
		.ppse_entries = {
			{
				AID_A000000004101001,
				APP_LABEL_MASTERCARD,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A000000004101001,
				APP_LABEL_MASTERCARD,
				PDOL_9F66049F2A08,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting5.3 */
	{
		.ppse_entries = {
			{
				AID_A0000000651010,
				APP_LABEL_JCB,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000651010,
				APP_LABEL_JCB,
				PDOL_9F66049F2A08,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting5.4 */
	{
		.ppse_entries = {
			{
				AID_A000000025101001,
				APP_LABEL_AMEX,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A000000025101001,
				APP_LABEL_AMEX,
				PDOL_9F66049F2A08,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting5.5 */
	{
		.ppse_entries = {
			{
				AID_A0000000031010,
				APP_LABEL_VISA,
				KERNEL_ID_00,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000031010,
				APP_LABEL_VISA,
				PDOL_9F66049F2A08,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting5.6 */
	{
		.ppse_entries = {
			{
				AID_A000000004101001,
				APP_LABEL_MASTERCARD,
				KERNEL_ID_00,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A000000004101001,
				APP_LABEL_MASTERCARD,
				PDOL_9F66049F2A08,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting5.7 */
	{
		.ppse_entries = {
			{
				AID_A0000000651010,
				APP_LABEL_JCB,
				KERNEL_ID_00,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000651010,
				APP_LABEL_JCB,
				PDOL_9F66049F2A08,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting5.8 */
	{
		.ppse_entries = {
			{
				AID_A000000025101001,
				APP_LABEL_AMEX,
				KERNEL_ID_00,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A000000025101001,
				APP_LABEL_AMEX,
				PDOL_9F66049F2A08,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting5.10 */
	{
		.ppse_entries = {
			{
				AID_A0000000651010,
				APP_LABEL_JCB,
				KERNEL_ID_TK1,
				API_04,
			},
			{
				AID_A000000004101001,
				APP_LABEL_MASTERCARD,
				KERNEL_ID_TK4,
				API_03,
			},
			{
				AID_A0000000031010,
				APP_LABEL_VISA,
				KERNEL_ID_TK3,
				API_03,
			},
			{
				AID_A0000000251010,
				APP_LABEL_AMEX,
				API_01,
			}
		},
		.ppse_entries_num = 4,
		.aid_fci = {
			{
				AID_A0000000651010,
				APP_LABEL_JCB,
				PDOL_9F66049F2A08,
				API_04,
			},
			{
				AID_A000000004101001,
				APP_LABEL_MASTERCARD,
				PDOL_9F66049F2A08,
				API_03,
			},
			{
				AID_A0000000031010,
				APP_LABEL_VISA,
				PDOL_9F66049F2A08,
				API_03,
			},
			{
				AID_A0000000251010,
				APP_LABEL_AMEX,
				PDOL_9F66049F2A08,
				API_01,
			}
		},
		.aid_fci_num = 4,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting5.11 */
	{
		.ppse_entries = {
			{
				AID_A0000000651010,
				APP_LABEL_JCB,
				KERNEL_ID_TK1,
				API_04,
			},
			{
				AID_A000000004101001,
				APP_LABEL_MASTERCARD,
				API_01,
			},
			{
				AID_A0000000031010,
				APP_LABEL_VISA,
				KERNEL_ID_TK3,
				API_03,
			},
			{
				AID_A0000000251010,
				APP_LABEL_AMEX,
				KERNEL_ID_TK4,
				API_02,
			}
		},
		.ppse_entries_num = 4,
		.aid_fci = {
			{
				AID_A0000000651010,
				APP_LABEL_JCB,
				PDOL_9F66049F2A08,
				API_04,
			},
			{
				AID_A000000004101001,
				APP_LABEL_MASTERCARD,
				PDOL_9F66049F2A08,
				API_01,
			},
			{
				AID_A0000000031010,
				APP_LABEL_VISA,
				PDOL_9F66049F2A08,
				API_03,
			},
			{
				AID_A0000000251010,
				APP_LABEL_AMEX,
				PDOL_9F66049F2A08,
				API_02,
			}
		},
		.aid_fci_num = 4,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting5.12 */
	{
		.ppse_entries = {
			{
				AID_A0000000651010,
				APP_LABEL_JCB,
				KERNEL_ID_TK1,
				API_02,
			},
			{
				AID_A0000000251010,
				APP_LABEL_AMEX,
				KERNEL_ID_00,
				API_01,
			}
		},
		.ppse_entries_num = 2,
		.aid_fci = {
			{
				AID_A0000000651010,
				APP_LABEL_JCB,
				PDOL_9F66049F2A08,
				API_02,
			},
			{
				AID_A0000000251010,
				APP_LABEL_AMEX,
				PDOL_9F66049F2A08,
				API_01,
			}
		},
		.aid_fci_num = 2,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting5.13 */
	{
		.ppse =
		  "\x6F""\x2D"
		    "\x84""\x0E""\x32\x50\x41\x59\x2E\x53\x59\x53\x2E\x44\x44"
								  "\x46\x30\x31"
		    "\xA5""\x1B"
		      "\xBF\x0C""\x18"
			"\x61""\x16"
			  "\x4F""\x08""\xA0\x00\x00\x00\x03\x10\x10\x01"
			  "\x50""\x04""\x56\x49\x53\x41"
			  "\x87""\x01""\x01"
			  "\x9F\x2A""\x00",
		.ppse_len = 47,
		.aid_fci = {
			{
				AID_A000000003101001,
				APP_LABEL_VISA,
				PDOL_9F66049F2A08,
				API_01,
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting5.14 */
	{
		.ppse =
		  "\x6F""\x32"
		    "\x84""\x0E""\x32\x50\x41\x59\x2E\x53\x59\x53\x2E\x44\x44"
								  "\x46\x30\x31"
		    "\xA5""\x20"
		      "\xBF\x0C""\x1D"
			"\x61""\x1B"
			  "\x4F""\x07""\xA0\x00\x00\x00\x04\x10\x10"
			  "\x50""\x0A""\x4D\x41\x53\x54\x45\x52\x43\x41\x52\x44"
			  "\x87""\x01""\x01"
			  "\x9F\x2A""\x00",
		.ppse_len = 52,
		.aid_fci = {
			{
				AID_A0000000041010,
				APP_LABEL_MASTERCARD,
				API_01,
				PDOL_9F66049F2A08,
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting5.15 */
	{
		.ppse =
		  "\x6F""\x2C"
		    "\x84""\x0E""\x32\x50\x41\x59\x2E\x53\x59\x53\x2E\x44\x44"
								  "\x46\x30\x31"
		    "\xA5""\x1A"
		      "\xBF\x0C""\x17"
			"\x61""\x15"
			  "\x4F""\x08""\xA0\x00\x00\x00\x65\x10\x10\x01"
			  "\x50""\x03""\x4A\x43\x42"
			  "\x87""\x01""\x01"
			  "\x9F\x2A""\x00",
		.ppse_len = 46,
		.aid_fci = {
			{
				AID_A000000065101001,
				APP_LABEL_JCB,
				API_01,
				PDOL_9F66049F2A08,
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting5.16 */
	{
		.ppse =
		  "\x6F""\x2D"
		    "\x84""\x0E""\x32\x50\x41\x59\x2E\x53\x59\x53\x2E\x44\x44"
								  "\x46\x30\x31"
		    "\xA5""\x1B"
		      "\xBF\x0C""\x18"
			"\x61""\x16"
			  "\x4F""\x08""\xA0\x00\x00\x00\x25\x10\x10\x01"
			  "\x50""\x04""\x41\x4D\x45\x58"
			  "\x87""\x01""\x01"
			  "\x9F\x2A""\x00",
		.ppse_len = 47,
		.aid_fci = {
			{
				AID_A000000025101001,
				APP_LABEL_AMEX,
				API_01,
				PDOL_9F66049F2A08,
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting5.17 */
	{
		.ppse_entries = {
			{
				AID_A0000001523010,
				APP_LABEL_DISCOVER,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000001523010,
				APP_LABEL_DISCOVER,
				API_01,
				PDOL_9F66049F2A08,
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting5.18 */
	{
		.ppse =
		  "\x6F""\x31"
		    "\x84""\x0E""\x32\x50\x41\x59\x2E\x53\x59\x53\x2E\x44\x44"
								  "\x46\x30\x31"
		    "\xA5""\x1F"
		      "\xBF\x0C""\x1C"
			"\x61""\x1A"
			  "\x4F""\x08""\xA0\x00\x00\x01\x52\x30\x10\x01"
			  "\x50""\x08""\x44\x49\x53\x43\x4F\x56\x45\x52"
			   "\x87\x01\x01"
			   "\x9F\x2A""\x00",
		.ppse_len = 51,
		.aid_fci = {
			{
				AID_A000000152301001,
				APP_LABEL_DISCOVER,
				API_01,
				PDOL_9F66049F2A08,
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting5.19 */
	{
		.ppse_entries = {
			{
				AID_A0000003330101,
				APP_LABEL_CUP,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000003330101,
				APP_LABEL_CUP,
				API_01,
				PDOL_9F66049F2A08,
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting5.20 */
	{
		.ppse =
		  "\x6F""\x2B"
		    "\x84""\x0E""\x32\x50\x41\x59\x2E\x53\x59\x53\x2E\x44\x44"
								  "\x46\x30\x31"
		    "\xA5""\x19"
		      "\xBF\x0C""\x16"
			"\x61""\x14"
			  "\x4F""\x07""\xA0\x00\x00\x03\x33\x01\x01"
			  "\x50""\x03""\x43\x55\x50"
			  "\x87""\x01""\x01"
			  "\x9F\x2A""\x00",
		.ppse_len = 45,
		.aid_fci = {
			{
				AID_A0000003330101,
				APP_LABEL_CUP,
				API_01,
				PDOL_9F66049F2A08,
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting5.21 */
	{
		.ppse_entries = {
			{
				AID_A0000001523010,
				APP_LABEL_DISCOVER,
				KERNEL_ID_00,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000001523010,
				APP_LABEL_DISCOVER,
				API_01,
				PDOL_9F66049F2A08
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting5.22 */
	{
		.ppse_entries = {
			{
				AID_A0000003330101,
				APP_LABEL_CUP,
				KERNEL_ID_00,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000003330101,
				APP_LABEL_CUP,
				API_01,
				PDOL_9F66049F2A08
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting5.23 */
	{
		.ppse_entries = {
			{
				AID_A0000003241010,
				APP_LABEL_DISCOVER,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000003241010,
				APP_LABEL_DISCOVER,
				API_01,
				PDOL_9F66049F2A08,
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting5.24 */
	{
		.ppse =
		  "\x6F""\x31"
		    "\x84""\x0E""\x32\x50\x41\x59\x2E\x53\x59\x53\x2E\x44\x44"
								  "\x46\x30\x31"
		    "\xA5""\x1F"
		      "\xBF\x0C""\x1C"
			"\x61""\x1A"
			  "\x4F""\x08""\xA0\x00\x00\x03\x24\x10\x10\x01"
			  "\x50""\x08""\x44\x49\x53\x43\x4F\x56\x45\x52"
			  "\x87""\x01""\x01"
			  "\x9F\x2A""\x00",
		.ppse_len = 51,
		.aid_fci = {
			{
				AID_A000000324101001,
				APP_LABEL_DISCOVER,
				API_01,
				PDOL_9F66049F2A08,
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting5.25 */
	{
		.ppse_entries = {
			{
				AID_A0000003241010,
				APP_LABEL_DISCOVER,
				KERNEL_ID_00,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000003241010,
				APP_LABEL_DISCOVER,
				API_01,
				PDOL_9F66049F2A08,
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		}
	},
	/* LTsetting6.1 */
	{
		.ppse_sw = EMV_SW_6A82_FILE_NOT_FOUND
	},
	/* LTsetting6.2 */
	{
		.ppse_sw = EMV_SW_6283_SELECTED_FILE_INVALIDATED
	},
	/* LTsetting6.3 */
	{
	},
	/* LTsetting6.5 */
	{
		.ppse_entries = {
			{
				AID_A00000000400040102030405060708090A,
				APP_LABEL_APP4,
				KERNEL_ID_24,
				API_01,
			},
			{
				AID_A0000000,
				APP_LABEL_APP2,
				KERNEL_ID_22,
				API_02,
			},
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				API_03,
			}
		},
		.ppse_entries_num = 3,
		.aid_fci = {
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				PDOL_9F66049F2A08,
				API_F2
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		},
		.gpo_resp_num = 1
	},
	/* LTsetting6.6 */
	{
		.ppse_entries = {
			{
				AID_A0000000661010,
				APP_LABEL_APP2,
				KERNEL_ID_21,
				API_01,
			},
			{
				AID_A000000005,
				APP_LABEL_APP2,
				KERNEL_ID_22,
				API_01,
			},
			{
				AID_A000000006101001,
				APP_LABEL_APP2,
				KERNEL_ID_22,
				API_01,
			},
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				KERNEL_ID_22,
				API_02,
			}
		},
		.ppse_entries_num = 4,
		.aid_fci = {
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				PDOL_9F66049F2A08,
				API_02
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		},
		.gpo_resp_num = 1
	},
	/* LTsetting6.7 */
	{
		.ppse_entries = {
			{
				AID_A000000002000201,
				APP_LABEL_APP2,
				KERNEL_ID_80,
				API_01,
			},
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				KERNEL_ID_C001,
				API_01,
			},
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				KERNEL_ID_24,
				API_02,
			}
		},
		.ppse_entries_num = 3,
		.aid_fci = {
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				PDOL_9F66049F2A08,
				API_02
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		},
		.gpo_resp_num = 1
	},
	/* LTsetting6.8 */
	{
		.ppse_entries = {
			{
				AID_A00000000200020101,
				APP_LABEL_APP2,
				API_02,
				KERNEL_ID_22,
			},
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				KERNEL_ID_24,
				API_04,
			},
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				API_03,
				KERNEL_ID_23,
			},
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				KERNEL_ID_21,
				API_01,
			}
		},
		.ppse_entries_num = 4,
		.aid_fci = {
			{
				AID_A00000000200020101,
				APP_LABEL_APP2,
				API_02,
				PDOL_9F66049F2A08,
				.sw = EMV_SW_6283_SELECTED_FILE_INVALIDATED
			},
			{
				.sw = EMV_SW_9000_OK
			},
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				API_03,
				PDOL_9F66049F2A08,
			},
			{
				.sw = EMV_SW_6A82_FILE_NOT_FOUND
			}
		},
		.aid_fci_num = 4,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		},
		.gpo_resp_num = 1
	},
	/* LTsetting6.9 */
	{
		.ppse_entries = {
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				API_01,
				KERNEL_ID_22,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				API_01,
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		},
		.gpo_resp_num = 1
	},
	/* LTsetting6.10 */
	{
		.ppse_entries = {
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				KERNEL_ID_24FFFF,
				API_F2,
			},
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				KERNEL_ID_240000,
				API_04,
			},
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				KERNEL_ID_23FF00FF00FF00FF,
				API_FF,
			},
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				KERNEL_ID_2102030405060708,
				API_41,
			}
		},
		.ppse_entries_num = 4,
		.aid_fci = {
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				PDOL_9F66049F2A08,
				API_F2
			},
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				PDOL_9F66049F2A08,
				API_04
			},
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				PDOL_9F66049F2A08,
				API_FF
			},
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				PDOL_9F66049F2A08,
				API_41
			}
		},
		.aid_fci_num = 4,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		},
		.gpo_resp_num = 1
	},
	/* LTsetting6.11 */
	{
		.ppse_entries = {
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				KERNEL_ID_23,
				API_01
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				PDOL_9F66049F02069F03069F3704DF0101C102__9F2502,
				API_01,
				.other = "\x9F\x02\x06\x10\x20\x30\x40\x50\x60"
					 "\x9F\x03\x06\x01\x02\x03\x04\x05\x06"
					 "\x9F\x37\x04\x11\x22\x33\x44"
					 "\xDF\x01\x01\xFF"
					 "\xC1\x02\xEE\xEE"
					 "\x85\x01\xDD"
					 "\x9F\x25\x02\xCC\xCC",
				.other_len = 41
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		},
		.gpo_resp_num = 1
	},
	/* LTsetting6.12 */
	{
		.ppse =
		  "\x6F""\x30"
		    "\x84""\x0E""\x32\x50\x41\x59\x2E\x53\x59\x53\x2E\x44\x44"
								  "\x46\x30\x31"
		    "\x00"
		    "\xA5""\x1D"
		      "\x00"
		      "\xBF\x0C""\x18"
			"\x61""\x16"
			  "\x4F""\x07""\xA0\x00\x00\x00\x02\x00\x02"
			  "\x50""\x04\x41\x50\x50\x32"
			  "\x87""\x01""\x01"
			  "\x9F\x2A""\x01""\x22"
		      "\x00",
		.ppse_len = 50,
		.aid_fci = {
			{
				.bin =
				  "\x6F""\x26"
				    "\x84""\x07""\xA0\x00\x00\x00\x02\x00\x02"
				    "\x00"
				    "\xA5""\x1A"
				      "\x50""\x04""\x41\x50\x50\x32"
				      "\x00"
				      "\x87""\x01""\x01"
				      "\x00"
				      "\x9F\x38""\x0C""\x9F\x02\x07\x9F\x37\x05"
						     "\x9F\x16\x10\x9F\x1C\x09",
				.bin_len = 40,
				AID_A0000000020002,
				APP_LABEL_APP2,
				PDOL_9F02079F37059F16109F1C09,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		},
		.gpo_resp_num = 1
	},
	/* LTsetting6.13 */
	{
		.ppse =
		  "\x6F""\x4B"
		    "\x84""\x0E""\x32\x50\x41\x59\x2E\x53\x59\x53\x2E\x44\x44"
								  "\x46\x30\x31"
		    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
		    "\xA5""\x25"
		      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
		      "\xBF\x0C""\x18"
			"\x61""\x16"
			  "\x4F""\x07""\xA0\x00\x00\x00\x04\x00\x04"
			  "\x50""\x04""\x41\x50\x50\x34"
			  "\x87""\x01""\x01"
			  "\x9F\x2A""\x01""\x24"
		      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
		.ppse_len = 77,
		.aid_fci = {
			{
				.bin =
				  "\x6F""\x41"
				    "\x84""\x07""\xA0\x00\x00\x00\x04\x00\x04"
				    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
				    "\xA5""\x2C"
				      "\x50""\x04""\x41\x50\x50\x34"
				      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
				      "\x87""\x01""\x01"
				      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
				      "\x9F\x38""\x0C""\x9F\x02\x10\x9F\x37\x0E"
						     "\x9F\x16\x19\x9F\x1C\x12",
				.bin_len = 67,
				AID_A0000000040004,
				APP_LABEL_APP4,
				PDOL_9F02109F370E9F16199F1C12,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		},
		.gpo_resp_num = 1
	},
	/* LTsetting6.14 */
	{
		.ppse =
		  "\x6F""\x2D"
		    "\xA5""\x1B"
		      "\xBF\x0C""\x18"
			"\x61""\x16"
			  "\x4F""\x07""\xA0\x00\x00\x00\x03\x00\x03"
			  "\x50""\x04""\x41\x50\x50\x33"
			  "\x87""\x01""\x01"
			  "\x9F\x2A""\x01""\x21"
		      "\x84""\x0E""\x32\x50\x41\x59\x2E\x53\x59\x53\x2E\x44\x44"
								 "\x46\x30\x31",
		.ppse_len = 47,
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				.bin =
				  "\x6F""\x1A"
				    "\xA5""\x0F"
				      "\x9F\x38""\x03""\x9F\x02\x06"
				      "\x87""\x01""\x01"
				      "\x50""\x04""\x41\x50\x50\x33"
				    "\x84""\x07""\xA0\x00\x00\x00\x03\x00\x03",
				.bin_len = 28,
				AID_A0000000030003,
				APP_LABEL_APP3,
				PDOL_9F0206,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		},
		.gpo_resp_num = 1
	},
	/* LTsetting6.16 */
	{
		.ppse_entries = {
			{
				APP_LABEL_APP4,
				KERNEL_ID_24,
				API_01,
			},
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				KERNEL_ID_23,
				API_02,
			}
		},
		.ppse_entries_num = 2,
		.aid_fci = {
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				PDOL_D1029F66049F02069F03069C019F37049F2A08,
				API_02
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		},
		.gpo_resp_num = 1
	},
	/* LTsetting6.17 */
	{
		.ppse_entries = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				API_02,
				KERNEL_ID_22,
			},
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				API_01,
				KERNEL_ID_21,
			}
		},
		.ppse_entries_num = 2,
		.aid_fci = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				API_02,
				PDOL_9F2A08
			},
			{
				.sw = EMV_SW_6A82_FILE_NOT_FOUND
			}
		},
		.aid_fci_num = 2,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		},
		.gpo_resp_num = 1
	},
	/* LTsetting6.18 */
	{
		.ppse_entries = {
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				API_02,
				KERNEL_ID_25,
			},
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				API_01,
				KERNEL_ID_25,
			}
		},
		.ppse_entries_num = 2,
		.aid_fci = {
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				API_02,
				PDOL_9F2A08
			},
			{
				.sw = EMV_SW_6A82_FILE_NOT_FOUND
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_approved
				}
			}
		},
		.gpo_resp_num = 1
	},
	/* LTsetting6.19 */
	{
		.ppse_entries = {
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				API_01,
				KERNEL_ID_23,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				API_01,
				PDOL_D102D2019F6604
			},
			{
				.sw = EMV_SW_6A83_RECORD_NOT_FOUND
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_online_request,
					.start = start_a,
					.online_response_type = ort_any,
					.data_record = {
						.data = "\x91\x10\x01\x02\x03"
							"\x04\x05\x06\x07\x08"
							"\x09\x0A\x0B\x0C\x0D"
							"\x0E\x0F\x10",
						.len = 18
					}
				}
			}
		},
		.gpo_resp_num = 1
	},
	/* LTsetting6.20 */
	{
		.ppse_entries = {
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				API_01,
				KERNEL_ID_22,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				API_01,
				PDOL_D102D201
			},
			{
				.sw = EMV_SW_6A82_FILE_NOT_FOUND
			}
		},
		.aid_fci_num = 2,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_online_request,
					.start = start_b,
					.online_response_type = ort_emv_data,
					.removal_timeout = 100,
					.data_record = {
						.data = "\x91\x10\x01\x02\x03"
							"\x04\x05\x06\x07\x08"
							"\x09\x0A\x0B\x0C\x0D"
							"\x0E\x0F\x10",
						.len = 18
					}
				}
			}
		},
		.gpo_resp_num = 1
	},
	/* LTsetting6.21 */
	{
		.ppse_entries = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				API_01,
				KERNEL_ID_21,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				API_01,
				PDOL_D102D201
			},
			{
				.sw = EMV_SW_6A83_RECORD_NOT_FOUND
			}
		},
		.aid_fci_num = 2,
		.gpo_resp = {
			{
				.outcome_parms = {
					.outcome = out_online_request,
					.start = start_c,
					.online_response_type = ort_any
				}
			}
		},
		.gpo_resp_num = 1
	},
	/* LTsetting8.0 */
	{
		.ppse_entries = {
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				KERNEL_ID_22,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.present = {
						.ui_request_on_outcome = true
					},
					.outcome = out_approved,
					.ui_request_on_outcome = {
						.msg_id = msg_approved,
						.status =
						     sts_card_read_successfully,
						.value_qualifier = val_balance,
						.value =
						     "\x00\x00\x00\x12\x34\x00",
						.currency_code = ISO4217_USD
					}
				}
			}
		},
		.gpo_resp_num = 1
	},
	/* LTsetting8.1 */
	{
		.ppse_entries = {
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				KERNEL_ID_22,
				API_01,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				API_01
			}
		},
		.aid_fci_num = 1,
		.gpo_resp = {
			{
				.outcome_parms = {
					.present = {
						.ui_request_on_outcome = true
					},
					.outcome = out_approved,
					.ui_request_on_outcome = {
						.msg_id = msg_approved,
						.status =
						     sts_card_read_successfully,
						.value_qualifier = val_balance,
						.value =
						     "\x00\x00\x00\x12\x34\x00",
						.currency_code = ISO4217_EUR
					}
				}
			}
		},
		.gpo_resp_num = 1
	}
};

struct lt_app *lt_def_app_factory(struct lt *lt, const uint8_t *aid,
				  size_t aid_sz, const struct aid_fci *aid_fci);

static struct tlv *tlv_get_ppse_entry(const struct ppse_entry *ent)
{
	struct tlv *tlv_ppse_entry = NULL, *tlv = NULL;

	if (ent->aid_len)
		tlv = tlv_insert_after(tlv, tlv_new(EMV_ID_ADF_NAME,
						       ent->aid_len, ent->aid));

	if (ent->app_label_len)
		tlv = tlv_insert_after(tlv, tlv_new(EMV_ID_APPLICATION_LABEL,
					   ent->app_label_len, ent->app_label));

	if (ent->app_prio_len)
		tlv = tlv_insert_after(tlv,
				  tlv_new(EMV_ID_APPLICATION_PRIORITY_INDICATOR,
					    ent->app_prio_len, &ent->app_prio));

	if (ent->kernel_id_len)
		tlv = tlv_insert_after(tlv, tlv_new(EMV_ID_KERNEL_IDENTIFIER,
					   ent->kernel_id_len, ent->kernel_id));

	if (ent->ext_select_len)
		tlv = tlv_insert_after(tlv, tlv_new(EMV_ID_EXTENDED_SELECTION,
					 ent->ext_select_len, ent->ext_select));

	while (tlv_get_prev(tlv))
		tlv = tlv_get_prev(tlv);

	tlv_ppse_entry = tlv_new(EMV_ID_DIRECTORY_ENTRY, 0, NULL);
	tlv_insert_below(tlv_ppse_entry, tlv);

	return tlv_ppse_entry;
}

static void outcome_to_gpo_outcome(const struct emv_outcome_parms *in,
						   struct outcome_gpo_resp *out)
{
	memset(out, 0, sizeof(*out));
	out->outcome		= (uint8_t)in->outcome;
	out->start		= (uint8_t)in->start;
	out->online_resp	= (uint8_t)in->online_response_type;
	out->cvm		= (uint8_t)in->cvm;
	out->alt_iface_pref	= (uint8_t)in->alternate_interface_pref;
	out->receipt		= (uint8_t)in->receipt;
	out->field_off_request	= in->present.field_off_request ?
			     htons((uint16_t)in->field_off_hold_time) : 0xffffu;
	out->removal_timeout	= htons((uint16_t)in->removal_timeout);
}

static void ui_req_to_gpo_ui_req(const struct emv_ui_request *in,
						    struct ui_req_gpo_resp *out)
{
	memset(out, 0, sizeof(*out));
	out->msg_id	= (uint8_t)in->msg_id;
	out->status     = (uint8_t)in->status;
	out->hold_time  = htons(in->hold_time);
	out->value_qual = (uint8_t)in->value_qualifier;
	memcpy(out->lang_pref, in->lang_pref, sizeof(in->lang_pref));
	memcpy(out->value, in->value, sizeof(in->value));
	memcpy(out->currency_code, in->currency_code,
						     sizeof(in->currency_code));
}

static int ber_get_gpo_resp(const struct gpo_resp *resp, void *ber,
								 size_t *ber_sz)
{
	struct outcome_gpo_resp out_resp;
	struct tlv *gpo_resp = NULL, *tlv = NULL;
	int rc = TLV_RC_OK;

	gpo_resp = tlv_new(EMV_ID_RESP_MSG_TEMPLATE_FMT_2, 0, NULL);
	if (!gpo_resp)
		return TLV_RC_OUT_OF_MEMORY;

	outcome_to_gpo_outcome(&resp->outcome_parms, &out_resp);

	tlv = tlv_insert_below(gpo_resp, tlv_new(EMV_ID_OUTCOME_DATA,
						  sizeof(out_resp), &out_resp));

	if (resp->outcome_parms.present.ui_request_on_outcome) {
		struct ui_req_gpo_resp ui_req_resp;

		ui_req_to_gpo_ui_req(&resp->outcome_parms.ui_request_on_outcome,
								  &ui_req_resp);

		tlv = tlv_insert_after(tlv, tlv_new(EMV_ID_UI_REQ_ON_OUTCOME,
					    sizeof(ui_req_resp), &ui_req_resp));
	}

	if (resp->outcome_parms.present.ui_request_on_restart) {
		struct ui_req_gpo_resp ui_req_resp;

		ui_req_to_gpo_ui_req(&resp->outcome_parms.ui_request_on_restart,
								  &ui_req_resp);

		tlv = tlv_insert_after(tlv, tlv_new(EMV_ID_UI_REQ_ON_RESTART,
					    sizeof(ui_req_resp), &ui_req_resp));
	}

	if (resp->outcome_parms.data_record.len) {
		struct tlv *tlv_data_record = NULL;

		rc = tlv_shallow_parse(resp->outcome_parms.data_record.data,
					    resp->outcome_parms.data_record.len,
							      &tlv_data_record);
		if (rc != TLV_RC_OK)
			goto done;

		tlv = tlv_insert_after(tlv, tlv_data_record);
	}

	rc = tlv_encode(gpo_resp, ber, ber_sz);

done:
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

	if (num_entries) {
		tlv = tlv_insert_below(tlv, tlv_get_ppse_entry(&entries[0]));
		for (i = 1; i < num_entries; i++)
			tlv = tlv_insert_after(tlv,
					       tlv_get_ppse_entry(&entries[i]));
	}

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
	if (aid_fci->app_prio_len)
		tlv = tlv_insert_after(tlv, tlv_new(
					  EMV_ID_APPLICATION_PRIORITY_INDICATOR,
				    aid_fci->app_prio_len, &aid_fci->app_prio));
	if (aid_fci->pdol_len)
		tlv = tlv_insert_after(tlv, tlv_new(EMV_ID_PDOL,
					     aid_fci->pdol_len, aid_fci->pdol));
	if (aid_fci->lang_pref[0])
		tlv = tlv_insert_after(tlv, tlv_new(EMV_ID_LANGUAGE_PREFERENCE,
							2, aid_fci->lang_pref));
	if (aid_fci->other) {
		struct tlv *tlv_other = NULL;

		rc = tlv_parse(aid_fci->other, aid_fci->other_len, &tlv_other);
		if (rc != TLV_RC_OK)
			goto done;

		tlv = tlv_insert_after(tlv, tlv_other);
	}

	rc = tlv_encode(tlv_aid_fci, ber, ber_size);

done:
	tlv_free(tlv_aid_fci);

	return rc;
}


struct lt {
	const struct emv_hal_ops *ops;
	enum ltsetting		  setting;
	int			  mode;
	int			  state;
	int                       selected_aid;
	int			  i_gpo_resp;
	struct emv_chk		 *checker;
	log4c_category_t	 *log_cat;
	struct lt_app		 *apps[8];
};

uint32_t lt_get_unpredictable_number(struct emv_hal *hal)
{
	return (uint32_t)random();
}

int lt_field_on(struct emv_hal *hal)
{
	struct lt *lt = (struct lt *)hal;

	emv_chk_field_on(lt->checker);

	log4c_category_log(lt->log_cat, LOG4C_PRIORITY_TRACE,
						     "%s(): success", __func__);
	return EMV_RC_OK;
}

int lt_field_off(struct emv_hal *hal, int hold_time)
{
	struct lt *lt = (struct lt *)hal;

	emv_chk_field_off(lt->checker, hold_time);

	log4c_category_log(lt->log_cat, LOG4C_PRIORITY_TRACE,
			  "%s(hold_time: %d ms): success", __func__, hold_time);
	return EMV_RC_OK;
}

int lt_wait_for_card(struct emv_hal *hal, int timeout)
{
	struct lt *lt = (struct lt *)hal;

	log4c_category_log(lt->log_cat, LOG4C_PRIORITY_TRACE,
						     "%s(): success", __func__);

	if (lt->mode == LT_NORMAL)
		return EMV_RC_OK;

	if (lt->mode == LT_COLLISION_THEN_WITHDRAW_ONE) {
		switch (lt->state) {
		case 0:
			lt->state = 1;
			return EMV_RC_COLLISION;
		case 1:
			lt->state = 0;
			return EMV_RC_OK;
		default:
			return EMV_RC_FAIL;
		}
	}

	if (lt->mode == LT_COLLISION_THEN_WITHDRAW_BOTH) {
		switch (lt->state) {
		case 0:
			lt->state = 1;
			return EMV_RC_COLLISION;
		case 1:
			lt->state = 2;
			return EMV_RC_CONTINUE;
		case 2:
			lt->state = 0;
			return EMV_RC_OK;
		default:
			return EMV_RC_FAIL;
		}
	}

	return EMV_RC_OK;
}

static int lt_select_application(struct lt *lt, uint8_t p1, uint8_t p2,
		      size_t lc, const uint8_t *data, size_t *le, uint8_t *resp,
								    uint8_t *sw)
{
	size_t i_aid;
	int rc = EMV_RC_OK;

	emv_chk_select(lt->checker, data, lc);

	if ((lc == strlen(DF_NAME_2PAY_SYS_DDF01)) &&
	    !memcmp(data, DF_NAME_2PAY_SYS_DDF01, lc)) {
		const struct lt_setting *lt_setting = &ltsetting[lt->setting];

		if (lt_setting->ppse_len) {
			*le = lt_setting->ppse_len;
			memcpy(resp, lt_setting->ppse, lt_setting->ppse_len);
		} else {
			rc = ber_get_ppse(lt_setting->ppse_entries,
					lt_setting->ppse_entries_num, resp, le);
		}

		if (rc != EMV_RC_OK)
			goto done;

		if (memcmp(ltsetting[lt->setting].ppse_sw, "\x00\x00", 2))
			memcpy(sw, ltsetting[lt->setting].ppse_sw, 2);
		else
			memcpy(sw, EMV_SW_9000_OK, 2);

		goto done;
	}

	for (i_aid = 0; i_aid < ltsetting[lt->setting].aid_fci_num; i_aid++) {
		const struct aid_fci *fci = NULL;

		fci = &ltsetting[lt->setting].aid_fci[i_aid];
		if ((lc == fci->aid_len) && !memcmp(data, fci->aid, lc)) {
			if (lt->apps[i_aid] == NULL) {
				lt_app_factory_t app_factory =
							    fci->lt_app_factory;

				if (app_factory == NULL)
					app_factory = (lt_app_factory_t)
							     lt_def_app_factory;

				lt->apps[i_aid] = app_factory(lt, fci->aid, lc,
									   fci);
			}

			rc = lt->apps[i_aid]->ops->select(lt->apps[i_aid], resp,
									le, sw);
			lt->selected_aid = i_aid;

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
	const struct gpo_resp *gpo_resp = NULL;
	int rc = EMV_RC_OK;
	char hex[lc * 2 + 1];
	struct tlv *tlv = NULL;
	uint8_t ber_tlv[2048];
	size_t ber_tlv_len = sizeof(ber_tlv);
	char ber_tlv_hex[4096];
	const struct aid_fci *aid_fci = NULL;

	log4c_category_log(lt->log_cat, LOG4C_PRIORITY_TRACE,
						"%s(PDOL data: '%s')", __func__,
					      libtlv_bin_to_hex(data, lc, hex));

	memcpy(sw, EMV_SW_9000_OK, 2);

	gpo_resp = &ltsetting[lt->setting].gpo_resp[lt->i_gpo_resp++];

	rc = ber_get_gpo_resp(gpo_resp, resp, le);
	if (rc != EMV_RC_OK) {
		log4c_category_log(lt->log_cat, LOG4C_PRIORITY_ERROR,
			   "%s() ber_get_gpo_resp failed. rc %d", __func__, rc);
		goto done;
	}


	aid_fci = &ltsetting[lt->setting].aid_fci[lt->selected_aid];

	rc = dol_and_del_to_tlv(aid_fci->pdol, aid_fci->pdol_len, data, lc,
									  &tlv);
	if (rc != EMV_RC_OK) {
		log4c_category_log(lt->log_cat, LOG4C_PRIORITY_ERROR,
			 "%s() dol_and_del_to_tlv failed. rc %d", __func__, rc);
		goto done;
	}

	rc = tlv_encode(tlv, ber_tlv, &ber_tlv_len);
	if (rc != TLV_RC_OK) {
		log4c_category_log(lt->log_cat, LOG4C_PRIORITY_ERROR,
				 "%s() tlv_encode failed. rc %d", __func__, rc);
		goto done;
	}

	log4c_category_log(lt->log_cat, LOG4C_PRIORITY_TRACE,
					"%s(): TLV(PDOL, DEL) = '%s'", __func__,
			  libtlv_bin_to_hex(ber_tlv, ber_tlv_len, ber_tlv_hex));

	emv_chk_gpo_data(lt->checker, tlv);

	tlv_free(tlv);

done:
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
	char hex[256 * 2 + 1], capdu_hex[capdu_sz * 2 + 1];
	size_t resp_sz = sizeof(resp);
	int i = 0;
	int rc = EMV_RC_OK;

	log4c_category_log(lt->log_cat, LOG4C_PRIORITY_TRACE, "%s(capdu: '%s')",
		       __func__, libtlv_bin_to_hex(capdu, capdu_sz, capdu_hex));

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

	if (*rapdu_sz < resp_sz)
		return EMV_RC_OVERFLOW;

	memcpy(rapdu, resp, resp_sz);
	*rapdu_sz = resp_sz;

	if (rc != EMV_RC_OK) {
		log4c_category_log(lt->log_cat, LOG4C_PRIORITY_NOTICE,
					  "%s(): failed. rc: %d", __func__, rc);
	} else {
		log4c_priority_level_t prio = LOG4C_PRIORITY_TRACE;

		if (memcmp(sw, EMV_SW_9000_OK, 2))
			prio = LOG4C_PRIORITY_NOTICE;

		log4c_category_log(lt->log_cat, prio,
					 "%s(): success, rapdu: '%s'", __func__,
				      libtlv_bin_to_hex(rapdu, *rapdu_sz, hex));
	}

	return rc;
}

void lt_ui_request(struct emv_hal *hal, const struct emv_ui_request *ui_request)
{
	struct lt *lt = (struct lt *)hal;

	emv_chk_ui_request(lt->checker, ui_request);
}

void lt_get_interface_device_serial_number(struct emv_hal *hal, char sn[8])
{
	memcpy(sn, INTERFACE_DEVICE_SERIAL_NUMBER, 8);
}

const struct emv_hal_ops lt_ops  = {
	.get_interface_device_serial_number =
				    lt_get_interface_device_serial_number,
	.get_unpredictable_number = lt_get_unpredictable_number,
	.field_on		  = lt_field_on,
	.field_off		  = lt_field_off,
	.wait_for_card		  = lt_wait_for_card,
	.transceive		  = lt_transceive,
	.ui_request		  = lt_ui_request
};

struct emv_hal *lt_new(enum ltsetting ltsetting, struct emv_chk *checker,
					   const char *log4c_category, int mode)
{
	struct lt *lt = NULL;
	char cat[64];

	if (ltsetting >= num_ltsettings)
		return NULL;

	lt = malloc(sizeof(struct lt));
	if (!lt)
		return NULL;

	memset(lt, 0, sizeof(*lt));

	lt->ops = &lt_ops;

	snprintf(cat, sizeof(cat), "%s.lt", log4c_category);
	lt->log_cat = log4c_category_get(cat);
	lt->setting = ltsetting;
	lt->checker = checker;
	lt->mode    = mode;

	return (struct emv_hal *)lt;
}

void lt_free(struct emv_hal *hal)
{
	struct lt *lt = (struct lt *)hal;

	if (lt) {
		int i_aid;

		for (i_aid = 0; i_aid < ARRAY_SIZE(lt->apps); i_aid++)
			if (lt->apps[i_aid])
				lt->apps[i_aid]->ops->free(lt->apps[i_aid]);

		free(lt);
	}
}

void lt_init(void)
{
	int i_ltset;

	for (i_ltset = 0; i_ltset < ARRAY_SIZE(ltsetting); i_ltset++) {
		struct lt_setting *ltset = &ltsetting[i_ltset];
		int i_ppse;

		for (i_ppse = 0; i_ppse < ARRAY_SIZE(ltset->ppse_entries);
								     i_ppse++) {
			struct ppse_entry *ppse = &ltset->ppse_entries[i_ppse];
			struct tk_id id;

			id.kernel_id_len = ppse->kernel_id_len;
			memcpy(id.kernel_id, ppse->kernel_id, id.kernel_id_len);

			emvco_ep_ta_update_tk_kernel_id(&id);

			ppse->kernel_id_len = id.kernel_id_len;
			memcpy(ppse->kernel_id, id.kernel_id, id.kernel_id_len);
		}
	}
}

struct lt_def_app {
	const struct lt_app_ops *ops;
	struct lt *lt;
	const struct aid_fci *fci;
};

static void lt_def_app_free(struct lt_app *lt_app)
{
	free(lt_app);
}

static int lt_def_app_select(struct lt_app *lt_app, uint8_t *resp,
						   size_t *resp_sz, uint8_t *sw)
{
	struct lt_def_app *app = (struct lt_def_app *)lt_app;
	int rc = EMV_RC_OK;

	if (app->lt->setting == ltsetting6_9) {
		switch (app->lt->state) {
		case 0:
			rc = EMV_RC_RF_TIMEOUT;
			app->lt->state = 1;
			break;
		case 1:
			app->lt->state = 0;
			break;
		default:
			break;
		}
	}

	if (app->lt->setting == ltsetting6_17) {
		switch (app->lt->state) {
		case 0:
			app->fci = &ltsetting[app->lt->setting].aid_fci[1];
			app->lt->state = 1;
			break;
		case 1:
			app->fci = &ltsetting[app->lt->setting].aid_fci[0];
			app->lt->state = 0;
			break;
		default:
			break;
		}
	}

	if ((app->lt->setting >= ltsetting6_19) &&
	    (app->lt->setting <= ltsetting6_21)) {
		switch (app->lt->state) {
		case 0:
			app->fci = &ltsetting[app->lt->setting].aid_fci[0];
			app->lt->state = 1;
			break;
		case 1:
			app->fci = &ltsetting[app->lt->setting].aid_fci[1];
			app->lt->state = 0;
			break;
		default:
			break;
		}
	}

	if (app->fci->bin_len) {
		*resp_sz = app->fci->bin_len;
		memcpy(resp, app->fci->bin, app->fci->bin_len);
	} else {
		ber_get_aid_fci(app->fci, resp, resp_sz);
	}

	if (memcmp(app->fci->sw, "\x00\x00", 2))
		memcpy(sw, app->fci->sw, 2);
	else
		memcpy(sw, EMV_SW_9000_OK, 2);

	return rc;
}

static const struct lt_app_ops lt_def_app_ops = {
	.free	= lt_def_app_free,
	.select	= lt_def_app_select
};

struct lt_app *lt_def_app_factory(struct lt *lt, const uint8_t *aid,
				   size_t aid_sz, const struct aid_fci *aid_fci)
{
	struct lt_def_app *app = NULL;

	app = (struct lt_def_app *)malloc(sizeof(struct lt_def_app));
	app->ops = &lt_def_app_ops;
	app->lt = lt;
	app->fci = aid_fci;

	return (struct lt_app *)app;
}
