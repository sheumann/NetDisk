#include <types.h>
#include <errno.h>
#include "session.h"
#include "urlparser.h"
#include "strcasecmp.h"
#include "hostname.h"
#include "http.h"
#include "tcpconnection.h"
#include "seturl.h"

#define DEFAULT_HTTP_PORT 80

/* Limit to make sure sizes stay within the range of 16-bit values */
#define MAX_URL_LENGTH 30000


enum SetURLResult
SetURL(Session *sess, char *url, Boolean permissive, Boolean partialOK) {
    if (strlen(url) > MAX_URL_LENGTH) {
        return URL_TOO_LONG;
    }
    
    for(unsigned int i = 0; url[i] != '\0'; i++) {
        if (url[i] <= ' ' && url[i] >= 0) {
            return INVALID_CHARACTER_IN_URL;
        }
    }

    URLParts urlParts = ParseURL(url);

    if (urlParts.errorFound)
        return BAD_URL_SYNTAX;

    if (urlParts.scheme == NULL) {
        if (permissive) {
            urlParts.scheme = "http";
        } else {
            return BAD_URL_SYNTAX;
        }
    } else if (strcasecmp(urlParts.scheme, "http") != 0) {
        return UNSUPPORTED_URL_SCHEME;
    }
    
    if (urlParts.username != NULL || urlParts.password != NULL) {
        return AUTHENTICATION_NOT_SUPPORTED;
    }
    
    if (urlParts.fragment != NULL) {
        return FRAGMENT_NOT_SUPPORTED;
    }
    
    unsigned long portNum;
    char *endPtr;
    if (urlParts.port == NULL || *urlParts.port == '\0') {
        portNum = DEFAULT_HTTP_PORT;
    } else {
        errno = 0;
        portNum = strtoul(urlParts.port, &endPtr, 10);
        if (errno || *endPtr != '\0' || portNum > 0xFFFF) {
            return INVALID_PORT_NUMBER;
        }
    }
    sess->port = portNum;

    if (urlParts.host == NULL) {
        if (!partialOK || sess->hostName[0] == 0) {
            return NO_HOST_SPECIFIED;
        }
    } else if (*urlParts.host == '\0') {
        return NO_HOST_SPECIFIED;
    } else if (*urlParts.host == '[') {
        return IPV6_NOT_SUPPORTED;
    } else {
        size_t len;
        if ((len = strlen(urlParts.host)) > 255) {
            return HOSTNAME_TOO_LONG;
        }
        
        strcpy(&sess->hostName[1], urlParts.host);
        sess->hostName[0] = len;
        
        if (!DoLookupName(sess)) {
            return NAME_LOOKUP_FAILED;
        }
    }

    if (!BuildHTTPRequest(sess, urlParts.path)) {
        return OUT_OF_MEMORY;
    }
    
    /* End any existing TCP connection to old URL */
    EndTCPConnection(sess);
    
    return SETURL_SUCCESSFUL;
}
