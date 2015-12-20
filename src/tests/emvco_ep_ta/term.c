#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <tlv.h>
#include <emv.h>

#include "emvco_ep_ta.h"

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
	struct emv_ep_aid_kernel combinations[54];
	struct emv_ep_config config;
};

struct emv_ep_combination termset1[] = {
	{
		.txn_types = {
			txn_purchase,
			txn_purchase_with_cashback,
			txn_cash_advance,
			txn_refund
		},
		.combinations = {
			{ AID_A0000000031010, KERNEL_ID_TK1	},
			{ AID_A0000000041010, KERNEL_ID_TK1	},
			{ AID_A0000000651010, KERNEL_ID_TK1	},
			{ AID_A0000000251010, KERNEL_ID_TK1	},
			{ AID_A0000000041010, KERNEL_ID_TK2	},
			{ AID_A0000000651010, KERNEL_ID_TK2	},
			{ AID_A0000000031010, KERNEL_ID_TK3	},
			{ AID_A0000000041010, KERNEL_ID_TK3	},
			{ AID_A0000000651010, KERNEL_ID_TK3	},
			{ AID_A0000000251010, KERNEL_ID_TK3	},
			{ AID_A0000000041010, KERNEL_ID_TK4	},
			{ AID_A0000000651010, KERNEL_ID_TK4	},
			{ AID_A0000000251010, KERNEL_ID_TK4	},
			{ AID_A0000000651010, KERNEL_ID_TK5	},
			{ AID_A0000000251010, KERNEL_ID_TK5	},
			{ AID_A0000001523010, KERNEL_ID_TK6	},
			{ AID_A0000003241010, KERNEL_ID_TK6	},
			{ AID_A0000003330101, KERNEL_ID_TK7	},
			{ AID_A0000000010001, KERNEL_ID_21	},
			{ AID_A0000000020002, KERNEL_ID_21	},
			{ AID_A0000000030003, KERNEL_ID_21	},
			{ AID_A0000000040004, KERNEL_ID_21	},
			{ AID_A0000000020002, KERNEL_ID_22	},
			{ AID_A0000000030003, KERNEL_ID_22	},
			{ AID_A0000000010001, KERNEL_ID_23	},
			{ AID_A0000000020002, KERNEL_ID_23	},
			{ AID_A0000000030003, KERNEL_ID_23	},
			{ AID_A0000000040004, KERNEL_ID_23	},
			{ AID_A0000000020002, KERNEL_ID_24	},
			{ AID_A0000000030003, KERNEL_ID_24	},
			{ AID_A0000000040004, KERNEL_ID_24	},
			{ AID_A0000000030003, KERNEL_ID_25	},
			{ AID_A0000000040004, KERNEL_ID_25	},
			{ AID_A0000000010001, KERNEL_ID_32	},
			{ AID_A0000000020002, KERNEL_ID_32	},
			{ AID_A0000000030003, KERNEL_ID_32	},
			{ AID_A0000000040004, KERNEL_ID_32	},
			{ AID_A0000000010001, KERNEL_ID_810978	},
			{ AID_A0000000020002, KERNEL_ID_810978	},
			{ AID_A0000000030003, KERNEL_ID_810978	},
			{ AID_A0000000040004, KERNEL_ID_810978	},
			{ AID_A0000000010001, KERNEL_ID_BF0840	},
			{ AID_A0000000020002, KERNEL_ID_BF0840	},
			{ AID_A0000000030003, KERNEL_ID_BF0840	},
			{ AID_A0000000040004, KERNEL_ID_BF0840	},
			{ AID_A0000000010001, KERNEL_ID_C11111	},
			{ AID_A0000000020002, KERNEL_ID_C11111	},
			{ AID_A0000000030003, KERNEL_ID_C11111	},
			{ AID_A0000000040004, KERNEL_ID_C11111	},
			{ AID_A0000000010001, KERNEL_ID_FF2222	},
			{ AID_A0000000020002, KERNEL_ID_FF2222	},
			{ AID_A0000000030003, KERNEL_ID_FF2222	},
			{ AID_A0000000040004, KERNEL_ID_FF2222	}
		},
		.config = {
			.present = {
				.status_check_support = true,
				.zero_amount_allowed = true,
				.reader_ctls_txn_limit = true,
				.reader_ctls_floor_limit = true,
				.reader_cvm_reqd_limit = true,
				.ttq = true,
				.ext_selection_support = true,
			},
			.enabled = {
				.status_check_support = true,
				.zero_amount_allowed = true,
				.ext_selection_support = true,
			},
			.reader_ctls_txn_limit = 120,
			.reader_ctls_floor_limit = 12,
			.reader_cvm_reqd_limit = 15,
			TTQ_B6208000
		}
	}
};

