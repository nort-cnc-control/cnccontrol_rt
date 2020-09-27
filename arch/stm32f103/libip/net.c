#define STM32F1

#include <string.h>
#include <libopencm3/stm32/gpio.h>
#include <enc28j60.h>

#include "platform.h"
#include "icmp.h"
#include "arp.h"
#include "ip.h"
#include "udp.h"
#include "net.h"
#include "shell.h"

static struct enc28j60_state_s *state;

static const uint8_t local_mac[6] = { 0x0C, 0x00, 0x00, 0x00, 0x00, 0x02 };
static const uint32_t local_ip = 10 << 24 | 55 << 16 | 1 << 8 | 110;
static const uint16_t local_udp_port = CONFIG_UDP_PORT;

static uint8_t  current_remote_mac[6];
static uint32_t current_remote_ip;
static uint16_t current_remote_port;

static uint8_t  remote_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
static uint32_t remote_ip;
static uint16_t remote_udp_port;

static uint8_t response_buf[1518];

static void (*on_send_completed)(void);
static void (*on_packet_received)(const char *data, size_t len);

static volatile bool application_busy = false;

void net_setup(struct enc28j60_state_s *eth_state, void (*on_send_completed_cb)(void), void (*on_packet_received_cb)(const char *data, size_t len))
{
    state = eth_state;

    on_packet_received = on_packet_received_cb;
    on_send_completed = on_send_completed_cb;

    if (!enc28j60_configure(state, local_mac, 4096, false))
    {
        hard_fault_handler();
    }
    enc28j60_interrupt_enable(state, true); 
}

static void eth_send_completed(void)
{
    if (application_busy)
    {
        on_send_completed();
        application_busy = false;
    }
}

static void handle_input_packet(const uint8_t *payload, size_t len)
{
    memcpy(remote_mac, current_remote_mac, 6);
    remote_ip = current_remote_ip;
    remote_udp_port = current_remote_port;

    /* Target logic */
    on_packet_received(payload, len);
}

static void handle_udp(const uint8_t *payload, size_t len)
{ 
    uint16_t port = udp_get_destination(payload, len);
    current_remote_port = udp_get_source(payload, len);
    size_t payload_len;
    payload = udp_get_payload(payload, len, &payload_len);
    switch (port)
    {
        case CONFIG_UDP_PORT:
        {
            handle_input_packet(payload, payload_len);
            break;
        }
        default:
            break;
    }
}

static void handle_icmp(const uint8_t *payload, size_t len)
{
    uint8_t type = icmp_get_type(payload, len);
    uint8_t code = icmp_get_code(payload, len);
    switch (type)
    {
        case ICMP_TYPE_ECHO:
        {
            uint16_t id = icmp_echo_get_identifier(payload, len);
            uint16_t sn = icmp_echo_get_sequence_number(payload, len);
            size_t payload_len;
            payload = icmp_echo_get_payload(payload, len, &payload_len);

            /* Send ICMP ECHO REPLY response */
            memset(response_buf, 0, sizeof(response_buf));
            size_t echo_len = icmp_fill_echo(response_buf + ETHERNET_HEADER_LEN + IP_HEADER_LEN, id, sn, payload, payload_len);
            size_t icmp_len = icmp_fill_header(response_buf + ETHERNET_HEADER_LEN + IP_HEADER_LEN, ICMP_TYPE_ECHO_REPLY, 0, echo_len);
            size_t ip_len = ip_fill_header(response_buf + ETHERNET_HEADER_LEN, local_ip, current_remote_ip, IP_PROTOCOL_ICMP, 30, icmp_len);
            size_t eth_len = enc28j60_fill_header(response_buf, local_mac, current_remote_mac, ETHERTYPE_IP, ip_len);
            enc28j60_send_data(state, response_buf, eth_len);
            break;
        }
        default:
            break;
    }
}

static void handle_ip(const uint8_t *payload, size_t len)
{
    uint8_t protocol = ip_get_protocol(payload, len);
    size_t payload_len;
    current_remote_ip = ip_get_source(payload, len);

    payload = ip_get_payload(payload, len, &payload_len);
    switch (protocol)
    {
        case IP_PROTOCOL_UDP:
        {
            handle_udp(payload, payload_len);
            break;
        }
        case IP_PROTOCOL_ICMP:
        {
            handle_icmp(payload, payload_len);
            return;
        }
        default:
            break;
    }
}

