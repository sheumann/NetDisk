#pragma noroot

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
enum NetDiskError StartTCPConnection(Session *sess) {
    Word tcperr;
    srBuff mySRBuff;
    LongWord initialTime;
    
    /* End any existing TCP connection */
    EndTCPConnection(sess);
    
    sess->ipid = 
        TCPIPLogin(userid(), sess->ipAddr, sess->port, 0, 0x40);
    if (toolerror())
        return NETWORK_ERROR;
    
    tcperr = TCPIPOpenTCP(sess->ipid);
    if (toolerror()) {
        TCPIPLogout(sess->ipid);
        return NETWORK_ERROR;
    } else if (tcperr != tcperrOK) {
        TCPIPLogout(sess->ipid);
        return NO_RESPONSE;
    }
    
    initialTime = GetTick();
    do {
        TCPIPPoll();
        TCPIPStatusTCP(sess->ipid, &mySRBuff);
    } while (mySRBuff.srState == TCPSSYNSENT && GetTick()-initialTime < 15*60);
    if (mySRBuff.srState != TCPSESTABLISHED) {
        TCPIPAbortTCP(sess->ipid);
        TCPIPLogout(sess->ipid);
        return NO_RESPONSE;
    }
    
    sess->tcpLoggedIn = TRUE;
    return 0;
}

void EndTCPConnection(Session *sess) {
    if (sess->tcpLoggedIn) {
        TCPIPPoll();
        TCPIPAbortTCP(sess->ipid);
        TCPIPLogout(sess->ipid);
        sess->ipid = 0;
        sess->tcpLoggedIn = FALSE;
    }
}
