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
};

Boolean BuildHTTPRequest(Session *sess, char *resourceStr);
void UpdateRequestRange(Session *sess, unsigned long start, unsigned long end);
enum RequestResult DoHTTPRequest(Session *sess);

#endif