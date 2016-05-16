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

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "emvco_ep_ta.h"

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
			{ AID_A0000000010001, KERNEL_ID_C11111	},
			{ AID_A0000000020002, KERNEL_ID_C11111	},
			{ AID_A0000000030003, KERNEL_ID_C11111	},
			{ AID_A0000000040004, KERNEL_ID_C11111	},
			{ AID_B0000000010101, KERNEL_ID_C11111	},
			{ AID_A0000000010001, KERNEL_ID_FF2222	},
			{ AID_A0000000020002, KERNEL_ID_FF2222	},
			{ AID_A0000000030003, KERNEL_ID_FF2222	},
			{ AID_A0000000040004, KERNEL_ID_FF2222	},
			{ AID_B0000000010101, KERNEL_ID_FF2222	}
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

struct emv_ep_combination termset4and5[] = {
	{
		.txn_types = {
			txn_purchase,
			num_txn_types
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
			txn_purchase,
			num_txn_types
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
			txn_purchase,
			num_txn_types
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
			txn_purchase,
			num_txn_types
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
			txn_refund,
			num_txn_types
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
			txn_cash_advance,
			num_txn_types
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
			txn_purchase_with_cashback,
			num_txn_types
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
			txn_refund,
			num_txn_types
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
			txn_refund,
			num_txn_types
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

struct emv_ep_combination termset6and7[] = {
	{
		.txn_types = {
			txn_purchase,
			txn_purchase_with_cashback,
			txn_cash_advance,
			txn_refund
		},
		.combinations = {
			{ AID_A0000000041010, KERNEL_ID_TK2	},
			{ AID_A0000000041010, KERNEL_ID_2B	},
		},
		.config = {
			.present = {
				.status_check_support = true,
				.reader_cvm_reqd_limit = true,
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
			{ AID_A0000000031010, KERNEL_ID_TK3	},
			{ AID_A0000000031010, KERNEL_ID_2B	},
		},
		.config = {
			.present = {
				.status_check_support = true,
				.zero_amount_allowed = true,
				.reader_ctls_txn_limit = true,
				.terminal_floor_limit = true,
				.ttq = true,
				.ext_selection_support = true,
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
			{ AID_A0000000651010, KERNEL_ID_TK5	},
			{ AID_A0000000251010, KERNEL_ID_TK4	},
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
			{ AID_A0000000041010, KERNEL_ID_TK4	},
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
			{ AID_A0000000651010, KERNEL_ID_TK2	},
			{ AID_A0000000251010, KERNEL_ID_TK5	},
		},
		.config = {
			.present = {
				.status_check_support = true,
				.zero_amount_allowed = true,
				.reader_ctls_txn_limit = true,
				.terminal_floor_limit = true,
				.ttq = true,
			},
			.enabled = {
				.status_check_support = true,
				.zero_amount_allowed = true,
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
			{ AID_A0000000251010, KERNEL_ID_TK3	},
		},
		.config = {
			.present = {
				.status_check_support = true,
				.zero_amount_allowed = true,
				.reader_ctls_floor_limit = true,
				.reader_cvm_reqd_limit = true,
				.ext_selection_support = true,
			},
			.enabled = {
				.status_check_support = true,
				.zero_amount_allowed = true,
				.ext_selection_support = true,
			},
			.reader_ctls_floor_limit = 50,
			.reader_cvm_reqd_limit = 20,
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
			{ AID_A0000000251010, KERNEL_ID_2B	},
		},
		.config = {
			.present = {
				.status_check_support = true,
				.reader_ctls_floor_limit = true,
				.reader_cvm_reqd_limit = true,
				.ext_selection_support = true,
			},
			.reader_ctls_floor_limit = 100,
			.reader_cvm_reqd_limit = 10,
		}
	},
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

struct emv_ep_combination termset13[] = {
	{
		.txn_types = {
			txn_purchase,
			txn_purchase_with_cashback,
			txn_cash_advance,
			txn_refund
		},
		.combinations = {
			{ AID_A0000000010001, KERNEL_ID_23	},
			{ AID_A0000000020002, KERNEL_ID_22	},
			{ AID_A0000000030003, KERNEL_ID_21	},
			{ AID_A0000000030003, KERNEL_ID_25	},
			{ AID_A0000000040004, KERNEL_ID_24	},
			{ AID_B0000000010101, KERNEL_ID_810978	},
			{ AID_B0000000010102, KERNEL_ID_BF0840	},
			{ AID_B0000000010103, KERNEL_ID_C11111	},
			{ AID_B0000000010104, KERNEL_ID_FF2222	},
			{ AID_B0000000010105, KERNEL_ID_32	}
		}
	}
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

struct emv_ep_terminal_settings termsettings[num_termsettings] = {
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
		.combination_sets	= termset4and5,
		.num_combination_sets	= ARRAY_SIZE(termset4and5),
		.terminal_data		= &terminal_data
	},
	{
		.combination_sets	= termset4and5,
		.num_combination_sets	= ARRAY_SIZE(termset4and5),
		.terminal_data		= &terminal_data,
		.autorun = {
			.enabled	   = true,
			.txn_type	   = txn_purchase,
			.amount_authorized = 10
		}
	},
	{
		.combination_sets	= termset4and5,
		.num_combination_sets	= ARRAY_SIZE(termset4and5),
		.terminal_data		= &terminal_data,
		.autorun = {
			.enabled	   = true,
			.txn_type	   = txn_purchase_with_cashback,
			.amount_authorized = 10
		}
	},
	{
		.combination_sets	= termset4and5,
		.num_combination_sets	= ARRAY_SIZE(termset4and5),
		.terminal_data		= &terminal_data,
		.autorun = {
			.enabled	   = true,
			.txn_type	   = txn_cash_advance,
			.amount_authorized = 10
		}
	},
	{
		.combination_sets	= termset4and5,
		.num_combination_sets	= ARRAY_SIZE(termset4and5),
		.terminal_data		= &terminal_data,
		.autorun = {
			.enabled	   = true,
			.txn_type	   = txn_refund,
			.amount_authorized = 10
		}
	},
	{
		.combination_sets	= termset6and7,
		.num_combination_sets	= ARRAY_SIZE(termset6and7),
		.terminal_data		= &terminal_data
	},
	{
		.combination_sets	= termset6and7,
		.num_combination_sets	= ARRAY_SIZE(termset6and7),
		.terminal_data		= &terminal_data,
		.autorun = {
			.enabled	   = true,
			.txn_type	   = txn_purchase,
			.amount_authorized = 10
		}
	},
	{
		.combination_sets	= termset8,
		.num_combination_sets	= ARRAY_SIZE(termset8),
		.terminal_data		= &terminal_data
	},
	{
		.combination_sets	= termset13,
		.num_combination_sets	= ARRAY_SIZE(termset13),
		.terminal_data		= &terminal_data,
		.autorun = {
			.enabled	   = true,
			.txn_type	   = txn_purchase,
			.amount_authorized = 10
		}
	}
};

bool term_is_kernel_supported(enum termsetting termsetting,
				    const void *kernel_id, size_t kernel_id_len)
{
	struct emv_ep_terminal_settings *settings = NULL;
	int i;

	if ((termsetting >= num_termsettings) || !kernel_id || !kernel_id_len)
		return false;

	settings = &termsettings[termsetting];

	for (i = 0; i < settings->num_combination_sets; i++) {
		struct emv_ep_aid_kernel *aid_kernel;

		for (aid_kernel = settings->combination_sets[i].combinations;
		     aid_kernel->aid_len;
		     aid_kernel++) {

			if ((kernel_id_len == aid_kernel->kernel_id_len) &&
			    (!memcmp(kernel_id, aid_kernel->kernel_id,
								kernel_id_len)))
				return true;
		}
	}

	return false;
}

const struct emv_ep_terminal_settings *term_get_setting(
						   enum termsetting termsetting)
{
	if (termsetting >= num_termsettings)
		return NULL;

	return &termsettings[termsetting];
}
