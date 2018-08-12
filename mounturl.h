#ifndef MOUNTURL_H
#define MOUNTURL_H

#include "netdiskerror.h"

/* Custom DControl code and request code for MountURL operation */
#define MountURL 0x8080

struct MountURLRec {
    Word byteCount;
    enum NetDiskError result; /* output value */
    char *url; /* C-string; will be modified */
};

#endif
