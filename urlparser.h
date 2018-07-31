#ifndef URLPARSER_H
#define URLPARSER_H

#ifdef __ORCAC__
# include <types.h>
#else
typedef _Bool Boolean;
#endif

typedef struct URLParts {
    char *scheme;
    char *username;
    char *password;
    char *host;
    char *port;
    char *path; /* Omits leading '/'.  Includes query, if any. */
    char *fragment;
    
    Boolean errorFound;
} URLParts;


URLParts ParseURL(char *url);

#endif
