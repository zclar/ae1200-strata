#ifndef STRATA_JDI_H
#define STRATA_JDI_H

#include <stddef.h>
#include <stdint.h>

#include "strata_display.h"

#define STRATA_JDI_WRITE_4BIT_COMMAND 0x90u
#define STRATA_JDI_ALL_CLEAR_COMMAND 0x20u
#define STRATA_JDI_TRAILER_BYTES 2u
#define STRATA_JDI_LINE_PACKET_BYTES \
    (2u + STRATA_PACKED_LINE_BYTES + STRATA_JDI_TRAILER_BYTES)
#define STRATA_JDI_CLEAR_PACKET_BYTES 2u

int strata_jdi_encode_line(const uint8_t *frame, unsigned int line,
                           uint8_t *packet, size_t packet_size);
int strata_jdi_encode_all_clear(uint8_t *packet, size_t packet_size);

#endif