struct emv_ep_combination termset2[] = {
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
			TTQ_84008000
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
			TTQ_28000000
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
			TTQ_84C08000
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
			TTQ_84008000
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
			TTQ_28000000
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

struct emv_ep_combination termset3[] = {
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
			TTQ_84008000
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
			TTQ_28000000
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
			TTQ_84C08000
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
			TTQ_84008000
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
			TTQ_28000000
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

struct emv_ep_combination termset4[] = {
	{
		.txn_types = {
			txn_purchase
		},
		.combinations = {
			{ AID_A0000000030003, KERNEL_ID_21  }
		},
		.config = {
			.present = {
				.zero_amount_allowed = true,
				.reader_ctls_floor_limit = true,
				.ttq = true,
				.ext_selection_support = true,
			},
			.enabled = {
				.zero_amount_allowed = true,
				.ext_selection_support = true,
			},
			.reader_ctls_floor_limit = 20,
			TTQ_84008000
		}
	},
	{
		.txn_types = {
			txn_purchase
		},
		.combinations = {
			{ AID_A0000000020002, KERNEL_ID_22  },
			{ AID_A0000000030003, KERNEL_ID_25  }
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
			txn_purchase
		},
		.combinations = {
			{ AID_A0000000010001, KERNEL_ID_23  }
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
			TTQ_28000000
		}
	},
	{
		.txn_types = {
			txn_purchase
		},
		.combinations = {
			{ AID_A0000000040004, KERNEL_ID_24  }
		},
		.config = {
			.present = {
				.status_check_support = true,
				.zero_amount_allowed = true,
				.reader_ctls_txn_limit = true,
				.reader_cvm_reqd_limit = true,
				.ttq = true,
				.ext_selection_support = true,
			},
			.enabled = {
				.ext_selection_support = true,
			},
			.reader_ctls_txn_limit = 120,
			.reader_cvm_reqd_limit = 20,
			TTQ_84C08000
		}
	},
	{
		.txn_types = {
			txn_refund
		},
		.combinations = {
			{ AID_A0000000020002, KERNEL_ID_22  }
		},
		.config = {
			.present = {
				.zero_amount_allowed = true,
				.reader_ctls_txn_limit = true,
				.reader_ctls_floor_limit = true,
				.ttq = true,
				.ext_selection_support = true
			},
			.enabled = {
				.zero_amount_allowed = true,
				.ext_selection_support = true
			},
			.reader_ctls_txn_limit = 120,
			.reader_ctls_floor_limit = 20,
			TTQ_84008000
		}
	},
	{
		.txn_types = {
			txn_cash_advance
		},
		.combinations = {
			{ AID_A0000000010001, KERNEL_ID_23  },
			{ AID_A0000000030003, KERNEL_ID_25  }
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
			txn_purchase_with_cashback
		},
		.combinations = {
			{ AID_A0000000030003, KERNEL_ID_21  }
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
			TTQ_28000000
		}
	},
	{
		.txn_types = {
			txn_refund
		},
		.combinations = {
			{ AID_A0000000040004, KERNEL_ID_24  }
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
			txn_refund
		},
		.combinations = {
			{ AID_A0000000030003, KERNEL_ID_21  },
		},
		.config = {
			.present = {
				.reader_ctls_floor_limit = true,
				.ttq = true,
				.ext_selection_support = true
			},
			.enabled = {
				.ext_selection_support = true
			},
			.reader_ctls_floor_limit = 50,
			TTQ_84008000
		}
	}
};

struct emv_ep_combination termset8[] = {
	{
		.txn_types = {
			txn_purchase,
			txn_purchase_with_cashback,
			txn_cash_advance,
			txn_refund
		},
		.combinations = {
			{ AID_A0000000010001, KERNEL_ID_21	},
			{ AID_A0000000020002, KERNEL_ID_21	},
			{ AID_A0000000030003, KERNEL_ID_21	},
			{ AID_A0000000040004, KERNEL_ID_21	},
			{ AID_B0000000010101, KERNEL_ID_21	},
			{ AID_A0000000020002, KERNEL_ID_22	},
			{ AID_A0000000030003, KERNEL_ID_22	},
			{ AID_A0000000010001, KERNEL_ID_23	},
			{ AID_A0000000020002, KERNEL_ID_23	},
			{ AID_A0000000030003, KERNEL_ID_23	},
			{ AID_A0000000040004, KERNEL_ID_23	},
			{ AID_B0000000010101, KERNEL_ID_23	},
			{ AID_A0000000020002, KERNEL_ID_24	},
			{ AID_A0000000030003, KERNEL_ID_24	},
			{ AID_A0000000040004, KERNEL_ID_24	},
			{ AID_B0000000010101, KERNEL_ID_24	},
			{ AID_A0000000030003, KERNEL_ID_25	},
			{ AID_A0000000040004, KERNEL_ID_25	},
			{ AID_B0000000010101, KERNEL_ID_25	},
			{ AID_A0000000010001, KERNEL_ID_32	},
			{ AID_A0000000020002, KERNEL_ID_32	},
			{ AID_A0000000030003, KERNEL_ID_32	},
			{ AID_A0000000040004, KERNEL_ID_32	},
			{ AID_B0000000010101, KERNEL_ID_32	},
			{ AID_A0000000010001, KERNEL_ID_810978	},
			{ AID_A0000000020002, KERNEL_ID_810978	},
			{ AID_A0000000030003, KERNEL_ID_810978	},
			{ AID_A0000000040004, KERNEL_ID_810978	},
			{ AID_B0000000010101, KERNEL_ID_810978	},
			{ AID_A0000000010001, KERNEL_ID_BF0840	},
			{ AID_A0000000020002, KERNEL_ID_BF0840	},
			{ AID_A0000000030003, KERNEL_ID_BF0840	},
			{ AID_A0000000040004, KERNEL_ID_BF0840	},
			{ AID_B0000000010101, KERNEL_ID_BF0840	},
			{ AID_A0000000010001, KERNEL_ID_811111	},
			{ AID_A0000000020002, KERNEL_ID_811111	},
			{ AID_A0000000030003, KERNEL_ID_811111	},
			{ AID_A0000000040004, KERNEL_ID_811111	},
			{ AID_B0000000010101, KERNEL_ID_811111	},
			{ AID_A0000000010001, KERNEL_ID_BF2222	},
			{ AID_A0000000020002, KERNEL_ID_BF2222	},
			{ AID_A0000000030003, KERNEL_ID_BF2222	},
			{ AID_A0000000040004, KERNEL_ID_BF2222	},
			{ AID_B0000000010101, KERNEL_ID_BF2222	},
		},
		.config = {
			.present = {
				.status_check_support = true,
				.zero_amount_allowed = true,
				.reader_ctls_txn_limit = true,
				.reader_ctls_floor_limit = true,
				.reader_cvm_reqd_limit = true,
				.ttq = true,
				.ext_selection_support = true,
			},
			.enabled = {
				.status_check_support = true,
				.zero_amount_allowed = true,
				.ext_selection_support = true,
			},
			.reader_ctls_txn_limit = 120,
			.reader_ctls_floor_limit = 12,
			.reader_cvm_reqd_limit = 15,
			TTQ_F72A8A55
		}
	}
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

struct emv_ep_terminal_data terminal_data = {
	.acquirer_identifier		  = ACQUIRER_IDENTIFIER,
	.merchant_category_code		  = MERCHANT_CATEGORY_CODE,
	.merchant_identifier		  = MERCHANT_IDENTIFIER,
	.terminal_country_code		  = TERMINAL_COUNTRY_CODE,
	.terminal_identification	  = TERMINAL_IDENTIFICATION,
	.terminal_type			  = TERMINAL_TYPE,
	.pos_entry_mode			  = POS_ENTRY_MODE,
	.terminal_capabilities		  = TERMINAL_CAPABILITIES,
	.additional_terminal_capabilities = ADDITIONAL_TERMINAL_CAPABILITIES,
	.merchant_name_and_location	  = MERCHANT_NAME_AND_LOCATION
};

struct termset termsettings[num_termsettings] = {
	{
		.combination_sets	= termset1,
		.num_combination_sets	= ARRAY_SIZE(termset1),
		.terminal_data		= &terminal_data
	},
	{
		.combination_sets	= termset2,
		.num_combination_sets	= ARRAY_SIZE(termset2),
		.terminal_data		= &terminal_data
	},
	{
		.combination_sets	= termset3,
		.num_combination_sets	= ARRAY_SIZE(termset3),
		.terminal_data		= &terminal_data,
		.autorun = {
			.enabled	   = true,
			.txn_type	   = txn_purchase,
			.amount_authorized = 10
		}
	},
	{
		.combination_sets	= termset4,
		.num_combination_sets	= ARRAY_SIZE(termset4),
		.terminal_data		= &terminal_data
	},
	{
		.combination_sets	= termset8,
		.num_combination_sets	= ARRAY_SIZE(termset8),
		.terminal_data		= &terminal_data
	}
};

static struct tlv *get_combinations(struct emv_ep_aid_kernel *aid_kernel)
{
	struct tlv *tlv_combinations = NULL;

