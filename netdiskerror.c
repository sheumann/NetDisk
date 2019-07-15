#pragma noroot

#include "netdiskerror.h"

char *ErrorString(enum NetDiskError err) {
    switch (err) {
    case NETDISK_NOT_PRESENT:
        return "The NetDisk initialization file was not found. You must have "
               "NetDiskInit installed in the *:System:System.Setup folder.";
    case DISK_ALREADY_MOUNTED:
        return "Tried to mount a disk on a device that is already in use.";
    case NO_DIBS_AVAILABLE:
        return "No more disks can be mounted via NetDisk.";
    case OUT_OF_MEMORY:
        return "Out of memory.";
    case MARINETTI_NOT_PRESENT:
        return "Marinetti is not available. You must install Marinetti to use "
               "NetDisk.";

    /* SetURL errors */
    case URL_TOO_LONG:
        return "The URL is too long.";
    case INVALID_CHARACTER_IN_URL:
        return "There was an invalid character in the URL.";
    case BAD_URL_SYNTAX:
        return "The URL is invalid.";
    case UNSUPPORTED_URL_SCHEME:
        return "Protocols other than unencrypted HTTP are not supported.";
    case AUTHENTICATION_NOT_SUPPORTED:
        return "Authentication is not supported.";
    case FRAGMENT_NOT_SUPPORTED:
        return "The URL may not specify a document fragment.";
    case INVALID_PORT_NUMBER:
        return "The port number is invalid.";
    case NO_HOST_SPECIFIED:
        return "No host name or IP address was specified.";
    case IPV6_NOT_SUPPORTED:
        return "IPv6 is not supported.";
    case HOSTNAME_TOO_LONG:
        return "The specified host name was too long.";
    case NAME_LOOKUP_FAILED:
        return "The specified server could not be found.";
    
    /* StartTCPConnection and DoHTTPRequest errors */
    case NETWORK_ERROR:
        return "A network error was encountered.";
    case NO_RESPONSE:
        return "The server did not respond to a request.";
    case INVALID_RESPONSE:
        return "The response from the server was invalid.";
    case EXCESSIVE_REDIRECTS:
        return "There were too many HTTP redirects.";
    case UNSUPPORTED_RESPONSE:
        return "An unsupported response was received from the server.";
    case UNSUPPORTED_HEADER_VALUE:
        return "An unsupported header value was received from the server.";
    case REDIRECT_ERROR:
        return "An error was encountered when trying to redirect to the "
               "location specified by the server.";
    case NOT_DESIRED_CONTENT:
        return "The server did not send the content that was expected. It may "
               "not support HTTP range requests, which are required by NetDisk.";
    case DIFFERENT_LENGTH:
        return "The length of the file on the server was different from what "
               "was expected.";

    /* File format errors */
    case UNSUPPORTED_2IMG_FILE:
        return "This 2mg file is not supported by NetDisk.";
    case NOT_MULTIPLE_OF_BLOCK_SIZE:
        return "The file is not a multiple of 512 bytes. It may not be a disk "
               "image file, or is not in a supported format.";
    case NOT_SPECIFIED_IMAGE_TYPE:
        return "The file is not a valid disk image of the specified type.";

    /* HTTP errors */
    case 400:
        return "HTTP error 400 (Bad Request) received from server.";
    case 401:
        return "HTTP error 401 (Unauthorized) received from server.";
    case 402:
        return "HTTP error 402 (Payment Required) received from server.";
    case 403:
        return "HTTP error 403 (Forbidden) received from server.";
    case 404:
        return "HTTP error 404 (Not Found) received from server.";
    case 405:
        return "HTTP error 405 (Method Not Allowed) received from server.";
    case 406:
        return "HTTP error 406 (Not Acceptable) received from server.";
    case 407:
        return "HTTP error 407 (Proxy Authentication Required) received from server.";
    case 408:
        return "HTTP error 408 (Request Timeout) received from server.";
    case 409:
        return "HTTP error 409 (Conflict) received from server.";
    case 410:
        return "HTTP error 410 (Gone) received from server.";
    case 411:
        return "HTTP error 411 (Length Required) received from server.";
    case 412:
        return "HTTP error 412 (Precondition Failed) received from server.";
    case 413:
        return "HTTP error 413 (Payload Too Large) received from server.";
    case 414:
        return "HTTP error 414 (URI Too Long) received from server.";
    case 415:
        return "HTTP error 415 (Unsupported Media Type) received from server.";
    case 416:
        return "HTTP error 416 (Range Not Satisfiable) received from server.";
    case 417:
        return "HTTP error 417 (Expectation Failed) received from server.";
    case 426:
        return "HTTP error 426 (Upgrade Required) received from server.";
    case 500:
        return "HTTP error 500 (Internal Server Error) received from server.";
    case 501:
        return "HTTP error 501 (Not Implemented) received from server.";
    case 502:
        return "HTTP error 502 (Bad Gateway) received from server.";
    case 503:
        return "HTTP error 503 (Service Unavailable) received from server.";
    case 504:
        return "HTTP error 504 (Gateway Timeout) received from server.";
    case 505:
        return "HTTP error 505 (HTTP Version Not Supported) received from server.";
    }
    
    if (err >= 400 && err < 600) {
        return "HTTP error *0 received from server.";
    }
    
    return "NetDisk error *0.";
}
