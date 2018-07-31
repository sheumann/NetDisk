#ifndef SESSION_H
#define SESSION_H

#include <types.h>

typedef struct Session {
    /* Marinetti TCP connection status */
    Word ipid;
    Boolean tcpLoggedIn;

    /* ReadTCP status */
    LongWord readCount;
    Byte *readPtr;

    /* Marinetti error codes, both the tcperr* value and any tool error */
    Word tcperr;
    Word toolerr;

    /* HTTP request to send (if non-NULL, points to malloc'd buffer) */
    char *httpRequest;
    /* Pointer to the range part within httpRequest */
    char *httpRequestRange;

    /* IP address and TCP port of host */
    LongWord ipAddr;
    Word port;

    /* Domain name or IP address of host (p-string, but also null-terminated) */
    char hostName[257];
    /* Time (GetTick) of last DNS lookup */
    LongWord dnsTime;
} Session;

#endif
