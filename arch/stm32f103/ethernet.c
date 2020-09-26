#define STM32F1

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/dma.h>

#include <enc28j60.h>

#include <stdio.h>
#include <string.h>

#define FCPU 72000000UL
#define FTIMER 100000UL
#define PSC ((FCPU) / (FTIMER) - 1)
#define TIMEOUT_TIMER_STEP 1000UL

static int shell_cfg = 0;

static const uint8_t mac[6] = {0x0C, 0x00, 0x00, 0x00, 0x00, 0x02};
static uint8_t host[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

struct enc28j60_state_s eth_state;
bool configured = false;

static bool poll_lock_f = false;
static void poll_lock(void)
{
    while (poll_lock_f)
            ;
    poll_lock_f = true;
}

static void poll_unlock(void)
{
    poll_lock_f = false;
}

static bool eth_has_pkg(void)
{
    bool res = enc28j60_has_package(&eth_state);
    return res;
}

static void run_eth(void)
{
    static bool eth_running = false;
    if (eth_running)
	return;
    eth_running = true;
    bool ehp;
    while ((ehp = eth_has_pkg()) || mnum > 0)
    {
        if (mnum > 0)
        {
            size_t len = strlen(messages[mfirst]);
            char hdr[ETHERNET_HEADER_LEN];
            enc28j60_fill_header(hdr, mac, host, NORT_ETHERTYPE);
            while (!enc28j60_tx_ready(&eth_state))
                ;
            uint8_t plen[2] = {len >> 8, len & 0xFF};
            enc28j60_send_start(&eth_state);
            enc28j60_send_append(&eth_state, hdr, ETHERNET_HEADER_LEN);
            enc28j60_send_append(&eth_state, plen, 2);
            enc28j60_send_append(&eth_state, messages[mfirst], len);
            enc28j60_send_commit(&eth_state);
            mfirst = (mfirst + 1) % MNUM;
            mnum--;
        }
        if (ehp)
        {
            char buf[1518];
            uint32_t status;
            uint32_t crc;

            ssize_t len = enc28j60_read_packet(&eth_state, buf, 1518, &status, &crc);

            uint16_t ethertype = enc28j60_get_ethertype(buf, len);
            uint8_t src[6];
            uint8_t dst[6];
            uint16_t plen;
            uint8_t *pl = (uint8_t *)enc28j60_get_payload(buf, len);
            memcpy(src, enc28j60_get_source(buf, len), 6);
            memcpy(dst, enc28j60_get_target(buf, len), 6);
            switch (ethertype)
            {
            case NORT_ETHERTYPE:
                memcpy(host, src, 6);
                plen = ((uint16_t)pl[0]) << 8 | pl[1];
                execute_g_command(pl+2, plen);
                break;
            default:
                break;
            }
        }
    }
    eth_running = false;
}

static ssize_t write_fun(int fd, const void *data, ssize_t len)
{
    int i;
    if (len < 0)
        len = strlen((const char *)data);
    if (fd == 0)
    {
        if (mnum == MNUM)
            return -1;
        memcpy(messages[mpos], data, len);
        messages[mpos][len] = 0;
        mpos = (mpos + 1) % MNUM;
        mnum++;
    }
    else
    {
        uart_send(data, len);
    }
    return 0;
}


static void shell_setup(void)
{
    output_set_write_fun(write_fun);
    output_control_set_fd(0);
    output_shell_set_fd(1);
}

/************* END SHELL ****************/

/* Ethernet */

static void enc28j60setup(struct enc28j60_state_s *state)
{
    // interrupt pin
    gpio_set_mode(GPIOB, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, GPIO1);

    // enable interrupt
/*    nvic_set_priority(NVIC_EXTI1_IRQ, 0x22);
    nvic_enable_irq(NVIC_EXTI1_IRQ);
    exti_select_source(EXTI1, GPIOB);
    exti_set_trigger(EXTI1, EXTI_TRIGGER_FALLING);
    exti_enable_request(EXTI1);*/

    enc28j60_init(state, spi_rw, spi_cs, spi_write_buf, spi_read_buf);
    enc28j60_configure(state, mac, 4096, false);
    enc28j60_interrupt_enable(state, true);
}

/*void exti1_isr(void)
{
    run_eth();
    exti_reset_request(EXTI1);
}*/


void hardware_loop(void)
{
	run_eth();
}