	for (; aid_kernel; aid_kernel++) {
		struct tlv *tlv = NULL, *tail = NULL;

		if (!aid_kernel->aid_len)
			break;

		tlv = tlv_new(EMV_ID_LIBEMV_COMBINATION, 0, NULL);
		tail = tlv_insert_below(tlv, tlv_new(EMV_ID_LIBEMV_AID,
					 aid_kernel->aid_len, aid_kernel->aid));
		tail = tlv_insert_after(tail, tlv_new(EMV_ID_LIBEMV_KERNEL_ID,
			     aid_kernel->kernel_id_len, aid_kernel->kernel_id));

		if (!tlv_combinations)
			tlv_combinations = tlv;
		else
			tlv_insert_after(tlv_combinations, tlv);
	}

	return tlv_combinations;
}

static uint8_t get_txn_type(enum emv_txn_type txn_type)
{
	switch (txn_type) {
	case txn_purchase:
		return 0x00;
	case txn_purchase_with_cashback:
		return 0x09;
	case txn_cash_advance:
		return 0x01;
	case txn_refund:
		return 0x20;
	default:
		assert(false);
		return 0xff;
	}
}

static struct tlv *get_txn_types(enum emv_txn_type *txn_types)
{
	size_t len, i;
	uint8_t value[4];

