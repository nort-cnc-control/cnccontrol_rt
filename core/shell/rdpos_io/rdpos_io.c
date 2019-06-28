#include <rdpos_io.h>
#include <rdpos.h>
#include <rdp.h>


static void (*cb_line_received)(const unsigned char *, size_t);
static void (*cb_sended)(void);
static void (*cb_connected)(void);
static void (*cb_disconnected)(void);

static void (*cb_serial_reset)(void);
static void (*cb_byte_transmit)(uint8_t b);

static struct rdpos_connection_s sconn;
static struct rdp_connection_s conn;

//****************** SERIAL ************************

static uint8_t serial_outbuf[RDP_MAX_SEGMENT_SIZE];
static int serial_outpos = 0;
static int serial_outlast = 0;

static struct
{
    uint8_t rdy : 1;
    uint8_t connected : 1;
    uint8_t ack_wait : 1;
    uint8_t close_wait : 1;
} opts;

static void send_serial(void *arg, const void *data, size_t len)
{
    int i;
    if (len == 0)
        return;
    const uint8_t *buf = data;
    for (i = 0; i < len; i++)
    {
        serial_outbuf[serial_outlast] = buf[i];
        serial_outlast = (serial_outlast + 1) % sizeof(serial_outbuf);
    }
    if (opts.rdy)
    {
        opts.rdy = 0;
        int pos = serial_outpos;
        serial_outpos = (serial_outpos + 1) % sizeof(serial_outbuf);
        cb_byte_transmit(serial_outbuf[pos]);
    }
}

static void byte_received(unsigned char c)
{
    bool res = rdpos_byte_received(&sconn, c);
}

static void byte_transmitted(void)
{
    if (serial_outpos == serial_outlast)
    {
        opts.rdy = 1;
        return;
    }
    int pos = serial_outpos;
    serial_outpos = (serial_outpos + 1) % sizeof(serial_outbuf);
    cb_byte_transmit(serial_outbuf[pos]);
}

static void register_byte_transmit(void (*f)(uint8_t))
{
    cb_byte_transmit = f;
}

static void register_reset(void (*f)(void))
{
    cb_serial_reset = f;
}

struct serial_cbs_s rdpos_io_serial_cbs = {
    .register_reset = register_reset,
    .register_byte_transmit = register_byte_transmit,
    .byte_received = byte_received,
    .byte_transmitted = byte_transmitted,
};

//**************** END SERIAL ***********************

//***************** RDPOS *****************************

static uint8_t rdp_recv_buf[RDP_MAX_SEGMENT_SIZE];
static uint8_t rdp_outbuf[RDP_MAX_SEGMENT_SIZE];
static uint8_t serial_inbuf[RDP_MAX_SEGMENT_SIZE];

static void connected(struct rdp_connection_s *conn)
{
    opts.connected = 1;
    opts.close_wait = 0;
    if (cb_connected)
        cb_connected();
}

static void closed(struct rdp_connection_s *conn)
{
    opts.connected = 0;
    opts.close_wait = 0;
    if (cb_serial_reset)
        cb_serial_reset();
    rdp_listen(conn, 1);
    if (cb_disconnected)
        cb_disconnected();   
}

static void data_send_completed(struct rdp_connection_s *conn)
{
    if (cb_sended)
        cb_sended();
}

static void data_received(struct rdp_connection_s *conn, const uint8_t *data, size_t len)
{
    if (cb_line_received)
        cb_line_received(data, len);
}

void rdpos_io_init(void)
{
    opts.close_wait = 0;
    opts.ack_wait = 0;
    opts.connected = 0;
    opts.rdy = 1;
    rdp_init_connection(&conn, rdp_outbuf, rdp_recv_buf);
    rdpos_init_connection(&sconn, &conn, serial_inbuf, sizeof(serial_inbuf));
    
    rdp_set_closed_cb(&conn, closed);
    rdp_set_connected_cb(&conn, connected);
    rdp_set_data_received_cb(&conn, data_received);
    rdp_set_data_send_completed_cb(&conn, data_send_completed);
    rdpos_set_send_cb(&sconn, send_serial);

    rdp_listen(&conn, 1);
}

void rdpos_io_clock(int dt)
{
    rdp_clock(&conn, dt);
}

//****************** END RDPOS *********************


//**************** SHELL ****************************

static void send_buffer(const unsigned char *buf, size_t len)
{
    rdp_send(&conn, buf, len);
}

static void register_received_cb(void (*f)(const unsigned char *, size_t))
{
    cb_line_received = f;
}

static void register_sended_cb(void (*f)(void))
{
    cb_sended = f;
}

static void register_connected_cb(void (*f)(void))
{
    cb_connected = f;
}

static void register_disconnected_cb(void (*f)(void))
{
    cb_disconnected = f;
}

struct shell_cbs_s rdpos_io_shell_cbs = {
    .register_received_cb = register_received_cb,
    .register_sended_cb = register_sended_cb,
    .send_buffer = send_buffer,
    .register_connected_cb = register_connected_cb,
    .register_disconnected_cb = register_disconnected_cb,
};

//**************** END SHELL **************************
