#include "urlparser.h"
#include <string.h>

/*
 * Parse the URL and break it up into its component parts.
 *
 * This handles http URLs and other schemes with the same syntax.
 *
 * It can also handle relative URLs containing an absolute-path reference
 * (i.e. starting with /) and URL-like addresses without the scheme prefix.
 * In these cases, the portions of the URL not provided will be NULL.
 *
 * This modifies the original string passed in, breaking it up into components.
 */
URLParts ParseURL(char *url) {
    char *sep, sep2;
    URLParts urlParts = {0};

    sep = strrchr(url, '#');
    if (sep) {
        *sep = '\0';
        urlParts.fragment = sep + 1;
    }

    /* Detect relative URL (absolute-path reference only) */
    if (url[0] == '/') {
        if (url[1] != '/') {
            urlParts.path = url + 1;
            return urlParts;
        } else {
            urlParts.errorFound = 1;
            return urlParts;
        }
    }
    
    sep = strchr(url, ':');
    if (sep) {
        urlParts.scheme = url;
        *sep = '\0';
        url = sep + 1;
    }
    
    if (urlParts.scheme != NULL) {
        if (strncmp(url, "//", 2) == 0) {
            url += 2;
        } else {
            urlParts.errorFound = 1;
            return urlParts;
        }
    }
    
    urlParts.path = strchr(url, '/');
    if (urlParts.path == NULL) {
        urlParts.path = "";
    } else {
        *urlParts.path = '\0';
        urlParts.path++;
    }
    
    sep = strchr(url, '@');
    if (sep) {
        *sep = '\0';
        urlParts.username = url;
        
        urlParts.password = strchr(url, ':');
        if (urlParts.password != NULL) {
            *urlParts.password = '\0';
            urlParts.password++;
        }
        
        url = sep + 1;
    }
    
    urlParts.host = url;
    
    /* Handle IPv6 address syntax */
    if (*url == '[') {
        sep = strchr(url, ']');
        if (sep) {
            url = sep + 1;
        } else {
            urlParts.errorFound = 1;
            return urlParts;
        }
    }
    
    sep = strchr(url, ':');
    if (sep) {
        *sep = '\0';
        urlParts.port = sep + 1;
    }

    return urlParts;
}


#ifdef URLPARSER_TEST
#include <stdio.h>

int main(int argc, char **argv)
{
    URLParts urlParts;

    if (argc < 2)
        return 1;
    
    urlParts = ParseURL(argv[1]);
    
    printf("scheme:   %s\n", urlParts.scheme ? urlParts.scheme : "(NULL)");
    printf("username: %s\n", urlParts.username ? urlParts.username : "(NULL)");
    printf("password: %s\n", urlParts.password ? urlParts.password : "(NULL)");
    printf("host:     %s\n", urlParts.host ? urlParts.host : "(NULL)");
    printf("port:     %s\n", urlParts.port ? urlParts.port : "(NULL)");
    printf("path:     %s\n", urlParts.path ? urlParts.path : "(NULL)");
    printf("fragment: %s\n", urlParts.fragment ? urlParts.fragment : "(NULL)");
    
    if (urlParts.errorFound) {
        printf("Error found\n");
    }
}
#endif
