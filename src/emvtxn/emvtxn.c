#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <feig/feclr.h>
#include <emv.h>
#include <tlv.h>

int geldkarte_get_id(struct emv_kernel *kernel, void *kernel_id, size_t *len)
{
	const uint8_t geldkarte_id[] = { 0xC0, 0x61, 0x50 };

	if (*len < sizeof(geldkarte_id))
		return EMV_RC_OVERFLOW;

	*len = sizeof(geldkarte_id);
	memcpy(kernel_id, geldkarte_id, sizeof(geldkarte_id));

	return EMV_RC_OK;
}

int geldkarte_configure(struct emv_kernel *kernel, const void *configuration,
								  size_t length)
{
	printf("%s()\n", __func__);
	return EMV_RC_OK;
}


int geldkarte_activate(struct emv_kernel *kernel, struct emv_hal *hal,
			    struct emv_ep_preproc_indicators *prepoc_indicators,
			   const void *fci, size_t fci_len, const uint8_t sw[2],
			      struct emv_outcome_parms *outcome, void *txn_data,
							   size_t *txn_data_len)
{
	uint8_t *my_fci = (uint8_t *)fci;
	size_t i = 0;

	printf("%s()\n", __func__);
	printf("sw: %02x%02x\n", sw[0], sw[1]);
	for (i = 0; i < fci_len; i++)
		printf("%02X", my_fci[i]);
	printf("\n");

	return EMV_RC_OK;
}

const struct emv_kernel_ops geldkarte_ops = {
	.get_id	   = geldkarte_get_id,
	.configure = geldkarte_configure,
	.activate  = geldkarte_activate
};

struct cvend_hal {
	const struct emv_hal_ops *ops;
	int fd_feclr;
};

int cvend_hal_start_polling(struct emv_hal *emv_hal)
{
	return EMV_RC_OK;
}

int cvend_hal_wait_for_card(struct emv_hal *emv_hal)
{
	return EMV_RC_OK;
}

int cvend_hal_transceive(struct emv_hal *emv_hal, const uint8_t *capdu,
			    size_t capdu_len, uint8_t *rapdu, size_t *rapdu_len)
{
	struct cvend_hal *hal = (struct cvend_hal *)emv_hal;
	int rc = 0;
	uint64_t status = FECLR_STS_OK;
	uint8_t rx_last_bits;
	size_t buffer_len = *rapdu_len;

	rc = feclr_transceive(hal->fd_feclr, 0, capdu, capdu_len, 0, rapdu,
			      buffer_len, rapdu_len, &rx_last_bits, 0, &status);
	if (rc < 0) {
		fprintf(stderr, "feclr_transceive failed: %s\n",
							       strerror(errno));
		return EMV_RC_RF_COMMUNICATION_ERROR;
	}

	if (status != FECLR_STS_OK) {
		fprintf(stderr, "feclr_transceive failed. status: 0x%08llX\n",
									status);
		return EMV_RC_RF_COMMUNICATION_ERROR;
	}

	if (rx_last_bits != 0) {
		fprintf(stderr, "rx_last_bits: %hhu\n", rx_last_bits);
		return EMV_RC_CARD_PROTOCOL_ERROR;
	}

	return EMV_RC_OK;
}

void cvend_hal_ui_request(struct emv_hal *emv_hal,
					      struct emv_ui_request *ui_request)
{
}

const struct emv_hal_ops cvend_hal_ops = {
	.start_polling = cvend_hal_start_polling,
	.wait_for_card = cvend_hal_wait_for_card,
	.transceive    = cvend_hal_transceive,
	.ui_request    = cvend_hal_ui_request
};

#define TXN_PURCHASE	((const uint8_t []) { 0x00 })
#define AID_MASTERCARD  ((const uint8_t []) { 0xA0, 0x00, 0x00, 0x00, 0x04 })
#define AID_GELDKARTE	((const uint8_t [])				       \
		       { 0xD2, 0x76, 0x00, 0x00, 0x25, 0x45, 0x50, 0x02, 0x00 })
#define KERNEL_ID_C2	((const uint8_t []) { 0x2 })
#define KERNEL_ID_GELDKARTE ((const uint8_t []) { 0xc0, 0x61, 0x50 })

