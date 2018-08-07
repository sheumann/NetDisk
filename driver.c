#include <stdlib.h>
#include <stdio.h>
#include "driver.h"
#include "version.h"

struct DIB dibs[NDIBS] = {0};
struct DIBList dibList = {NDIBS};

void *gsosDP = (void*)0x00BD00;  /* GS/OS direct page ptr */

static asm dummy(void) {
        rtl
}

void InitDIBs(void) {
    for (unsigned i = 0; i < NDIBS; i++) {
        dibs[i].linkPtr = (i < NDIBS-1) ? &dibs[i+1] : NULL;
        dibs[i].entryPtr = dummy; // TODO driver entry point
            /* speed-independent, block device, read allowed, removable */
        dibs[i].characteristics = 0x03A4;
        dibs[i].blockCount = 0;
        
        int nameLen = sprintf(dibs[i].devName + 1, "NETDISK%u", i+1);
        dibs[i].devName[0] = nameLen;
        for (unsigned j = nameLen + 1; j < sizeof(dibs[i].devName); j++) {
            dibs[i].devName[j] = ' ';
        }
        
        dibs[i].slotNum = 0x8003;
        dibs[i].unitNum = i+1;
        dibs[i].version = DRIVER_VERSION;
        dibs[i].deviceID = DEVICE_GENERIC_FLOPPY_DRIVE;
        
        dibList.dibPointers[i] = &dibs[i];
    }

}
