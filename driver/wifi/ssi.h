#include "lwip/apps/httpd.h"
#include "pico/cyw43_arch.h"
#include "hardware/adc.h"

// SSI tags - tag length limited to 8 bytes by default
const char * ssi_tags[] = {"starting","timer","barcode"};

u16_t ssi_handler(int iIndex, char *pcInsert, int iInsertLen) {
  size_t printed;
  switch (iIndex) {
  case 0: // msg1
    {
      printed = snprintf(pcInsert, iInsertLen, "RECEIVING starting FROM PICO");
    }
    break;
  case 1: // msg2
    {
    printed = snprintf(pcInsert, iInsertLen, "RECEIVING timer FROM PICO");
    }
    break;
  case 2: // msg3
    {
      printed = snprintf(pcInsert, iInsertLen, "RECEIVING barcode FROM PICO");
    }
    break;
  default:
    printed = 0;
    break;
  }

  return (u16_t)printed;
}

// Initialise the SSI handler
void ssi_init() {
  http_set_ssi_handler(ssi_handler, ssi_tags, LWIP_ARRAYSIZE(ssi_tags));
}