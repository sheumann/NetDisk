#pragma lint -1
#pragma noroot

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <types.h>
#include <tcpip.h>
#include <misctool.h>
#include <memory.h>
#include <orca.h>
#include <errno.h>
#include "session.h"
#include "http.h"
#include "tcpconnection.h"
#include "strcasecmp.h"
#include "seturl.h"
#include "version.h"

#define buffTypePointer 0x0000      /* For TCPIPReadTCP() */
#define buffTypeHandle 0x0001
#define buffTypeNewHandle 0x0002

#define HTTP_RESPONSE_TIMEOUT 15 /* seconds to wait for response headers */

#define MAX_REDIRECTS 5

enum ResponseHeader {
    UNKNOWN_HEADER = 0,
    CONTENT_RANGE,
    CONTENT_LENGTH, 
    TRANSFER_ENCODING,
    CONTENT_ENCODING,
    LOCATION,
};

static void
UpdateRequestRange(Session *sess, unsigned long start, unsigned long end) {
    int count = 
        snprintf(sess->httpRequestRange, 10+1+10+5, "%lu-%lu\r\n\r\n", start, end);

    sess->desiredStart = start;
    sess->desiredEnd = end;

    sess->httpRequestLen = sess->httpRequestRange - sess->httpRequest + count;
}