	for (len = 0; len < num_txn_types; len++)
		if (txn_types[len] == num_txn_types)
			break;

	for (i = 0; i < len; i++)
		value[i] = get_txn_type(txn_types[i]);

	return tlv_new(EMV_ID_LIBEMV_TRANSACTION_TYPES, len, value);
}

static struct tlv *get_combination_set(struct emv_ep_combination *comb)
{
	struct tlv *tlv = NULL, *tail = NULL;

	tlv = tlv_new(EMV_ID_LIBEMV_COMBINATION_SET, 0, NULL);

	tail = tlv_insert_below(tlv, get_txn_types(comb->txn_types));

	tail = tlv_insert_after(tail, get_combinations(comb->combinations));

	while (tlv_get_next(tail))
		tail = tlv_get_next(tail);

	if (comb->config.present.status_check_support) {
		uint8_t enabled = comb->config.enabled.status_check_support;

		tail = tlv_insert_after(tail, tlv_new(
			    EMV_ID_LIBEMV_STATUS_CHECK_SUPPORTED, 1, &enabled));
	}

	if (comb->config.present.zero_amount_allowed) {
		uint8_t enabled = comb->config.enabled.zero_amount_allowed;

		tail = tlv_insert_after(tail, tlv_new(
			       EMV_ID_LIBEMV_ZERO_AMOUNT_ALLOWED, 1, &enabled));
	}

