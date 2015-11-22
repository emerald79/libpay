#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <feig/feclr.h>
#include <emv.h>

struct cvend_hal {
	const struct emv_hal_ops *ops;
	int fd_feclr;
};

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
	.transceive = cvend_hal_transceive,
	.ui_request = cvend_hal_ui_request
};

int main(int argc, char **argv)
{
	struct cvend_hal hal;
	struct emv_ep emv_ep;
	union tech_data tech_data;
	uint64_t status = FECLR_STS_OK, tech = 0;
	int fd = -1, rc = 0;
	struct emv_ep_combination combinations[2] = {
		{
			.aid = { 0xA0, 0x00, 0x00, 0x00, 0x04 },
			.aid_len = 5,
			.kernel_id = { 0x02 },
			.kernel_id_len = 1,
			.config = {
				.present = {
					.reader_ctls_tx_limit = true,
					.reader_ctls_floor_limit = true,
				},
				.reader_ctls_tx_limit = {
					0x00, 0x00, 0x00, 0x05, 0x00, 0x00 },
				.reader_ctls_floor_limit = {
					0x00, 0x00, 0x00, 0x00, 0x25, 0x00 }
			}
		},
		{
			.aid = { 0xD2, 0x76, 0x00, 0x00, 0x25, 0x45, 0x50, 0x02,
									 0x00 },
			.aid_len = 9,
			.kernel_id = { 0xc0, 0x61, 0x50 },
			.kernel_id_len = 3,
			.config = {
				.present = {
					.reader_ctls_tx_limit = true,
				},
				.reader_ctls_tx_limit = {
					0x00, 0x00, 0x00, 0x01, 0x00, 0x00 },
			}
		}
	};

	hal.ops = &cvend_hal_ops;
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

	memset(&emv_ep, 0, sizeof(emv_ep));
	emv_ep.hal = (struct emv_hal *)&hal;
	emv_ep.restart = false;
	emv_ep.combination_set[txn_purchase].combinations = combinations;
	emv_ep.combination_set[txn_purchase].size	  = 2;

	rc = emv_ep_activate(&emv_ep, 0, NULL, NULL, NULL, 0x12345678u);
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
