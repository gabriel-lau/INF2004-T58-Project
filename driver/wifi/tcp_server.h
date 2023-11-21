#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <stdbool.h>
#include "FreeRTOS.h"
#include "task.h"
#include "message_buffer.h"
#include "semphr.h"

#define TCP_PORT 4242
#define BUF_SIZE 1000

// Function prototypes
void initialize_mutex(void);
void run_tcp_server(void);
void send_message(const char *message);

// err_t tcp_server_close(void* arg);

#endif