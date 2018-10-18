#ifndef MOUNTURL_H
#define MOUNTURL_H

#include "netdiskerror.h"

/* Custom DControl code and request code for MountURL operation */
#define MountURL 0x8080

#define NETDISK_REQUEST_NAME "\pSTH~NetDisk~"

/* Bits in flags */
#define flgUseCache 0x0001

enum DiskImageFormat {
    formatAutoDetect = 0,
    format2mg,
    formatRaw,
    formatDOSOrder,
    formatDiskCopy42
};

struct MountURLRec {
    Word byteCount;
    enum NetDiskError result; /* output value */
    char *url; /* C-string; will be modified */
    Word flags;
    enum DiskImageFormat format; /* input/output value */
    Word devNum; /* output value: device number */
};

#endif
