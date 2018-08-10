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
    /* Length of HTTP request */
    LongWord httpRequestLen;
    
    /* HTTP response code */
    LongWord responseCode;

    /* IP address and TCP port of host */
    LongWord ipAddr;
    Word port;

    /* Domain name or IP address of host (p-string, but also null-terminated) */
    char hostName[257];
    /* Time (GetTick) of last DNS lookup */
    LongWord dnsTime;
    
    /* Number of redirects followed */
    Word redirectCount;
    
    /* Values reported by server in Content-Range header */
    LongWord rangeStart, rangeEnd, totalLength;
    /* Value reported by server in Content-Length header */
    LongWord contentLength;
    
    /* Desired start/end of range for next request */
    LongWord desiredStart, desiredEnd;
    /* Expected length of disk image */
    LongWord expectedLength;
    
    /* Buffer for initial bytes of file (which may be a disk image header) */
    unsigned char fileHeaderBuf[32];
} Session;

#endif
