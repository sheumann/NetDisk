#pragma lint -1
#pragma noroot

#include <stdlib.h>
#include <stdio.h>
#include <types.h>
#include "session.h"
#include "http.h"

Boolean BuildHTTPRequest(Session *sess, char *resourceStr) {
    int sizeNeeded = 0;
    int rangeOffset;
    int round = 0;

    do {
        sizeNeeded = snprintf(sess->httpRequest, sizeNeeded, 
            "GET /%s HTTP/1.1\r\n"
            "Host: %s\r\n"
            "User-Agent: GSRemoteDisk/0.1\r\n"
            "Accept-Encoding: identity\r\n"
            //"Accept: */*\r\n" /* default, but some clients send explicitly */
            //"Connection: Keep-Alive\r\n" /* same */
            "Range: bytes=%n1234567890-1234567890\r\n"
            "\r\n",
            resourceStr,
            sess->hostName+1,
            &rangeOffset);

        if (sizeNeeded <= 0) {
            free(sess->httpRequest);
            sess->httpRequest = NULL;
            sess->httpRequestRange = NULL;
            return FALSE;
        }

        if (round == 0) {
            sizeNeeded++; /* account for terminating NUL */
            free(sess->httpRequest);
            sess->httpRequest = malloc(sizeNeeded);
            if (sess->httpRequest == NULL) {
                sess->httpRequestRange = NULL;
                return FALSE;
            }
        }
    } while (round++ == 0);
    
    sess->httpRequestRange = sess->httpRequest + rangeOffset;
    return TRUE;
}

void UpdateRequestRange(Session *sess, unsigned long start, unsigned long end) {
    snprintf(sess->httpRequestRange, 10+1+10+5, "%lu-%lu\r\n\r\n", start, end);
}
