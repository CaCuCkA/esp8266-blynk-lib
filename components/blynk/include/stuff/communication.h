/*
 * MIT License - CaCuCkA (2023)
 *
 * Permission to use, copy, modify, and distribute this software for any purpose with or without fee
 * is hereby granted, provided the above copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY. See the full MIT License for details.
 */

#ifndef ESP8266_BLYNK_LIB_COMMUNICATION_H
#define ESP8266_BLYNK_LIB_COMMUNICATION_H

#include "stuff/types.h"
#include "stuff/exceptions.h"


/**
 * @brief Configures a pair of sockets for internal communication.
 *
 * This function sets up two connected datagram (UDP) sockets for local (loopback) communication.
 * The first socket (`fds[READ_SOCK_ID]`) is bound to an automatically assigned port, and the second
 * socket (`fds[WRITE_SOCK_ID]`) is connected to the first socket's port. This pair of sockets can
 * subsequently be used for inter-thread or inter-process communication on the same machine.
 *
 * Note: If any error occurs during the configuration, cleanup is performed for allocated resources
 *       (i.e., sockets are closed) to ensure there are no resource leaks.
 *
 * @param fds An array where the file descriptors of the created sockets will be stored.
 *            `fds[READ_SOCK_ID]` will be the reading socket, while `fds[WRITE_SOCK_ID]`
 *            will be the writing socket.
 * @return CONN_EC_OK on success, or a corresponding error code on failure.
 */
conn_err_t configure_internal_communication(int* socket_fds);


/**
 * @brief Establish a cloud connection to a given server URL.
 *
 * This function extracts the hostname and port from the provided server URL,
 * retrieves address information for the extracted hostname, and then attempts
 * to establish a socket connection to the server. Upon success, the communication
 * socket will be set to non-blocking mode.
 *
 * @param server_url The server URL in the format "hostname:port".
 * @param communication_socket Pointer to an integer where the established socket's
 *                             file descriptor will be stored.
 * @return CONN_EC_OK on successful connection setup, or a corresponding error
 *         code on failure.
 */
conn_err_t setup_cloud_connection(const char* server_url, int* communication_socket);

#endif //ESP8266_BLYNK_LIB_COMMUNICATION_H
