#define STM32F1

#include <string.h>
#include <libopencm3/stm32/gpio.h>

#ifdef CONFIG_ETHERNET_DEVICE_ENC28J60
#include <enc28j60.h>
#endif

#ifdef CONFIG_ETHERNET
#include <ethernet.h>
#endif

#ifdef CONFIG_IP
#include <icmp.h>
#include <arp.h>
#include <ip.h>
#include <udp.h>
#endif

#include "platform.h"
#include "net.h"

#include <shell.h>

#ifdef CONFIG_ETHERNET_DEVICE_ENC28J60
static struct enc28j60_state_s *state;
#endif

#ifdef CONFIG_ETHERNET
static const uint8_t local_mac[6] = { 0x0C, 0x00, 0x00, 0x00, 0x00, 0x02 };
static uint8_t       current_remote_mac[6];
static uint8_t       remote_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
static uint8_t       ethernet_packet_buffer[1518+100];
#endif

#ifdef CONFIG_IP
static const uint32_t local_ip = 10 << 24 | 55 << 16 | 1 << 8 | 110;
static const uint16_t local_udp_port = CONFIG_UDP_PORT;
static const int TTL = 30;
static uint32_t current_remote_ip;
static uint16_t current_remote_port;
static uint32_t remote_ip;
static uint16_t remote_udp_port;
#endif

static const uint8_t *(*next_message)(ssize_t *);
static void (*on_packet_received)(const char *data, size_t len);
static void (*on_send_completed)(void);

static volatile bool application_busy = false;

void net_setup(void *conn_state,
               const uint8_t *(*next_message_cb)(ssize_t *),
               void (*on_send_completed_cb)(void),
               void (*on_packet_received_cb)(const char *data, size_t len))
{
    state = conn_state;

    next_message = next_message_cb;
    on_send_completed = on_send_completed_cb;
    on_packet_received = on_packet_received_cb;

#ifdef CONFIG_ETHERNET_DEVICE_ENC28J60
    if (!enc28j60_configure(state, local_mac, 4096, false))
    {
        hard_fault_handler();
    }
    enc28j60_interrupt_enable(state, true);
#endif
}

static void handle_input_packet(const uint8_t *payload, size_t len)
{
#ifdef CONFIG_ETHERNET
    memcpy(remote_mac, current_remote_mac, 6);
#endif

#ifdef CONFIG_IP
    remote_ip = current_remote_ip;
    remote_udp_port = current_remote_port;
#endif

    /* Target logic */
    on_packet_received(payload, len);
}

#ifdef CONFIG_IP
static void handle_udp(const uint8_t *payload, size_t len)
{ 
    uint16_t port = udp_get_destination(payload, len);
    current_remote_port = udp_get_source(payload, len);
    size_t udp_payload_len;
    const uint8_t *udp_payload = udp_get_payload(payload, len, &udp_payload_len);
    switch (port)
    {
        case CONFIG_UDP_PORT:
        {
            handle_input_packet(udp_payload, udp_payload_len);
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
            uint8_t *buffer = ethernet_packet_buffer;

            uint16_t id = icmp_echo_get_identifier(payload, len);
            uint16_t sn = icmp_echo_get_sequence_number(payload, len);
            size_t icmp_payload_len;
            const uint8_t *icmp_payload = icmp_echo_get_payload(payload, len, &icmp_payload_len);

            /* Send ICMP ECHO REPLY response */
            //memset(response_buf, 0, sizeof(response_buf));
            size_t echo_len = icmp_fill_echo(buffer + ETHERNET_HEADER_LEN + IP_HEADER_LEN, id, sn, icmp_payload, icmp_payload_len);
            size_t icmp_len = icmp_fill_header(buffer + ETHERNET_HEADER_LEN + IP_HEADER_LEN, ICMP_TYPE_ECHO_REPLY, 0, echo_len);
            size_t ip_len = ip_fill_header(buffer + ETHERNET_HEADER_LEN, local_ip, current_remote_ip, IP_PROTOCOL_ICMP, 30, icmp_len);
            size_t eth_len = enc28j60_fill_header(buffer, local_mac, current_remote_mac, ETHERTYPE_IP, ip_len);
            enc28j60_send_data(state, buffer, eth_len);
            break;
        }
        default:
            break;
    }
}

