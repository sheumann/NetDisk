#pragma noroot

#define aspNetworkErr 1
#define aspNoRespErr 2

#include <tcpip.h>
#include <stdlib.h>
#include <orca.h>
#include <misctool.h>
#include "session.h"
#include "tcpconnection.h"

/* Make a TCP connection for a session.
 *
 * On success, returns 0 and sets sess->ipid.
 * On failure, returns an error code.
 */
Word StartTCPConnection(Session *sess) {
    Word tcperr;
    srBuff mySRBuff;
    LongWord initialTime;
    
    if (TCPIPGetConnectStatus() == FALSE) {
        TCPIPConnect(NULL);
        if (toolerror())
            return aspNetworkErr;
    }
    
    sess->ipid = 
        TCPIPLogin(userid(), sess->ipAddr, sess->port, 0, 0x40);
    if (toolerror())
        return aspNetworkErr;
    
    tcperr = TCPIPOpenTCP(sess->ipid);
    if (toolerror()) {
        TCPIPLogout(sess->ipid);
        return aspNetworkErr;
    } else if (tcperr != tcperrOK) {
        TCPIPLogout(sess->ipid);
        return aspNoRespErr;
    }
    
    initialTime = GetTick();
    do {
        TCPIPPoll();
        TCPIPStatusTCP(sess->ipid, &mySRBuff);
    } while (mySRBuff.srState == TCPSSYNSENT && GetTick()-initialTime < 15*60);
    if (mySRBuff.srState != TCPSESTABLISHED) {
        TCPIPAbortTCP(sess->ipid);
        TCPIPLogout(sess->ipid);
        return aspNoRespErr;
    }
    
    sess->tcpLoggedIn = TRUE;
    return 0;
}

void EndTCPConnection(Session *sess) {
    if (sess->tcpLoggedIn) {
        TCPIPPoll();
        TCPIPAbortTCP(sess->ipid);
        TCPIPLogout(sess->ipid);
        sess->tcpLoggedIn = FALSE;
    }
}
