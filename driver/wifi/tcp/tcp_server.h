#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <stdbool.h>

#define TCP_PORT 4242
#define BUF_SIZE 2048
#define TEST_ITERATIONS 10
#define POLL_TIME_S 5

typedef struct TCP_SERVER_T_ {
    struct tcp_pcb *server_pcb;
    struct tcp_pcb *client_pcb;
    bool complete;
    uint8_t buffer_sent[BUF_SIZE];
    uint8_t buffer_recv[BUF_SIZE];
    int sent_len;
    int recv_len;
    int run_count;
} TCP_SERVER_T;

// Function prototypes
void run_tcp_server(void);

// err_t tcp_server_close(void* arg);

#endif