Boolean BuildHTTPRequest(Session *sess, char *resourceStr) {
    int sizeNeeded = 0;
    int rangeOffset;
    int round = 0;

    do {
        sizeNeeded = snprintf(sess->httpRequest, sizeNeeded, 
            "GET /%s HTTP/1.1\r\n"
            "Host: %s\r\n"
            "User-Agent: GS-NetDisk/" USER_AGENT_VERSION "\r\n"
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
    
    UpdateRequestRange(sess, sess->desiredStart, sess->desiredEnd);
    
    return TRUE;
}

enum NetDiskError 
DoHTTPRequest(Session *sess, unsigned long start, unsigned long end) {
top:;
    union {
        srBuff srBuff;
        rlrBuff rlrBuff;
    } u;
    Word tcpError;
    Boolean wantRedirect = FALSE, gotRedirect = FALSE;
    enum NetDiskError result;
    
    UpdateRequestRange(sess, start, end);
    
    sess->responseCode = 0;

    /* Send out request */
    result = NETWORK_ERROR;
    unsigned int netErrors = 0;
    
    TCPIPPoll();
    if (sess->tcpLoggedIn) {
        tcpError = TCPIPStatusTCP(sess->ipid, &u.srBuff);
        if (tcpError || toolerror() || u.srBuff.srState != TCPSESTABLISHED) {
            EndTCPConnection(sess);
        }
    }

    u.rlrBuff.rlrBuffHandle = NULL;
    
netRetry:
    if (!sess->tcpLoggedIn || netErrors) {
        if (StartTCPConnection(sess) != 0)
            goto errorReturn;
    }
    tcpError = TCPIPWriteTCP(sess->ipid, sess->httpRequest,
                             sess->httpRequestLen, TRUE, FALSE);
    if (tcpError || toolerror()) {
        if (netErrors == 0) {
            netErrors++;
            goto netRetry;
        } else {
            goto errorReturn;
        }
    }
    
    /* Get response status line & headers */
    LongWord startTime = GetTick();
    do {
        TCPIPPoll();
        tcpError = TCPIPReadLineTCP(sess->ipid,
                (void*)((LongWord)"\p\r\n\r\n" | 0x80000000),
                buffTypeNewHandle, (Ref)NULL,
                0xFFFFFF, &u.rlrBuff);
        if (tcpError || toolerror()) {
            if (netErrors == 0) {
                netErrors++;
                goto netRetry;
            } else {
                goto errorReturn;
            }
        }
    } while (u.rlrBuff.rlrBuffCount == 0 
             && GetTick() - startTime < HTTP_RESPONSE_TIMEOUT * 60);

    result = NO_RESPONSE;
    if (!u.rlrBuff.rlrIsDataFlag)
        goto errorReturn;

    result = INVALID_RESPONSE;
    /* Response must be at least long enough for a status line & final CRLF */
    if (u.rlrBuff.rlrBuffCount < 8+1+3+1+2+2)
        goto errorReturn;
    
    HLock(u.rlrBuff.rlrBuffHandle);
    
    char *response = *u.rlrBuff.rlrBuffHandle;
    char *responseEnd = response + u.rlrBuff.rlrBuffCount;
    /* Make response a C-string. Specifically, it will end "CR LF NUL NUL". */
    response[u.rlrBuff.rlrBuffCount - 2] = '\0';
    response[u.rlrBuff.rlrBuffCount - 1] = '\0';
    
    /* Parse status line of HTTP response */
    char *endPtr;
    
    if (strncmp(response, "HTTP/1.", 7) != 0)
        goto errorReturn;
    response += 7;
    if (!(*response >= '1' && *response <= '9'))
        goto errorReturn;
    response += 1;
    if (*response != ' ')
        goto errorReturn;
    response += 1;
    
    errno = 0;
    sess->responseCode = strtoul(response, &endPtr, 10);
    if (errno || sess->responseCode > 999 || endPtr != response + 3)
        goto errorReturn;
    response += 3;
    
    if (*response != ' ')
        goto errorReturn;
    response++;
    
    while (*response != '\r' && response < responseEnd)
        response++;
    response++;
    
    if (*response != '\n' && response < responseEnd)
        goto errorReturn;
    response++;


    switch((unsigned int)sess->responseCode) {
    /* "Partial content" response, as expected */
    case 206:
        break;
    
    /* Redirect responses */
    case 300: case 301: case 302: case 307: case 308:
        if (sess->redirectCount < MAX_REDIRECTS) {
            sess->redirectCount++;
            wantRedirect = TRUE;
            break;
        } else {
            result = EXCESSIVE_REDIRECTS;
            goto errorReturn;
        }

    /* Full content in response to range request: not supported by NetDisk */
    case 200:
        result = NOT_DESIRED_CONTENT;
        goto errorReturn;
      
    default:
        if (sess->responseCode < 400 || sess->responseCode > 599) {
            result = UNSUPPORTED_RESPONSE;
        } else {
            result = sess->responseCode;
        }
        goto errorReturn;
    }

    /* Parse response headers */
    sess->rangeStart = 0;
    sess->rangeEnd = 0;
    sess->totalLength = 0;
    sess->contentLength = 0;
    
    result = UNSUPPORTED_HEADER_VALUE;

    while (response < responseEnd - 2) {
        enum ResponseHeader header = UNKNOWN_HEADER;
        
        if (wantRedirect) {
            if (strncasecmp(response, "Location:", 9) == 0) {
                response += 9;
                header = LOCATION;
            }
        } else {
            if (strncasecmp(response, "Content-Range:", 14) == 0) {
                response += 14;
                header = CONTENT_RANGE;
            } else if (strncasecmp(response, "Content-Length:", 15) == 0) {
                response += 15;
                header = CONTENT_LENGTH;
            } else if (strncasecmp(response, "Transfer-Encoding:", 18) == 0) {
                response += 18;
                header = TRANSFER_ENCODING;
            } else if (strncasecmp(response, "Content-Encoding:", 17) == 0) {
                response += 17;
                header = CONTENT_ENCODING;
            }
        }
        
        while ((*response == ' ' || *response == '\t') && response < responseEnd)
            response++;
        
        switch (header) {
        case CONTENT_RANGE:
            if (strncasecmp(response, "bytes ", 6) != 0)
                goto errorReturn;
            response += 6;
        
            errno = 0;
            sess->rangeStart = strtoul(response, &response, 10);
            if (errno)
                goto errorReturn;
            if (*response != '-')
                goto errorReturn;
            response++;
            
            sess->rangeEnd = strtoul(response, &response, 10);
            if (errno || sess->rangeEnd == 0)
                goto errorReturn;
            if (*response != '/')
                goto errorReturn;
            response++;
            
            /* require numeric total length, not indeterminate '*' */
            sess->totalLength = strtoul(response, &response, 10);
            if (errno || sess->totalLength == 0)
                goto errorReturn;
            break;
        
        case CONTENT_LENGTH:
            errno = 0;
            sess->contentLength = strtoul(response, &endPtr, 10);
            if (errno)
                goto errorReturn;
            if (sess->contentLength == 0 && !wantRedirect)
                goto errorReturn;
            response = endPtr;
            break;
        
        case CONTENT_ENCODING:
        case TRANSFER_ENCODING:
            if (strcasecmp(response, "identity") == 0) {
                response += 8;
            } else {
                goto errorReturn;
            }
            break;
        
        case LOCATION:
            endPtr = response;
            char c;
            while ((c = *endPtr)!=0 && c!='\r' && c!='\n' && c!=' ' && c!='\t')
                endPtr++;
            if (c == '\0' || c == '\n')
                goto errorReturn;
            if (wantRedirect) {
                *endPtr = '\0';
                if (SetURL(sess, response, FALSE, TRUE) != OPERATION_SUCCESSFUL) {
                    result = REDIRECT_ERROR;
                    goto errorReturn;
                }
                *endPtr = c;
                response = endPtr;
                gotRedirect = TRUE;
            }
        
        /* Unknown headers: ignored */
        case UNKNOWN_HEADER:
        default:
            while (*response != '\r' && response < responseEnd)
                response++;
            break;
        }
        
        while ((*response == ' ' || *response == '\t') && response < responseEnd)
            response++;
        
        if (*response != '\r')
            goto errorReturn;
        response++;
        if (*response != '\n')
            goto errorReturn;
        response++;
    }
    
    /* Wanted redirect: Retry with new location if we got it. */
    if (wantRedirect) {
        if (gotRedirect) {
            DisposeHandle(u.rlrBuff.rlrBuffHandle);
            goto top;
        } else {
            result = UNSUPPORTED_RESPONSE;
            goto errorReturn;
        }
    }
    
    /* See if we got what we wanted */
    if (sess->expectedLength != 0 && sess->totalLength != sess->expectedLength) {
        result = DIFFERENT_LENGTH;
        goto errorReturn;
    }
    
    result = NOT_DESIRED_CONTENT;
    if (sess->rangeStart != sess->desiredStart)
        goto errorReturn;
    if (sess->rangeEnd != sess->desiredEnd)
        goto errorReturn;
    if (sess->contentLength != 0 
        && sess->contentLength != (sess->desiredEnd - sess->desiredStart + 1))
        goto errorReturn;

    result = OPERATION_SUCCESSFUL;
    DisposeHandle(u.rlrBuff.rlrBuffHandle);
    return result;
    
errorReturn:
    if (u.rlrBuff.rlrBuffHandle != NULL) {
        DisposeHandle(u.rlrBuff.rlrBuffHandle);
    }

    /* Error condition on this TCP connection means it can't be reused. */
    EndTCPConnection(sess);
    return result;
}
