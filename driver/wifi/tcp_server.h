#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <stdbool.h>

#define TCP_PORT 4242
#define BUF_SIZE 5000

typedef struct
{
    struct tcp_pcb *server_pcb;
    struct tcp_pcb *client_pcb;
} TCP_SERVER_T;

static int count = 0;

// Function prototypes
void run_tcp_server(void);

// err_t tcp_server_close(void* arg);

#endif
