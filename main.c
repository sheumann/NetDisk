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

#define buffTypePointer 0x0000      /* For TCPIPReadTCP() */
#define buffTypeHandle 0x0001
#define buffTypeNewHandle 0x0002

/*
http://archive.org/download/a2gs_System_1.0_1986_Apple_FW/System_1.0_1986_Apple_FW.2mg
redirects to:
http://ia800505.us.archive.org/16/items/a2gs_System_1.0_1986_Apple_FW/System_1.0_1986_Apple_FW.2mg
*/

static char delimitStr[] = "\p\r\n\r\n";

char *defaultURL = "http://ia800505.us.archive.org/16/items/a2gs_System_1.0_1986_Apple_FW/System_1.0_1986_Apple_FW.2mg";


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
    
    enum SetURLResult result = SetURL(&sess, url, TRUE, FALSE);

    if (result != SETURL_SUCCESSFUL) {
        printf("SetURL error %i\n", (int)result);
        goto exit;
    }
    
    unsigned long startByte = 0;
    if (argc > 2) {
        startByte = strtoul(argv[2], NULL, 0);
    }
    UpdateRequestRange(&sess, startByte, startByte + 511);

    printf("Request:\n");
    printf("=========\n");
    int i;
    for (i = 0; sess.httpRequest[i] != '\0'; i++) {
        char ch = sess.httpRequest[i];
        if (ch != '\r')
            putchar(ch);
    }
    printf("=========\n");
    printf("\n", i);
    
    int err;
    if ((err = StartTCPConnection(&sess)) != 0) {
        printf("StartTCPConnection error %i\n", err);
        goto exit;
    }
    
    TCPIPWriteTCP(sess.ipid, sess.httpRequest, strlen(sess.httpRequest), TRUE, FALSE);

    rlrBuff rlrBuff = {0};
    Word tcpError;
    do {
        TCPIPPoll();
        tcpError = TCPIPReadLineTCP(sess.ipid,
                (void*)((LongWord)delimitStr | 0x80000000),
                buffTypeNewHandle, (Ref)NULL,
                0xFFFFFF, &rlrBuff);
        if (tcpError || toolerror()) {
            printf("tcpError = %u, toolerror = %u\n", tcpError, toolerror());
            break;
        }
    } while (rlrBuff.rlrBuffCount == 0);

    printf("Response:\n");
    printf("=========\n");
    for (int i = 0; i < rlrBuff.rlrBuffCount; i++) {
        char ch = ((char*)*(rlrBuff.rlrBuffHandle))[i];
        if (ch != '\r')
            putchar(ch);
    }
    printf("=========\n");
    printf("Response size = %lu\n", rlrBuff.rlrBuffCount);
    
    EndTCPConnection(&sess);

exit:
    TCPIPShutDown();
    UnloadOneTool(54);
    TLShutDown();
}
