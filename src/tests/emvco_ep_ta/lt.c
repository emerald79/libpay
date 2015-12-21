#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <log4c.h>
#include <assert.h>

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
	bool lang_pref:1;
};

struct aid_fci {
	struct aid_fci_flags present;
	uint8_t		     aid[16];
	size_t		     aid_len;
	uint8_t		     app_label[17];
	size_t		     app_label_len;
	uint8_t		     app_prio;
	uint8_t		     pdol[256];
	size_t		     pdol_len;
	char		     lang_pref[2];
	uint8_t		     other[64];
	size_t		     other_len;
};

struct gpo_resp {
	struct emv_outcome_parms outcome_parms;
};

struct lt_setting {
	struct ppse_entry ppse_entries[8];
	size_t		  ppse_entries_num;
	struct aid_fci	  aid_fci[8];
	size_t		  aid_fci_num;
	struct gpo_resp	  gpo_resp[8];
	size_t		  gpo_resp_num;
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
				PDOL_D1029F66049F02069F03069C019F37049F2A08,
				.app_prio = 1
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
				.present = {
					.app_label = true,
					.app_prio  = true,
					.kernel_id = true,
				},
				AID_A0000000020002,
				APP_LABEL_APP2,
				KERNEL_ID_22,
				.app_prio = 1,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				PDOL_D1029F66049F02069F03069C019F37049F2A08,
				.app_prio = 1
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
				.present = {
					.app_label = true,
					.app_prio  = true,
					.kernel_id = true,
				},
				AID_A0000000030003,
				APP_LABEL_APP3,
				KERNEL_ID_21,
				.app_prio = 1,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				PDOL_D1029F66049F02069F03069C019F37049F2A08,
				.app_prio = 1
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
				.present = {
					.app_label = true,
					.app_prio  = true,
					.kernel_id = true,
				},
				AID_A0000000040004,
				APP_LABEL_APP4,
				KERNEL_ID_24,
				.app_prio = 1,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				PDOL_D1029F66049F02069F03069C019F37049F2A08,
				.app_prio = 1
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
	/* LTsetting1.7 */
	{
		.ppse_entries = {
			{
				.present = {
					.app_label = true,
					.app_prio  = true,
					.kernel_id = true,
				},
				AID_A0000000030003,
				APP_LABEL_APP3,
				KERNEL_ID_21,
				.app_prio = 1,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				PDOL_D102D201,
				.app_prio = 1
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
					.present = {
						.receipt = true
					},
					.outcome = out_approved,
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
				.present = {
					.app_label = true,
					.app_prio  = true,
					.kernel_id = true,
				},
				AID_A0000000030003,
				APP_LABEL_APP3,
				KERNEL_ID_21,
				.app_prio = 1,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				PDOL_D102D201,
				.app_prio = 1
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
							"\x86\x0E\x13\x14\x15"
							"\x16\x17\x18\x19\x1A"
							"\x1B\x1C\x1D\x1E\x1F"
							"\x20",
						.len = 36
					},
				}
			},
			{
				.outcome_parms = {
					.present = {
						.receipt = true
					},
					.outcome = out_approved,
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
				PDOL_D102D2019F6604,
				.app_prio = 1
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
				.present = {
					.app_label = true,
					.app_prio  = true,
					.kernel_id = true,
				},
				AID_A0000000020002,
				APP_LABEL_APP2,
				KERNEL_ID_22,
				.app_prio = 1,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				PDOL_D102D2017110,
				.app_prio = 1
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
						/* FIXME */
						.data = "\x71\x10\x86\x0E\x13"
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
	/* LTsetting1.13 */
	{
		.ppse_entries = {
			{
				.present = {
					.app_label = true,
					.app_prio  = true,
					.kernel_id = true,
				},
				AID_A0000000040004,
				APP_LABEL_APP4,
				KERNEL_ID_24,
				.app_prio = 1,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				PDOL_D102D20191107110,
				.app_prio = 1
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
							"\x86\x0E\x13\x14\x15"
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
					.present = {
						.receipt = true
					},
					.outcome = out_approved,
					.cvm = cvm_obtain_signature,
				}
			},
		},
		.gpo_resp_num = 2
	},
	/* LTsetting1.15 */
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
				PDOL_9F6604,
				.app_prio = 1
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
				PDOL_D102D2019F6604,
				.app_prio = 1
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
	/* LTsetting1.18 */
	{
		.ppse_entries = {
			{
				.present = {
					.app_label = true,
					.app_prio  = true,
					.kernel_id = true,
				},
				AID_A0000000020002,
				APP_LABEL_APP2,
				KERNEL_ID_22,
				.app_prio = 1,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				.app_prio = 1
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
	/* LTsetting1.20 */
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
				PDOL_D102D2019F66049110,
				.app_prio = 1
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
				.present = {
					.app_label = true,
					.app_prio  = true,
					.kernel_id = true,
				},
				AID_A0000000020002,
				APP_LABEL_APP2,
				KERNEL_ID_22,
				.app_prio = 1,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				PDOL_D102D201,
				.app_prio = 1
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
				.present = {
					.app_label = true,
					.app_prio  = true,
					.kernel_id = true,
				},
				AID_A0000000030003,
				APP_LABEL_APP3,
				KERNEL_ID_21,
				.app_prio = 1,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				PDOL_D102D2017210,
				.app_prio = 1
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
						.data = "\x72\x10\x86\x0E\x23"
							"\x24\x25\x26\x27\x28"
							"\x29\x2A\x2B\x2C\x2D"
							"\x2E\x2F\x30",
						.len = 18
					}
				}
			},
			{
				.outcome_parms = {
					.present = {
						.receipt = true
					},
					.outcome = out_approved,
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
				.present = {
					.app_label = true,
					.app_prio  = true,
					.kernel_id = true,
				},
				AID_A0000000040004,
				APP_LABEL_APP4,
				KERNEL_ID_24,
				.app_prio = 1,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				PDOL_D102D201,
				.app_prio = 1
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
				.present = {
					.app_label = true,
					.app_prio  = true,
					.kernel_id = true,
				},
				AID_A0000000030003,
				APP_LABEL_APP3,
				KERNEL_ID_21,
				.app_prio = 1,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				.app_prio = 1
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
				.present = {
					.app_label = true,
					.app_prio  = true,
					.kernel_id = true,
				},
				AID_A0000000020002,
				APP_LABEL_APP2,
				KERNEL_ID_22,
				.app_prio = 1,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				PDOL_D102D201,
				.app_prio = 1
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
						/* FIXME */
						.data = "\x71\x10\x86\x0E\x13"
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
	/* LTsetting1.27 */
	{
		.ppse_entries = {
			{
				.present = {
					.app_label = true,
					.app_prio  = true,
					.kernel_id = true,
				},
				AID_A0000000030003,
				APP_LABEL_APP3,
				KERNEL_ID_21,
				.app_prio = 1,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				PDOL_D102D20191107210,
				.app_prio = 1
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
							"\x86\x0E\x23\x24\x25"
							"\x26\x27\x28\x29\x2A"
							"\x2B\x2C\x2D\x2E\x2F"
							"\x30",
						.len = 36
					},
				}
			},
			{
				.outcome_parms = {
					.present = {
						.receipt = true
					},
					.outcome = out_approved,
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
				.present = {
					.app_label = true,
					.app_prio  = true,
					.kernel_id = true,
				},
				AID_A0000000030003,
				APP_LABEL_APP3,
				KERNEL_ID_21,
				.app_prio = 1,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				.app_prio = 1
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
				.present = {
					.app_label = true,
					.app_prio  = true,
					.kernel_id = true,
				},
				AID_A0000000040004,
				APP_LABEL_APP4,
				KERNEL_ID_24,
				.app_prio = 1,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				PDOL_D102D201,
				.app_prio = 1
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
							"\x86\x0E\x23\x24\x25"
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
	/* LTsetting1.34 */
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
				.app_prio = 1
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				PDOL_D102D2019F6604,
				.app_prio = 1
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
				.present = {
					.app_label = true,
					.app_prio  = true,
					.kernel_id = true,
				},
				AID_A0000000020002,
				APP_LABEL_APP2,
				KERNEL_ID_22,
				.app_prio = 1
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				PDOL_D102D201,
				.app_prio = 1
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
	/* LTsetting1.60 */
	{
		.ppse_entries = {
			{
				.present = {
					.app_label = true,
					.app_prio  = true,
					.kernel_id = true,
				},
				AID_A0000000030003,
				APP_LABEL_APP3,
				KERNEL_ID_21,
				.app_prio = 1,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				.app_prio = 1
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
				.present = {
					.app_label = true,
					.app_prio  = true,
					.kernel_id = true,
				},
				AID_A0000000020002,
				APP_LABEL_APP2,
				KERNEL_ID_22,
				.app_prio = 1,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				.app_prio = 1
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
				PDOL_9F6604,
				.app_prio = 1
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
				PDOL_9F6604,
				.app_prio = 1,
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
				.present = {
					.app_label = true,
					.app_prio  = true,
					.kernel_id = true,
				},
				AID_A0000000040004,
				APP_LABEL_APP4,
				KERNEL_ID_24,
				.app_prio = 1,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				.app_prio = 1,
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
				PDOL_9F6604,
				.app_prio = 1,
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
				.present = {
					.app_label = true,
					.app_prio  = true,
					.kernel_id = true,
				},
				AID_A0000000040004,
				APP_LABEL_APP4,
				KERNEL_ID_24,
				.app_prio = 1,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				.app_prio = 1,
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
				.present = {
					.app_label = true,
					.app_prio  = true,
					.kernel_id = true,
				},
				AID_A0000000030003,
				APP_LABEL_APP3,
				KERNEL_ID_21,
				.app_prio = 1,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				.app_prio = 1
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
				.present = {
					.app_label = true,
					.app_prio  = true,
					.kernel_id = true,
				},
				AID_A0000000020002,
				APP_LABEL_APP2,
				KERNEL_ID_22,
				.app_prio = 1,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				.app_prio = 1
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
				.present = {
					.app_label = true,
					.app_prio  = true,
					.kernel_id = true,
				},
				AID_A0000000030003,
				APP_LABEL_APP3,
				KERNEL_ID_21,
				.app_prio = 1,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				.app_prio = 1
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
	/* LTsetting1.90 */
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
				PDOL_9F01069F40059F06079F0902___9F21039C01,
				.app_prio = 1
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
				.present = {
					.app_label = true,
					.app_prio  = true,
					.kernel_id = true,
				},
				AID_A0000000030003,
				APP_LABEL_APP3,
				KERNEL_ID_21,
				.app_prio = 1,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				PDOL_9F01069F40059F06079F0902___9F21039C01,
				.app_prio = 1
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
				.present = {
					.app_label = true,
					.app_prio  = true,
					.kernel_id = true,
				},
				AID_A0000000030003,
				APP_LABEL_APP3,
				KERNEL_ID_25,
				.app_prio = 1,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				PDOL_D1029F66049F02069F03069C019F37049F2A08,
				.app_prio = 1
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
				.present = {
					.app_label = true,
					.app_prio  = true,
					.kernel_id = true,
				},
				AID_A0000000030003,
				APP_LABEL_APP3,
				KERNEL_ID_25,
				.app_prio = 1,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				PDOL_9F01069F40059F06079F0902___9F21039C01,
				.app_prio = 1
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
				.present = {
					.app_label = true,
					.app_prio  = true,
					.kernel_id = true,
				},
				AID_A0000000040004,
				APP_LABEL_APP4,
				KERNEL_ID_25,
				.app_prio = 1,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				PDOL_D102D20191107210,
				.app_prio = 1
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
							"\x86\x0E\x13\x14\x15"
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
					.present = {
						.receipt = true
					},
					.outcome = out_approved,
					.cvm = cvm_obtain_signature,
				}
			},
		},
		.gpo_resp_num = 2
	},
	/* LTsettin2.10 */
	{
		.ppse_entries = {
			{
				.present = {
					.app_label = true,
					.app_prio  = true,
					.kernel_id = true,
				},
				AID_A0000000030003,
				APP_LABEL_APP3,
				KERNEL_ID_21,
				.app_prio = 1,
			},
			{
				.present = {
					.app_label = true,
					.app_prio  = true,
					.kernel_id = true,
				},
				AID_A0000000020002,
				APP_LABEL_APP2,
				KERNEL_ID_24,
				.app_prio = 2,
			},
			{
				.present = {
					.app_label = true,
					.app_prio  = true,
					.kernel_id = true,
				},
				AID_A0000000010001,
				APP_LABEL_APP1,
				KERNEL_ID_23,
				.app_prio = 3,
			},
			{
				.present = {
					.app_label = true,
					.app_prio  = true,
					.kernel_id = true,
				},
				AID_A0000000040004,
				APP_LABEL_APP4,
				KERNEL_ID_24,
				.app_prio = 4,
			}
		},
		.ppse_entries_num = 4,
		.aid_fci = {
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				PDOL_D1029F66049F2A086F23,
				.app_prio = 1
			},
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				PDOL_D1029F66049F2A086F23,
				.app_prio = 2
			},
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				PDOL_D1029F66049F2A086F23,
				.app_prio = 3
			},
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				PDOL_D1029F66049F2A086F23,
				.app_prio = 4
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
	/* LTsetting2.40 */
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
			},
			{
				.present = {
					.app_label = true,
					.app_prio  = true,
					.kernel_id = true,
				},
				AID_A0000000030003,
				APP_LABEL_APP3,
				KERNEL_ID_21,
				.app_prio = 2,
			},
			{
				.present = {
					.app_label = true,
					.app_prio  = true,
					.kernel_id = true,
				},
				AID_A0000000020002,
				APP_LABEL_APP3,
				KERNEL_ID_22,
				.app_prio = 3,
			}
		},
		.ppse_entries_num = 3,
		.aid_fci = {
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				PDOL_9F66049F2A089F3704,
				.app_prio = 1
			},
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				PDOL_9F66049F2A089F3704,
				.app_prio = 2
			},
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				PDOL_9F66049F2A089F3704,
				.app_prio = 3
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
	/* LTsetting6.10 */
	{
		.ppse_entries = {
			{
				.present = {
					.app_label = true,
					.app_prio  = true,
					.kernel_id = true,
				},
				AID_A0000000020002,
				APP_LABEL_APP2,
				KERNEL_ID_24FFFF,
				.app_prio = 0xF2,
			},
			{
				.present = {
					.app_label = true,
					.app_prio  = true,
					.kernel_id = true,
				},
				AID_A0000000040004,
				APP_LABEL_APP4,
				KERNEL_ID_240000,
				.app_prio = 4,
			},
			{
				.present = {
					.app_label = true,
					.app_prio  = true,
					.kernel_id = true,
				},
				AID_A0000000010001,
				APP_LABEL_APP1,
				KERNEL_ID_23FF00FF00FF00FF,
				.app_prio = 0xFF,
			},
			{
				.present = {
					.app_label = true,
					.app_prio  = true,
					.kernel_id = true,
				},
				AID_A0000000030003,
				APP_LABEL_APP3,
				KERNEL_ID_2102030405060708,
				.app_prio = 0x41,
			}
		},
		.ppse_entries_num = 4,
		.aid_fci = {
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				PDOL_9F66049F2A08,
				.app_prio = 0xF2
			},
			{
				AID_A0000000040004,
				APP_LABEL_APP4,
				PDOL_9F66049F2A08,
				.app_prio = 4
			},
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				PDOL_9F66049F2A08,
				.app_prio = 0xFF
			},
			{
				AID_A0000000030003,
				APP_LABEL_APP3,
				PDOL_9F66049F2A08,
				.app_prio = 0x41
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
				.present = {
					.app_label = true,
					.app_prio  = true,
					.kernel_id = true,
				},
				AID_A0000000010001,
				APP_LABEL_APP1,
				KERNEL_ID_23,
				.app_prio = 1
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000010001,
				APP_LABEL_APP1,
				PDOL_9F66049F02069F03069F3704DF0101C102__9F2502,
				.app_prio = 1,
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
	/* LTsetting8.0 */
	{
		.ppse_entries = {
			{
				.present = {
					.app_label = true,
					.app_prio  = true,
					.kernel_id = true,
				},
				AID_A0000000020002,
				APP_LABEL_APP2,
				KERNEL_ID_22,
				.app_prio = 1,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				.app_prio = 1
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
				.present = {
					.app_label = true,
					.app_prio  = true,
					.kernel_id = true,
				},
				AID_A0000000020002,
				APP_LABEL_APP2,
				KERNEL_ID_22,
				.app_prio = 1,
			}
		},
		.ppse_entries_num = 1,
		.aid_fci = {
			{
				AID_A0000000020002,
				APP_LABEL_APP2,
				.app_prio = 1
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

		rc = tlv_parse(resp->outcome_parms.data_record.data,
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
	const struct lt_setting	 *setting;
	int                       selected_aid;
	int			  i_gpo_resp;
	struct chk		 *checker;
	log4c_category_t	 *log_cat;
};

uint32_t lt_get_unpredictable_number(struct emv_hal *hal)
{
	return (uint32_t)random();
}

int lt_field_on(struct emv_hal *hal)
{
	struct lt *lt = (struct lt *)hal;

	if (lt->checker && lt->checker->ops->field_on)
		lt->checker->ops->field_on(lt->checker);

	log4c_category_log(lt->log_cat, LOG4C_PRIORITY_TRACE,
						     "%s(): success", __func__);
	return EMV_RC_OK;
}

int lt_field_off(struct emv_hal *hal)
{
	struct lt *lt = (struct lt *)hal;

	if (lt->checker && lt->checker->ops->field_off)
		lt->checker->ops->field_off(lt->checker);

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

	if (lt->checker && lt->checker->ops->select)
		lt->checker->ops->select(lt->checker, data, lc);

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

			lt->selected_aid = i_aid;

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
	const struct gpo_resp *gpo_resp = NULL;
	int rc = EMV_RC_OK;
	char hex[lc * 2 + 1];

	log4c_category_log(lt->log_cat, LOG4C_PRIORITY_TRACE,
						"%s(PDOL data: '%s')", __func__,
					      libtlv_bin_to_hex(data, lc, hex));

	memcpy(sw, EMV_SW_9000_OK, 2);

	gpo_resp = &lt->setting->gpo_resp[lt->i_gpo_resp++];

	rc = ber_get_gpo_resp(gpo_resp, resp, le);
	if (rc != EMV_RC_OK) {
		log4c_category_log(lt->log_cat, LOG4C_PRIORITY_ERROR,
			   "%s() ber_get_gpo_resp failed. rc %d", __func__, rc);
		goto done;
	}

	if (!lc)
		goto done;

	if (lt->checker && lt->checker->ops->gpo_data) {
		struct tlv *tlv = NULL;
		uint8_t ber_tlv[2048];
		size_t ber_tlv_len = sizeof(ber_tlv);
		char ber_tlv_hex[4096];

		rc = dol_and_del_to_tlv(
				    lt->setting->aid_fci[lt->selected_aid].pdol,
				lt->setting->aid_fci[lt->selected_aid].pdol_len,
								data, lc, &tlv);
		if (rc != EMV_RC_OK) {
			log4c_category_log(lt->log_cat, LOG4C_PRIORITY_ERROR,
					"%s() dol_and_del_to_tlv failed. rc %d",
								  __func__, rc);
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

		lt->checker->ops->gpo_data(lt->checker, tlv);

		tlv_free(tlv);
	}

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
	char hex[capdu_sz * 2 + 1];
	size_t resp_sz = sizeof(resp);
	int i = 0;
	int rc = EMV_RC_OK;

	log4c_category_log(lt->log_cat, LOG4C_PRIORITY_TRACE, "%s(capdu: '%s')",
			     __func__, libtlv_bin_to_hex(capdu, capdu_sz, hex));

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

void lt_ui_request(struct emv_hal *hal, const struct emv_ui_request *ui_request)
{
	struct lt *lt = (struct lt *)hal;

	if (lt->checker && lt->checker->ops->ui_request)
		lt->checker->ops->ui_request(lt->checker, ui_request);
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

struct emv_hal *lt_new(enum ltsetting i_lts, struct chk *checker,
						     const char *log4c_category)
{
	struct lt *lt = NULL;
	char cat[64];

	if (i_lts >= num_ltsettings)
		return NULL;

	lt = malloc(sizeof(struct lt));
	if (!lt)
		return NULL;

	memset(lt, 0, sizeof(*lt));

	lt->ops = &lt_ops;

	snprintf(cat, sizeof(cat), "%s.lt", log4c_category);
	lt->log_cat = log4c_category_get(cat);
	lt->setting = &ltsetting[i_lts];
	lt->checker = checker;

	return (struct emv_hal *)lt;
}

void lt_free(struct emv_hal *lt)
{
	if (lt)
		free(lt);
}
