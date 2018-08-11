#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <tcpip.h>
#include <locator.h>
#include <misctool.h>
#include <memory.h>
#include <orca.h>
#include "http.h"
#include "session.h"
#include "seturl.h"
#include "hostname.h"
#include "tcpconnection.h"
#include "readtcp.h"

/*
http://archive.org/download/a2gs_System_1.0_1986_Apple_FW/System_1.0_1986_Apple_FW.2mg
redirects to:
http://ia800505.us.archive.org/16/items/a2gs_System_1.0_1986_Apple_FW/System_1.0_1986_Apple_FW.2mg
*/

char *defaultURL = "http://archive.org/download/a2gs_System_1.0_1986_Apple_FW/System_1.0_1986_Apple_FW.2mg";

char buf[512];

int main(int argc, char **argv) {
    TLStartUp();
    
    LoadOneTool(54,0x200);
    TCPIPStartUp();
    
    if (TCPIPGetConnectStatus() == FALSE) {
        TCPIPConnect(NULL);
        if (toolerror())
            goto exit;
    }

    Session sess = {0};

    char *url = (argc > 1) ? argv[1] : defaultURL;
    
    enum NetDiskError result = SetURL(&sess, url, TRUE, FALSE);

    if (result != OPERATION_SUCCESSFUL) {
        printf("SetURL error %i\n", (int)result);
        goto exit;
    }
    
    unsigned long startByte = 0;
    if (argc > 2) {
        startByte = strtoul(argv[2], NULL, 0);
    }

    printf("Request:\n");
    printf("=========\n");
    int i;
    for (i = 0; sess.httpRequest[i] != '\0'; i++) {
        char ch = sess.httpRequest[i];
        if (ch != '\r')
            putchar(ch);
    }
    printf("=========\n");
    printf("\n");

    enum NetDiskError requestResult;
    requestResult = DoHTTPRequest(&sess, startByte, startByte + 511);
    printf("RequestResult %i\n", requestResult);
    printf("Response code %lu\n", sess.responseCode);

    if (requestResult == OPERATION_SUCCESSFUL) {
        printf("rangeStart    = %lu\n", sess.rangeStart);
        printf("rangeEnd      = %lu\n", sess.rangeEnd);
        printf("totalLength   = %lu\n", sess.totalLength);
        printf("contentLength = %lu\n", sess.contentLength);
    }
    
    InitReadTCP(&sess, sess.rangeEnd - sess.rangeStart + 1, buf);
    while (TryReadTCP(&sess) == rsWaiting)
        /* keep reading */ ; 
    
    LongWord startTime = GetTick();
    while (GetTick() - startTime < 70)
        /* wait */ ;

    printf("Request 2:\n");
    requestResult = DoHTTPRequest(&sess, startByte + 512, startByte + 1023);
    printf("RequestResult %i\n", requestResult);
    printf("Response code %lu\n", sess.responseCode);

    if (requestResult == OPERATION_SUCCESSFUL) {
        printf("rangeStart    = %lu\n", sess.rangeStart);
        printf("rangeEnd      = %lu\n", sess.rangeEnd);
        printf("totalLength   = %lu\n", sess.totalLength);
        printf("contentLength = %lu\n", sess.contentLength);
    }


    EndTCPConnection(&sess);

exit:
    TCPIPShutDown();
    UnloadOneTool(54);
    TLShutDown();
}
