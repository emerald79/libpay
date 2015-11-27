#include <stdio.h>

#include <emv.h>

int lt_start_polling(struct emv_hal *hal)
{
	printf("%s()\n", __func__);
	return EMV_RC_OK;
}

int lt_wait_for_card(struct emv_hal *hal)
{
	printf("%s()\n", __func__);
	return EMV_RC_OK;
}

int lt_transceive(struct emv_hal *hal, const void *capdu, size_t capdu_sz,
						  void *rapdu, size_t *rapdu_sz)
{
	printf("%s()\n", __func__);
	return EMV_RC_OK;
}

void lt_ui_request(struct emv_hal *hal, struct emv_ui_request *ui_request)
{
	printf("%s()\n", __func__);
}

const struct emv_hal_ops lt_hal_ops  = {
	.start_polling = lt_start_polling,
	.wait_for_card = lt_wait_for_card,
	.transceive    = lt_transceive,
	.ui_request    = lt_ui_request
};

struct lt_hal {
	const struct emv_hal_ops *ops;
};

struct emv_hal *lt_hal_new(void)
{
	struct lt_hal *lt_hal = NULL;

	lt_hal = malloc(sizeof(struct lt_hal));
	if (lt_hal)
		lt_hal->ops = &lt_hal_ops;

	return (struct emv_hal *)lt_hal;
}

void lt_hal_free(struct emv_hal *lt_hal)
{
	if (lt_hal)
		free(lt_hal);
}