static void handle_ip(const uint8_t *payload, size_t len)
{
    uint8_t protocol = ip_get_protocol(payload, len);
    current_remote_ip = ip_get_source(payload, len);

    size_t ip_payload_len;
    const uint8_t *ip_payload = ip_get_payload(payload, len, &ip_payload_len);
    switch (protocol)
    {
        case IP_PROTOCOL_UDP:
        {
            handle_udp(ip_payload, ip_payload_len);
            break;
        }
        case IP_PROTOCOL_ICMP:
        {
            handle_icmp(ip_payload, ip_payload_len);
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
    size_t plen;

    uint8_t sender_hw[6]; 
    memcpy(sender_hw, arp_get_sender_hardware(payload, len, &hwlen), sizeof(sender_hw));

    uint8_t sender_p[4];
    memcpy(sender_p, arp_get_sender_protocol(payload, len, &plen), sizeof(sender_p));

    const uint8_t *target_hw = arp_get_target_hardware(payload, len, &hwlen);
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
                uint8_t *buffer = ethernet_packet_buffer;
                uint8_t rshw[6], rsp[4];

                /* Send ARP response */
                arp_set_sender_hardware(buffer + ETHERNET_HEADER_LEN, local_mac, 6, 4);
                arp_set_sender_protocol(buffer + ETHERNET_HEADER_LEN, ip, 6, 4);

                arp_set_target_hardware(buffer + ETHERNET_HEADER_LEN, sender_hw, 6, 4);
                arp_set_target_protocol(buffer + ETHERNET_HEADER_LEN, sender_p, 6, 4);

                size_t arp_len = arp_fill_header(buffer + ETHERNET_HEADER_LEN, ARP_HTYPE_ETHERNET, 6, ETHERTYPE_IP, 4, ARP_OPERATION_RESPONSE);
                size_t eth_len = enc28j60_fill_header(buffer, local_mac, sender_hw, ETHERTYPE_ARP, arp_len);

                enc28j60_send_data(state, buffer, eth_len);
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
#endif /* CONFIG_IP */

#ifdef CONFIG_ETHERNET
static void handle_ethernet(const uint8_t *payload, size_t len)
{
    uint16_t ethertype = ethernet_get_ethertype(payload, len);

    memcpy(current_remote_mac, ethernet_get_source(payload, len), 6);

    size_t ethernet_payload_len;
    const uint8_t *ethernet_payload = ethernet_get_payload(payload, len, &ethernet_payload_len);
    switch (ethertype)
    {
#ifdef CONFIG_IP
        case ETHERTYPE_IP:
            handle_ip(ethernet_payload, ethernet_payload_len);
            break;
        case ETHERTYPE_ARP:
            handle_arp(ethernet_payload, ethernet_payload_len);
            break;
#endif
        default:
            break;
    }
}
#endif


bool net_ready(void)
{
#ifdef CONFIG_IP
    return remote_udp_port != 0;
#else
    return true;
#endif
}

void net_receive(void)
{
    uint32_t status, crc;
    const size_t length = sizeof(ethernet_packet_buffer);
    uint8_t *buffer = ethernet_packet_buffer;

#ifdef CONFIG_ETHERNET_DEVICE_ENC28J60
    if (!enc28j60_has_package(state))
    {
        return;
    }
    ssize_t len = enc28j60_read_packet(state, buffer, length, &status, &crc);
    if (len > 0)
    {
        handle_ethernet(ethernet_packet_buffer, len);
    }
#else
    return;
#endif
}

void net_send(void)
{
    ssize_t len;
    const uint8_t *data = next_message(&len);
    if (data == NULL)
        return;

#ifdef CONFIG_ETHERNET
    uint8_t *buffer = ethernet_packet_buffer;
    const size_t length = sizeof(ethernet_packet_buffer);
    size_t payload_len = 0;

    /* Send UDP datagram */
    memset(buffer, 0, length);

#ifdef CONFIG_IP
    udp_fill_payload(buffer + ETHERNET_HEADER_LEN + IP_HEADER_LEN, data, len);
    size_t udp_len = udp_fill_header(buffer + ETHERNET_HEADER_LEN + IP_HEADER_LEN, local_udp_port, remote_udp_port, len);
    payload_len = ip_fill_header(buffer + ETHERNET_HEADER_LEN, local_ip, remote_ip, IP_PROTOCOL_UDP, TTL, udp_len);
#endif

    size_t eth_len = enc28j60_fill_header(buffer, local_mac, remote_mac, ETHERTYPE_IP, payload_len);

    application_busy = true;
#ifdef CONFIG_ETHERNET_DEVICE_ENC28J60
    enc28j60_send_data(state, buffer, eth_len);
#endif

#endif

    on_send_completed();
}

