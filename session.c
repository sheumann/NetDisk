#pragma noroot

#include <stdlib.h>
#include "session.h"
#include "tcpconnection.h"

/* End a session and free associated data. */
void EndNetDiskSession(Session *sess) {
    if (sess != NULL) {
        EndTCPConnection(sess);
        free(sess->httpRequest);
        free(sess);
    }
}
