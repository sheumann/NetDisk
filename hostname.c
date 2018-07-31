#include <tcpip.h>
#include <misctool.h>
#include "hostname.h"

#define DNR_WAIT_TIME 15 /*seconds*/

Boolean DoLookupName(Session *sess) {
    dnrBuffer dnrInfo;

    if (TCPIPValidateIPString(sess->hostName)) {
        cvtRec cvtInfo;
        TCPIPConvertIPToHex(&cvtInfo, sess->hostName);
        sess->ipAddr = cvtInfo.cvtIPAddress;
        return TRUE;
    }

    TCPIPDNRNameToIP(sess->hostName, &dnrInfo);
    if (toolerror())
        return FALSE;

    sess->dnsTime = GetTick();
    while (dnrInfo.DNRstatus == DNR_Pending) {
        if (GetTick() - sess->dnsTime >= DNR_WAIT_TIME * 60)
            break;
        TCPIPPoll();
    }

    if (dnrInfo.DNRstatus == DNR_OK) {
        sess->ipAddr = dnrInfo.DNRIPaddress;
        return TRUE;
    } else {
        if (dnrInfo.DNRstatus == DNR_Pending) {
            TCPIPCancelDNR(&dnrInfo);
        }
        return FALSE;
    }
}
