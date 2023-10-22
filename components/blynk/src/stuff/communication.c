#include <lwip/netdb.h>

#include "stuff/util.h"
#include "stuff/defines.h"
#include "stuff/communication.h"

#define TAG  "[CONNECTION]"


typedef struct addrinfo addrinfo_t;


static void close_sockets(int* fds, int count);

static void set_socket_nonblocking_opt(int socket);

static conn_err_t establish_socket_connection(int* conn_socket, addrinfo_t* addrinfo);

static conn_err_t get_hostname_and_port(const char* server_url, char* hostname, char** port);

static conn_err_t get_addrinfo(const char* hostname, const char* port, addrinfo_t** addrinfo);


conn_err_t
setup_cloud_connection(const char* server_url, int* communication_socket) {
    char hostname[HOSTNAME_SIZE];
    char* port;

    if (get_hostname_and_port(server_url, hostname, &port) != CONN_EC_OK) {
        log_error("%s: Function %s failed to get hostname and port from server url", TAG, __func__);
        return CONN_EC_GET_HOST_AND_PORT;
    }

    addrinfo_t* address_information;
    if (get_addrinfo(hostname, port, &address_information) != CONN_EC_OK) {
        log_error("%s: Function %s cannot retrieve address information", TAG, __func__);
        return CONN_EC_GET_ADDRINFO;
    }

    if (establish_socket_connection(communication_socket, address_information) != CONN_EC_OK) {
        log_error("%s: Function %s cannot establish socket connection", TAG, __func__);
        return CONN_EC_FAILED_ESTALE_CONN;
    }
    set_socket_nonblocking_opt(*communication_socket);

    return CONN_EC_OK;
}


static conn_err_t
get_hostname_and_port(const char* server_url, char* hostname, char** port) {
    if (CHECK_PTR(TAG, server_url, hostname, port)) return CONN_EC_NULL_PTR;

    strlcpy(hostname, server_url, HOSTNAME_SIZE);

    char* delimiter_position = strchr(hostname, URL_DELIMITER);

    if (CHECK_PTR(TAG, delimiter_position)) return CONN_EC_NULL_PTR;

    *delimiter_position = '\0';
    *port = delimiter_position + 1;

    return CONN_EC_OK;
}


static conn_err_t
get_addrinfo(const char* hostname, const char* port, addrinfo_t** addrinfo) {
    if (CHECK_PTR(TAG, hostname, port, addrinfo)) return CONN_EC_NULL_PTR;

    addrinfo_t hints;
    memset(&hints, 0, sizeof(addrinfo_t));

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_NUMERICSERV;
    if (getaddrinfo(hostname, port, &hints, addrinfo) != CONN_EC_OK) return CONN_EC_GET_ADDRINFO;
    return CONN_EC_OK;
}


static conn_err_t
establish_socket_connection(int* conn_socket, addrinfo_t* addrinfo) {
    if (CHECK_PTR(TAG, addrinfo)) return CONN_EC_FAILED_ESTALE_CONN;

    addrinfo_t* cur_addr;
    for (cur_addr = addrinfo; cur_addr != NULL; cur_addr = cur_addr->ai_next) {
        *conn_socket = socket(cur_addr->ai_family, cur_addr->ai_socktype, cur_addr->ai_protocol);

        if (IS_SOCKET_INVALID(*conn_socket)) {
            continue;
        }

        if (!SYSCALL_FAILED(connect(*conn_socket, cur_addr->ai_addr, cur_addr->ai_addrlen))) return CONN_EC_OK;

        close(*conn_socket);
    }

    *conn_socket = CONNECTION_FAILED;
    freeaddrinfo(addrinfo);

    return CONN_EC_FAILED_ESTALE_CONN;
}


static void
set_socket_nonblocking_opt(int socket) {
    int flag = fcntl(socket, F_GETFL);
    if (SYSCALL_FAILED(flag)) {
        log_error("Error getting socket flags");
        return;
    }

    if (SYSCALL_FAILED(fcntl(socket, F_SETFL, flag | O_NONBLOCK))) {
        log_error("Error setting socket to non-blocking");
    }
}


conn_err_t
configure_internal_communication(int* fds) {
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);

    fds[READ_SOCK_ID] = socket(AF_INET, SOCK_DGRAM, UDP_PROTOCOL);
    if (SYSCALL_FAILED(fds[READ_SOCK_ID])) return CONN_EC_CREATE_SOCK;

    memset(&addr, 0, len);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(IPV4_LOCALHOST);
    addr.sin_port = htons(AUTO_ASSIGN_PORT);

    if (SYSCALL_FAILED(bind(fds[READ_SOCK_ID], (struct sockaddr*) &addr, len))) {
        close_sockets(fds, 1);
        return CONN_EC_SOCK_BIND;
    }

    if (SYSCALL_FAILED(getsockname(fds[READ_SOCK_ID], (struct sockaddr*) &addr, &len))) {
        close_sockets(fds, 1);
        return CONN_EC_GET_NAME;
    }

    fds[1] = socket(AF_INET, SOCK_DGRAM, UDP_PROTOCOL);
    if (fds[1] < 0) {
        close_sockets(fds, 1);
        return CONN_EC_CREATE_SOCK;
    }

    if (SYSCALL_FAILED(connect(fds[WRITE_SOCK_ID], (struct sockaddr*) &addr, len))) {
        close_sockets(fds, 2);
        return CONN_EC_SOCK_CONN;
    }

    if (SYSCALL_FAILED(getsockname(fds[WRITE_SOCK_ID], (struct sockaddr*) &addr, &len))) {
        close_sockets(fds, 2);
        return CONN_EC_GET_NAME;
    }

    if (SYSCALL_FAILED(connect(fds[READ_SOCK_ID], (struct sockaddr*) &addr, len))) {
        close_sockets(fds, 2);
        return CONN_EC_SOCK_CONN;
    }

    return CONN_EC_OK;
}


static void
close_sockets(int* fds, int count) {
    for (int i = 0; i < count; i++) {
        close(fds[i]);
    }
}
