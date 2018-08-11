#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

#include "session.h"
#include "netdiskerror.h"

enum NetDiskError StartTCPConnection(Session *sess);
void EndTCPConnection(Session *sess);

#endif
