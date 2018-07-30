#ifndef HTTP_H
#define HTTP_H

#include <types.h>
#include "session.h"

Boolean BuildHTTPRequest(Session *sess, char *resourceStr, char *hostStr);
void UpdateRequestRange(Session *sess, unsigned long start, unsigned long end);

#endif