static const uint8_t *amount(uint64_t amount)
{
	static uint8_t result[6];

	result[0]  = (uint8_t)(((amount / 100000000000) % 10) << 4);
	result[0] |= (uint8_t) ((amount / 10000000000) % 10);
	result[1]  = (uint8_t)(((amount / 1000000000) % 10) << 4);
	result[1] |= (uint8_t) ((amount / 100000000) % 10);
	result[2]  = (uint8_t)(((amount / 10000000) % 10) << 4);
	result[2] |= (uint8_t) ((amount / 1000000) % 10);
	result[3]  = (uint8_t)(((amount / 100000) % 10) << 4);
	result[3] |= (uint8_t) ((amount / 10000) % 10);
	result[4]  = (uint8_t)(((amount / 1000) % 10) << 4);
	result[4] |= (uint8_t) ((amount / 100) % 10);
	result[5]  = (uint8_t)(((amount / 10) % 10) << 4);
	result[5] |= (uint8_t) ((amount / 1) % 10);

	return result;
}

int get_config(void *config, size_t *size)
{
	struct tlv *tlv_config = NULL;
	struct tlv *tlv_aid = NULL;
	struct tlv *tlv_kernel_id = NULL;
	struct tlv *tlv_rdr_ctls_txn_limit = NULL;
	struct tlv *tlv_rdr_ctls_floor_limit = NULL;
	struct tlv *tlv_txn_type = NULL;
	struct tlv *tlv_combination = NULL;
	struct tlv *tlv_combination_set;

	tlv_config = tlv_new(TLV_ID_LIBEMV_CONFIGURATION, 0, NULL);

	tlv_combination_set = tlv_new(TLV_ID_LIBEMV_COMBINATION_SET, 0, NULL);
	tlv_insert_below(tlv_config, tlv_combination_set);

	tlv_txn_type = tlv_new(TLV_ID_LIBEMV_TRANSACTION_TYPE,
					    sizeof(TXN_PURCHASE), TXN_PURCHASE);

	tlv_insert_below(tlv_combination_set, tlv_txn_type);

	tlv_combination = tlv_new(TLV_ID_LIBEMV_COMBINATION, 0, NULL);
	tlv_insert_after(tlv_txn_type, tlv_combination);
	tlv_aid = tlv_new(TLV_ID_LIBEMV_AID, sizeof(AID_MASTERCARD),
								AID_MASTERCARD);
	tlv_insert_below(tlv_combination, tlv_aid);
	tlv_kernel_id = tlv_new(TLV_ID_LIBEMV_KERNEL_ID, sizeof(KERNEL_ID_C2),
								  KERNEL_ID_C2);
	tlv_insert_after(tlv_aid, tlv_kernel_id);
	tlv_rdr_ctls_txn_limit = tlv_new(TLV_ID_LIBEMV_RDR_CTLS_TXN_LIMIT, 6,
								 amount(50000));
	tlv_insert_after(tlv_kernel_id, tlv_rdr_ctls_txn_limit);
	tlv_rdr_ctls_floor_limit = tlv_new(TLV_ID_LIBEMV_RDR_CTLS_FLOOR_LIMIT,
							       6, amount(2500));
	tlv_insert_after(tlv_rdr_ctls_txn_limit, tlv_rdr_ctls_floor_limit);

	tlv_combination = tlv_new(TLV_ID_LIBEMV_COMBINATION, 0, NULL);
	tlv_insert_after(tlv_txn_type, tlv_combination);
	tlv_aid = tlv_new(TLV_ID_LIBEMV_AID, sizeof(AID_GELDKARTE),
								 AID_GELDKARTE);
	tlv_insert_below(tlv_combination, tlv_aid);
	tlv_kernel_id = tlv_new(TLV_ID_LIBEMV_KERNEL_ID,
			      sizeof(KERNEL_ID_GELDKARTE), KERNEL_ID_GELDKARTE);
	tlv_insert_after(tlv_aid, tlv_kernel_id);
	tlv_rdr_ctls_txn_limit = tlv_new(TLV_ID_LIBEMV_RDR_CTLS_TXN_LIMIT, 6,
								 amount(10000));
	tlv_insert_after(tlv_kernel_id, tlv_rdr_ctls_txn_limit);

	tlv_encode(tlv_config, config, size);
	tlv_free(tlv_config);

	return 0;
}


