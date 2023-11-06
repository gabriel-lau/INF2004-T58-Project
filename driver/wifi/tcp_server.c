#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "lwipopts.h"
#include "tcp_server.h"

// TCP_SERVER_T* tcp_server_init(void) {
//     TCP_SERVER_T *state = calloc(1, sizeof(TCP_SERVER_T));
//     if (!state) {
//         return NULL;
//     }
//     return state;
// }

// static err_t tcp_server_close(void *arg) {
//     TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
//     err_t err = ERR_OK;
//     if (state->client_pcb != NULL) {
//         // Close the client connection
//         tcp_abort(state->client_pcb);
//         state->client_pcb = NULL;
//     }
//     if (state->server_pcb) {
//         // Close the server socket
//         tcp_close(state->server_pcb);
//         state->server_pcb = NULL;
//     }
//     return err;
// }

// err_t tcp_server_send_data(void *arg, struct tcp_pcb *tpcb)
// {
//     TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
//     const char *hello = "hello";
//     uint16_t hello_len = strlen(hello);

//     // state->sent_len = 0;
//     printf("Writing %u bytes to client: %s\n", hello_len, hello);

//     // this method is callback from lwIP, so cyw43_arch_lwip_begin is not required, however you
//     // can use this method to cause an assertion in debug mode, if this method is called when
//     // cyw43_arch_lwip_begin IS needed
//     cyw43_arch_lwip_check();

//     err_t err = tcp_write(tpcb, hello, hello_len, TCP_WRITE_FLAG_COPY);

//     if (err != ERR_OK) {
//         printf("Failed to write data %d\n", err);
//     }
//     return ERR_OK;
// }

// err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
//     TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
//     // if (!p) {
//     //     return tcp_server_result(arg, -1);
//     // }
//     // this method is callback from lwIP, so cyw43_arch_lwip_begin is not required, however you
//     // can use this method to cause an assertion in debug mode, if this method is called when
//     // cyw43_arch_lwip_begin IS needed
//     cyw43_arch_lwip_check();

//     // Process the received data
//     char* received_data = (char*)p->payload;
    
//     printf("Received message: %s\n", received_data);

//     pbuf_free(p);

//     // Send another buffer (or "hello" message) to the client
//     return tcp_server_send_data(arg, state->client_pcb);
// }

// static err_t tcp_server_accept(void *arg, struct tcp_pcb *client_pcb, err_t err) {
//     TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
//     if (err != ERR_OK || client_pcb == NULL) {
//         return ERR_VAL;
//     }

//     state->client_pcb = client_pcb;
//     tcp_arg(client_pcb, state);
//     tcp_recv(client_pcb, tcp_server_recv);

//     // Send data to the client
//     return tcp_server_send_data(arg, client_pcb);
// }

// bool tcp_server_open(void *arg) {
//     TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
//     printf("Starting tcp server at %s on port %u\n\n", ip4addr_ntoa(netif_ip4_addr(netif_list)), TCP_PORT);

//     // Create a new PCB
//     state->server_pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
//     if (!state->server_pcb) {
//         return false;
//     }

//     // Bind and listen
//     if (tcp_bind(state->server_pcb, NULL, TCP_PORT) != ERR_OK) {
//         return false;
//     }

//     state->server_pcb = tcp_listen(state->server_pcb);
//     if (!state->server_pcb) {
//         return false;
//     }

//     tcp_accept(state->server_pcb, tcp_server_accept);

//     return true;
// }

static TCP_SERVER_T* tcp_server_init(void) {
    TCP_SERVER_T *state = calloc(1, sizeof(TCP_SERVER_T));
    if (!state) {
        printf("failed to allocate state\n");
        return NULL;
    }
    return state;
}

static err_t tcp_server_close(void *arg) {
    TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
    err_t err = ERR_OK;
    if (state->client_pcb != NULL) {
        tcp_arg(state->client_pcb, NULL);
        tcp_poll(state->client_pcb, NULL, 0);
        tcp_sent(state->client_pcb, NULL);
        tcp_recv(state->client_pcb, NULL);
        tcp_err(state->client_pcb, NULL);
        err = tcp_close(state->client_pcb);
        if (err != ERR_OK) {
            printf("close failed %d, calling abort\n", err);
            tcp_abort(state->client_pcb);
            err = ERR_ABRT;
        }
        state->client_pcb = NULL;
    }
    if (state->server_pcb) {
        tcp_arg(state->server_pcb, NULL);
        tcp_close(state->server_pcb);
        state->server_pcb = NULL;
    }
    return err;
}

static err_t tcp_server_result(void *arg, int status) {
    TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
    if (status == 0) {
        printf("test success\n");
    } else {
        printf("test failed %d\n", status);
    }
    state->complete = true;
    return tcp_server_close(arg);
}

static err_t tcp_server_sent(void *arg, struct tcp_pcb *tpcb, u16_t len) {
    TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
    printf("tcp_server_sent %u\n", len);
    state->sent_len += len;

    if (state->sent_len >= BUF_SIZE) {

        // We should get the data back from the client
        state->recv_len = 0;
        printf("Waiting for buffer from client\n");
    }

    return ERR_OK;
}

