#ifndef __EMVCO_EP_TA_H__
#define __EMVCO_EP_TA_H__

#include <emv.h>

int get_termsetting_n(int n, void *buffer, size_t *size);

struct emv_hal *lt_hal_new(void);

void lt_hal_free(struct emv_hal *lt_hal);

#endif						    /* ndef __EMVCO_EP_TA_H__ */
