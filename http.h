#ifndef HTTP_H
#define HTTP_H

#include <types.h>
#include "session.h"

enum RequestResult {
    REQUEST_SUCCESSFUL = 0,
    NETWORK_ERROR,
    NO_RESPONSE,
    INVALID_RESPONSE,
    EXCESSIVE_REDIRECTS,
    UNSUPPORTED_RESPONSE,
    UNSUPPORTED_HEADER_VALUE,
    REDIRECT_ERROR,
    NOT_DESIRED_CONTENT,
    DIFFERENT_LENGTH, /* Might be considered successful later */
};

Boolean BuildHTTPRequest(Session *sess, char *resourceStr);
enum RequestResult DoHTTPRequest(Session *sess, unsigned long start, unsigned long end);

#endif