err_t tcp_server_send_data(void *arg, struct tcp_pcb *tpcb)
{
    TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
    const char *hello = "hello";
    uint16_t hello_len = strlen(hello);
    // for(int i=0; i< BUF_SIZE; i++) {
    //     state->buffer_sent[i] = rand();
    // }

    state->sent_len = 0;
    // printf("Writing %ld bytes to client\n", BUF_SIZE);
    printf("Writing %u bytes to client: %s\n", hello_len, hello);

    // this method is callback from lwIP, so cyw43_arch_lwip_begin is not required, however you
    // can use this method to cause an assertion in debug mode, if this method is called when
    // cyw43_arch_lwip_begin IS needed
    cyw43_arch_lwip_check();
    // err_t err = tcp_write(tpcb, state->buffer_sent, BUF_SIZE, TCP_WRITE_FLAG_COPY);
    err_t err = tcp_write(tpcb, hello, hello_len, TCP_WRITE_FLAG_COPY);

    if (err != ERR_OK) {
        printf("Failed to write data %d\n", err);
        return tcp_server_result(arg, -1);
    }
    return ERR_OK;
}

err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
    if (!p) {
        return tcp_server_result(arg, -1);
    }
    // this method is callback from lwIP, so cyw43_arch_lwip_begin is not required, however you
    // can use this method to cause an assertion in debug mode, if this method is called when
    // cyw43_arch_lwip_begin IS needed
    cyw43_arch_lwip_check();

    // Process the received data
    char* received_data = (char*)p->payload;
    
    printf("Received message: %s\n", received_data);

    pbuf_free(p);

    // Check if the test is complete
    if (state->run_count >= TEST_ITERATIONS) {
        tcp_server_result(arg, 0);
        return ERR_OK;
    }

    // Send another buffer (or "hello" message) to the client
    return tcp_server_send_data(arg, state->client_pcb);
}

static err_t tcp_server_poll(void *arg, struct tcp_pcb *tpcb) {
    printf("tcp_server_poll_fn\n");
    return tcp_server_result(arg, -1); // no response is an error?
}

static void tcp_server_err(void *arg, err_t err) {
    if (err != ERR_ABRT) {
        printf("tcp_client_err_fn %d\n", err);
        tcp_server_result(arg, err);
    }
}

static err_t tcp_server_accept(void *arg, struct tcp_pcb *client_pcb, err_t err) {
    TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
    if (err != ERR_OK || client_pcb == NULL) {
        printf("Failure in accept\n");
        tcp_server_result(arg, err);
        return ERR_VAL;
    }
    printf("Client connected\n");

    state->client_pcb = client_pcb;
    tcp_arg(client_pcb, state);
    tcp_sent(client_pcb, tcp_server_sent);
    tcp_recv(client_pcb, tcp_server_recv);
    tcp_poll(client_pcb, tcp_server_poll, POLL_TIME_S * 2);
    tcp_err(client_pcb, tcp_server_err);

    return tcp_server_send_data(arg, state->client_pcb);
}

static bool tcp_server_open(void *arg) {
    TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
    printf("Starting tcp server at %s on port %u\n", ip4addr_ntoa(netif_ip4_addr(netif_list)), TCP_PORT);

    struct tcp_pcb *pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
    if (!pcb) {
        printf("failed to create pcb\n");
        return false;
    }

    err_t err = tcp_bind(pcb, NULL, TCP_PORT);
    if (err) {
        printf("failed to bind to port %u\n", TCP_PORT);
        return false;
    }

    state->server_pcb = tcp_listen_with_backlog(pcb, 1);
    if (!state->server_pcb) {
        printf("failed to listen\n");
        if (pcb) {
            tcp_close(pcb);
        }
        return false;
    }

    tcp_arg(state->server_pcb, state);
    tcp_accept(state->server_pcb, tcp_server_accept);

    return true;
}

void run_tcp_server(void) {
    TCP_SERVER_T *state = tcp_server_init();
    if (!state) {
        return;
    }
    
    if (!tcp_server_open(state)) {
        tcp_server_result(state, -1);
        return;
    }
    // Initialise tcp server
    printf("TCP Server initialised\n");

    while(!state->complete) {
      // the following #ifdef is only here so this same example can be used in multiple modes;
      // you do not need it in your code
      #if PICO_CYW43_ARCH_POLL
              // if you are using pico_cyw43_arch_poll, then you must poll periodically from your
              // main loop (not from a timer) to check for Wi-Fi driver or lwIP work that needs to be done.
              cyw43_arch_poll();
              // you can poll as often as you like, however if you have nothing else to do you can
              // choose to sleep until either a specified time, or cyw43_arch_poll() has work to do:
              cyw43_arch_wait_for_work_until(make_timeout_time_ms(1000));
      #else
              // if you are not using pico_cyw43_arch_poll, then WiFI driver and lwIP work
              // is done via interrupt in the background. This sleep is just an example of some (blocking)
              // work you might be doing.
              sleep_ms(1000);
      #endif
    }
    free(state);
}

// // Clean up
// tcp_server_close(state);
// cyw43_arch_deinit();