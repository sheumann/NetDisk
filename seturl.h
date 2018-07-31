#ifndef SETURL_H
#define SETURL_H

enum SetURLResult {
    SETURL_SUCCESSFUL = 0,
    URL_TOO_LONG,
    INVALID_CHARACTER_IN_URL,
    BAD_URL_SYNTAX,
    UNSUPPORTED_URL_SCHEME,
    AUTHENTICATION_NOT_SUPPORTED,
    FRAGMENT_NOT_SUPPORTED,
    INVALID_PORT_NUMBER,
    NO_HOST_SPECIFIED,
    IPV6_NOT_SUPPORTED,
    HOSTNAME_TOO_LONG,
    NAME_LOOKUP_FAILED,
    OUT_OF_MEMORY
};

enum SetURLResult
SetURL(Session *sess, char *url, Boolean permissive, Boolean partialOK);

#endif
