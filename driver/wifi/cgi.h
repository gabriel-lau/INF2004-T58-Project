#include "lwip/apps/httpd.h"
#include "pico/cyw43_arch.h"

#include "wifi.h"

// CGI handler which is run when a request for /car.cgi is detected
const char *cgi_start_handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    // Check if a request for car has been made (/car.cgi?time=x&mode=y)
    int modeFound = 0; // Flag to check if 'mode' parameter is found
    int timeFound = 0; // Flag to check if 'time' parameter is found
    int speedFound = 0; // Flag to check if 'time' parameter is found

    for (int i = 0; i < iNumParams; i++) {
        // Check if an request for car has been made (/car.cgi?time=x&mode=y)
        if (strcmp(pcParam[i], "mode") == 0) {
            // Look at the 'mode' argument to perform mode-specific actions
            if (strcmp(pcValue[i], "map") == 0) {
                printf("scanning map\n");
                testCase1();
            } else if(strcmp(pcValue[i], "shortest") == 0) {
                printf("getting shortest path\n");
                testCase2();
            }
            modeFound = 1;
        } else if (strcmp(pcParam[i] , "time") == 0){
            // Look at the 'time' argument to perform time-specific actions
            printf("start in %ss\n", pcValue[i]);
            snprintf(cgi_result, sizeof(cgi_result), pcValue[i]);
            timeFound = 1;
        } else if (strcmp(pcParam[i] , "speed") == 0){
            // Look at the 'speed' argument to perform speed-specific actions
            printf("speed is %ss\n", pcValue[i]);
            speedFound = 1;
        }
    }

    if (modeFound && timeFound && speedFound) {
        printf("ready\n\n");
    }
        
    // Send the index page back to the user
    return "/index.shtml";
}

// tCGI Struct
// Fill this with all of the CGI requests and their respective handlers
static const tCGI cgi_handlers[] = {
    {// Html request for "/start.cgi" triggers cgi_handler
     "/start.cgi", cgi_start_handler},
};

void cgi_init(void)
{
    http_set_cgi_handlers(cgi_handlers, 1);
}