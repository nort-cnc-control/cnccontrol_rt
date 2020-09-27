#include "icmp.h"
#include <string.h>

uint8_t icmp_get_type(const uint8_t *data, size_t len)
{
   const struct icmp_header_s *hdr = (const struct icmp_header_s *)data;
   return hdr->type;
}

uint8_t icmp_get_code(const uint8_t *data, size_t len)
{
   const struct icmp_header_s *hdr = (const struct icmp_header_s *)data;
   return hdr->type;
}


uint16_t icmp_echo_get_identifier(const uint8_t *data, size_t len)
{
    const struct icmp_echo_s *hdr = (const struct icmp_echo_s *)(data + ICMP_HEADER_LEN);
    return hdr->identifier_h << 8 | hdr->identifier_l;
}


uint16_t icmp_echo_get_sequence_number(const uint8_t *data, size_t len)
{
    const struct icmp_echo_s *hdr = (const struct icmp_echo_s *)(data + ICMP_HEADER_LEN);
    return hdr->sequence_number_h << 8 | hdr->sequence_number_l;
}

const uint8_t *icmp_echo_get_payload(const uint8_t *data, size_t len, size_t *payload_len)
{
    *payload_len = len - ICMP_HEADER_LEN - ICMP_ECHO_LEN;
    return data + ICMP_HEADER_LEN + ICMP_ECHO_LEN;
}

size_t icmp_fill_echo(uint8_t *buf, uint16_t identifier, uint16_t sequence_number, const uint8_t *data, size_t len)
{
    struct icmp_echo_s *hdr = (struct icmp_echo_s *)(buf + ICMP_HEADER_LEN);
    hdr->identifier_h = identifier >> 8;
    hdr->identifier_l = identifier;
    hdr->sequence_number_h = sequence_number >> 8;
    hdr->sequence_number_l = sequence_number;
    if (len > 0)
        memcpy(buf + ICMP_HEADER_LEN + ICMP_ECHO_LEN, data, len); 
    return sizeof(struct icmp_echo_s) + len;
}

static uint16_t checksum(const uint8_t *buf, size_t len)
{
    uint32_t sum = 0;
    
    while (len > 1)
    {
        uint16_t word = (uint16_t)(*buf) << 8 | *(buf+1);
        sum += word;
        buf += 2;
        len -= 2;
    }
    if (len == 1)
    {
        sum += *buf;
    }

    sum =  (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    return ~sum;
}

size_t icmp_fill_header(uint8_t *buf, uint8_t type, uint8_t code, size_t len)
{
    struct icmp_header_s *hdr = (struct icmp_header_s *)(buf);
    uint16_t chs;
    hdr->type = type;
    hdr->code = code;
    chs = checksum(buf, ICMP_HEADER_LEN + len);
    hdr->checksum_h = chs >> 8;
    hdr->checksum_l = chs;
    return ICMP_HEADER_LEN + len;
}

