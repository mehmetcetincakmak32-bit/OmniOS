/*
 * OmniOS Network Stack
 * Lightweight TCP/IP stack skeleton for mobile devices
 */

#include "../include/omnios_kernel.h"
#include <string.h>
#include <stdio.h>

#define MAX_INTERFACES     4
#define MAX_PROTOCOLS      8
#define MAX_SOCKETS        64

typedef struct {
    uint32_t    ip;
    uint32_t    netmask;
    uint32_t    gateway;
    mac_addr_t  mac;
    bool        up;
    char        name[16];
} net_interface_t;

typedef struct {
    net_protocol_t proto;
    void (*handler)(const uint8_t* data, uint32_t len);
} protocol_handler_t;

typedef struct {
    uint32_t        id;
    socket_type_t   type;
    net_protocol_t  proto;
    uint32_t        local_ip;
    uint16_t        local_port;
    uint32_t        remote_ip;
    uint16_t        remote_port;
    bool            connected;
    bool            bound;
    uint8_t*        buffer;
    uint32_t        buffer_size;
} socket_t;

static net_interface_t _interfaces[MAX_INTERFACES];
static uint32_t _if_count = 0;

static protocol_handler_t _protocols[MAX_PROTOCOLS];
static uint32_t _proto_count = 0;

static socket_t _sockets[MAX_SOCKETS];
static uint32_t _socket_count = 0;
static uint32_t _next_socket_id = 1;

static uint32_t _packets_sent = 0;
static uint32_t _packets_recv = 0;

status_t net_init(void) {
    memset(_interfaces, 0, sizeof(_interfaces));
    memset(_protocols, 0, sizeof(_protocols));
    memset(_sockets, 0, sizeof(_sockets));
    _if_count = 0;
    _proto_count = 0;
    _socket_count = 0;
    _packets_sent = 0;
    _packets_recv = 0;

    printf("[NET] Ag yigini baslatildi\n");
    return STATUS_SUCCESS;
}

status_t net_interface_add(const char* name, uint32_t ip, uint32_t netmask, uint32_t gateway) {
    if (_if_count >= MAX_INTERFACES) return STATUS_NO_MEMORY;

    net_interface_t* iface = &_interfaces[_if_count];
    strncpy(iface->name, name, sizeof(iface->name) - 1);
    iface->ip = ip;
    iface->netmask = netmask;
    iface->gateway = gateway;
    iface->up = true;
    _if_count++;

    printf("[NET] Arayuz %s: %d.%d.%d.%d/%d\n",
           name,
           (ip >> 24) & 0xFF, (ip >> 16) & 0xFF,
           (ip >> 8) & 0xFF, ip & 0xFF,
           netmask);

    return STATUS_SUCCESS;
}

status_t net_send_packet(const uint8_t* data, uint32_t len, net_protocol_t proto) {
    if (!data || len == 0 || len > MAX_PACKET_SIZE) return STATUS_INVALID;
    if (_if_count == 0) return STATUS_NOT_FOUND;

    /* In real implementation: DMA to NIC, handle ARP, etc. */
    _packets_sent++;
    return STATUS_SUCCESS;
}

status_t net_receive_packet(const uint8_t* data, uint32_t len, net_protocol_t proto) {
    if (!data || len == 0) return STATUS_INVALID;
    _packets_recv++;

    /* Dispatch to protocol handler */
    for (uint32_t i = 0; i < _proto_count; i++) {
        if (_protocols[i].proto == proto && _protocols[i].handler) {
            _protocols[i].handler(data, len);
            return STATUS_SUCCESS;
        }
    }
    return STATUS_NOT_FOUND;
}

status_t net_register_protocol(net_protocol_t proto, void (*handler)(const uint8_t*, uint32_t)) {
    if (!handler) return STATUS_INVALID;
    if (_proto_count >= MAX_PROTOCOLS) return STATUS_NO_MEMORY;

    _protocols[_proto_count].proto = proto;
    _protocols[_proto_count].handler = handler;
    _proto_count++;

    printf("[NET] Protokol kaydedildi: 0x%04x\n", proto);
    return STATUS_SUCCESS;
}

/* Socket API */
int net_socket_create(socket_type_t type, net_protocol_t proto) {
    if (_socket_count >= MAX_SOCKETS) return -1;

    socket_t* sock = &_sockets[_socket_count];
    sock->id = _next_socket_id++;
    sock->type = type;
    sock->proto = proto;
    sock->connected = false;
    sock->bound = false;
    sock->buffer = NULL;
    sock->buffer_size = 0;
    _socket_count++;

    return sock->id;
}

status_t net_socket_bind(int socket_id, uint16_t port) {
    for (uint32_t i = 0; i < _socket_count; i++) {
        if (_sockets[i].id == (uint32_t)socket_id) {
            _sockets[i].local_port = port;
            _sockets[i].bound = true;
            return STATUS_SUCCESS;
        }
    }
    return STATUS_NOT_FOUND;
}

status_t net_socket_connect(int socket_id, uint32_t ip, uint16_t port) {
    for (uint32_t i = 0; i < _socket_count; i++) {
        if (_sockets[i].id == (uint32_t)socket_id) {
            _sockets[i].remote_ip = ip;
            _sockets[i].remote_port = port;
            _sockets[i].connected = true;
            return STATUS_SUCCESS;
        }
    }
    return STATUS_NOT_FOUND;
}

status_t net_socket_send(int socket_id, const uint8_t* data, uint32_t len) {
    for (uint32_t i = 0; i < _socket_count; i++) {
        if (_sockets[i].id == (uint32_t)socket_id && _sockets[i].connected) {
            return net_send_packet(data, len, _sockets[i].proto);
        }
    }
    return STATUS_NOT_FOUND;
}

uint32_t net_get_stats(void) {
    return _packets_sent + _packets_recv;
}
