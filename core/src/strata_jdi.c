#include "strata_jdi.h"

int strata_jdi_encode_line(const uint8_t *frame, unsigned int line,
                           uint8_t *packet, size_t packet_size)
{
    if (!frame || !packet || line >= STRATA_HEIGHT ||
        packet_size < STRATA_JDI_LINE_PACKET_BYTES) return -1;

    packet[0] = STRATA_JDI_WRITE_4BIT_COMMAND;
    /* JDI gate addresses are one-based and sent in normal bit order. */
    packet[1] = (uint8_t)(line + 1u);
    strata_pack_line_rgb111(frame, line, packet + 2);
    packet[2u + STRATA_PACKED_LINE_BYTES] = 0;
    packet[3u + STRATA_PACKED_LINE_BYTES] = 0;
    return (int)STRATA_JDI_LINE_PACKET_BYTES;
}

int strata_jdi_encode_all_clear(uint8_t *packet, size_t packet_size)
{
    if (!packet || packet_size < STRATA_JDI_CLEAR_PACKET_BYTES) return -1;
    packet[0] = STRATA_JDI_ALL_CLEAR_COMMAND;
    packet[1] = 0;
    return (int)STRATA_JDI_CLEAR_PACKET_BYTES;
}
