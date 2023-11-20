#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "lwipopts.h"

#include "tcp_server.h"

static struct tcp_pcb *client_pcb = NULL;
void send_message(const char *message);

static void tcp_server_err(void *arg, err_t err)
{
    printf("TCP Server Error: %d\n", err);
}

static err_t tcp_server_accept(void *arg, struct tcp_pcb *pcb, err_t err)
{
    if (err != ERR_OK || pcb == NULL)
    {
        printf("Error in accept\n");
        return ERR_VAL;
    }

    printf("Client connected\n");
    client_pcb = pcb;

    tcp_arg(client_pcb, NULL);
    tcp_err(client_pcb, tcp_server_err);

    // Send initial data to the client
    send_message("Hello, Pico!");

    return ERR_OK;
}

static bool tcp_server_open(void)
{
    struct tcp_pcb *server_pcb;

    printf("Starting TCP server on port %u\n", TCP_PORT);

    server_pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
    if (!server_pcb)
    {
        printf("Failed to create PCB\n");
        return false;
    }

    err_t bind_err = tcp_bind(server_pcb, NULL, TCP_PORT);
    if (bind_err)
    {
        printf("Failed to bind to port %u\n", TCP_PORT);
        return false;
    }

    struct tcp_pcb *listen_pcb = tcp_listen(server_pcb);
    if (!listen_pcb)
    {
        printf("Failed to listen\n");
        return false;
    }

    tcp_arg(listen_pcb, NULL);
    tcp_accept(listen_pcb, tcp_server_accept);

    return true;
}

void send_message(const char *message)
{
    if (client_pcb)
    {
        // cyw43_arch_lwip_begin IS needed
        cyw43_arch_lwip_check();

        err_t send_err = tcp_write(client_pcb, message, strlen(message), TCP_WRITE_FLAG_COPY);

        if (send_err != ERR_OK)
        {
            printf("Failed to write data %d\n", send_err);
        }
        else
        {
            printf("Data sent to client: %s\n", message);
        }
    }
    else
    {
        printf("No client connected\n");
    }
}

void run_tcp_server(void)
{
    if (!tcp_server_open())
    {
        printf("Failed to open TCP server\n");
        return;
    }

    // Wait for the server to complete its task
    while (1)
    {
        cyw43_arch_poll();
        sleep_ms(1000);
    }
}
