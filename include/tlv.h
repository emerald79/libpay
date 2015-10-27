/**
 * @file
 * TLV Parser Library
 */

#ifndef __TLV_H__
#define __TLV_H__

#include <stdint.h>

#define TLV_RC_OK				0
#define TLV_RC_INVALID_ARG			1
#define TLV_RC_BUFFER_OVERFLOW			2
#define TLV_RC_TAG_NUMBER_TOO_LARGE		3
#define TLV_RC_OUT_OF_MEMORY			4
#define TLV_RC_INDEFINITE_LENGTH_NOT_SUPPORTED	5
#define TLV_RC_VALUE_LENGTH_TOO_LARGE		6

#define TLV_TAG_CLASS_UNIVERSAL		0x00u
#define TLV_TAG_CLASS_APPLICATION	0x40u
#define TLV_TAG_CLASS_CONTEXT_SPECIFIC	0x80u
#define TLV_TAG_CLASS_PRIVATE		0xC0u

struct tlv;

/**
 * Parse DER-TLV encoded data into a TLV data structure
 *
 * @param[in]  buffer  The DER-TLV encoded data to parse.
 * @param[in]  size    Length of the DER-TLV encoded data.
 * @param[out] tlv     The corresponding TLV data structure.
 *
 * @return TLV_RC_OK on success. Other TLV_RC_* codes on failure.
 */
int tlv_parse(const void *buffer, size_t size, struct tlv **tlv);
int tlv_encode(struct tlv *tlv, void *buffer, size_t *size);
int tlv_free(struct tlv *tlv);

#endif /* ndef __TLV_H__ */