static void handle_arp(const uint8_t *payload, size_t len)
{
    uint16_t hw = arp_get_hardware(payload, len);
    uint16_t proto = arp_get_protocol(payload, len);
    if (hw != ARP_HTYPE_ETHERNET || proto != ETHERTYPE_IP)
        return;

    size_t hwlen;
    const uint8_t *sender_hw = arp_get_sender_hardware(payload, len, &hwlen);
    const uint8_t *target_hw = arp_get_target_hardware(payload, len, &hwlen);

    size_t plen;
    const uint8_t *sender_p = arp_get_sender_protocol(payload, len, &plen);
    const uint8_t *target_p = arp_get_target_protocol(payload, len, &plen);

    uint32_t sender_ip = (uint32_t)sender_p[0] << 24 | (uint32_t)sender_p[1] << 16 | (uint32_t)sender_p[2] << 8 | (uint32_t)sender_p[3];
    uint32_t target_ip = (uint32_t)target_p[0] << 24 | (uint32_t)target_p[1] << 16 | (uint32_t)target_p[2] << 8 | (uint32_t)target_p[3];

    uint16_t op = arp_get_operation(payload, len);

    switch (op)
    {
        case ARP_OPERATION_REQUEST:
        {
            if (target_ip == local_ip)
            {
                uint8_t ip[4] = {(uint8_t)(local_ip >> 24), (uint8_t)(local_ip >> 16), (uint8_t)(local_ip >> 8), (uint8_t)(local_ip)};
                /* Send ARP response */
                memset(response_buf, 0, sizeof(response_buf));

                arp_set_sender_hardware(response_buf + ETHERNET_HEADER_LEN, local_mac, 6, 4);
                arp_set_target_hardware(response_buf + ETHERNET_HEADER_LEN, sender_hw, 6, 4);
                arp_set_sender_protocol(response_buf + ETHERNET_HEADER_LEN, ip, 6, 4);
                arp_set_target_protocol(response_buf + ETHERNET_HEADER_LEN, sender_p, 6, 4);

                size_t arp_len = arp_fill_header(response_buf + ETHERNET_HEADER_LEN, ARP_HTYPE_ETHERNET, 6, ETHERTYPE_IP, 4, ARP_OPERATION_RESPONSE);
                size_t eth_len = enc28j60_fill_header(response_buf, local_mac, sender_hw, ETHERTYPE_ARP, arp_len);

                enc28j60_send_data(state, response_buf, eth_len);
            }
            break;
        }
        case ARP_OPERATION_RESPONSE:
        {
            break;
        }
        default:
            break;
    }
}

static void handle_ethernet(const uint8_t *payload, size_t len)
{
    uint16_t ethertype = enc28j60_get_ethertype(payload, len);
    size_t payload_len;

    memcpy(current_remote_mac, enc28j60_get_source(payload, len), 6);

    payload = enc28j60_get_payload(payload, len, &payload_len);
    switch (ethertype)
    {
        case ETHERTYPE_IP:
            handle_ip(payload, payload_len);
            break;
        case ETHERTYPE_ARP:
            handle_arp(payload, payload_len);
            break;
        default:
            break;
    }
}

bool net_ready(void)
{
    return remote_udp_port != 0;
}

void net_receive(void)
{

    uint32_t status, crc;
    uint8_t buf[1518];

    if (!enc28j60_has_package(state))
        return;

    ssize_t len = enc28j60_read_packet(state, buf, 1518, &status, &crc);
    if (len > 0)
    {
        handle_ethernet(buf, len);
    }
}

int net_send(const uint8_t *data, ssize_t len)
{
    while (application_busy)
        ;

    /* Send UDP datagram */
    memset(response_buf, 0, sizeof(response_buf));

    udp_fill_payload(response_buf + ETHERNET_HEADER_LEN + IP_HEADER_LEN, data, len);

    size_t udp_len = udp_fill_header(response_buf + ETHERNET_HEADER_LEN + IP_HEADER_LEN, local_udp_port, remote_udp_port, len);
    size_t ip_len = ip_fill_header(response_buf + ETHERNET_HEADER_LEN, local_ip, remote_ip, IP_PROTOCOL_UDP, 30, udp_len);
    size_t eth_len = enc28j60_fill_header(response_buf, local_mac, remote_mac, ETHERTYPE_IP, ip_len);

    application_busy = true;
    enc28j60_send_data(state, response_buf, eth_len);

    eth_send_completed();
}

