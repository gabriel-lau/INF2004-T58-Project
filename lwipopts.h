#ifndef _LWIPOPTS_H
#define _LWIPOPTS_H

// Generally you would define your own explicit list of lwIP options
// (see https://www.nongnu.org/lwip/2_1_x/group__lwip__opts.html)
//
// This example uses a common include to avoid repetition
#include "lwipopts_examples_common.h"

// This section enables HTTPD server with SSI, SGI
// and tells server which converted HTML files to use
#define LWIP_HTTPD 1
#define LWIP_HTTPD_SSI 1
#define LWIP_HTTPD_CGI 1
#define LWIP_HTTPD_SSI_INCLUDE_TAG 0
#define HTTPD_FSDATA_FILE "driver/wifi/makefsdata.c"

#if !NO_SYS
#define TCPIP_THREAD_STACKSIZE 1024
#define DEFAULT_THREAD_STACKSIZE 1024
#define DEFAULT_RAW_RECVMBOX_SIZE 8
#define TCPIP_MBOX_SIZE 8
#define LWIP_TIMEVAL_PRIVATE 0

// not necessary, can be done either way
#define LWIP_TCPIP_CORE_LOCKING_INPUT 1

// ping_thread sets socket receive timeout, so enable this feature
#define LWIP_SO_RCVTIMEO 1
#endif


#endif
