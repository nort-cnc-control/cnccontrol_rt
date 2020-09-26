#pragma once

#include <stdint.h>



typedef uint8_t u8_t;
typedef uint16_t u16_t;
typedef unsigned short uip_stats_t;

#define UIP_CONF_MAX_CONNECTIONS 2
#define UIP_CONF_MAX_LISTENPORTS 1
#define UIP_CONF_BUFFER_SIZE     420
#define UIP_CONF_BYTE_ORDER      LITTLE_ENDIAN
#define UIP_CONF_LOGGING         0
#define UIP_CONF_UDP             0
#define UIP_CONF_STATISTICS      0

#include <uipopt.h>
#include <psock.h>

typedef struct net_appstate {
    struct psock p;
    char inputbuffer[100];
} uip_tcp_appstate_t;

#define UIP_APPCALL net_appcall

