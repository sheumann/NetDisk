#ifndef NETDISKERROR_H
#define NETDISKERROR_H

enum NetDiskError {
    OPERATION_SUCCESSFUL = 0,
    
    NETDISK_NOT_PRESENT = 100,
    DISK_ALREADY_MOUNTED,
    NO_DIBS_AVAILABLE,
    OUT_OF_MEMORY,
    MARINETTI_NOT_PRESENT,

    /* SetURL errors */
    URL_TOO_LONG = 200,
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
    
    /* StartTCPConnection and DoHTTPRequest errors */
    NETWORK_ERROR = 300,
    NO_RESPONSE,
    INVALID_RESPONSE,
    EXCESSIVE_REDIRECTS,
    UNSUPPORTED_RESPONSE,
    UNSUPPORTED_HEADER_VALUE,
    REDIRECT_ERROR,
    NOT_DESIRED_CONTENT,
    DIFFERENT_LENGTH,
    
    /* Error values of 4xx and 5xx mean we got the corresponding HTTP error */
    HTTP_ERROR = 400,
    
    /* File format errors */
    UNSUPPORTED_2IMG_FILE = 600,
    NOT_MULTIPLE_OF_BLOCK_SIZE,
    NOT_SPECIFIED_IMAGE_TYPE,
    UNSUPPORTED_IMAGE_TYPE,
    
    /* Errors related to processing JSON result in the disk browser */
    JSON_PARSING_ERROR = 900,
    NOT_EXPECTED_CONTENTS,
};


/*
 * Return a string describing the error.  It may contain the substitution
 * string "*0", which should be replaced by the error number.
 */
char *ErrorString(enum NetDiskError err);

#endif
