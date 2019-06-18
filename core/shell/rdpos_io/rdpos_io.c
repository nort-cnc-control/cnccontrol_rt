#include <rdpos_io.h>
#include <rdpos.h>
#include <rdp.h>


static void (*cb_line_received)(const unsigned char *, size_t);
static void (*cb_sended)(void);

static void (*cb_byte_transmit)(uint8_t b);

static struct rdpos_connection_s sconn;

//****************** SERIAL ************************

static uint8_t serial_outbuf[RDP_MAX_SEGMENT_SIZE];
static int serial_outpos = 0;
static int serial_outlast = 0;

static struct
{
    uint8_t rdy : 1;
    uint8_t connected : 1;
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
        cb_byte_transmit(serial_outbuf[serial_outpos]);
        serial_outpos = (serial_outpos + 1) % sizeof(serial_outbuf);
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
    cb_byte_transmit(serial_outbuf[serial_outpos]);
    serial_outpos = (serial_outpos + 1) % sizeof(serial_outbuf);
}

static void register_byte_transmit(void (*f)(uint8_t))
{
    cb_byte_transmit = f;
}

struct serial_cbs_s rdpos_io_serial_cbs = {
    .register_byte_transmit = register_byte_transmit,
    .byte_received = byte_received,
    .byte_transmitted = byte_transmitted,
};


//**************** END SERIAL ***********************

//***************** RDPOS *****************************

static uint8_t rdp_recv_buf[RDP_MAX_SEGMENT_SIZE];
static uint8_t rdp_outbuf[RDP_MAX_SEGMENT_SIZE];
static uint8_t serial_inbuf[RDP_MAX_SEGMENT_SIZE];

void connected(struct rdp_connection_s *conn)
{
    opts.connected = 1;
}

void closed(struct rdp_connection_s *conn)
{
    opts.connected = 0;
}

void data_send_completed(struct rdp_connection_s *conn)
{
    if (cb_sended)
        cb_sended();
}

void data_received(struct rdp_connection_s *conn, const uint8_t *data, size_t len)
{
    if (cb_line_received)
        cb_line_received(data, len);
}

void ack_wait_start(struct rdp_connection_s *conn, uint32_t id)
{
}

void ack_wait_completed(struct rdp_connection_s *conn, uint32_t id)
{
}

void close_wait_start(struct rdp_connection_s *conn)
{
}

static struct rdpos_buffer_set_s bufs = {
    .rdp_outbuf = rdp_outbuf,
    .rdp_outbuf_len = RDP_MAX_SEGMENT_SIZE,
    .rdp_recvbuf = rdp_recv_buf,
    .rdp_recvbuf_len = RDP_MAX_SEGMENT_SIZE,
    .serial_receive_buf = serial_inbuf,
    .serial_receive_buf_len = RDP_MAX_SEGMENT_SIZE,
};

static struct rdp_cbs_s rdp_cbs = {
    .connected = connected,
    .closed = closed,
    .data_send_completed = data_send_completed,
    .data_received = data_received,
    .ack_wait_start = ack_wait_start,
    .ack_wait_completed = ack_wait_completed,
    .close_wait_start = close_wait_start,
};

static struct rdpos_cbs_s rdpos_cbs = {
    .send_fn = send_serial,
};

static struct rdp_connection_s * const conn = &sconn.rdp_conn;

void rdpos_io_init(void)
{
    rdpos_init_connection(&sconn, &bufs, &rdp_cbs, &rdpos_cbs);
    rdp_listen(conn, 1);
}

//****************** END RDPOS *********************


//**************** SHELL ****************************


static void send_buffer(const unsigned char *buf, size_t len)
{
    rdp_send(conn, buf, len);
}

static void register_received_cb(void (*f)(const unsigned char *, size_t))
{
    cb_line_received = f;
}

static void register_sended_cb(void (*f)(void))
{
    cb_sended = f;
}

struct shell_cbs_s rdpos_io_shell_cbs = {
    .register_received_cb = register_received_cb,
    .register_sended_cb = register_sended_cb,
    .send_buffer = send_buffer,
};

//**************** END SHELL **************************

