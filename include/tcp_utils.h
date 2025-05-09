#ifndef TCP_UTILS_H
#define TCP_UTILS_H
#include "common.h"
#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#define TCP_MAX_MSG_LEN 65535
#define TCP_MAX_CONNECTIONS 1023

    Error tcp_utils_server_init(uint16_t port);
    Error tcp_utils_accept(int*);
    void tcp_utils_close_server_socket(void);
    void tcp_utils_close_client_socket(int);
    Error tcp_utils_read(char*, int);
    Error tcp_utils_write(char*, int);
    Error tcp_utils_send_file(char*, long, int);

#if TEST == 1
    void test_tcp_utils(void);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TCP_UTILS_H */