	if (comb->config.present.ext_selection_support) {
		uint8_t enabled = comb->config.enabled.ext_selection_support;

		tail = tlv_insert_after(tail, tlv_new(
			   EMV_ID_LIBEMV_EXT_SELECTION_SUPPORTED, 1, &enabled));
	}

	if (comb->config.present.reader_ctls_txn_limit) {
		uint8_t amount[6];
		int rc;

		rc = libtlv_u64_to_bcd(comb->config.reader_ctls_txn_limit,
							amount, sizeof(amount));
		if (rc != EMV_RC_OK)
			goto error;

		tail = tlv_insert_after(tail, tlv_new(
					       EMV_ID_LIBEMV_RDR_CTLS_TXN_LIMIT,
						       sizeof(amount), amount));
	}

	if (comb->config.present.reader_ctls_floor_limit) {
		uint8_t amount[6];
		int rc;

		rc = libtlv_u64_to_bcd(comb->config.reader_ctls_floor_limit,
							amount, sizeof(amount));
		if (rc != EMV_RC_OK)
			goto error;

		tail = tlv_insert_after(tail, tlv_new(
					     EMV_ID_LIBEMV_RDR_CTLS_FLOOR_LIMIT,
						       sizeof(amount), amount));
	}

	if (comb->config.present.terminal_floor_limit) {
		uint8_t amount[6];
		int rc;

		rc = libtlv_u64_to_bcd(comb->config.terminal_floor_limit,
							amount, sizeof(amount));
		if (rc != EMV_RC_OK)
			goto error;

		tail = tlv_insert_after(tail, tlv_new(
					     EMV_ID_LIBEMV_TERMINAL_FLOOR_LIMIT,
						       sizeof(amount), amount));
	}

	if (comb->config.present.reader_cvm_reqd_limit) {
		uint8_t amount[6];
		int rc;

		rc = libtlv_u64_to_bcd(comb->config.reader_cvm_reqd_limit,
							amount, sizeof(amount));
		if (rc != EMV_RC_OK)
			goto error;

		tail = tlv_insert_after(tail, tlv_new(
					   EMV_ID_LIBEMV_RDR_CVM_REQUIRED_LIMIT,
						       sizeof(amount), amount));
	}

	if (comb->config.present.ttq)
		tail = tlv_insert_after(tail, tlv_new(EMV_ID_LIBEMV_TTQ,
				   sizeof(comb->config.ttq), comb->config.ttq));

	return tlv;

error:
	if (tlv)
		tlv_free(tlv);

	return NULL;
}

static struct tlv *get_autorun(struct emv_ep_autorun *autorun)
{
	struct tlv *tlv = NULL, *tail = NULL;
	uint8_t amount[6], txn_type = get_txn_type(autorun->txn_type);
	int rc = EMV_RC_OK;

	tlv = tlv_new(EMV_ID_LIBEMV_AUTORUN, 0, NULL);

	rc = libtlv_u64_to_bcd(autorun->amount_authorized, amount,
								sizeof(amount));
	if (rc != EMV_RC_OK)
		goto error;

	tail = tlv_insert_below(tlv,
				tlv_new(EMV_ID_LIBEMV_AUTORUN_AMOUNT_AUTHORIZED,
						       sizeof(amount), amount));
	tail = tlv_insert_after(tail,
				 tlv_new(EMV_ID_LIBEMV_AUTORUN_TRANSACTION_TYPE,
						  sizeof(txn_type), &txn_type));
	if (!tail)
		goto error;

