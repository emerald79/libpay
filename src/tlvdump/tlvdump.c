#include <stdint.h>
#include <stdio.h>
#include <tlv.h>

int main(void)
{
	struct tlv *tlv;
	uint8_t read_record_rsp[] = {
		0x70, 0x39, 0x5A, 0x0A, 0x67, 0x99, 0x99, 0x89, 0x00, 0x00,
		0x02, 0x00, 0x06, 0x9F, 0x5F, 0x24, 0x03, 0x25, 0x12, 0x31,
		0x5F, 0x25, 0x03, 0x04, 0x01, 0x01, 0x5F, 0x34, 0x01, 0x25,
		0x9F, 0x07, 0x02, 0xFF, 0x00, 0x9F, 0x0D, 0x05, 0xFC, 0x50,
		0xA0, 0x00, 0x00, 0x9F, 0x0E, 0x05, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x9F, 0x0F, 0x05, 0xF8, 0x70, 0xA4, 0x98, 0x00
	};
	int rc;

	rc = tlv_parse(read_record_rsp, sizeof(read_record_rsp), &tlv);
	if (rc != TLV_RC_OK)
		return EXIT_FAILURE;

	for (; tlv; tlv = tlv_get_next(tlv)) {
		uint8_t buffer[256];
		size_t size;
		int i;
		size = sizeof(buffer);
		tlv_encode_identifier(tlv, buffer, &size);
		for (i = 0; i < (int)size; i++)
			printf("%02X", buffer[i]);
		printf(" ");
		size = sizeof(buffer);
		tlv_encode_length(tlv, buffer, &size);
		for (i = 0; i < (int)size; i++)
			printf("%02X", buffer[i]);
		printf(" ");
		size = sizeof(buffer);
		tlv_encode_value(tlv, buffer, &size);
		for (i = 0; i < (int)size; i++)
			printf("%02X", buffer[i]);
		printf("\n");
	}

	tlv_free(tlv);

	return EXIT_SUCCESS;
}
