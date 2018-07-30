#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <tcpip.h>
#include <locator.h>
#include <misctool.h>
#include <memory.h>
#include "http.h"
#include "session.h"
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

int main(void) {
    TLStartUp();
    
    LoadOneTool(54,0x200);
    TCPIPStartUp();
    
    TCPIPConnect(NULL);

    Session sess;
    FILE *f = fopen("out", "wb");
    
    BuildHTTPRequest(&sess, "/16/items/a2gs_System_1.0_1986_Apple_FW/System_1.0_1986_Apple_FW.2mg", "ia800505.us.archive.org");
    fputs(sess.httpRequest, f);
    
    UpdateRequestRange(&sess, 0, 511);
    fputs(sess.httpRequest, f);
    
    strcpy(sess.domainName, "\pia800505.us.archive.org");
    Boolean result = DoLookupName(&sess);
    
    if (result) {
        printf("IP = %lu.%lu.%lu.%lu\n",
               sess.ipAddr & 0xFF,
               (sess.ipAddr >> 8)  & 0xFF,
               (sess.ipAddr >> 16) & 0xFF,
               (sess.ipAddr >> 24));
    } else {
        printf("Address lookup failed\n");
        goto exit;
    }
    
    sess.port = 80;
    
    int err;
    if ((err = StartTCPConnection(&sess)) != 0) {
        printf("StartTCPConnection error %i\n", err);
        goto exit;
    }
    
    TCPIPWriteTCP(sess.ipid, sess.httpRequest, strlen(sess.httpRequest), TRUE, FALSE);
    
    LongWord startTime = GetTick();
    
    while (GetTick() - startTime < 3*60) {
        //printf("polling\n");
        TCPIPPoll();
    }

    rlrBuff rlrBuff;
    TCPIPReadLineTCP(sess.ipid, (void*)((LongWord)delimitStr | 0x80000000),
                     buffTypeNewHandle, (Ref)NULL,
                     0xFFFFFF, &rlrBuff);

    printf("Response:\n");
    printf("=========\n");
    for (int i = 0; i < rlrBuff.rlrBuffCount; i++) {
        char ch = ((char*)*(rlrBuff.rlrBuffHandle))[i];
        if (ch != '\r')
            putchar(ch);
    }
    //printf("%s\n", *(rlrBuff.rlrBuffHandle));
    printf("=========\n");
    
    printf("Response headers handle size = %lu \n", GetHandleSize(rlrBuff.rlrBuffHandle));
    printf("Reported response size       = %lu \n", rlrBuff.rlrBuffCount);
    
    EndTCPConnection(&sess);

exit:
    TCPIPShutDown();
    UnloadOneTool(54);
    TLShutDown();
}
