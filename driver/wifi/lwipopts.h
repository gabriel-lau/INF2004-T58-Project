#include "../../lwipopts_examples_common.h"
#include "../../lwipopts.h"

// This section enables HTTPD server with SSI, SGI
// and tells server which converted HTML files to use
#define LWIP_HTTPD 1
#define LWIP_HTTPD_SSI 1
#define LWIP_HTTPD_CGI 1
#define LWIP_HTTPD_SSI_INCLUDE_TAG 0
#define HTTPD_FSDATA_FILE "makefsdata.c"