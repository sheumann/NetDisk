#pragma noroot

#include "readtcp.h"
#include "session.h"
#include <string.h>
#include <tcpip.h>
#include <memory.h>
#include <misctool.h>
#include <orca.h>

#define buffTypePointer 0x0000      /* For TCPIPReadTCP() */
#define buffTypeHandle 0x0001
#define buffTypeNewHandle 0x0002

/* Time out if no new data is received for this long */
#define READ_TIMEOUT 15 /* seconds */

void InitReadTCP(Session *sess, LongWord readCount, void *readPtr) {
    sess->readCount = readCount;
    sess->readPtr = readPtr;
    sess->lastReadTime = GetTick();
}


ReadStatus TryReadTCP(Session *sess) {
    rrBuff rrBuff;

    TCPIPPoll();
    sess->tcperr = TCPIPReadTCP(sess->ipid, buffTypeNewHandle, NULL, 
                                sess->readCount, &rrBuff);
    sess->toolerr = toolerror();
    if (sess->tcperr || sess->toolerr) {
        return rsError;
    }
    
    if (rrBuff.rrBuffCount != 0) {
        /* Work around Marinetti bug #57 */
        rrBuff.rrBuffCount = GetHandleSize(rrBuff.rrBuffHandle);
    
        HLock(rrBuff.rrBuffHandle);
        memcpy(sess->readPtr, *rrBuff.rrBuffHandle, rrBuff.rrBuffCount);
        DisposeHandle(rrBuff.rrBuffHandle);

        sess->readCount -= rrBuff.rrBuffCount;
        sess->readPtr += rrBuff.rrBuffCount;
    }
    
    
    if (sess->readCount == 0) {
        return rsDone;
    } else {
        if (rrBuff.rrBuffCount != 0) {
            sess->lastReadTime = GetTick();
        } else if (GetTick() - sess->lastReadTime > READ_TIMEOUT * 60) {
            return rsTimedOut;
        }
        
        return rsWaiting;
    }
}
