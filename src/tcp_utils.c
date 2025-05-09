#include "tcp_utils.h"
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/select.h>

#ifdef __linux__
#include <sys/sendfile.h>
#endif /* __linux__ */

static bool g_initialized = false;
static int g_server_socket;

void __flush(int sock)
{
    char* buff[4096];
    while (1)
    {
        int res = read(sock, buff, 4096);
        if (res < 0)
        {
            LOG_WARNING("Failed to flush socket - `%s`", strerror(errno));
            break;
        }
        if (res == 0)
        {
            break;
        }
    }
}

Error tcp_utils_server_init(uint16_t port)
{
    if (g_initialized)
    {
        LOG_ERROR("TCP server already initialized");
        return ERR_PERMISSION_DENIED;
    }

    const int domain   = AF_INET; // (= PF_INET) Internet domain sockets for use with IPv4 addresses
    const int type     = SOCK_STREAM; // bitstream socket used in TCP
    const int protocol = 0; // default type of socket that works with the other chosen params
    int on             = 1;

    // create a socket
    g_server_socket = socket(domain, type, protocol);
    if (g_server_socket == -1)
    {
        LOG_PERROR("Failed to create socket");
        return ERR_TCP_INTERNAL;
    }

    if (setsockopt(g_server_socket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1)
    {
        LOG_PERROR("Failed to set socket options");
        return ERR_TCP_INTERNAL;
    }
    // specify address
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port   = htons(port);
#ifdef __linux__
    // On RPi, we get traffic to/from the network.
    server_address.sin_addr.s_addr = INADDR_ANY;
#else
    // On MacOS, we are probably using localhost as server address.
    server_address.sin_addr.s_addr = htonl(0x7F000001 /*127.0.0.1*/);
#endif

    if (bind(g_server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1)
    {
        LOG_PERROR("Failed to bind to network socket");
        return ERR_TCP_INTERNAL;
    }

    // listen to connections
    if (listen(g_server_socket, SOMAXCONN) == -1)
    {
        LOG_PERROR("Failed to listen to socket");
        return ERR_TCP_INTERNAL;
    }
    g_initialized = true;
    return ERR_ALL_GOOD;
}

Error tcp_utils_accept(int* out_client_socket)
{
    if (!g_initialized)
    {
        LOG_ERROR("TCP server not initialized yet.");
        return ERR_PERMISSION_DENIED;
    }
    struct sockaddr_in client;
    socklen_t client_size;

    *out_client_socket = accept(g_server_socket, (struct sockaddr*)&client, &client_size);
    if (*out_client_socket == -1)
    {
        LOG_PERROR("Problem with client connecting");
        return ERR_TCP_INTERNAL;
    }

    LOG_INFO("Connection to socket nr. %d accepted.", *out_client_socket);
    return ERR_ALL_GOOD;
}

void tcp_utils_close_server_socket(void)
{
    LOG_INFO("Closing server socket Nr. `%d`.", g_server_socket);
    shutdown(g_server_socket, SHUT_WR);
    __flush(g_server_socket);
    close(g_server_socket);
}

void tcp_utils_close_client_socket(int client_socket)
{
    LOG_INFO("Closing client socket Nr. `%d`", client_socket);
    close(client_socket);
}

Error tcp_utils_read(char* in_buff, int client_socket)
{
    LOG_TRACE("Trying to receive data.");
    fd_set set;
    struct timeval timeout;
    int rv;
    FD_ZERO(&set);               /* clear the set */
    FD_SET(client_socket, &set); /* add our file descriptor to the set */
    timeout.tv_sec  = 1;
    timeout.tv_usec = 0;

    rv = select(TCP_MAX_CONNECTIONS + 1, &set, NULL, NULL, &timeout);
    if (rv == -1)
    {
        perror("select"); /* an error accured */
        FD_CLR(client_socket, &set);
        return ERR_TCP_INTERNAL;
    }
    else if (rv == 0)
    {
        LOG_ERROR("[SOCKET `%d`] timeout", client_socket); /* a timeout occured */
        FD_CLR(client_socket, &set);
        return ERR_TCP_INTERNAL;
    }
    FD_CLR(client_socket, &set);
    int bytes_recv = read(client_socket, in_buff, TCP_MAX_MSG_LEN);
    if (bytes_recv == -1)
    {
        LOG_PERROR("[SOCKET `%d`] read error", client_socket);
        return ERR_TCP_INTERNAL;
    }
    LOG_TRACE("[SOCKET `%d`] - bytes `%d`", client_socket, bytes_recv);
    return ERR_ALL_GOOD;
}

Error tcp_utils_write(char* out_buff_char_p, int client_socket)
{
    LOG_TRACE("[SOCKET `%d`] Trying to send data.", client_socket);
    if (write(client_socket, out_buff_char_p, strlen(out_buff_char_p)) == -1)
    {
        LOG_PERROR("[SOCKET `%d`] Failed to send data.", client_socket);
        return ERR_TCP_INTERNAL;
    }
    LOG_TRACE("[SOCKET `%d`] Data successfully sent.", client_socket);
    return ERR_ALL_GOOD;
}

Error tcp_utils_send_file(char* file_path, long file_size, int client_socket)
{
    int resource_file = open(file_path, O_RDONLY);
    if (resource_file == -1)
    {
        LOG_PERROR("Error opening file");
        return ERR_UNEXPECTED;
    }
#ifdef __linux__
    ssize_t bytes_sent = sendfile(client_socket, resource_file, NULL, file_size);
    LOG_INFO("Size `%ld` bytes.", file_size);
    LOG_INFO("Sent `%ld` bytes.", bytes_sent);
    if (bytes_sent == -1)
#else
    off_t len                      = file_size; // set to 0 will send all the origin file
    int res                        = sendfile(resource_file, client_socket, 0, &len, NULL, 0);
    LOG_INFO("Sent `%lld` bytes.", len);
    if (res == -1)
#endif
    {
        LOG_PERROR("Failed to send file");
        close(resource_file);
        return ERR_TCP_INTERNAL;
    }
    close(resource_file);
    return ERR_ALL_GOOD;
}

#if TEST == 1
void test_tcp_utils(void) {}
#endif
