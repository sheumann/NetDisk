#ifndef SETURL_H
#define SETURL_H

#include "netdiskerror.h"

enum NetDiskError
SetURL(Session *sess, char *url, Boolean permissive, Boolean partialOK);

#endif
