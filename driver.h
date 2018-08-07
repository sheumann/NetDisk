#ifndef DRIVER_H
#define DRIVER_H

#include <types.h>

#define DEVICE_GENERIC_FLOPPY_DRIVE 0x0013

#define NDIBS 16

struct DIB {
    void *linkPtr;
    void *entryPtr;
    Word characteristics;
    LongWord blockCount;
    char devName[32];
    Word slotNum;
    Word unitNum;
    Word version;
    Word deviceID;
    Word headlink;
    Word forwardLink;
    void *extendedDIBPtr;
    Word DIBDevNum;
};

struct DIBList {
    LongWord count;
    struct DIB *dibPointers[NDIBS];
};

extern struct DIB dibs[NDIBS];
extern struct DIBList dibList;

extern void *gsosDP;

void InitDIBs(void);

#endif