int main(int argc, char **argv)
{
	struct cvend_hal hal;
	struct emv_kernel kernel;
	struct emv_ep *emv_ep = emv_ep_new();
	union tech_data tech_data;
	uint64_t status = FECLR_STS_OK, tech = 0;
	int fd = -1, rc = 0;
	uint8_t config[512];
	size_t config_size = sizeof(config);

	get_config(config, &config_size);
	rc = emv_ep_configure(emv_ep, config, config_size);
	if (rc != EMV_RC_OK) {
		fprintf(stderr, "emv_ep_configure failed. rc %d\n", rc);
		goto error;
	}

	hal.ops = &cvend_hal_ops;
	emv_ep_register_hal(emv_ep, (struct emv_hal *)&hal);

	kernel.ops = &geldkarte_ops;
	emv_ep_register_kernel(emv_ep, &kernel);

	hal.fd_feclr = open("/dev/feclr0", O_RDWR);
	if (hal.fd_feclr < 0) {
		fprintf(stderr, "open('/dev/feclr0') failed: %s\n",
							       strerror(errno));
		goto error;
	}

	rc = feclr_start_polling(hal.fd_feclr, FECLR_LOOP_EMVCO,
				    FECLR_TECH_ISO14443A | FECLR_TECH_ISO14443B,
					      FECLR_FLAG_DISABLE_LPCD, &status);
	if (rc < 0) {
		fprintf(stderr, "feclr_start_polling failed: %s\n",
							       strerror(errno));
		goto error;
	}

	if (status != FECLR_STS_OK) {
		fprintf(stderr, "feclr_start_polling failed. status 0x%08llx\n",
									status);
		goto error;
	}

	/* FIXME: REQUIREMENT(EMVCO_BOOK_B_V2_5, "3.2.1.2"): PRESENT CARD     */
	printf("PLEASE PRESENT CARD\n");

	rc = feclr_wait_for_card(hal.fd_feclr, 60000000 /* us */, &tech,
						     &tech_data, NULL, &status);
	if (rc < 0) {
		fprintf(stderr, "feclr_wait_for_card failed: %s\n",
							       strerror(errno));
		goto error;
	}

	if (status != FECLR_STS_OK) {
		fprintf(stderr, "feclr_wait_for_card failed. status 0x%08llx\n",
									status);
		goto error;
	}

	if ((tech != FECLR_TECH_ISO14443A) && (tech != FECLR_TECH_ISO14443B)) {
		/* FIXME: INSERT, SWIPE OR TRY ANOTHER CARD */
		printf("TRY ANOTHER CARD\n");
		goto error;
	}

	if (((tech == FECLR_TECH_ISO14443A) &&
	     ((tech_data.iso14443a_jewel.iso14443a.sak & 0x20) == 0x00)) ||
	    ((tech == FECLR_TECH_ISO14443B) &&
	     ((tech_data.iso14443b.atqb[10] & 0x01) == 0x00))) {
		/* FIXME: INSERT, SWIPE OR TRY ANOTHER CARD */
		printf("TRY_ANOTHER_CARD\n");
		goto error;
	}

	rc = feclr_select_protocol(hal.fd_feclr, FECLR_PROTO_ISO14443_4,
								       &status);
	if (rc < 0) {
		fprintf(stderr, "feclr_select_protocol failed: %s\n",
							       strerror(errno));
		goto error;
	}

	if (status != FECLR_STS_OK) {
		fprintf(stderr, "select_protocol failed. status 0x%08llx\n",
									status);
		goto error;
	}

	rc = emv_ep_activate(emv_ep, start_a, txn_purchase,
					      1500, 0, ISO4217_EUR, 0x1234567u);
	if (rc != EMV_RC_OK) {
		fprintf(stderr, "emv_ep_activate failed. rc %d\n", rc);
		goto error;
	}

	printf("REMOVE CARD\n");

	rc = feclr_wait_for_card_removal(hal.fd_feclr, 60000000 /*us*/,
								       &status);
	if (rc < 0) {
		fprintf(stderr, "feclr_wait_for_card_removal failed: %s\n",
							       strerror(errno));
		goto error;
	}

	if (status != FECLR_STS_OK) {
		fprintf(stderr, "wait_for_card_removal failed. status %08llx\n",
									status);
		goto error;
	}

	rc = feclr_stop_polling(hal.fd_feclr);
	if (rc < 0)
		fprintf(stderr, "feclr_stop_polling failed: %s\n",
							       strerror(errno));
	close(hal.fd_feclr);

	return EXIT_SUCCESS;

error:
	if (hal.fd_feclr >= 0)
		close(fd);

	return EXIT_FAILURE;
}
