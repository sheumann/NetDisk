#ifndef HTTP_H
#define HTTP_H

#include <types.h>
#include "session.h"
#include "netdiskerror.h"

Boolean BuildHTTPRequest(Session *sess, char *resourceStr);
enum NetDiskError DoHTTPRequest(Session *sess, unsigned long start, unsigned long end);

#endif