	return tlv;

error:
	tlv_free(tlv);
	return NULL;
}

static struct tlv *get_terminal_data(struct emv_ep_terminal_data *data)
{
	struct tlv *term_data = NULL, *tail = NULL;

	term_data = tlv_new(EMV_ID_LIBEMV_TERMINAL_DATA, 0, NULL);

	tail = tlv_insert_below(term_data, tlv_new(EMV_ID_ACQUIRER_IDENTIFIER,
		 sizeof(data->acquirer_identifier), data->acquirer_identifier));
	tail = tlv_insert_after(tail, tlv_new(EMV_ID_MERCHANT_CATEGORY_CODE,
					   sizeof(data->merchant_category_code),
						 data->merchant_category_code));
	tail = tlv_insert_after(tail, tlv_new(EMV_ID_MERCHANT_IDENTIFIER,
		 strlen(data->merchant_identifier), data->merchant_identifier));
	tail = tlv_insert_after(tail, tlv_new(EMV_ID_TERMINAL_COUNTRY_CODE,
					    sizeof(data->terminal_country_code),
						  data->terminal_country_code));
	tail = tlv_insert_after(tail, tlv_new(EMV_ID_TERMINAL_IDENTIFICATION,
					  sizeof(data->terminal_identification),
						data->terminal_identification));
	tail = tlv_insert_after(tail, tlv_new(EMV_ID_TERMINAL_TYPE,
			    sizeof(data->terminal_type), &data->terminal_type));
	tail = tlv_insert_after(tail, tlv_new(EMV_ID_POS_ENTRY_MODE,
			  sizeof(data->pos_entry_mode), &data->pos_entry_mode));
	tail = tlv_insert_after(tail,
				tlv_new(EMV_ID_ADDITIONAL_TERMINAL_CAPABILITIES,
				 sizeof(data->additional_terminal_capabilities),
				       data->additional_terminal_capabilities));
	tail = tlv_insert_after(tail, tlv_new(EMV_ID_TERMINAL_CAPABILITIES,
					    sizeof(data->terminal_capabilities),
						  data->terminal_capabilities));
	tail = tlv_insert_after(tail, tlv_new(EMV_ID_MERCHANT_NAME_AND_LOCATION,
				       strlen(data->merchant_name_and_location),
					     data->merchant_name_and_location));
	if (!tail) {
		tlv_free(term_data);
		term_data = NULL;
	}

	return term_data;
};

int term_get_setting(enum termsetting termsetting, void *buffer, size_t *size)
{
	struct tlv *tlv = NULL, *tail = NULL;
	struct termset *settings = NULL;
	int i = 0, rc = TLV_RC_OK;

	if ((termsetting >= num_termsettings) || !buffer || !size) {
		rc = TLV_RC_INVALID_ARG;
		goto done;
	}

	settings = &termsettings[termsetting];

	tlv  = tlv_new(EMV_ID_LIBEMV_CONFIGURATION, 0, NULL);
	if (!tlv) {
		rc = TLV_RC_OUT_OF_MEMORY;
		goto done;
	}

	tail = tlv_insert_below(tlv,
			   get_combination_set(&settings->combination_sets[0]));

	for (i = 1; i < settings->num_combination_sets; i++)
		tail = tlv_insert_after(tail,
			   get_combination_set(&settings->combination_sets[i]));

	if (settings->autorun.enabled)
		tail = tlv_insert_after(tail, get_autorun(&settings->autorun));

	if (settings->terminal_data)
		tail = tlv_insert_after(tail,
				    get_terminal_data(settings->terminal_data));
	if (!tail) {
		rc = TLV_RC_OUT_OF_MEMORY;
		goto done;
	}

	rc = tlv_encode(tlv, buffer, size);
#if 0
	if (rc == TLV_RC_OK) {
		char hex[2 * *size + 1];

		printf("CONFIG: '%s'\n", libtlv_bin_to_hex(buffer, *size, hex));
	}
#endif
done:
	tlv_free(tlv);

	return rc;
}
